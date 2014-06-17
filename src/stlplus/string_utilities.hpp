#ifndef STRING_UTILITIES_HPP
#define STRING_UTILITIES_HPP
/*------------------------------------------------------------------------------

  Author:    Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License, see ../docs/license.html

  Utilities for manipulating std::strings, missing from the STL or C++ libraries

  ------------------------------------------------------------------------------*/
#include "os_fixes.hpp"
#include "format_types.hpp"
#include "textio.hpp"
#include <string>
#include <vector>
#include <bitset>
#include <list>
#include <map>
#include <set>
#include <vector>
#include <stdexcept>

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

std::string to_string(bool i,           unsigned radix = 10, radix_display_t display = radix_c_style_or_hash, unsigned width = 0)
  throw(std::invalid_argument);

std::string to_string(short i,          unsigned radix = 10, radix_display_t display = radix_c_style_or_hash, unsigned width = 0)
  throw(std::invalid_argument);

std::string to_string(unsigned short i, unsigned radix = 10, radix_display_t display = radix_c_style_or_hash, unsigned width = 0)
  throw(std::invalid_argument);

std::string to_string(int i,            unsigned radix = 10, radix_display_t display = radix_c_style_or_hash, unsigned width = 0)
  throw(std::invalid_argument);

std::string to_string(unsigned int i,   unsigned radix = 10, radix_display_t display = radix_c_style_or_hash, unsigned width = 0)
  throw(std::invalid_argument);

std::string to_string(long i,           unsigned radix = 10, radix_display_t display = radix_c_style_or_hash, unsigned width = 0)
  throw(std::invalid_argument);

std::string to_string(unsigned long i,  unsigned radix = 10, radix_display_t display = radix_c_style_or_hash, unsigned width = 0)
  throw(std::invalid_argument);

std::string to_string(const void*,      unsigned radix = 16, radix_display_t display = radix_c_style_or_hash, unsigned width = 0)
  throw(std::invalid_argument);

////////////////////////////////////////////////////////////////////////////////
// convert a real type to string
////////////////////////////////////////////////////////////////////////////////

// Only decimal radix is supported

// The way in which the number is displayed is defined in radix_types.hpp
// Using any other value for the display type causes std::invalid_argument to be thrown

std::string to_string(float f,  real_display_t display = display_mixed, unsigned width = 0, unsigned precision = 6)
  throw(std::invalid_argument);
std::string to_string(double f, real_display_t display = display_mixed, unsigned width = 0, unsigned precision = 6)
  throw(std::invalid_argument);

////////////////////////////////////////////////////////////////////////////////
// Convert a string to string
////////////////////////////////////////////////////////////////////////////////

// this is necessary for completeness, e.g. for use in vector_to_string for vector<string>
std::string to_string(const std::string& value);

// ditto for char*
std::string to_string(const char* value);

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

bool to_bool(const std::string& value, unsigned radix = 0)
  throw(std::invalid_argument);

short to_short(const std::string& value, unsigned radix = 0)
  throw(std::invalid_argument);

unsigned short to_ushort(const std::string& value, unsigned radix = 0)
  throw(std::invalid_argument);

int to_int(const std::string& value, unsigned radix = 0)
  throw(std::invalid_argument);

unsigned int to_uint(const std::string& value, unsigned radix = 0)
  throw(std::invalid_argument);

long to_long(const std::string& value, unsigned radix = 0)
  throw(std::invalid_argument);

unsigned long to_ulong(const std::string& value, unsigned radix = 0)
  throw(std::invalid_argument);

void* to_void_star(const std::string& value, unsigned radix = 0)
  throw(std::invalid_argument);

// Convert a floating-point type

float to_float(const std::string& value)
  throw(std::invalid_argument);

double to_double(const std::string& value)
  throw(std::invalid_argument);

////////////////////////////////////////////////////////////////////////////////
// template string conversions for pointers and STL containers
////////////////////////////////////////////////////////////////////////////////
// Note: STLplus containers tend to have built-in string conversion functions consistent with these

template <typename T>
std::string pointer_to_string(const T* value, const std::string& null_string, const std::string& prefix, const std::string& suffix);

template<size_t N>
std::string bitset_to_string(const std::bitset<N>& data);

template<typename T>
std::string list_to_string(const std::list<T>& values, const std::string& separator);

