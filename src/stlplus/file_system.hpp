#ifndef FILE_SYSTEM_HPP
#define FILE_SYSTEM_HPP
/*------------------------------------------------------------------------------

  Author:    Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License, see ../docs/license.html

  Simplified access to the File system

  All file system access and filename manipulation should be done
  with this package. Then it is only necessary to port this package
  to port all file handling.

------------------------------------------------------------------------------*/
#include "os_fixes.hpp"
#include <string>
#include <vector>
#include <time.h>

////////////////////////////////////////////////////////////////////////////////
// classifying functions

bool is_present(const std::string& thing);
bool is_folder(const std::string& thing);
bool is_file(const std::string& thing);

////////////////////////////////////////////////////////////////////////////////
// file functions

bool file_exists(const std::string& filespec);
bool file_readable(const std::string& filespec);
bool file_writable(const std::string& filespec);
size_t file_size(const std::string& filespec);
bool file_delete(const std::string& filespec);
bool file_rename (const std::string& old_filespec, const std::string& new_filespec);
bool file_move (const std::string& old_filespec, const std::string& new_filespec);
bool file_copy (const std::string& old_filespec, const std::string& new_filespec);

// Read-only versus read-write control. This is equivalent to chmod on Unix,
// but I've insulated the user from the low-level routine because of
// differences in the OSs' interpretation of the mode parameter. I've also
// defined a new set of constants to control this, again because of
// inconsistencies. The idea is to combine the constants as bit-masks so as to
// build up a set of permissions. The modes are ORed together to build up a
// set of permissions and then ANDed with a mask to control which people have
// that permission. Permissions can be ORed together too. So, for example, to
// give the owner read-write access and all others only read access, you would
// use the expression:
//   ((read_mode | write_mode) & owner_mask) | (read_mode & (group_mask | other_mask))
// This can be simplified by using combined modes and combined masks to:
//   (read_write_mode & owner_mask) | (read_mode & non_owner_mask)

// basic modes
extern const int read__mode;
extern const int write_mode;
extern const int execute_mode;
// combined modes
extern const int none_mode;
extern const int read_write_mode;
extern const int all_mode;
// basic users
extern const int owner_mask;
extern const int group_mask;
extern const int other_mask;
// combined users
extern const int non_owner_mask;
extern const int all_mask;
// common settings
extern const int read_mode_all;
extern const int read_write_mode_owner_read_mode_all;
extern const int read_mode_owner_only;
extern const int read_write_mode_owner_only;
// the function itself
bool file_set_mode(const std::string& filespec, int mode);

// get the file's time stamps as a time_t - see the stlplus time.hpp package
time_t file_created(const std::string& filespec);
time_t file_modified(const std::string& filespec);
time_t file_accessed(const std::string& filespec);

std::string create_filespec(const std::string& folder, const std::string& filename);
std::string create_filespec(const std::string& folder, const std::string& basename, const std::string& extension);
std::string create_filename(const std::string& basename, const std::string& extension);

////////////////////////////////////////////////////////////////////////////////
// folder functions

bool folder_create(const std::string& folder);
bool folder_exists(const std::string& folder);
bool folder_readable(const std::string& folder);
bool folder_writable(const std::string& folder);
bool folder_delete(const std::string& folder, bool recurse = false);
bool folder_rename (const std::string& old_directory, const std::string& new_directory);
bool folder_empty(const std::string& folder);

bool folder_set_current(const std::string& folder);
std::string folder_current(void);
std::string folder_current_full(void);
std::string folder_home(void);
std::string folder_down(const std::string& folder, const std::string& subfolder);
std::string folder_up(const std::string& folder, unsigned levels = 1);

std::vector<std::string> folder_subdirectories(const std::string& folder);
std::vector<std::string> folder_files(const std::string& folder);
std::vector<std::string> folder_all(const std::string& folder);
std::vector<std::string> folder_wildcard(const std::string& folder, const std::string& wildcard, bool subdirs = true, bool files = true);

////////////////////////////////////////////////////////////////////////////////
// path functions

bool is_full_path(const std::string& path);
bool is_relative_path(const std::string& path);

// convert to a full path relative to the root path
std::string folder_to_path(const std::string& root, const std::string& folder);
std::string filespec_to_path(const std::string& root, const std::string& filespec);

// convert to a full path relative to the current working directory
std::string folder_to_path(const std::string& folder);
std::string filespec_to_path(const std::string& filespec);

// convert to a relative path relative to the root path
std::string folder_to_relative_path(const std::string& root, const std::string& folder);
std::string filespec_to_relative_path(const std::string& root, const std::string& filespec);

// convert to a relative path relative to the current working directory
std::string folder_to_relative_path(const std::string& folder);
std::string filespec_to_relative_path(const std::string& filespec);

// append a folder separator to the path to make it absolutely clear that it is a folder
std::string folder_append_separator(const std::string& folder);

////////////////////////////////////////////////////////////////////////////////
// access functions split a filespec into its elements

std::string basename_part(const std::string& filespec);
std::string filename_part(const std::string& filespec);
std::string extension_part(const std::string& filespec);
std::string folder_part(const std::string& filespec);

// split a path into a vector of elements - i.e. split at the folder separator
std::vector<std::string> folder_elements(const std::string& folder);
std::vector<std::string> filespec_elements(const std::string& filespec);

////////////////////////////////////////////////////////////////////////////////
// Path lookup functions

#ifdef _WIN32
#define PATH_SPLITTER ";"
#else
#define PATH_SPLITTER ":"
#endif

// The lookup normally carried out by the shell to find a command in a
// directory in the PATH. Give this function the name of a command and it
// will return the full path. It returns an empty string on failure.
std::string path_lookup (const std::string& command);

// Generalised form of the above, takes a second argument
// - the list to search. This can be used to do other path lookups,
// such as LD_LIBRARY_PATH. The third argument specifies the splitter -
// the default value of PATH_SPLITTER is appropriate for environment variables.
std::string lookup (const std::string& file, const std::string& path, const std::string& splitter = PATH_SPLITTER);

// utility function for finding the folder that contains the current executable
// the argument is the argv[0] parameter passed to main
std::string install_path(const std::string& argv0);

////////////////////////////////////////////////////////////////////////////////

#endif
