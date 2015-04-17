/*------------------------------------------------------------------------------

  Author:    Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License, see ../docs/license.html

  ------------------------------------------------------------------------------*/
#include "os_fixes.hpp"
#include "textio.hpp"
#include "dprintf.hpp"
#include "string_utilities.hpp"
#include <ctype.h>

////////////////////////////////////////////////////////////////////////////////
// error codes

const int textio_uninitialised = -1;
const int textio_put_failed = -2;
const int textio_format_error = -3;
const int textio_get_failed = -4;
const int textio_open_failed = -5;

static std::string textio_error_string(int error)
{
  switch(error)
  {
  case 0: return std::string("no error");
  case textio_uninitialised: return std::string("the TextIO device hasn't been initialised");
  case textio_put_failed: return std::string("an attempt to put text failed");
  case textio_format_error: return std::string("a format error was found reading input");
  case textio_get_failed: return std::string("an attempt to get text failed");
  case textio_open_failed: return std::string("the device could not be opened");
  default: return std::string("unknown error number " + to_string(error));
  }
}

////////////////////////////////////////////////////////////////////////////////
// Output
////////////////////////////////////////////////////////////////////////////////

std::string to_string(otext::newline_t mode)
{
  switch(mode)
  {
  case otext::binary_mode: return std::string("binary");
  case otext::unix_mode: return std::string("unix");
  case otext::msdos_mode: return std::string("msdos");
  case otext::macos_mode: return std::string("macos");
  }
  return std::string("*unknown mode*");
}

std::string to_string(otext::open_t mode)
{
  switch(mode)
  {
  case otext::overwrite: return std::string("overwrite");
  case otext::append: return std::string("append");
  }
  return std::string("*unknown mode*");
}

////////////////////////////////////////////////////////////////////////////////

otext::otext (void) : m_buffer(0)
{
}

otext::otext (obuff* b) : m_buffer(0)
{
  open(b);
}

otext::~otext (void)
{
  close();
}

bool otext::initialised (void) const
{
  return m_buffer != 0;
}

void otext::open (obuff* b)
{
  close();
  m_buffer = b;
  if (m_buffer) m_buffer->m_aliases++;
}

void otext::close (void)
{
  if (m_buffer)
  {
    if (--m_buffer->m_aliases == 0) 
      delete m_buffer;
    m_buffer = 0;
  }
}

otext::otext (const otext& source) : m_buffer(0)
{
  if (m_buffer != source.m_buffer)
    open(source.m_buffer);
}

otext& otext::operator = (const otext& source)
{
  if (m_buffer != source.m_buffer)
    open(source.m_buffer);
  return *this;
}

bool otext::error (void) const
{
  return error_number() != 0;
}

int otext::error_number (void) const
{
  return m_buffer ? m_buffer->error_number() : textio_uninitialised;
}

std::string otext::error_string(void) const
{
  if (!error()) return std::string();
  return m_buffer ? m_buffer->error_string() : textio_error_string(textio_uninitialised);
}

void otext::set_error(int error)
{
  if (m_buffer) m_buffer->set_error(error);
}

void otext::clear_error (void)
{
  if (m_buffer) m_buffer->clear_error();
}

void otext::set_newline_mode(otext::newline_t _newline)
{
  if (m_buffer) m_buffer->set_newline_mode(_newline);
}

void otext::set_unix_mode(void)
{
  set_newline_mode(otext::unix_mode);
}

void otext::set_msdos_mode(void)
{
  set_newline_mode(otext::msdos_mode);
}

void otext::set_macos_mode(void)
{
  set_newline_mode(otext::macos_mode);
}

void otext::set_native_mode(void)
{
  set_newline_mode(otext::native_mode);
}

void otext::set_binary_mode(void)
{
  set_newline_mode(otext::binary_mode);
}

otext::newline_t otext::newline_mode(void) const
{
  return m_buffer ? m_buffer->newline_mode() : otext::native_mode;
}

bool otext::is_unix_mode(void) const
{
  return newline_mode() == otext::unix_mode;
}

bool otext::is_msdos_mode(void) const
{
  return newline_mode() == otext::msdos_mode;
}

bool otext::is_macos_mode(void) const
{
  return newline_mode() == otext::macos_mode;
}

bool otext::is_native_mode(void) const
{
  return newline_mode() == otext::native_mode;
}

