#ifndef TEXTIO_HPP
#define TEXTIO_HPP
/*------------------------------------------------------------------------------

  Author:    Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License, see ../docs/license.html

  ------------------------------------------------------------------------------*/
#include "os_fixes.hpp"
#include "format_types.hpp"
#include <vector>
#include <string>

////////////////////////////////////////////////////////////////////////////////
// Builtin error codes - you can add your own when you create a new derivative
// buffer in which case you may wish to overload the error_string function of
// the buffer to give a textual form to your errors.
// All internal error codes are negative to differentiate them from system error codes.

extern const int textio_uninitialised;
extern const int textio_put_failed;
extern const int textio_format_error;
extern const int textio_get_failed;
extern const int textio_open_failed;

////////////////////////////////////////////////////////////////////////////////
// Text Output Device
////////////////////////////////////////////////////////////////////////////////

// declaring class as friend is no longer sufficient to inform compiler of
// type's existence as of gcc 4.1
class obuff;
class itext;

class otext
{
protected:
  friend class obuff;
  friend class itext;
  obuff* m_buffer;

public:
  ////////////////////////////////////////////////////////////////////////////////
  // Local Types
  // These local enumerations are in the otext namespace

  // Newline conversion
  // The user of TextIO should always use '\n' for newlines, then TextIO will do the conversions
  // default: native
  // I appended the _mode suffix to avoid conflicts with macros
  enum newline_t
  {
    binary_mode, // no end of line conversion
    unix_mode,   // Unix conversions (LF)
    msdos_mode,  // MS-DOS conversion (CR-LF)
    macos_mode,  // MacOS conversion (CR)
    // the mode of the platform you compiled this on
#if defined(_WIN32)
    native_mode = msdos_mode
#else
    native_mode = unix_mode
#endif
  };
#ifndef __CINT__
  friend std::string to_string(newline_t);
#else
  friend std::string to_string(int);
#endif
  // Open Mode
  // only used for otext devices where the two modes make sense, e.g. files but not pipes
  enum open_t
  {
    overwrite,  // destroy previous contents (default)
    append      // append to previous contents
  };

#ifndef __CINT__
  friend std::string to_string(open_t);
#else
  friend std::string to_string(int);
#endif

  // profile for function manipulators
  typedef void (*manipulator_function)(otext&);

  ////////////////////////////////////////////////////////////////////////////////
  // Member functions

  // create an uninitialised otext
  otext(void);

  // create an initialised otext by attaching a buffer (any derivative of obuff)
  otext(obuff*);

  // close the otext if its open by deleting the buffer (thus calling its destructor)
  virtual ~otext(void);

  // test whether this otext has a buffer attached
  bool initialised(void) const;

  // initialise otext by attaching a buffer (any derivative of obuff)
  // if a buffer is already attached, delete it first
  void open(obuff*);

  // close the otext if its open by deleting the buffer (thus calling its destructor)
  void close(void);

  // copy and assign create aliases - no deep copy is available - no deep copy makes sense!
  otext(const otext&);
  otext& operator = (const otext&);

  // test the buffer's error function
  bool error(void) const;
  int error_number(void) const;
  std::string error_string(void) const;
  void set_error(int error);
  void clear_error(void);

  // newline control
  void set_newline_mode(newline_t newline = native_mode);
  void set_unix_mode(void);
  void set_msdos_mode(void);
  void set_macos_mode(void);
  void set_native_mode(void);
  void set_binary_mode(void);
  newline_t newline_mode(void) const;
  bool is_unix_mode(void) const;
  bool is_msdos_mode(void) const;
  bool is_macos_mode(void) const;
  bool is_native_mode(void) const;
  bool is_binary_mode(void) const;

  // integer formatting control
  void set_integer_width(unsigned width = 0);
  unsigned integer_width(void) const;
  void set_integer_radix(unsigned radix = 10);
  unsigned integer_radix(void) const;
  void set_integer_display(radix_display_t display = radix_c_style_or_hash);
  radix_display_t integer_display(void) const;

  // floating-point formatting control
  void set_real_width(unsigned width = 0);
  unsigned real_width(void) const;
  void set_real_precision(unsigned width = 6);
  unsigned real_precision(void) const;
  void set_real_display(real_display_t display = display_mixed);
  real_display_t real_display(void) const;

