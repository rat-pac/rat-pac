#ifndef __GLG4StringUtil_hh__
#define __GLG4StringUtil_hh__

// Miscellaneous string utilities derived from BSD-licensed stlplus library
// Copyright 2004, Andy Rushton
// Renamed to prevent linker collision - (SS)

#include <RAT/format_types.hpp>
#include <stdexcept>
#include <string>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <stdarg.h>

////////////////////////////////////////////////////////////////////////////////
// Conversions of Integer types to string
////////////////////////////////////////////////////////////////////////////////

// The radix (i.e. base) for these conversions can be any value from base 2 to base 36
// specifying any other radix causes std::invalid_argument to be thrown

// The way in which the radix is displayed is defined in radix_types.hpp
// If any other value is used, std::invalid_argument is thrown

// The width argument specifies the number of numerical digits to use in the result
// This is a minimum - if the value requires more digits then it will be wider than the width argument
// However, if it is smaller, then it will be extended to the specified width
// Then, the radix display prefix is added to this width
// For example, using the hash representation of 0 in hex with width=4 gives: 16#0000 - so there's 4 digits in the number part

std::string util_to_string(bool i,           unsigned radix = 10, radix_display_t display = radix_c_style_or_hash, unsigned width = 0)
  throw(std::invalid_argument);

std::string util_to_string(short i,          unsigned radix = 10, radix_display_t display = radix_c_style_or_hash, unsigned width = 0)
  throw(std::invalid_argument);

std::string util_to_string(unsigned short i, unsigned radix = 10, radix_display_t display = radix_c_style_or_hash, unsigned width = 0)
  throw(std::invalid_argument);

std::string util_to_string(int i,            unsigned radix = 10, radix_display_t display = radix_c_style_or_hash, unsigned width = 0)
  throw(std::invalid_argument);

std::string util_to_string(unsigned int i,   unsigned radix = 10, radix_display_t display = radix_c_style_or_hash, unsigned width = 0)
  throw(std::invalid_argument);

std::string util_to_string(long i,           unsigned radix = 10, radix_display_t display = radix_c_style_or_hash, unsigned width = 0)
  throw(std::invalid_argument);

std::string util_to_string(unsigned long i,  unsigned radix = 10, radix_display_t display = radix_c_style_or_hash, unsigned width = 0)
  throw(std::invalid_argument);

std::string util_to_string(const void*,      unsigned radix = 16, radix_display_t display = radix_c_style_or_hash, unsigned width = 0)
  throw(std::invalid_argument);

////////////////////////////////////////////////////////////////////////////////
// convert a real type to string
////////////////////////////////////////////////////////////////////////////////

// Only decimal radix is supported

// The way in which the number is displayed is defined in radix_types.hpp
// Using any other value for the display type causes std::invalid_argument to be thrown

std::string util_to_string(float f,  real_display_t display = display_mixed, unsigned width = 0, unsigned precision = 6)
  throw(std::invalid_argument);
std::string util_to_string(double f, real_display_t display = display_mixed, unsigned width = 0, unsigned precision = 6)
  throw(std::invalid_argument);

////////////////////////////////////////////////////////////////////////////////
// convert a string to a simple type
////////////////////////////////////////////////////////////////////////////////

// Convert a string to an integer type
// supports all the formats described above for the reverse conversion
// If the radix is set to zero, the conversions deduce the radix from the string representation
// So, 0b prefix is binary, 0 prefix is octal, 0x is hex and <base># prefix is my hash format
// A non-zero radix should be used when the string value has no radix information and is non-decimal
// e.g. the hex value FEDCBA has no indication that it is hex, so specify radix 16
// The radix must be either zero as explained above, or in the range 2 to 16
// Any other value will cause std::invalid_argument to be thrown

bool util_to_bool(const std::string& value, unsigned radix = 0)
  throw(std::invalid_argument);

short util_to_short(const std::string& value, unsigned radix = 0)
  throw(std::invalid_argument);

unsigned short util_to_ushort(const std::string& value, unsigned radix = 0)
  throw(std::invalid_argument);

int util_to_int(const std::string& value, unsigned radix = 0)
  throw(std::invalid_argument);

unsigned int util_to_uint(const std::string& value, unsigned radix = 0)
  throw(std::invalid_argument);

long util_to_long(const std::string& value, unsigned radix = 0)
  throw(std::invalid_argument);

unsigned long util_to_ulong(const std::string& value, unsigned radix = 0)
  throw(std::invalid_argument);

void* util_to_void_star(const std::string& value, unsigned radix = 0)
  throw(std::invalid_argument);

// Convert a floating-point type

float util_to_float(const std::string& value)
  throw(std::invalid_argument);

double util_to_double(const std::string& value)
  throw(std::invalid_argument);

////////////////////////////////////////////////////////////////////////////////
// Perl-inspired split/join functions
////////////////////////////////////////////////////////////////////////////////

// splits the string at every occurance of splitter and adds it as a separate string to the return value
// the splitter is removed
// a string with no splitter in it will give a single-vector string
// an empty string gives an empty vector
std::vector<std::string> util_split (const std::string& str, const std::string& splitter = "\n");

// the reverse of the above
// joins the string vector to create a single string with the joiner inserted between the joins
// Note: the joiner will not be added at the beginning or the end
// However, there are optional fields to add such prefix and suffix strings
std::string util_join (const std::vector<std::string>&,
                  const std::string& joiner = "\n", const std::string& prefix = "", const std::string& suffix = "");

// strips leading and trailing characters from s. This function by 
// Glenn Horton-Smith.
std::string util_strip(const std::string &s, const std::string &stripchars);
// strip spaces, tabs, and quotation marks
std::string util_strip_default(const std::string &s);

// Really handy functions which work like sprintf, but operate on
// C++ strings instead

int util_dprintf (std::string& formatted, const char* format, ...);
int util_vdprintf (std::string& formatted, const char* format, va_list args);

std::string util_dformat (const char* format, ...);
std::string util_vdformat (const char* format, va_list);

#endif
