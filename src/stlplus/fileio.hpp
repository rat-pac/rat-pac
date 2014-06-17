#ifndef FILEIO_HPP
#define FILEIO_HPP
/*------------------------------------------------------------------------------

  Author:    Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License, see ../docs/license.html

  TextIO classes customised to use C stdio FILE*

------------------------------------------------------------------------------*/
#include "os_fixes.hpp"
#include "textio.hpp"
#include <stddef.h>
#include <stdio.h>
#include <cstring>
////////////////////////////////////////////////////////////////////////////////
// Output Files

class oftext : public otext
{
public:
  static size_t preferred_buffer;

  // create an uninitialised device which acts like /dev/null
  oftext (void);
  // attach an already-opened file to this device
  // this will not be closed when the device is destroyed
  // Note: on Windoze, already-open files such as stdout/err are treated as Unix mode because Windoze does eoln conversion
  oftext (FILE* handle,
          bool line_buffer = false);
  // open the file and attach it to this device
  // this will be closed automatically when the device is destroyed
  oftext (const char* filename,
          size_t bufsize = preferred_buffer,
          open_t mode = overwrite,
          bool line_buffer = false);
  oftext (const std::string& filename,
          size_t bufsize = preferred_buffer,
          open_t mode = overwrite,
          bool line_buffer = false);

  // similar to the constructors - these destroy the previous device contents (closing if appropriate)
  // then perform the open/attach as above
  void open (FILE* handle,
             bool line_buffer = false);
  void open (const char* filename,
             size_t bufsize = preferred_buffer,
             open_t mode = overwrite,
             bool line_buffer = false);
  void open (const std::string& filename,
             size_t bufsize = preferred_buffer,
             open_t mode = overwrite,
             bool line_buffer = false);

  // get at the internal handle
  // note that the handle and this device are guaranteed to be synchronised!
  operator FILE* (void);
};

////////////////////////////////////////////////////////////////////////////////
// Input Files

class iftext : public itext
{
public:
  static size_t preferred_buffer;

  // create an uninitialised device which acts like /dev/null
  iftext (void);
  // attach an already-opened file to this device
  // this will not be closed when the device is destroyed
  iftext (FILE* handle);
  // open the file and attach it to this device
  // this will be closed automatically when the device is destroyed
  iftext (const char* filename, size_t bufsize = preferred_buffer);
  iftext (const std::string& filename, size_t bufsize = preferred_buffer);

  // similar to the constructors - these destroy the previous device contents (closing if appropriate)
  // then perform the open/attach as above
  void open (FILE* handle);
  void open (const char* filename,
             size_t bufsize = preferred_buffer);
  void open (const std::string& filename,
             size_t bufsize = preferred_buffer);

  // get at the internal handle
  // note that the handle and this device are guaranteed to be synchronised!
  operator FILE* (void);
};

////////////////////////////////////////////////////////////////////////////////
// Internal buffers

class ofbuff : public obuff
{
  friend class oftext;
public:
  ofbuff (FILE* handle,
          bool line_buffer);
  ofbuff (const char* filename,
          size_t bufsize,
          otext::open_t mode,
          bool line_buffer);
  ofbuff (const std::string& filename,
          size_t bufsize,
          otext::open_t mode,
          bool line_buffer);
  virtual ~ofbuff(void);

  void open(const std::string& fname, size_t bufsize, otext::open_t mode, bool line_buffer);
  virtual void flush (void);
  virtual unsigned put (unsigned char);
  virtual std::string error_string(void) const;

private:
  bool m_managed;
  FILE* m_handle;

  // make this class uncopyable
  ofbuff(const ofbuff&);
  ofbuff& operator = (const ofbuff&);
};

class ifbuff : public ibuff
{
  friend class iftext;
public:
  ifbuff (FILE*);
  ifbuff (const char*, size_t bufsize = iftext::preferred_buffer);
  ifbuff (const std::string&, size_t bufsize = iftext::preferred_buffer);
  virtual ~ifbuff(void);

  void open(const std::string& filename, size_t bufsize);
  virtual int peek (void);
  virtual int get (void);
  virtual std::string error_string(void) const;

private:
  bool m_managed;
  FILE* m_handle;

  // make this class uncopyable
  ifbuff(const ifbuff&);
  ifbuff& operator = (const ifbuff&);
};

////////////////////////////////////////////////////////////////////////////////
// Standard Files
////////////////////////////////////////////////////////////////////////////////

extern iftext fin;
extern oftext fout;
extern oftext ferr;
extern oftext fnull;

////////////////////////////////////////////////////////////////////////////////

#endif