  // low-level character write
  // put writes a single character
  // it is an error to put negative values, specifically EOF (-1)
  // the character must be unsigned char, since otherwise there are two definitions of -1
  bool put(int ch);

  // composite operations on strings (C and STL) which repeatly call the above
  bool put(const char*);
  bool put(const std::string&);

  // number of characters written through the otext member functions
  unsigned long bytes(void) const;
  // line and column for last character written
  // this only really has meaning if you are using text conversion mode (i.e. not binary)
  unsigned line(void) const;
  unsigned column(void) const;

  // flush the buffer explicitly (for example to synchronise standard output with standard input)
  void flush(void);

  // test whether the device is capable of accepting/not accepting output
  operator bool (void) const;
  bool operator ! (void) const;

  // the pipe operators << are the main functions used with otext and its derivates
  // they are used in the form:
  //   device << object1 << object2 << object3;

  // single character
  // TODO - wide char
  otext& operator << (char);
  otext& operator << (signed char);
  otext& operator << (unsigned char);

  // string output
  otext& operator << (const char*);
  otext& operator << (const std::string&);

  // string vector - writes whole array as a series of newline separated strings
  otext& operator << (const std::vector<std::string>&);

  // integer output
  otext& operator << (bool);
  otext& operator << (short);
  otext& operator << (unsigned short);
  otext& operator << (int);
  otext& operator << (unsigned int);
  otext& operator << (long); 
  otext& operator << (unsigned long);

  // floating point output
  otext& operator << (float);
  otext& operator << (double);

  // pointer output, compatible with >> operator for void*;
  otext& operator << (const void*);

  // manipulator - applies passed function to stream;
  otext& operator << (manipulator_function);

  // pipe operator - pours one stream into the other until eof();
  // this is an easy way to copy one device into another
  otext& operator << (itext&);
};

////////////////////////////////////////////////////////////////////////////////
// output manipulators, used in the form: fout << flush << endl;

// simply flush the buffer (if any)
void flush(otext&);

// write a newline, iostream style
void endl(otext&);

// close the device
void close(otext&);

// set radix to hex/octal/decimal
void hex(otext&);
void oct(otext&);
void dec(otext&);

// the following used to be manipulators, but for efficiency they are now defined as consts;
const char newline = '\n';
const char space = ' ';
const char tab = '\t';
const char null = '\0';

////////////////////////////////////////////////////////////////////////////////
// Text Input Device
////////////////////////////////////////////////////////////////////////////////

class ibuff;

class itext
{
protected:
  friend class otext;
  friend class ibuff;
  ibuff* m_buffer;

public:
  ////////////////////////////////////////////////////////////////////////////////
  // Newline conversion
  // When conversion is on, all end-of-line conventions (LF, CR-LF, CR) will be converted into a '\n'
  // When conversion is off, data comes in unmodified - note that this can make the eoln() test act strangely
  enum newline_t
  {
    binary_mode,      // no end of line conversion
    convert_mode      // recognise and convert all end-of-line conventions
  };
#ifndef __CINT__
  friend std::string to_string(newline_t);
#else
  friend std::string to_string(int);
#endif

  typedef void (*manipulator_function)(itext&);

public:
  // create an uninitialised itext
  itext(void);

  // create an itext and initialise it with a buffer (any derivative of ibuff)
  itext(ibuff*);

  // closes the itext if it is open and destroys any structures - including the buffer
  virtual ~itext(void);
   
  // test whether the itext has a buffer attached
  bool initialised(void) const;

  // attach a buffer (any derivative of ibuff)
  void open(ibuff*);

  // detach the buffer and delete it (causing the destructor and therefore any closedown actions to be called)
  void close(void);

  // copy and assignment create aliases - it is not sensible to allow a deep copy (think about it)
  itext(const itext&);
  itext& operator = (const itext&);
   
  // test the buffer's error flag or retrieve its value
  bool error(void) const;
  int error_number(void) const;
  std::string error_string(void) const;
  void set_error(int error);
  void clear_error(void);

  // formatting control
  void set_newline_mode(newline_t newline = convert_mode);
  void set_convert_mode(void);
  void set_binary_mode(void);
  newline_t newline_mode(void) const;
  bool is_convert_mode(void);
  bool is_binary_mode(void);

