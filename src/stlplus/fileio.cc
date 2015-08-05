/*------------------------------------------------------------------------------

  Author:    Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License, see ../docs/license.html

  ------------------------------------------------------------------------------*/
#include "os_fixes.hpp"
#include "fileio.hpp"
#include <errno.h>
#include "debug.hpp" 
////////////////////////////////////////////////////////////////////////////////
// Output

ofbuff::ofbuff (FILE* fh, bool _line_buffer) : 
  // for unmanaged files, just pump raw bytes to the file - it is not TextIO's job to do line-endings
  obuff(_line_buffer,otext::binary_mode),
  m_managed(false),
  m_handle(fh)
{
}

ofbuff::ofbuff (const char* fname, size_t bufsize, otext::open_t mode, bool _line_buffer) :
  m_managed(false),
  m_handle(0)
{
  open(std::string(fname ? fname : ""), bufsize, mode, _line_buffer);
}

ofbuff::ofbuff (const std::string& fname, size_t bufsize, otext::open_t mode, bool _line_buffer) :
  m_managed(false),
  m_handle(0)
{
  open(fname, bufsize, mode, _line_buffer);
}

std::string ofbuff::error_string(void) const
{
  // if the number is positive, it is an errno, otherwise it is a built-in and can be handled by the baseclass
  if (error_number() > 0)
    return std::string(strerror(error_number()));
  else
    return obuff::error_string();
}

void ofbuff::open(const std::string& fname, size_t bufsize, otext::open_t mode, bool _line_buffer)
{
  // open a managed file
  if (fname.empty())
  {
    // an empty filename is an error without even trying to open the file
    // on Windoze fopen raises an assertion failure in this case which is unacceptable so I trap it first
    set_error(textio_open_failed);
  }
  else
  {
    m_managed = true;
    // open in binary mode so that *I* have control over newline conventions
    m_handle = fopen(fname.c_str(),(mode==otext::overwrite ? "wb" : "ab"));
    if (!m_handle)
    {
      if (errno)
        set_error(errno);
      else
        set_error(textio_open_failed);
    }
    else if (setvbuf(m_handle,0,(bufsize ? _IOFBF : _IONBF),bufsize) != 0)
    {
      int error = ferror(m_handle);
      if (error)
        set_error(error);
      else
        set_error(textio_open_failed);
    }
  }
  set_line_buffer(_line_buffer);
}

void ofbuff::flush (void)
{
  if (m_handle && fflush(m_handle) == EOF)
  {
    int error = ferror(m_handle);
    if (error)
      set_error(error);
    else
      set_error(textio_put_failed);
  }
}

ofbuff::~ofbuff(void)
{
  flush();
  if (m_managed && m_handle)
    fclose(m_handle);
}

unsigned ofbuff::put (unsigned char ch)
{
  if (!m_handle) return 0;
  if (fputc(ch, m_handle) == EOF)
  {
    int error = ferror(m_handle);
    if (error)
      set_error(error);
    return 0;
  }
  return 1;
}

////////////////////////////////////////////////////////////////////////////////

size_t oftext::preferred_buffer = 4096;

oftext::oftext (void) : 
  otext() 
{
}

oftext::oftext (FILE* fh, bool line_buffer) :
  otext(new ofbuff(fh,line_buffer))
{
}

oftext::oftext (const char* fname, size_t bufsize, open_t mode, bool line_buffer) :
  otext(new ofbuff(fname, bufsize, mode, line_buffer))
{
}

oftext::oftext (const std::string& fname, size_t bufsize, open_t mode, bool line_buffer) :
  otext(new ofbuff(fname, bufsize, mode, line_buffer))
{
}

void oftext::open (FILE* fh, bool line_buffer)
{
  otext::open(new ofbuff(fh,line_buffer));
} 

void oftext::open (const char* fname, size_t bufsize, open_t mode, bool line_buffer)
{
  otext::open(new ofbuff(fname, bufsize, mode, line_buffer));
} 