template<typename L, typename R>
std::string pair_to_string(const std::pair<L,R>& values, const std::string& separator);

template<typename K, typename T, typename P>
std::string map_to_string(const std::map<K,T,P>& values, const std::string& pair_separator, const std::string& separator);

template<typename K, typename T, typename P>
std::string multimap_to_string(const std::multimap<K,T,P>& values, const std::string& pair_separator, const std::string& separator);

template<typename K, typename P>
std::string set_to_string(const std::set<K,P>& values, const std::string& separator);

template<typename K, typename P>
std::string multiset_to_string(const std::multiset<K,P>& values, const std::string& separator);

template<typename T>
std::string vector_to_string(const std::vector<T>& values, const std::string& separator);

////////////////////////////////////////////////////////////////////////////////
// Print routines for basic types
////////////////////////////////////////////////////////////////////////////////

// The convention is to have a print(str,val) for printing in-line (i.e. the
// value is on one line) and to have a print(str,val,indent) to print on a whole
// line, with indent before and newline after.

// set the number of spaces to indent per indent step (i.e. the number of spaces = indent*indent_step)
// default is built-in and set to 2
void set_indent_step(unsigned step);
unsigned indent_step(void);
// utility for printing the indent, called from within the second form of print
otext& print_indent(otext& str, unsigned indent);

// print routines for integer types
// the arguments are as for the to_string

otext& print(otext& str, const bool& value,
             unsigned radix = 10, radix_display_t display = radix_c_style_or_hash,
             unsigned width = 0)
  throw(std::invalid_argument);

otext& print(otext& str, const short& value,
             unsigned radix = 10, radix_display_t display = radix_c_style_or_hash,
             unsigned width = 0)
  throw(std::invalid_argument);

otext& print(otext& str, const unsigned short& value,
             unsigned radix = 10, radix_display_t display = radix_c_style_or_hash,
             unsigned width = 0)
  throw(std::invalid_argument);

otext& print(otext& str, const int& value,
             unsigned radix = 10, radix_display_t display = radix_c_style_or_hash,
             unsigned width = 0)
  throw(std::invalid_argument);

otext& print(otext& str, const unsigned int& value,
             unsigned radix = 10, radix_display_t display = radix_c_style_or_hash,
             unsigned width = 0)
  throw(std::invalid_argument);

otext& print(otext& str, const long& value,
             unsigned radix = 10, radix_display_t display = radix_c_style_or_hash,
             unsigned width = 0)
  throw(std::invalid_argument);

otext& print(otext& str, const unsigned long& value,
             unsigned radix = 10, radix_display_t display = radix_c_style_or_hash,
             unsigned width = 0)
  throw(std::invalid_argument);

otext& print(otext& str, const void*& value,
             unsigned radix = 10, radix_display_t display = radix_c_style_or_hash,
             unsigned width = 0)
  throw(std::invalid_argument);

// print routines for floating-point types

otext& print(otext& str, float f, 
             real_display_t display = display_mixed,
             unsigned width = 0, unsigned precision = 6)
  throw(std::invalid_argument);

otext& print(otext& str, double f,
             real_display_t display = display_mixed,
             unsigned width = 0, unsigned precision = 6)
  throw(std::invalid_argument);

// print routines for string
// this is needed for completeness, e.g. when calling print_vector on a vector of strings

otext& print(otext& str, const std::string& value);
otext& print(otext& str, const std::string& value, unsigned indent);

////////////////////////////////////////////////////////////////////////////////
// template print routines for pointers and STL containers
////////////////////////////////////////////////////////////////////////////////
// STLplus containers have these built-in

template <typename T>
otext& print_pointer(otext& str, const T* value,
                     const std::string& null_string, const std::string& prefix, const std::string& suffix);
template <typename T>
otext& print_pointer(otext& str, const T* value, unsigned indent,
                     const std::string& null_string, const std::string& prefix, const std::string& suffix);

template<size_t N>
otext& print_bitset(otext& str, const std::bitset<N>& value);
template<size_t N>
otext& print_bitset(otext& str, const std::bitset<N>& value, unsigned indent);

template<typename T> 
otext& print_list(otext& str, const std::list<T>& values, const std::string& separator);
template<typename T> 
otext& print_list(otext& str, const std::list<T>& values, unsigned indent);