bool otext::is_binary_mode(void) const
{
  return newline_mode() == otext::binary_mode;
}

void otext::set_integer_width(unsigned width)
{
  if (m_buffer) m_buffer->set_integer_width(width);
}

unsigned otext::integer_width(void) const
{
  return m_buffer ? m_buffer->integer_width() : 0;
}

void otext::set_integer_radix(unsigned radix)
{
  if (m_buffer) m_buffer->set_integer_radix(radix);
}

unsigned otext::integer_radix(void) const
{
  return m_buffer ? m_buffer->integer_radix() : 0;
}

void otext::set_integer_display(radix_display_t display)
{
  if (m_buffer) m_buffer->set_integer_display(display);
}

radix_display_t otext::integer_display(void) const
{
  return m_buffer ? m_buffer->integer_display() : radix_c_style_or_hash;
}

void otext::set_real_width(unsigned width)
{
  if (m_buffer) m_buffer->set_real_width(width);
}

unsigned otext::real_width(void) const
{
  return m_buffer ? m_buffer->real_width() : 0;
}

void otext::set_real_precision(unsigned width)
{
  if (m_buffer) m_buffer->set_real_precision(width);
}

unsigned otext::real_precision(void) const
{
  return m_buffer ? m_buffer->real_precision() : 0;
}

void otext::set_real_display(real_display_t display)
{
  if (m_buffer) m_buffer->set_real_display(display);
}

real_display_t otext::real_display(void) const

{
  return m_buffer ? m_buffer->real_display() : display_mixed;
}

bool otext::put (int ch)
{
  if (!m_buffer) return false;
  bool success = true;
  // its illegal to put a value outside the range of unsigned char
  if (ch < 0 || ch > 255)
  {
    set_error(textio_put_failed);
    success = false;
  }
  else if (ch != newline)
  {
    // no special handling for non-newline characters
    if (m_buffer->put((unsigned char)ch))
      m_buffer->increment();
    else
    {
      set_error(textio_put_failed);
      success = false;
    }
  }
  else
  {
    switch(m_buffer->newline_mode())
    {
    case otext::unix_mode:
      // convert the abstract newline character to the literal linefeed character
      if (m_buffer->put('\xa'))
        m_buffer->increment(true);
      else
      {
        set_error(textio_put_failed);
        success = false;
      }
      // test for line buffer condition only on end-of-line
      if (m_buffer->line_buffer())
        flush();
      break;
    case otext::msdos_mode:
      // convert the abstract newline character to the literal carriage-return/linefeed pair
      if (m_buffer->put('\xd'))
        m_buffer->increment();
      else
      {
        set_error(textio_put_failed);
        success = false;
      }
      if (m_buffer->put('\xa'))
        m_buffer->increment(true);
      else
      {
        set_error(textio_put_failed);
        success = false;
      }
      // test for line buffer condition only on end-of-line
      if (m_buffer->line_buffer())
        flush();
      break;
    case otext::macos_mode:
      // convert the abstract newline character to the literal carriage-return character
      if (m_buffer->put('\xd'))
        m_buffer->increment(true);
      else
      {
        set_error(textio_put_failed);
        success = false;
      }
      // test for line buffer condition only on end-of-line
      if (m_buffer->line_buffer())
        flush();
      break;
    case otext::binary_mode:
      // raw data mode so no special handling of newlines
      if (m_buffer->put((unsigned char)ch))
        m_buffer->increment(true);
      else
      {
        set_error(textio_put_failed);
        success = false;
      }
      break;
    }
  }
  return success;
}

bool otext::put (const char* str)
{
  // treat a null string as acceptable - the method does nothing but returns a success flag
  if (!str) return true;
  bool result = true;
  // C arrays are null terminated
  for (unsigned i = 0; str[i]; ++i)
  {
    // rely on the put method to set the error status
    if (!put((unsigned char)str[i]))
      result = false;
  }
  return result;
}

bool otext::put (const std::string& str)
{
  bool result = true;
  // STL strings are delimited by their size method and may contain nulls
  // it would therefore be a bug to call str.c_str() and use the above put(char*)
  for (unsigned i = 0; i < str.size(); ++i)
  {
    // rely on the put method to set the error status
    if (!put((unsigned char)str[i]))
      result = false;
  }
  return result;
}