  // test for the two special 'characters' end-of-file (-1) and end-of-line (\n) using peek()
  bool eof(void);
  bool eoln(void);

  // low-level character access - peek allows one character lookahead, get allows one character to be read
  // Both return -1 to indicate EOF
  int peek(void);
  int get(void);
   
  // number of characters read through the itext member functions
  unsigned long bytes(void) const;
  // line and column for last character read
  // this only really has meaning if you are using text conversion mode (i.e. not binary)
  unsigned line(void) const;
  unsigned column(void) const;

  // tests for whether an itext has/hasn't got text to be read
  bool good (void);
  operator bool (void);
  bool operator ! (void);
   
  ////////////////////////////////////////////////////////////////////////////////
  // input-pipe operators are the main usage of itext
  // used in the form:
  //    device >> object1 >> object2 >> object3;
  // device is any derivative of itext, object is any type with a >> operator defined (you can overload)

  // just get next character, including any whitespace or end of line character;
  itext& operator >> (char&);
  itext& operator >> (signed char&);
  itext& operator >> (unsigned char&);

  // skipwhite before reading then read until a whitespace is found
  // this is a kind of tokenising operator
  itext& operator >> (std::string&);
  // gets the whole line
  bool getline(std::string& line);

  // get the whole file as a vector of strings, using newlines to split the input
  itext& operator >> (std::vector<std::string>&);

  // integer operations: skipwhite then read an integer in any of the recognised formats:
  // decimal: 12345
  // octal:   012345
  // hex:     0x12345
  // hash:    13#12345
  itext& operator >> (bool&);
  itext& operator >> (short&);
  itext& operator >> (unsigned short&);
  itext& operator >> (int&);
  itext& operator >> (unsigned int&);
  itext& operator >> (long&);
  itext& operator >> (unsigned long&);

  // real operations, skipwhite then read floating-point number - fraction and exponent are optional
  itext& operator >> (float&);
  itext& operator >> (double&);

  // Hide this function to make some GCC versions happy
  // pointer operator, skipwhite, then reads a pointer written by << operator for void*
  //itext& operator >> (void*&);

  // manipulator - applies passed manipulator function to stream;
  itext& operator >> (manipulator_function);

  // pipe operator - pours one stream into the other until eof();
  // this is the easiest way to make a copy
  itext& operator >> (otext&);
};

// manipulators, used in the form: fin >> skipwhite >> ch

// skip all whitespace as defined by isspace() in <cctype>
void skipwhite(itext&);

// skip up to one whitespace as defined by isspace() in <cctype>
void skiponewhite(itext&);

// skip whitespace excluding end of line - i.e. only spacing characters
void skipspaces(itext&);

// skip all text until and including the next end-of-line character
void skipline(itext&);

// skip all whitespace, but stop after an end-of-line character
void skipendl(itext&);

// close the device
void close(itext&);

////////////////////////////////////////////////////////////////////////////////
// Internals
////////////////////////////////////////////////////////////////////////////////

// Output Buffer

class obuff
{
public:
  // constructor initialises output buffer with the mode fields - line buffering mode and newline conversion mode
  obuff(bool line_buffer = false, otext::newline_t newline = otext::native_mode);

  // Total number of characters written through put()
  // also number of lines and column number of last byte written
  // Note this may not correspond to the number of characters sent to otext due to newline conversion
  void increment(bool newline = false);
  unsigned long bytes(void) const;
  unsigned line(void) const;
  unsigned column(void) const;

  // get line buffering mode - the TextIO device calls flush on every newline when in line beffering mode
  void set_line_buffer(bool mode);
  bool line_buffer(void) const;

  // get newline handling mode - used by TextIO device to process CR/LF characters
  void set_newline_mode(otext::newline_t newline);
  otext::newline_t newline_mode(void) const;

  // Integer formatting fields
  // field width for next formattable integer type, default: 0
  unsigned integer_width(void) const;
  void set_integer_width(unsigned);
  // base for integer display - from 2-36, default: 10
  unsigned integer_radix(void) const; 
  void set_integer_radix(unsigned); 
  // how to display an integer - see string_utilities, default: c_style_or_hash
  radix_display_t integer_display(void) const; 
  void set_integer_display(radix_display_t); 

