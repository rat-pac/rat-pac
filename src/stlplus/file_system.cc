/*------------------------------------------------------------------------------

  Author:    Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License, see ../docs/license.html

  This is a portable interface to the file system.

  The idea is that you write all file system access code using these functions,
  which are ported to all platforms that we are interested in. Therefore your
  code is inherently portable.

  Native Windows version: switched on by macro _WIN32 which is defined by VC++/Borland/Mingw compilers
  Unix/Gnu version:   default variant, no compiler directives are required but _WIN32 must be absent
  Cygwin/Gnu version: as Unix version

  ------------------------------------------------------------------------------*/
#include "os_fixes.hpp"
#include "debug.hpp"
#include "file_system.hpp"
#include "string_utilities.hpp"
#include "fileio.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <dos.h>
#include <direct.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <sys/types.h>
#else
#include <dirent.h>
#include <fcntl.h>
//#include <io.h>
#include <sys/param.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// definitions of separators

#ifdef _WIN32
static const char* separator_set = "\\/";
static const char preferred_separator = '\\';
#else
static const char* separator_set = "/";
static const char preferred_separator = '/';
#endif

static bool is_separator (char ch)
{
  for (int i = 0; separator_set[i]; i++)
  {
    if (separator_set[i] == ch)
      return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// implement string comparison of paths - Unix is case-sensitive, Windoze is case-insensitive

static bool path_compare(const std::string& l, const std::string& r)
{
#ifdef _WIN32
  return lowercase(l) == lowercase(r);
#else
  return l == r;
#endif
}

////////////////////////////////////////////////////////////////////////////////
// Internal data structure used to hold the different parts of a filespec

class file_specification
{
private:
  bool m_relative;                 // true = relative, false = absolute
  std::string m_drive;             // drive - drive letter (e.g. "c:") or the path for an UNC (e.g. "\\somewhere")
                                   //         empty if not known or on Unix
  std::vector<std::string> m_path; // the subdirectory path to follow from the drive
  std::string m_filename;          // the filename
public:
  file_specification(void) : m_relative(false) {}
  ~file_specification(void) {}

  bool initialise_folder(const std::string& spec);
  bool initialise_file(const std::string& spec);
  bool simplify(void);
  bool make_absolute(const std::string& root = folder_current_full());
  bool make_absolute(const file_specification& root);
  bool make_relative(const std::string& root);
  bool make_relative(const file_specification& root);
  bool relative(void) const {return m_relative;}
  bool absolute(void) const {return !relative();}
  void set_relative(void) {m_relative = true;}
  void set_absolute(void) {m_relative = false;}

  const std::string& drive(void) const {return m_drive;}
  std::string& drive(void) {return m_drive;}
  void set_drive(const std::string& _drive) {m_drive = _drive;}

  const std::vector<std::string>& path(void) const {return m_path;}
  std::vector<std::string>& path(void) {return m_path;}
  void set_path(const std::vector<std::string>& _path) {m_path = _path;}

  void add_subpath(const std::string& subpath) {m_path.push_back(subpath);}
  unsigned subpath_size(void) const {return m_path.size();}
  const std::string& subpath_element(unsigned i) const {return m_path[i];}
  void subpath_erase(unsigned i) {m_path.erase(m_path.begin()+i);}

  const std::string& file(void) const {return m_filename;}
  std::string& file(void) {return m_filename;}
  void set_file(const std::string& _file) {m_filename = _file;}

  std::string image(void) const;
  otext& print(otext& str) const {return str << image();}
  friend otext& operator << (otext& str, const file_specification& spec) {return spec.print(str);}
};

bool file_specification::initialise_folder(const std::string& folder_spec)
{
  std::string spec = folder_spec;
  m_relative = true;
  m_drive.erase();
  m_path.clear();
  unsigned i = 0;
#if defined(_WIN32) || defined(CYGWIN)
  // first split off the drive letter or UNC prefix on Windows - the Cygwin environment supports these too
  if (spec.size() >= 2 && isalpha(spec[0]) && spec[1] == ':')
  {
    // found a drive letter
    i = 2;
    m_drive = spec.substr(0, 2);
    m_relative = false;
    // if there is a drive but no path or a relative path, get the current path for this drive and prepend it to the path
    if (i == spec.size() || !is_separator(spec[i]))
    {
#ifdef _WIN32
      // getdcwd requires the drive number (1..26) not the letter (A..Z)
      char path [MAX_PATH+1];
      int drivenum = toupper(m_drive[0]) - 'A' + 1;
      if (_getdcwd(drivenum, path, MAX_PATH+1))
      {
        // the path includes the drive so we have the drive info twice
        // need to prepend this absolute path to the spec such that any remaining relative path is still retained
        if (!is_separator(path[strlen(path)-1])) spec.insert(2, 1, preferred_separator);
        spec.insert(2, path+2);
      }
      else
#endif
      {
        // non-existent drive - fill in just the root directory
        spec.insert(2, 1, preferred_separator);
      }
    }
  }
  else if (spec.size() >= 2 && is_separator(spec[0]) && is_separator(spec[1]))
  {
    // found an UNC prefix
    i = 2;
    // find the end of the prefix by scanning for the next seperator or the end of the spec
    while (i < spec.size() && !is_separator(spec[i])) i++;
    m_drive = spec.substr(0, i);
    m_relative = false;
  }
#endif
  // check whether the path is absolute or relative and discard the leading / if absolute
  if (i < spec.size() && is_separator(spec[i]))
  {
    m_relative = false;
    i++;
#ifdef _WIN32
    // if there's no drive, fill it in on Windows since absolute paths must have a drive
    if (m_drive.empty())
    {
      m_drive += (char)(_getdrive() - 1 + 'A');
      m_drive += ':';
    }
#endif
  }
  // now extract the path elements - note that a trailing / is not significant since /a/b/c/ === /a/b/c
  // also note that the leading / has been discarded - all paths are relative
  // if absolute() is set, then paths are relative to the drive, else they are relative to the current path
  unsigned start = i;
  while(i <= spec.size())
  {
    if (i == spec.size())
    {
      // path element terminated by the end of the string
      // discard this element if it is zero length because that represents the trailing /
      if (i != start)
        m_path.push_back(spec.substr(start, i-start));
    }
    else if (is_separator(spec[i]))
    {
      // path element terminated by a separator
      m_path.push_back(spec.substr(start, i-start));
      start = i+1;
    }
    i++;
  }
  // TODO - some error handling?
  return true;
}

bool file_specification::initialise_file(const std::string& spec)
{
  m_filename.erase();
  // remove last element as the file and then treat the rest as a folder
  unsigned i = spec.size();
  while (--i)
  {
    if (is_separator(spec[i]))
      break;
#ifdef _WIN32
    // on windoze you can say a:fred.txt so the colon separates the path from the filename
    else if (i == 1 && spec[i] == ':')
      break;
#endif
  }
  m_filename = spec.substr(i+1,spec.size()-i-1);
  // TODO - some error handling?
  return initialise_folder(spec.substr(0,i+1));
}

bool file_specification::simplify(void)
{
  // simplify the path by removing unnecessary . and .. entries - Note that zero-length entries are treated like .
  for (unsigned i = 0; i < m_path.size(); )
  {
    if (m_path[i].empty() || m_path[i].compare(".") == 0)
    {
      // found . or null
      // these both mean do nothing - so simply delete this element
      m_path.erase(m_path.begin()+i);
    }
    else if (m_path[i].compare("..") == 0)
    {
      // found ..
      if (i == 0 && !m_relative)
      {
        // up from the root does nothing so can be deleted
        m_path.erase(m_path.begin()+i);
        i++;
      }
      else if (i == 0 || m_path[i-1].compare("..") == 0)
      {
        // the first element of a relative path or the previous element is .. then keep it
        i++;
      }
      else
      {
        // otherwise delete this element and the previous one
        m_path.erase(m_path.begin()+i);
        m_path.erase(m_path.begin()+i-1);
        i--;
      }
    }
    // keep all other elements
    else
      i++;
  }
  // TODO - error checking?
  return true;
}

bool file_specification::make_absolute(const std::string& root)
{
  if (absolute()) return true;
  file_specification rootspec;
  rootspec.initialise_folder(root);
  return make_absolute(rootspec);
}

bool file_specification::make_absolute(const file_specification& rootspec)
{
  if (absolute()) return true;
  DEBUG_ASSERT(rootspec.absolute());
  // now append this's relative path and filename to the root's absolute path
  file_specification result = rootspec;
  for (unsigned i = 0; i < subpath_size(); i++)
    result.add_subpath(subpath_element(i));
  result.set_file(file());
  // now the rootspec is the absolute path, so transfer it to this
  *this = result;
  // and simplify to get rid of any unwanted .. or . elements
  simplify();
  return true;
}

bool file_specification::make_relative(const std::string& root)
{
  if (relative()) return true;
  file_specification rootspec;
  rootspec.initialise_folder(root);
  return make_relative(rootspec);
}

bool file_specification::make_relative(const file_specification& rootspec)
{
  if (relative()) return true;
  DEBUG_ASSERT(rootspec.absolute());
  // now compare elements of the root with elements of this to find the common path
  // if the drives are different, no conversion can take place, else clear the drive
  if (!path_compare(drive(), rootspec.drive())) return true;
  set_drive("");
  // first remove leading elements that are identical to the corresponding element in root
  unsigned i = 0;
  while(subpath_size() > 0 && i < rootspec.subpath_size() && path_compare(subpath_element(0), rootspec.subpath_element(i)))
  {
    subpath_erase(0);
    i++;
  }
  // now add a .. prefix for every element in root that is different from this
  while (i < rootspec.subpath_size())
  {
    m_path.insert(m_path.begin(), "..");
    i++;
  }
  set_relative();
  return true;
}

std::string file_specification::image(void) const
{
  std::string result = m_drive;
  if (absolute())
    result += preferred_separator;
  if (!m_path.empty())
    result += vector_to_string(m_path, std::string(1,preferred_separator));
  else if (relative())
    result += '.';
  if (!m_filename.empty())
  {
    result += preferred_separator;
    result += m_filename;
  }
  return result;
}

////////////////////////////////////////////////////////////////////////////////
// classifying functions

#ifdef _WIN32
// file type tests are not defined for some reason on Windows despite them providing the stat() function!
#define R_OK 4
#define W_OK 2
#define X_OK 1
#define F_OK 0
#endif

bool is_present (const std::string& thing)
{
  file_specification spec;
  spec.initialise_folder(thing);
  struct stat buf;
  return stat(spec.image().c_str(), &buf) == 0;
}

bool is_folder (const std::string& thing)
{
  file_specification spec;
  spec.initialise_folder(thing);
  struct stat buf;
  if (!(stat(spec.image().c_str(), &buf) == 0)) {return false;}
  return (buf.st_mode & S_IFDIR) != 0;
}

bool is_file (const std::string& thing)
{
  file_specification spec;
  spec.initialise_folder(thing);
  struct stat buf;
  if (!(stat(spec.image().c_str(), &buf) == 0)) {return false;}
  return (buf.st_mode & S_IFREG) != 0;
}

////////////////////////////////////////////////////////////////////////////////
// file functions

bool file_exists (const std::string& filespec)
{
  return is_file(filespec);
}

bool file_readable (const std::string& filespec)
{
  // a file is readable if it exists and can be read
  if (!file_exists(filespec)) return false;
  return access(filespec.c_str(),R_OK)==0;
}

bool file_writable (const std::string& filespec)
{
  // a file is writable if it exists as a file and is writable or if it doesn't exist but could be created and would be writable
  if (is_present(filespec))
  {
    if (!is_file(filespec)) return false;
    return access(filespec.c_str(),W_OK)==0;
  }
  std::string dir = folder_part(filespec);
  if (dir.empty()) dir = ".";
  return folder_writable(dir);
}

size_t file_size (const std::string& filespec)
{
  struct stat buf;
  if (!(stat(filespec.c_str(), &buf) == 0)) return 0;
  return buf.st_size;
}

bool file_delete (const std::string& filespec)
{
  if (!is_file(filespec)) return false;
  return remove(filespec.c_str())==0;
}

bool file_rename (const std::string& old_filespec, const std::string& new_filespec)
{
  if (!is_file(old_filespec)) return false;
  return rename(old_filespec.c_str(), new_filespec.c_str())==0;
}

bool file_copy (const std::string& old_filespec, const std::string& new_filespec)
{
  if (!is_file(old_filespec)) return false;
  // do an exact copy - to do this, use binary mode
  bool result = true;
  FILE* old_file = fopen(old_filespec.c_str(),"rb");
  FILE* new_file = fopen(new_filespec.c_str(),"wb");
  if (!old_file)
    result = false;
  else if (!new_file)
    result = false;
  else
  {
    for (int byte = getc(old_file); byte != EOF; byte = getc(old_file))
      putc(byte,new_file);
  }
  if (old_file) fclose(old_file);
  if (new_file) fclose(new_file);
  return result;
}

bool file_move (const std::string& old_filespec, const std::string& new_filespec)
{
  // try to move the file by renaming - if that fails then do a copy and delete the original
  if (file_rename(old_filespec, new_filespec))
    return true;
  if (!file_copy(old_filespec, new_filespec))
    return false;
  // I'm not sure what to do if the delete fails - is that an error?
  // I've made it an error and then delete the copy so that the original state is recovered
  if (file_delete(old_filespec))
    return true;
  file_delete(new_filespec);
  return false;
}

const int read_mode = 0444;
const int write_mode = 0222;
const int execute_mode = 0111;
const int none_mode = 0000;
const int read_write_mode = read_mode | write_mode;
const int all_mode = read_mode | write_mode | execute_mode;
const int owner_mask = 0700;
const int group_mask = 0070;
const int other_mask = 0007;
const int non_owner_mask = group_mask | other_mask;
const int all_mask = owner_mask | group_mask | other_mask;
const int read_mode_all = read_mode & all_mask;
const int read_write_mode_owner_read_mode_all = (read_write_mode & owner_mask) | (read_mode & non_owner_mask);
const int read_mode_owner_only = read_mode & owner_mask;
const int read_write_mode_owner_only = read_write_mode & owner_mask;

bool file_set_mode (const std::string& filespec, int mode)
{
  if (!is_file(filespec)) return false;
  return chmod(filespec.c_str(), mode)==0;
}

time_t file_created (const std::string& filespec)
{
  struct stat buf;
  if (!(stat(filespec.c_str(), &buf) == 0)) return 0;
  return buf.st_ctime;
}

time_t file_modified (const std::string& filespec)
{
  struct stat buf;
  if (!(stat(filespec.c_str(), &buf) == 0)) return 0;
  return buf.st_mtime;
}

time_t file_accessed (const std::string& filespec)
{
  struct stat buf;
  if (!(stat(filespec.c_str(), &buf) == 0)) return 0;
  return buf.st_atime;
}

std::string create_filespec (const std::string& directory, const std::string& filename)
{
  std::string result = directory;
  // if directory is empty then no directory part will be added
  // add trailing slash if the directory was specified and does not have a trailing slash
  if (!result.empty() && !is_separator(result[result.size()-1]))
    result += preferred_separator;
  // if filename is null or empty, nothing will be added so the path is then a directory path
  result += filename;
  return result;
}

std::string create_filespec (const std::string& directory, const std::string& basename, const std::string& extension)
{
  return create_filespec(directory, create_filename(basename, extension));
}

std::string create_filename(const std::string& basename, const std::string& extension)
{
  std::string name = basename;
  // extension is optional - so the dot is also optional
  if (!extension.empty())
  {
    if (extension[0] != '.') name += '.';
    name += extension;
  }
  return name;
}

////////////////////////////////////////////////////////////////////////////////
// folder functions

bool folder_create (const std::string& directory)
{
#ifdef _WIN32
  return mkdir(directory.c_str()) == 0;
#else
  return mkdir(directory.c_str(), 0777) == 0;
#endif
}

bool folder_exists (const std::string& directory)
{
  return is_folder(directory);
}

bool folder_readable (const std::string& directory)
{
  // a folder is readable if it exists and has read access
  std::string dir = directory;
  if (dir.empty()) dir = ".";
  if (!folder_exists(dir)) return false;
  return access(dir.c_str(),R_OK)==0;
}

bool folder_writable (const std::string& directory)
{
  // a folder is writable if it exists and has write access
  std::string dir = directory;
  if (dir.empty()) dir = ".";
  if (!folder_exists(dir)) return false;
  return access(dir.c_str(),W_OK)==0;
}

bool folder_delete (const std::string& directory, bool recurse)
{
  std::string dir = directory;
  if (dir.empty()) dir = ".";
  if (!folder_exists(dir)) return false;
  bool result = true;
  // depth-first traversal ensures that directory contents are deleted before trying to delete the directory itself
  if (recurse)
  {
    std::vector<std::string> subdirectories = folder_subdirectories(dir);
    for (std::vector<std::string>::size_type d = 0; d < subdirectories.size(); ++d)
      if (!folder_delete(folder_down(dir,subdirectories[d]),true)) 
	result = false;
    std::vector<std::string> files = folder_files(dir);
    for (std::vector<std::string>::size_type f = 0; f < files.size(); ++f)
      if (!file_delete(create_filespec(dir, files[f]))) 
	result = false;
  }
  if (rmdir(dir.c_str())!=0) result = false;
  return result;
}

bool folder_rename (const std::string& old_directory, const std::string& new_directory)
{
  if (!folder_exists(old_directory)) return false;
  return rename(old_directory.c_str(), new_directory.c_str())==0;
}

bool folder_empty(const std::string& directory)
{
  std::string dir = directory.empty() ? std::string(".") : directory;
  bool result = true;
#ifdef _WIN32
  std::string wildcard = create_filespec(dir, "*.*");
  long handle = -1;
  _finddata_t fileinfo;
  for (bool OK = (handle = _findfirst((char*)wildcard.c_str(), &fileinfo)) != -1; OK; OK = (_findnext(handle, &fileinfo)==0))
  {
    std::string strentry = fileinfo.name;
    if (strentry.compare(".")!=0 && strentry.compare("..")!=0)
    {
      result = false;
      break;
    }
  }
  _findclose(handle);
#else
  DIR* d = opendir(dir.c_str());
  if (d)
  {
    for (dirent* entry = readdir(d); entry; entry = readdir(d))
    {
      std::string strentry = entry->d_name;
      if (strentry.compare(".")!=0 && strentry.compare("..")!=0)
      {
        result = false;
        break;
      }
    }
    closedir(d);
  }
#endif
  return result;
}

bool folder_set_current(const std::string& folder)
{
  if (!folder_exists(folder))
    return false;
#ifdef _WIN32
  // Windose implementation - this returns non-zero for success
  return (SetCurrentDirectory(folder.c_str()) != 0);
#else
  // Unix implementation - this returns zero for success
  return (chdir(folder.c_str()) == 0);
#endif
}

std::string folder_current (void)
{
  return ".";
}

std::string folder_current_full(void)
{
  // It's not clear from the documentation whether the buffer for a path should be one byte longer
  // than the maximum path length to allow for the null termination, so I have made it so anyway
#ifdef _WIN32
  char abspath [MAX_PATH+1];
  return std::string(_fullpath(abspath, ".", MAX_PATH+1));
#else
  char pathname [MAXPATHLEN+1];
  getcwd(pathname,MAXPATHLEN+1);
  return std::string(pathname);
#endif
}

std::string folder_down (const std::string& directory, const std::string& subdirectory)
{
  file_specification spec;
  spec.initialise_folder(directory);
  spec.add_subpath(subdirectory);
  return spec.image();
}

std::string folder_up (const std::string& directory, unsigned levels)
{
  file_specification spec;
  spec.initialise_folder(directory);
  for (unsigned i = 0; i < levels; i++)
    spec.add_subpath("..");
  spec.simplify();
  return spec.image();
}

std::vector<std::string> folder_subdirectories (const std::string& directory)
{
  return folder_wildcard(directory, "*", true, false);
}

std::vector<std::string> folder_files (const std::string& directory)
{
  return folder_wildcard(directory, "*", false, true);
}

std::vector<std::string> folder_all(const std::string& directory)
{
  return folder_wildcard(directory, "*", true, true);
}

std::vector<std::string> folder_wildcard (const std::string& directory, const std::string& wild, bool subdirs, bool files)
{
  std::string dir = directory.empty() ? std::string(".") : directory;
  std::vector<std::string> results;
#ifdef _WIN32
  std::string wildcard = create_filespec(dir, wild);
  long handle = -1;
  _finddata_t fileinfo;
  for (bool OK = (handle = _findfirst((char*)wildcard.c_str(), &fileinfo)) != -1; OK; OK = (_findnext(handle, &fileinfo)==0))
  {
    std::string strentry = fileinfo.name;
    if (strentry.compare(".")!=0 && strentry.compare("..")!=0)
      if ((subdirs && (fileinfo.attrib & _A_SUBDIR)) || (files && !(fileinfo.attrib & _A_SUBDIR)))
        results.push_back(strentry);
  }
  _findclose(handle);
#else
  DIR* d = opendir(dir.c_str());
  if (d)
  {
    for (dirent* entry = readdir(d); entry; entry = readdir(d))
    {
      std::string strentry = entry->d_name;
      if (strentry.compare(".")!=0 && strentry.compare("..")!=0)
      {
        std::string subpath = create_filespec(dir, strentry);
        if (((subdirs && is_folder(subpath)) || (files && is_file(subpath))) && (match_wildcard(wild, strentry)))
          results.push_back(strentry);
      }
    }
    closedir(d);
  }
#endif
  return results;
}

std::string folder_home (void)
{
  if (getenv("HOME"))
    return std::string(getenv("HOME"));
#ifdef _WIN32
  if (getenv("HOMEDRIVE") || getenv("HOMEPATH"))
    return std::string(getenv("HOMEDRIVE")) + std::string(getenv("HOMEPATH"));
  return "C:\\";
#else
  if (getenv("USER"))
    return folder_down("/home", std::string(getenv("USER")));
  if (getenv("USERNAME"))
    return folder_down("/home", std::string(getenv("USERNAME")));
  return "";
#endif
}

////////////////////////////////////////////////////////////////////////////////
// path functions convert between full and relative paths

bool is_full_path(const std::string& path)
{
  file_specification spec;
  spec.initialise_folder(path.empty() ? std::string(".") : path);
  return spec.absolute();
}

bool is_relative_path(const std::string& path)
{
  file_specification spec;
  spec.initialise_folder(path.empty() ? std::string(".") : path);
  return spec.relative();
}

static std::string full_path(const std::string& root, const std::string& path)
{
  // convert path to a full path using root as the start point for relative paths
  // decompose the path and test whether it is already an absolute path, in which case just return it
  file_specification spec;
  spec.initialise_folder(path.empty() ? std::string(".") : path);
  if (spec.absolute()) return spec.image();
  // okay, so the path is relative after all, so we need to combine it with the root path
  // decompose the root path and check whether it is relative
  file_specification rootspec;
  rootspec.initialise_folder(root.empty() ? std::string(".") : root);
  if (rootspec.relative())
    rootspec.make_absolute();
  // Now do the conversion of the path relative to the root
  spec.make_absolute(rootspec);
  return spec.image();
}

static std::string relative_path(const std::string& root, const std::string& path)
{
  // convert path to a relative path, using the root path as its starting point
  // first convert both paths to full paths relative to CWD
  file_specification rootspec;
  rootspec.initialise_folder(root.empty() ? std::string(".") : root);
  if (rootspec.relative())
    rootspec.make_absolute();
  file_specification spec;
  spec.initialise_folder(path.empty() ? std::string(".") : path);
  if (spec.relative())
    spec.make_absolute();
  // now make path spec relative to the root spec
  spec.make_relative(rootspec);
  return spec.image();
}

std::string folder_to_path (const std::string& path, const std::string& directory)
{
  return full_path(path, directory);
}

std::string filespec_to_path (const std::string& path, const std::string& spec)
{
  return create_filespec(folder_to_path(path, folder_part(spec)),filename_part(spec));
}

std::string folder_to_path(const std::string& folder)
{
  return folder_to_path(folder_current(), folder);
}

std::string filespec_to_path(const std::string& filespec)
{
  return filespec_to_path(folder_current(), filespec);
}

std::string folder_to_relative_path(const std::string& root, const std::string& folder)
{
  return relative_path(root, folder);
}

std::string filespec_to_relative_path(const std::string& root, const std::string& spec)
{
  return create_filespec(folder_to_relative_path(root, folder_part(spec)),filename_part(spec));
}

std::string folder_to_relative_path(const std::string& folder)
{
  return folder_to_relative_path(folder_current(), folder);
}

std::string filespec_to_relative_path(const std::string& filespec)
{
  return filespec_to_relative_path(folder_current(), filespec);
}

std::string folder_append_separator(const std::string& folder)
{
  std::string result = folder;
  if (!is_separator(result[result.size()-1]))
    result += preferred_separator;
  return result;
}

////////////////////////////////////////////////////////////////////////////////

std::string basename_part (const std::string& spec)
{
  std::string fname = filename_part(spec);
  // scan back through filename until a '.' is found and remove suffix
  // the whole filename is the basename if there is no '.'
  std::string::size_type i = fname.find_last_of('.');
  // observe Unix convention that a dot at the start of a filename is part of the basename, not the extension
  if (i != 0 && i != std::string::npos)
    fname.erase(i, fname.size()-i);
  return fname;
}

std::string filename_part (const std::string& spec)
{
  // scan back through filename until a preferred_separator is found and remove prefix;
  // if there is no preferred_separator then remove nothing, i.e. the whole filespec is filename
  unsigned i = spec.size();
  while (i--)
  {
    if (is_separator(spec[i]))
      return spec.substr(i+1,spec.size()-i-1);
  }
  return spec;
}

std::string extension_part (const std::string& spec)
{
  std::string fname = filename_part(spec);
  // scan back through filename until a '.' is found and remove prefix;
  std::string::size_type i = fname.find_last_of('.');
  // observe Unix convention that a dot at the start of a filename is part of the name, not the extension;
  if (i != 0 && i != std::string::npos)
    fname.erase(0, i+1);
  else
    fname.erase();
  return fname;
}

std::string folder_part (const std::string& spec)
{
  // scan back through filename until a separator is found and remove prefix
  // if there is no separator, remove the whole
  unsigned i = spec.size();
  while (i--)
  {
    if (is_separator(spec[i]))
      return spec.substr(0,i);
  }
  return std::string();
}

std::vector<std::string> filespec_elements(const std::string& filespec)
{
  file_specification spec;
  spec.initialise_file(filespec);
  std::vector<std::string> result = spec.path();
  if (!spec.drive().empty()) result.insert(result.begin(),spec.drive());
  if (!spec.file().empty()) result.push_back(spec.file());
  return result;
}

std::vector<std::string> folder_elements(const std::string& folder)
{
  file_specification spec;
  spec.initialise_folder(folder);
  std::vector<std::string> result = spec.path();
  if (!spec.drive().empty()) result.insert(result.begin(),spec.drive());
  return result;
}

////////////////////////////////////////////////////////////////////////////////
// mimic the command lookup used by the shell

// Windows looks at the following locations:
// 1) application root
// 2) current directory
// 3) 32-bit system directory
// 4) 16-bit system directory
// 5) windows system directory
// 6) %path%
// currently only (2) and (6) has been implemented although many system folders are on the path anyway
// also implement the implied .exe extension on commands with no path (see CreateProcess documentation)
// TODO - PATHEXT handling to find non-exe executables

std::string path_lookup (const std::string& command)
{
  std::string path = std::string(".") + PATH_SPLITTER + getenv("PATH");
  return lookup(command, path);
}

std::string lookup (const std::string& command, const std::string& path, const std::string& splitter)
{
  // first check whether the command is already a path and check whether it exists
  if (!folder_part(command).empty())
  {
    if (file_exists(command))
      return command;
  }
  else
  {
    // command is just a name - so do path lookup
    std::vector<std::string> paths = split(path, splitter);
    for (unsigned i = 0; i < paths.size(); i++)
    {
      std::string spec = create_filespec(paths[i], command);
      if (file_exists(spec))
      {
        return spec;
      }
    }
  }
#ifdef _WIN32
  // if there is no extension, try recursing on each possible extension
  // TODO iterate through PATHEXT
  if (extension_part(command).empty())
    return lookup(create_filespec(folder_part(command), basename_part(command), "exe"), path, splitter);
#endif
  // if path lookup failed, return empty string to indicate error
  return std::string();
}

////////////////////////////////////////////////////////////////////////////////

std::string install_path(const std::string& argv0)
{
  std::string bin_directory = folder_part(argv0);
  if (bin_directory.empty())
  {
    // do path lookup to find the executable path
    bin_directory = folder_part(path_lookup(argv0));
  }
  return bin_directory;
}

////////////////////////////////////////////////////////////////////////////////