unsigned long otext::bytes (void) const
{
  return m_buffer ? m_buffer->bytes() : 0;
}

unsigned otext::line (void) const
{
  return m_buffer ? m_buffer->line() : 0;
}

unsigned otext::column (void) const
{
  return m_buffer ? m_buffer->column() : 0;
}

void otext::flush (void)
{
  if (m_buffer) m_buffer->flush();
}

otext::operator bool (void) const
{
  return !error();
}

bool otext::operator ! (void) const
{
  return error();
}

otext& otext::operator << (char ch)
{
  put((unsigned char)ch);
  return *this;
}

otext& otext::operator << (signed char ch)
{
  put((unsigned char)ch);
  return *this;
}

otext& otext::operator << (unsigned char ch)
{
  put(ch);
  return *this;
}

otext& otext::operator << (const char* str)
{
  put(str);
  return *this;
}

otext& otext::operator << (const std::string& str)
{
  put(str);
  return *this;
}

otext& otext::operator << (const std::vector<std::string>& str)
{
  for (unsigned i = 0; i < str.size(); i++)
  {
    operator << (str[i]);
    operator << (newline);
  }
  return *this;
}

otext& otext::operator << (bool b)
{
  return operator << (b ? '1' : '0');
}

otext& otext::operator << (short s)
{
  try
  {
    operator << (to_string(s, integer_radix(), integer_display(), integer_width()));
  }
  catch(const std::invalid_argument)
  {
    set_error(textio_format_error);
  }
  return *this;
}

otext& otext::operator << (unsigned short s)
{
  try
  {
    operator << (to_string(s, integer_radix(), integer_display(), integer_width()));
  }
  catch(const std::invalid_argument)
  {
    set_error(textio_format_error);
  }
  return *this;
}

otext& otext::operator << (int i)
{
  try
  {
    operator << (to_string(i, integer_radix(), integer_display(), integer_width()));
  }
  catch(const std::invalid_argument)
  {
    set_error(textio_format_error);
  }
  return *this;
}

otext& otext::operator << (unsigned int i)
{
  try
  {
    operator << (to_string(i, integer_radix(), integer_display(), integer_width()));
  }
  catch(const std::invalid_argument)
  {
    set_error(textio_format_error);
  }
  return *this;
}

otext& otext::operator << (long l)
{
  try
  {
    operator << (to_string(l, integer_radix(), integer_display(), integer_width()));
  }
  catch(const std::invalid_argument)
  {
    set_error(textio_format_error);
  }
  return *this;
}

otext& otext::operator << (unsigned long l)
{
  try
  {
    operator << (to_string(l, integer_radix(), integer_display(), integer_width()));
  }
  catch(const std::invalid_argument)
  {
    set_error(textio_format_error);
  }
  return *this;
}

otext& otext::operator << (float f)
{
  try
  {
    operator << (to_string(f, real_display(), real_width(), real_precision()));
  }
  catch(const std::invalid_argument)
  {
    set_error(textio_format_error);
  }
  return *this;
}

otext& otext::operator << (double d)
{
  try
  {
    operator << (to_string(d, real_display(), real_width(), real_precision()));
  }
  catch(const std::invalid_argument)
  {
    set_error(textio_format_error);
  }
  return *this;
}

otext& otext::operator << (const void* p)
{
  try
  {
    operator << (to_string(p, 16, integer_display(), integer_width()));
  }
  catch(const std::invalid_argument)
  {
    set_error(textio_format_error);
  }
  return *this;
}

otext& otext::operator << (otext::manipulator_function fn)
{
  if (fn) fn(*this);
  return *this;
}