template<typename L, typename R>
otext& print_pair(otext& str, const std::pair<L,R>& values, const std::string& separator);
template<typename L, typename R>
otext& print_pair(otext& str, const std::pair<L,R>& values, const std::string& separator, unsigned indent);

template<typename K, typename T, typename P>
otext& print_map(otext& str, const std::map<K,T,P>& values, const std::string& pair_separator, const std::string& separator);
template<typename K, typename T, typename P>
otext& print_map(otext& str, const std::map<K,T,P>& values, const std::string& pair_separator, unsigned indent);

template<typename K, typename T, typename P>
otext& print_multimap(otext& str, const std::multimap<K,T,P>& values, const std::string& pair_separator, const std::string& separator);
template<typename K, typename T, typename P>
otext& print_multimap(otext& str, const std::multimap<K,T,P>& values, const std::string& pair_separator, unsigned indent);

template<typename K, typename P>
otext& print_set(otext& str, const std::set<K,P>& values, const std::string& separator);
template<typename K, typename P> 
otext& print_set(otext& str, const std::set<K,P>& values, unsigned indent);

template<typename K, typename P> 
otext& print_multiset(otext& str, const std::multiset<K,P>& values, const std::string& separator);
template<typename K, typename P> 
otext& print_multiset(otext& str, const std::multiset<K,P>& values, unsigned indent);

template<typename T> 
otext& print_vector(otext& str, const std::vector<T>& values, const std::string& separator);
template<typename T> 
otext& print_vector(otext& str, const std::vector<T>& values, unsigned indent);

////////////////////////////////////////////////////////////////////////////////
// other string manipulations
////////////////////////////////////////////////////////////////////////////////

// Padding function allows a string to be printed in a fixed-width field

// The definitions for the alignment are declared in format_types.hpp
// Any other value will cause std::invalid_argument to be thrown

std::string pad(const std::string& str, alignment_t alignment, unsigned width, char padch = ' ')
  throw(std::invalid_argument);

// whitespace trimming
std::string trim_left(const std::string& val);
std::string trim_right(const std::string& val);
std::string trim(const std::string& val);

// case conversion for std::strings
std::string lowercase(const std::string& val);
std::string uppercase(const std::string& val);

// convert characters represented in from_set to the characters in the same position in to_set
// for example:
//   filename = translate(filename, "abcdefghijklmnopqrstuvwxyz", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
// converts the filename to uppercase and returns the result (Note that the uppercase function does this more easily).
// if the from_set is longer than the to_set, then the overlap represents characters to delete (i.e. they map to nothing)
std::string translate(const std::string& input, const std::string& from_set, const std::string& to_set);

////////////////////////////////////////////////////////////////////////////////
// wildcard matching
////////////////////////////////////////////////////////////////////////////////

// this function does wildcard matching of the wildcard expression against the candidate std::string
// wildcards are NOT regular expressions
// the wildcard characters are * and ? where * matches 1 or more characters and ? matches only one
// there are also character sets [a-z] [qwertyuiop] etc. which match 1 character
// TODO: character sets like [:alpha:]
// TODO eventually: regular expression matching and substitution (3rd party library?)

bool match_wildcard(const std::string& wild, const std::string& match);

////////////////////////////////////////////////////////////////////////////////
// Perl-inspired split/join functions
////////////////////////////////////////////////////////////////////////////////

// splits the string at every occurance of splitter and adds it as a separate string to the return value
// the splitter is removed
// a string with no splitter in it will give a single-vector string
// an empty string gives an empty vector
std::vector<std::string> split (const std::string& str, const std::string& splitter = "\n");

// the reverse of the above
// joins the string vector to create a single string with the joiner inserted between the joins
// Note: the joiner will not be added at the beginning or the end
// However, there are optional fields to add such prefix and suffix strings
std::string join (const std::vector<std::string>&,
                  const std::string& joiner = "\n", const std::string& prefix = "", const std::string& suffix = "");

////////////////////////////////////////////////////////////////////////////////
// special displays

// display the parameter as a number in bytes, kbytes, Mbytes, Gbytes depending on range
std::string display_bytes(long bytes);

// display the parameter in seconds as a string representation in weeks, days, hours, minutes, seconds
// e.g. "1d 1:01:01" means 1 day, 1 hour, 1 minute and 1 second
std::string display_time(unsigned seconds);

////////////////////////////////////////////////////////////////////////////////
#include "string_utilities.tpp"
#endif