  // Real formatting fields
  // field width for next formattable real type, default: 0
  unsigned real_width(void) const;
  void set_real_width(unsigned);
  // number of significant digits
  unsigned real_precision(void) const; 
  void set_real_precision(unsigned); 
  // how to display a real - see string_utilities, default: display_mixed
  real_display_t real_display(void) const; 
  void set_real_display(real_display_t); 

  // Error number
  // zero if there is no error, any integer for an error
  // this may be any of the definitions defined in <errno.h> or any derivative specific number
  // this is set by otext itself but may also be set in any of your own derivatives
  void set_error(int error);
  void clear_error(void);
  int error_number(void) const;
  virtual std::string error_string(void) const;

  // Customisation
  // To create a new output buffer, derive from this and then customise the
  // virtual functions. Also provide sensible constructors - for example FileIO
  // would have a constructor taking a filename and open mode. You only HAVE to
  // provide the abstract function put(char), all others are optional. These
  // functions are implicitly called by the otext class which contains your
  // buffer.

  // you should provide a destructor (also virtual) if your buffer needs closedown actions such as closing a file/pipe

  // the default flush does nothing
  // use this to flush any internal buffer - not relevant if your device isn't buffered
  // note that if you are buffering, you should not rely on flush being called explicitly when the buffer is full
  // - you need to manage the buffer yourself
  // flush is called from TextIO to synchronise devices e.g. stdin and stdout
  // It will be called on every newline if the buffer is in line buffer mode
  virtual void flush(void);

  // the only operation that you must provide
  // sends a single character to the target device
  // returns the number of characters written (0 or 1)
  virtual unsigned put(unsigned char) = 0;

  // use this to do any mopping up required on closing a buffer
  // the default destructor does nothing - overload this only if you need to
  virtual ~obuff(void);

private:
  // disallow copying
  obuff& operator = (const obuff&);
  obuff(const obuff&);

protected:
  friend class otext;
  bool m_line_buffer;
  otext::newline_t m_newline;
  unsigned long m_bytes;
  unsigned m_line;
  unsigned m_column;
  unsigned m_integer_width;
  unsigned m_integer_radix;
  radix_display_t m_integer_display;
  unsigned m_real_width;
  unsigned m_real_precision;
  real_display_t m_real_display;
  int m_error_number;
  int m_aliases;
};

// Input Buffer

class ibuff
{
public:
  ibuff(itext::newline_t newline = itext::convert_mode);

  // Total number of characters read through get()
  // also number of lines and column number of last byte read
  // Note this may not correspond to the number of characters read from the device due to newline conversion
  void increment(bool newline = false);
  unsigned long bytes(void) const;
  unsigned line(void) const;
  unsigned column(void) const;

  // get newline handling mode - used by TextIO device to process CR/LF characters
  void set_newline_mode(itext::newline_t newline);
  itext::newline_t newline_mode(void) const;

  // Error number
  // zero if there is no error, any integer for an error
  // this may be any of the errno definitions defined in <errno.h> or any derivative specific number
  // this is set by itext itself but may also be set in any of your own derivatives
  void set_error(int error);
  void clear_error(void);
  int error_number(void) const;
  virtual std::string error_string(void) const;


  // Customisation
  // create a new buffer by customising the following functions
  // you should also provide a destructor (also virtual) if your buffer needs closedown actions
  // it REALLY IS as simple as that - see the existing derivatives for examples

  // peek and get are abstract functions, so you must provide them
  // peek gets the next character without consuming it, get gets the same character but also consumes it
  // this means that text input requires one-character lookahead
  virtual int peek(void) = 0;
  virtual int get(void) = 0;

  // do any mopping up and close the input
  // default destructor does nothing
  virtual ~ibuff(void);

private:
  // disallow copying
  ibuff& operator = (const ibuff&);
  ibuff(const ibuff&);

protected:
  friend class itext;
  itext::newline_t m_newline_mode;
  unsigned long m_bytes;
  unsigned m_line;
  unsigned m_column;
  int m_error_number;
  int m_aliases;
};

////////////////////////////////////////////////////////////////////////////////
#endif