void oftext::open (const std::string& fname, size_t bufsize, open_t mode, bool line_buffer)
{
  otext::open(new ofbuff(fname.c_str(), bufsize, mode, line_buffer));
} 

oftext::operator FILE* (void)
{
  ofbuff* filebuf = dynamic_cast<ofbuff*>(m_buffer);
  return filebuf ? filebuf->m_handle : 0;
}

/////////////////////////////////////////////////////////////////////////////////
// Input

ifbuff::ifbuff (FILE* fp) :
  m_managed(false),
  m_handle(fp)
{
}

ifbuff::ifbuff (const char* fname, size_t bufsize) :
  m_managed(false),
  m_handle(0)
{
  open(std::string(fname ? fname : ""), bufsize);
}

ifbuff::ifbuff (const std::string& fname, size_t bufsize) :
  m_managed(false),
  m_handle(0)
{
  open(fname, bufsize);
}

ifbuff::~ifbuff (void)
{
  if (m_managed && m_handle)
    fclose(m_handle);
}

////////////////////////////////////////////////////////////////////////////////

void ifbuff::open(const std::string& fname, size_t bufsize)
{
  if (fname.empty())
  {
    // an empty filename is an error without even trying to open the file
    // on Windoze fopen raises an assertion failure in this case which is unacceptable so I trap it first
    set_error(textio_open_failed);
  }
  else
  {
    m_managed = true;
    m_handle = fopen(fname.c_str(), "rb");
    if (!m_handle)
    {
      if (errno)
        set_error(errno);
      else
        set_error(textio_open_failed);
    }
    else if (setvbuf(m_handle, 0, (bufsize ? _IOFBF : _IONBF), bufsize) != 0)
    {
      int error = ferror(m_handle);
      if (error)
        set_error(error);
      else
        set_error(textio_open_failed);
    }
  }
}

int ifbuff::peek (void)
{
  if (!m_handle) return -1;
  int ch = getc(m_handle);
  if (ch != EOF && ungetc(ch,m_handle) == EOF)
  {
    int error = ferror(m_handle);
    if (error)
      set_error(error);
  }
  return ch;
}

int ifbuff::get (void)
{
  if (!m_handle) return -1;
  int ch = getc(m_handle);
  if (ch == EOF)
  {
    // may be a stdio error or the user may be trying to read past the last valid character
    // in the latter case it is not actually an error - it is perfectly legal to read the EOF
    int error = ferror(m_handle);
    if (error)
      set_error(error);
  }
  return ch;
}

std::string ifbuff::error_string(void) const
{
  // if the number is positiove, it is an errno, otherwise it is a built-in and can be handled by the baseclass
  if (error_number() > 0)
    return std::string(strerror(error_number()));
  else
    return ibuff::error_string();
}

////////////////////////////////////////////////////////////////////////////////

size_t iftext::preferred_buffer = 4096;

iftext::iftext(void) :
  itext() 
{
}

iftext::iftext(FILE* fh) :
  itext(new ifbuff(fh))
{
}

iftext::iftext(const char* fname, size_t bufsize) :
  itext(new ifbuff(fname, bufsize))
{
}

iftext::iftext(const std::string& fname, size_t bufsize) :
  itext(new ifbuff(fname, bufsize))
{
}

void iftext::open(FILE* fh)
{
  itext::open(new ifbuff(fh));
} 

void iftext::open (const char* fname, size_t bufsize)
{
  itext::open(new ifbuff(fname, bufsize));
} 

void iftext::open (const std::string& fname, size_t bufsize)
{
  itext::open(new ifbuff(fname.c_str(), bufsize));
} 

iftext::operator FILE* (void)
{
  ifbuff* filebuf = dynamic_cast<ifbuff*>(m_buffer);
  return filebuf ? filebuf->m_handle : 0;
}


////////////////////////////////////////////////////////////////////////////////
// Standard Files
// TODO - fix known problems with global static objects

iftext fin(stdin);
oftext fout(stdout,true);
oftext ferr(stderr,true);
oftext fnull;

////////////////////////////////////////////////////////////////////////////////