otext& otext::operator << (itext& in)
{
  while (in)
    put((unsigned char)in.get());
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
// otext Manipulators

void flush (otext& ot)
{
  ot.flush();
}

void endl (otext& ot)
{
  ot.put(newline);
}

void close (otext& ot)
{
  ot.close();
}

void hex(otext& ot)
{
  ot.set_integer_radix(16);
}

void oct(otext& ot)
{
  ot.set_integer_radix(8);
}

void dec(otext& ot)
{
  ot.set_integer_radix(10);
}

////////////////////////////////////////////////////////////////////////////////
// Input
////////////////////////////////////////////////////////////////////////////////

std::string to_string(itext::newline_t mode)
{
  switch(mode)
  {
  case itext::binary_mode: return std::string("binary");
  case itext::convert_mode: return std::string("convert");
  }
  return std::string("*unknown mode*");
}

////////////////////////////////////////////////////////////////////////////////

itext::itext (void) : m_buffer(0)
{
}

itext::itext (ibuff* b) : m_buffer(0)
{
  open(b);
}

itext::~itext (void)
{
  close();
}

bool itext::initialised (void) const
{
  return m_buffer != 0;
}

void itext::open (ibuff* b)
{
  close();
  m_buffer = b;
  if (m_buffer) m_buffer->m_aliases++;
}

void itext::close (void)
{
  if (m_buffer)
  {
    if (--m_buffer->m_aliases == 0) 
      delete m_buffer;
    m_buffer = 0;
  }
}

itext::itext (const itext& source) : m_buffer(0)
{
  if (m_buffer != source.m_buffer)
    open(source.m_buffer);
}

itext& itext::operator = (const itext& source)
{
  if (m_buffer != source.m_buffer)
    open(source.m_buffer);
  return *this;
}

bool itext::error (void) const
{
  return error_number() != 0;
}

int itext::error_number (void) const
{
  return m_buffer ? m_buffer->error_number() : textio_uninitialised;
}

std::string itext::error_string(void) const
{
  if (!error()) return std::string();
  return m_buffer ? m_buffer->error_string() : textio_error_string(textio_uninitialised);
}

void itext::set_error(int error)
{
  if (m_buffer) m_buffer->set_error(error);
}

void itext::clear_error (void)
{
  if (m_buffer) m_buffer->clear_error();
}

void itext::set_newline_mode(itext::newline_t nl)
{
  if (m_buffer) m_buffer->set_newline_mode(nl);
}

void itext::set_convert_mode(void)
{
  set_newline_mode(itext::convert_mode);
}

void itext::set_binary_mode(void)
{
  set_newline_mode(itext::binary_mode);
}

itext::newline_t itext::newline_mode(void) const
{
  return m_buffer ? m_buffer->newline_mode() : itext::convert_mode;
}

bool itext::is_convert_mode(void)
{
  return newline_mode() == itext::convert_mode;
}

bool itext::is_binary_mode(void)
{
  return newline_mode() == itext::binary_mode;
}

bool itext::eof (void)
{
  return peek() == -1;
}

bool itext::eoln (void)
{
  return peek() == newline;
}

int itext::peek (void)
{
  if (!m_buffer) return -1;
  int ch = m_buffer->peek();
  if (m_buffer->newline_mode() == itext::convert_mode)
  {
    switch(ch)
    {
    case '\xd': // carriage-return
    case '\xa': // line-feed
      ch = newline;
      break;
    default:
      break;
    }
  }
  return ch;
}

int itext::get (void)
{
  if (!m_buffer) return -1;
  // get a raw character from the input buffer
  int ch = m_buffer->get();
  // Note: it used to be an error to get an eof
  //       However, this seems unnecessary - input buffers should be designed to just return -1 repeatedly
  //       It is then legal to get an eof any number of times
  if (ch == -1) return -1;
  // optionally do line-end conversion if this cg=haracter is a line-end character
  if (m_buffer->newline_mode() == itext::convert_mode)
  {
    switch(ch)
    {
    case '\xd': // carriage-return - optionally followed by linefeed
      ch = newline;
      if (m_buffer->peek() == '\xa')
      {
        m_buffer->get();
        m_buffer->increment();
      }
      break;
    case '\xa': // linefeed
      ch = newline;
      break;
    default:
      break;
    }
  }
  m_buffer->increment(ch == newline);
  return ch;
}

unsigned long itext::bytes (void) const
{
  return m_buffer ? m_buffer->bytes() : 0;
}

unsigned itext::line (void) const
{
  return m_buffer ? m_buffer->line() : 0;
}

unsigned itext::column (void) const
{
  return m_buffer ? m_buffer->column() : 0;
}

bool itext::good (void)
{
  return !error() && !eof();
}

itext::operator bool (void)
{
  return good();
}

bool itext::operator ! (void)
{
  return !good();
}

itext& itext::operator >> (char& c)
{
  // TODO - test for out of range or eof value
  c = (char)get();
  return *this;
}

itext& itext::operator >> (signed char& c)
{
  // TODO - test for out of range or eof value
  c = (signed char)get();
  return *this;
}

itext& itext::operator >> (unsigned char& c)
{
  // TODO - test for out of range or eof value
  c = (unsigned char)get();
  return *this;
}

static std::string get_token(itext& str)
{
  // Keep number handling simple by just getting a token
  // let the string-to-numeric conversions detect any errors
  // TODO - be fussier and allow other delimiters
  std::string result;
  int ch = 0;
  skipwhite(str);
  // get token
  for (ch = str.peek(); ch !=-1 && !isspace(ch); ch = str.peek())
    result += (char)str.get();
  return result;
}

itext& itext::operator >> (std::string& s)
{
  s = get_token(*this);
  return *this;
}

bool itext::getline(std::string& line)
{
  line.erase();
  bool result = false;
  for (;;)
  {
    int ch = peek();
    if (ch == -1) break;
    result = true;
    ch = get();
    if (ch == newline) break;
    line += (char)ch;
  }
  return result;
}

itext& itext::operator >> (std::vector<std::string>& d)
{
  d.clear();
  std::string line;
  while(getline(line))
    d.push_back(line);
  return *this;
}

itext& itext::operator >> (bool& b)
{
  try
  {
    b = to_bool(get_token(*this));
  }
  catch(const std::invalid_argument)
  {
    set_error(textio_format_error);
  }
  return *this;
}

itext& itext::operator >> (short& s)
{
  try
  {
    s = to_short(get_token(*this));
  }
  catch(const std::invalid_argument)
  {
    set_error(textio_format_error);
  }
  return *this;
}

itext& itext::operator >> (unsigned short& us)
{
  try
  {
    us = to_ushort(get_token(*this));
  }
  catch(const std::invalid_argument)
  {
    set_error(textio_format_error);
  }
  return *this;
}

itext& itext::operator >> (int& i)
{
  try
  {
    i = to_int(get_token(*this));
  }
  catch(const std::invalid_argument)
  {
    set_error(textio_format_error);
  }
  return *this;
}

itext& itext::operator >> (unsigned int& u)
{
  try
  {
    u = to_uint(get_token(*this));
  }
  catch(const std::invalid_argument)
  {
    set_error(textio_format_error);
  }
  return *this;
}

itext& itext::operator >> (long& l)
{
  try
  {
    l = to_long(get_token(*this));
  }
  catch(const std::invalid_argument)
  {
    set_error(textio_format_error);
  }
  return *this;
}

itext& itext::operator >> (unsigned long& ul)
{
  try
  {
    ul = to_ulong(get_token(*this));
  }
  catch(const std::invalid_argument)
  {
    set_error(textio_format_error);
  }
  return *this;
}

itext& itext::operator >> (float& f)
{
  try
  {
    f = to_float(get_token(*this));
  }
  catch(const std::invalid_argument)
  {
    set_error(textio_format_error);
  }
  return *this;
}

itext& itext::operator >> (double& d)
{
  try
  {
    d = to_double(get_token(*this));
  }
  catch(const std::invalid_argument)
  {
    set_error(textio_format_error);
  }
  return *this;
}

// Hide this function to make some GCC versions happy
/*itext& itext::operator >> (void*& p)
{
  try
  {
    p = to_void_star(get_token(*this));
  }
  catch(const std::invalid_argument)
  {
    set_error(textio_format_error);
  }
  return *this;
  }*/

itext& itext::operator >> (itext::manipulator_function fn)
{
  if (fn) fn(*this);
  return *this;
}

itext& itext::operator >> (otext& out)
{
  while (good())
    out.put(get());
  return *this;
}

////////////////////////////////////////////////////////////////////////////////;
// itext manipulators

void skipwhite (itext& it)
{
  for (int ch = it.peek(); ch != -1 && isspace(ch); ch = it.peek())
    it.get();
}

void skiponewhite (itext& it)
{
  int ch = it.peek();
  if (ch != -1 && isspace(ch))
    it.get();
}

void skipspaces (itext& it)
{
  for (int ch = it.peek(); ch != -1 && ch != newline && isspace(ch); ch = it.peek())
    it.get();
}

void skipline (itext& it)
{
  for (int ch = it.peek(); ch !=-1 ; ch = it.peek())
  {
    it.get();
    if (ch==newline) break;
  }
}

void skipendl (itext& it)
{
  for (int ch = it.peek(); ch != -1 && isspace(ch); ch = it.peek())
  {
    it.get();
    if (ch==newline) break;
  }
}

void close (itext& it)
{
  it.close();
}

////////////////////////////////////////////////////////////////////////////////
// obuff

obuff::obuff (bool line_buffer, otext::newline_t _newline) : 
  m_line_buffer(line_buffer), 
  m_newline(_newline),
  m_bytes(0),
  m_line(1),
  m_column(0),
  m_integer_width(0),
  m_integer_radix(10),
  m_integer_display(radix_c_style_or_hash),
  m_real_width(0),
  m_real_precision(6),
  m_real_display(display_mixed),
  m_error_number(0),
  m_aliases(0)
{
}

obuff::~obuff (void) 
{
}

void obuff::increment(bool _newline)
{
  m_bytes++;
  if (_newline)
  {
    m_line++;
    m_column = 0;
  }
  else
  {
    m_column++;
  }
}

unsigned long obuff::bytes(void) const
{
  return m_bytes;
}

unsigned obuff::line(void) const
{
  return m_line;
}

unsigned obuff::column(void) const
{
  return m_column;
}

bool obuff::line_buffer(void) const
{
  return m_line_buffer;
}

void obuff::set_line_buffer(bool mode)
{
  m_line_buffer = mode;
}

void obuff::set_newline_mode(otext::newline_t _newline)
{
  m_newline = _newline;
}

otext::newline_t obuff::newline_mode(void) const
{
  return m_newline;
}

unsigned obuff::integer_width(void) const
{
  return m_integer_width;
}

void obuff::set_integer_width(unsigned width)
{
  m_integer_width = width;
}

unsigned obuff::integer_radix(void) const
{
  return m_integer_radix;
}

void obuff::set_integer_radix(unsigned radix)
{
  m_integer_radix = radix;
}

radix_display_t obuff::integer_display(void) const
{
  return m_integer_display;
}

void obuff::set_integer_display(radix_display_t display)
{
  m_integer_display = display;
}

unsigned obuff::real_width(void) const
{
  return m_real_width;
}

void obuff::set_real_width(unsigned width)
{
  m_real_width = width;
}

unsigned obuff::real_precision(void) const
{
  return m_real_precision;
}

void obuff::set_real_precision(unsigned precision)
{
  m_real_precision = precision;
}

real_display_t obuff::real_display(void) const
{
  return m_real_display;
}

void obuff::set_real_display(real_display_t display)
{
  m_real_display = display;
}

void obuff::set_error(int error)
{
  if (!m_error_number) m_error_number = error;
}

void obuff::clear_error(void)
{
  m_error_number = 0;
}

int obuff::error_number (void) const
{
  return m_error_number;
}

std::string obuff::error_string(void) const
{
  return textio_error_string(error_number());
}

void obuff::flush (void)
{
}

////////////////////////////////////////////////////////////////////////////////
// ibuff

ibuff::ibuff (itext::newline_t _newline) :
  m_newline_mode(_newline),
  m_bytes(0),
  m_line(1),
  m_column(0),
  m_error_number(0),
  m_aliases(0)
{
}

ibuff::~ibuff (void)
{
}

void ibuff::increment(bool _newline)
{
  m_bytes++;
  if (_newline)
  {
    m_line++;
    m_column = 0;
  }
  else
  {
    m_column++;
  }
}

unsigned long ibuff::bytes(void) const
{
  return m_bytes;
}

unsigned ibuff::line(void) const
{
  return m_line;
}

unsigned ibuff::column(void) const
{
  return m_column;
}

void ibuff::set_newline_mode(itext::newline_t _newline)
{
  m_newline_mode = _newline;
}

itext::newline_t ibuff::newline_mode(void) const
{
  return m_newline_mode;
}

void ibuff::set_error(int error)
{
  if (!m_error_number) m_error_number = error;
}

void ibuff::clear_error(void)
{
  m_error_number = 0;
}

int ibuff::error_number (void) const
{
  return m_error_number;
}

std::string ibuff::error_string(void) const
{
  return textio_error_string(error_number());
}

////////////////////////////////////////////////////////////////////////////////
