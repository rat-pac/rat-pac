/*------------------------------------------------------------------------------

  Author:    Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License, see ../docs/license.html

------------------------------------------------------------------------------*/
#include "string_utilities.hpp"
#include "debug.hpp"
#include "dprintf.hpp"
#include "textio.hpp"
#include <ctype.h>
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////
// character mappings
// Note: this has been copied and modified for the inf class - so any changes here must be made there too

char to_char [] = "0123456789abcdefghijklmnopqrstuvwxyz";
int from_char [] = 
{
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
  -1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
  25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, -1, -1, -1, -1, -1,
  -1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
  25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

////////////////////////////////////////////////////////////////////////////////
// Conversions to string
// Local generic routines
// Note: this has been copied and modified for the inf class - so any changes here must be made there too

// signed version of the generic image generation function for all integer types
template<typename T>
static std::string simage (T i, unsigned radix, radix_display_t display, unsigned width)
  throw(std::invalid_argument)
{
  if (radix < 2 || radix > 36)
    throw std::invalid_argument("invalid radix value " + to_string(radix));
  // untangle all the options
  bool hashed = false;
  bool binary = false;
  bool octal = false;
  bool hex = false;
  switch(display)
  {
  case radix_none:
    break;
  case radix_hash_style:
    hashed = radix != 10;
    break;
  case radix_hash_style_all:
    hashed = true;
    break;
  case radix_c_style:
    if (radix == 16)
      hex = true;
    else if (radix == 8)
      octal = true;
    else if (radix == 2)
      binary = true;
    break;
  case radix_c_style_or_hash:
    if (radix == 16)
      hex = true;
    else if (radix == 8)
      octal = true;
    else if (radix == 2)
      binary = true;
    else if (radix != 10)
      hashed = true;
    break;
  default:
    throw std::invalid_argument("invalid radix display value");
  }
  // create constants of the same type as the template parameter to avoid type mismatches
  const T t_zero(0);
  const T t_radix(radix);
  // the C representations for binary, octal and hex use 2's-complement representation
  // all other represenations use sign-magnitude
  std::string result;
  if (hex || octal || binary)
  {
    // bit-pattern representation
    // this is the binary representation optionally shown in octal or hex
    // first generate the binary by masking the bits
    // ensure that it has at least one bit!
    for (T mask(1); ; mask <<= 1)
    {
      result.insert((std::string::size_type)0, 1, i & mask ? '1' : '0');
      if (mask == t_zero) break;
    }
    // the result is now the full width of the type - e.g. int will give a 32-bit result
    // now interpret this as either binary, octal or hex and add the prefix
    if (binary)
    {
      // the result is already binary - but the width may be wrong
      // if this is still smaller than the width field, sign extend
      // otherwise trim down to either the width or the smallest string that preserves the value
      while (result.size() < width)
        result.insert((std::string::size_type)0, 1, result[0]);
      while (result.size() > width)
      {
        // do not trim to less than 2 bits (sign plus 1-bit magnitude)
        if (result.size() <= 2) break;
        // only trim if it doesn't change the sign and therefore the value
        if (result[0] != result[1]) break;
        result.erase(0,1);
      }
      // add the prefix
      result.insert((std::string::size_type)0, "0b");
    }
    else if (octal)
    {
      // the result is currently binary - but before converting get the width right
      // the width is expressed in octal digits so make the binary 3 times this
      // if this is still smaller than the width field, sign extend
      // otherwise trim down to either the width or the smallest string that preserves the value
      // also ensure that the binary is a multiple of 3 bits to make the conversion to octal easier
      while (result.size() < 3*width)
        result.insert((std::string::size_type)0, 1, result[0]);
      while (result.size() > 3*width)
      {
        // do not trim to less than 2 bits (sign plus 1-bit magnitude)
        if (result.size() <= 2) break;
        // only trim if it doesn't change the sign and therefore the value
        if (result[0] != result[1]) break;
        result.erase(0,1);
      }
      while (result.size() % 3 != 0)
        result.insert((std::string::size_type)0, 1, result[0]);
      // now convert to octal
      std::string octal_result;
      for (unsigned ii = 0; ii < result.size()/3; ii++)
      {
        std::string slice = result.substr(ii*3, 3);
        unsigned value = to_uint(slice, 2);
        octal_result += to_char[value];
      }
      result = octal_result;
      // add the prefix
      result.insert((std::string::size_type)0, "0");
    }
    else
    {
      // hex - similar to octal
      while (result.size() < 4*width)
        result.insert((std::string::size_type)0, 1, result[0]);
      while (result.size() > 4*width)
      {
        // do not trim to less than 2 bits (sign plus 1-bit magnitude)
        if (result.size() <= 2) break;
        // only trim if it doesn't change the sign and therefore the value
        if (result[0] != result[1]) break;
        result.erase(0,1);
      }
      while (result.size() % 4 != 0)
        result.insert((std::string::size_type)0, 1, result[0]);
      // now convert to hex
      std::string hex_result;
      for (unsigned ii = 0; ii < result.size()/4; ii++)
      {
        std::string slice = result.substr(ii*4, 4);
        unsigned value = to_uint(slice, 2);
        hex_result += to_char[value];
      }
      result = hex_result;
      // add the prefix
      result.insert((std::string::size_type)0, "0x");
    }
  }
  else
  {
    // convert to sign-magnitude
    // the representation is:
    // [radix#][sign]magnitude
    bool negative = i < t_zero;
//    if (negative) i = -i;
    // create a representation of the magnitude by successive division
    do
    {
      T ch = abs(i % t_radix);
      i /= t_radix;
      result.insert((std::string::size_type)0, 1, to_char[ch]);
    }
    while(i != t_zero || result.size() < width);
    // add the prefixes
    // add a sign only for negative values
    if (negative)
      result.insert((std::string::size_type)0, 1, '-');
    // then prefix everything with the radix if the hashed representation was requested
    if (hashed)
      result.insert((std::string::size_type)0, to_string(radix) + "#");
  }
  return result;
}

// unsigned version
template<typename T>
static std::string uimage (T i, unsigned radix, radix_display_t display, unsigned width)
  throw(std::invalid_argument)
{
  if (radix < 2 || radix > 36)
    throw std::invalid_argument("invalid radix value " + to_string(radix));
  // untangle all the options
  bool hashed = false;
  bool binary = false;
  bool octal = false;
  bool hex = false;
  switch(display)
  {
  case radix_none:
    break;
  case radix_hash_style:
    hashed = radix != 10;
    break;
  case radix_hash_style_all:
    hashed = true;
    break;
  case radix_c_style:
    if (radix == 16)
      hex = true;
    else if (radix == 8)
      octal = true;
    else if (radix == 2)
      binary = true;
    break;
  case radix_c_style_or_hash:
    if (radix == 16)
      hex = true;
    else if (radix == 8)
      octal = true;
    else if (radix == 2)
      binary = true;
    else if (radix != 10)
      hashed = true;
    break;
  default:
    throw std::invalid_argument("invalid radix display value");
  }
  // create constants of the same type as the template parameter to avoid type mismatches
  const T t_zero(0);
  const T t_radix(radix);
  // the C representations for binary, octal and hex use 2's-complement representation
  // all other represenations use sign-magnitude
  std::string result;
  if (hex || octal || binary)
  {
    // bit-pattern representation
    // this is the binary representation optionally shown in octal or hex
    // first generate the binary by masking the bits
    // ensure at least one bit
    for (T mask(1); ; mask <<= 1)
    {
      result.insert((std::string::size_type)0, 1, i & mask ? '1' : '0');
      if (mask == t_zero) break;
    }
    // the result is now the full width of the type - e.g. int will give a 32-bit result
    // now interpret this as either binary, octal or hex and add the prefix
    if (binary)
    {
      // the result is already binary - but the width may be wrong
      // if this is still smaller than the width field, zero extend
      // otherwise trim down to either the width or the smallest string that preserves the value
      while (result.size() < width)
        result.insert((std::string::size_type)0, 1, '0');
      while (result.size() > width)
      {
        // do not trim to less than 1 bit (1-bit magnitude)
        if (result.size() <= 1) break;
        // only trim if it doesn't change the sign and therefore the value
        if (result[0] != '0') break;
        result.erase(0,1);
      }
      // add the prefix
      result.insert((std::string::size_type)0, "0b");
    }
    else if (octal)
    {
      // the result is currently binary - but before converting get the width right
      // the width is expressed in octal digits so make the binary 3 times this
      // if this is still smaller than the width field, sign extend
      // otherwise trim down to either the width or the smallest string that preserves the value
      // also ensure that the binary is a multiple of 3 bits to make the conversion to octal easier
      while (result.size() < 3*width)
        result.insert((std::string::size_type)0, 1, '0');
      while (result.size() > 3*width)
      {
        // do not trim to less than 1 bit (1-bit magnitude)
        if (result.size() <= 1) break;
        // only trim if it doesn't change the sign and therefore the value
        if (result[0] != '0') break;
        result.erase(0,1);
      }
      while (result.size() % 3 != 0)
        result.insert((std::string::size_type)0, 1, '0');
      // now convert to octal
      std::string octal_result;
      for (unsigned ii = 0; ii < result.size()/3; ii++)
      {
        std::string slice = result.substr(ii*3, 3);
        unsigned value = to_uint(slice, 2);
        octal_result += to_char[value];
      }
      result = octal_result;
      // add the prefix if the leading digit is not already 0
      if (result.empty() || result[0] != '0') result.insert((std::string::size_type)0, "0");
    }
    else
    {
      // similar to octal
      while (result.size() < 4*width)
        result.insert((std::string::size_type)0, 1, '0');
      while (result.size() > 4*width)
      {
        // do not trim to less than 1 bit (1-bit magnitude)
        if (result.size() <= 1) break;
        // only trim if it doesn't change the sign and therefore the value
        if (result[0] != '0') break;
        result.erase(0,1);
      }
      while (result.size() % 4 != 0)
        result.insert((std::string::size_type)0, 1, '0');
      // now convert to hex
      std::string hex_result;
      for (unsigned ii = 0; ii < result.size()/4; ii++)
      {
        std::string slice = result.substr(ii*4, 4);
        unsigned value = to_uint(slice, 2);
        hex_result += to_char[value];
      }
      result = hex_result;
      // add the prefix
      result.insert((std::string::size_type)0, "0x");
    }
  }
  else
  {
    // convert to sign-magnitude
    // the representation is:
    // [radix#]magnitude
    // create a representation of the magnitude by successive division
    do
    {
      T ch = i % t_radix;
      i /= t_radix;
      result.insert((std::string::size_type)0, 1, to_char[(int)ch]);
    }
    while(i != t_zero || result.size() < width);
    // prefix everything with the radix if the hashed representation was requested
    if (hashed)
      result.insert((std::string::size_type)0, to_string(radix) + "#");
  }
  return result;
}

////////////////////////////////////////////////////////////////////////////////
// exported conversions to string

// Integer types

std::string to_string(bool i, unsigned radix, radix_display_t display, unsigned width)
  throw(std::invalid_argument)
{
  // use the char representation for bool
  return uimage<char>(i, radix, display, width);
}

std::string to_string(short i, unsigned radix, radix_display_t display, unsigned width)
  throw(std::invalid_argument)
{
  return simage(i, radix, display, width);
}

std::string to_string(unsigned short i, unsigned radix, radix_display_t display, unsigned width)
  throw(std::invalid_argument)
{
  return uimage(i, radix, display, width);
}

std::string to_string(int i, unsigned radix, radix_display_t display, unsigned width)
  throw(std::invalid_argument)
{
  return simage(i, radix, display, width);
}

std::string to_string(unsigned i, unsigned radix, radix_display_t display, unsigned width)
  throw(std::invalid_argument)
{
  return uimage(i, radix, display, width);
}

std::string to_string(long i, unsigned radix, radix_display_t display, unsigned width)
  throw(std::invalid_argument)
{
  return simage(i, radix, display, width);
}

std::string to_string(unsigned long i, unsigned radix, radix_display_t display, unsigned width)
  throw(std::invalid_argument)
{
  return uimage(i, radix, display, width);
}

std::string to_string(const void* i, unsigned radix, radix_display_t display, unsigned width)
  throw(std::invalid_argument)
{
  // use the unsigned representation for pointers
  return uimage((unsigned long)i, radix, display, width);
}

// floating-point types

std::string to_string(float f, real_display_t display, unsigned width, unsigned precision)
  throw(std::invalid_argument)
{
  std::string format;
  switch(display)
  {
  case display_fixed:
    format = "%*.*f";
    break;
  case display_floating:
    format = "%*.*e";
    break;
  case display_mixed:
    format = "%*.*g";
    break;
  default:
    throw std::invalid_argument("invalid radix display value");
  }
  return dformat(format.c_str(), width, precision, f);
}

std::string to_string(double f, real_display_t display, unsigned width, unsigned precision)
  throw(std::invalid_argument)
{
  std::string format;
  switch(display)
  {
  case display_fixed:
    format = "%*.*f";
    break;
  case display_floating:
    format = "%*.*e";
    break;
  case display_mixed:
    format = "%*.*g";
    break;
  default:
    throw std::invalid_argument("invalid radix display value");
  }
  return dformat(format.c_str(), width, precision, f);
}

// strings

std::string to_string(const char* value)
{
  return std::string(value);
}

std::string to_string(const std::string& value)
{
  return value;
}

////////////////////////////////////////////////////////////////////////////////
// Print functions

static unsigned _indent_step = 2;

void set_indent_step(unsigned step)
{
  _indent_step = step;
}

unsigned indent_step(void)
{
  return _indent_step;
}

otext& print_indent(otext& str, unsigned indent)
{
  for (unsigned i = 0; i < indent; i++)
    for (unsigned j = 0; j < _indent_step; j++)
      str << ' ';
  return str;
}

otext& print(otext& str, const bool& value, unsigned radix, radix_display_t display, unsigned width)
  throw(std::invalid_argument)
{
  return str << to_string(value,radix,display,width);
}

otext& print(otext& str, const short& value, unsigned radix, radix_display_t display, unsigned width)
  throw(std::invalid_argument)
{
  return str << to_string(value,radix,display,width);
}

otext& print(otext& str, const unsigned short& value, unsigned radix, radix_display_t display, unsigned width)
  throw(std::invalid_argument)
{
  return str << to_string(value,radix,display,width);
}

otext& print(otext& str, const int& value, unsigned radix, radix_display_t display, unsigned width)
  throw(std::invalid_argument)
{
  return str << to_string(value,radix,display,width);
}

otext& print(otext& str, const unsigned int& value, unsigned radix, radix_display_t display, unsigned width)
  throw(std::invalid_argument)
{
  return str << to_string(value,radix,display,width);
}

otext& print(otext& str, const long& value, unsigned radix, radix_display_t display, unsigned width)
  throw(std::invalid_argument)
{
  return str << to_string(value,radix,display,width);
}

otext& print(otext& str, const unsigned long& value, unsigned radix, radix_display_t display, unsigned width)
  throw(std::invalid_argument)
{
  return str << to_string(value,radix,display,width);
}

otext& print(otext& str, const void*& value, unsigned radix, radix_display_t display, unsigned width)
  throw(std::invalid_argument)
{
  return str << to_string(value,radix,display,width);
}

// floating-point types

otext& print(otext& str, float value, real_display_t display, unsigned width, unsigned precision)
  throw(std::invalid_argument)
{
  return str << to_string(value,display,width,precision);
}

otext& print(otext& str, double value, real_display_t display, unsigned width, unsigned precision)
  throw(std::invalid_argument)
{
  return str << to_string(value,display,width,precision);
}

// strings

otext& print(otext& str, const std::string& value)
{
  return str << value;
}

otext& print(otext& str, const std::string& value, unsigned indent)
{
  print_indent(str, indent);
  print(str, value);
  return str << endl;
}

////////////////////////////////////////////////////////////////////////////////
// Conversions FROM string
// local template function
// Note: this has been copied and modified for the inf class - so any changes here must be made there too

// signed version
template<typename T>
static T svalue(const std::string& str, unsigned radix)
  throw(std::invalid_argument)
{
  if (radix != 0 && (radix < 2 || radix > 36))
    throw std::invalid_argument("invalid radix value " + to_string(radix));
  std::string::size_type i = 0;
  // the radix passed as a parameter is just the default - it can be overridden by either the C prefix or the hash prefix
  // Note: a leading zero is the C-style prefix for octal - I only make this override the default when the default prefix is not specified
  // first check for a C-style prefix
  bool c_style = false;
  if (i < str.size() && str[i] == '0')
  {
    // binary or hex
    if (i+1 < str.size() && tolower(str[i+1]) == 'x')
    {
      radix = 16;
      i += 2;
      c_style = true;
    }
    else if (i+1 < str.size() && tolower(str[i+1]) == 'b')
    {
      radix = 2;
      i += 2;
      c_style = true;
    }
    else if (radix == 0)
    {
      radix = 8;
      i += 1;
      c_style = true;
    }
  }
  // now check for a hash-style prefix if a C-style prefix was not found
  if (i == 0)
  {
    // scan for the sequence {digits}#
    bool hash_found = false;
    std::string::size_type j = i;
    for (; j < str.size(); j++)
    {
      if (!isdigit(str[j]))
      {
        if (str[j] == '#')
          hash_found = true;
        break;
      }
    }
    if (hash_found)
    {
      // use the hash prefix to define the radix
      // i points to the start of the radix and j points to the # character
      std::string slice = str.substr(i, j-i);
      radix = to_uint(slice);
      i = j+1;
    }
  }
  if (radix == 0)
    radix = 10;
  if (radix < 2 || radix > 36)
    throw std::invalid_argument("invalid radix value " + to_string(radix));
  T val(0);
  if (c_style)
  {
    // the C style formats are bit patterns not integer values - these need to be sign-extended to get the right value
    std::string binary;
    DEBUG_ASSERT(radix == 2 || radix == 8 || radix == 16);
    if (radix == 2)
    {
      for (std::string::size_type j = i; j < str.size(); j++)
      {
        switch(str[j])
        {
        case '0':
          binary += '0';
          break;
        case '1':
          binary += '1';
          break;
        default:
          throw std::invalid_argument("invalid character in string " + str + " for radix " + to_string(radix));
          break;
        }
      }
    }
    else if (radix == 8)
    {
      for (std::string::size_type j = i; j < str.size(); j++)
      {
        switch(str[j])
        {
        case '0':
          binary += "000";
          break;
        case '1':
          binary += "001";
          break;
        case '2':
          binary += "010";
          break;
        case '3':
          binary += "011";
          break;
        case '4':
          binary += "100";
          break;
        case '5':
          binary += "101";
          break;
        case '6':
          binary += "110";
          break;
        case '7':
          binary += "111";
          break;
        default:
          throw std::invalid_argument("invalid character in string " + str + " for radix " + to_string(radix));
          break;
        }
      }
    }
    else
    {
      for (std::string::size_type j = i; j < str.size(); j++)
      {
        switch(tolower(str[j]))
        {
        case '0':
          binary += "0000";
          break;
        case '1':
          binary += "0001";
          break;
        case '2':
          binary += "0010";
          break;
        case '3':
          binary += "0011";
          break;
        case '4':
          binary += "0100";
          break;
        case '5':
          binary += "0101";
          break;
        case '6':
          binary += "0110";
          break;
        case '7':
          binary += "0111";
          break;
        case '8':
          binary += "1000";
          break;
        case '9':
          binary += "1001";
          break;
        case 'a':
          binary += "1010";
          break;
        case 'b':
          binary += "1011";
          break;
        case 'c':
          binary += "1100";
          break;
        case 'd':
          binary += "1101";
          break;
        case 'e':
          binary += "1110";
          break;
        case 'f':
          binary += "1111";
          break;
        default:
          throw std::invalid_argument("invalid character in string " + str + " for radix " + to_string(radix));
          break;
        }
      }
    }
    // now sign-extend to the right number of bits for the type
    while (binary.size() < sizeof(T)*8)
      binary.insert((std::string::size_type)0, 1, binary.empty() ? '0' : binary[0]);
    // now convert the value
    for (std::string::size_type j = 0; j < binary.size(); j++)
    {
      val *= 2;
      int ch = from_char[(unsigned char)binary[j]] ;
      DEBUG_ASSERT(ch != -1);
      val += T(ch);
    }
  }
  else
  {
    // now scan for a sign and find whether this is a negative number
    bool negative = false;
    if (i < str.size())
    {
      switch (str[i])
      {
      case '-':
        negative = true;
        i++;
        break;
      case '+':
        i++;
        break;
      }
    }
    for (; i < str.size(); i++)
    {
      val *= T(radix);
      int ch = from_char[(unsigned char)str[i]] ;
      if (ch == -1 || (unsigned)ch >= radix)
      {
        throw std::invalid_argument("invalid character in string " + str + " for radix " + to_string(radix));
      }
      val += T(ch);
    }
    if (negative)
      val = -val;
  }
  return val;
}

// unsigned version
template<typename T>
static T uvalue(const std::string& str, unsigned radix)
  throw(std::invalid_argument)
{
  if (radix != 0 && (radix < 2 || radix > 36))
    throw std::invalid_argument("invalid radix value " + to_string(radix));
  unsigned i = 0;
  // the radix passed as a parameter is just the default - it can be overridden by either the C prefix or the hash prefix
  // Note: a leading zero is the C-style prefix for octal - I only make this override the default when the default prefix is not specified
  // first check for a C-style prefix
  bool c_style = false;
  if (i < str.size() && str[i] == '0')
  {
    // binary or hex
    if (i+1 < str.size() && tolower(str[i+1]) == 'x')
    {
      radix = 16;
      i += 2;
      c_style = true;
    }
    else if (i+1 < str.size() && tolower(str[i+1]) == 'b')
    {
      radix = 2;
      i += 2;
      c_style = true;
    }
    else if (radix == 0)
    {
      radix = 8;
      i += 1;
      c_style = true;
    }
  }
  // now check for a hash-style prefix if a C-style prefix was not found
  if (i == 0)
  {
    // scan for the sequence {digits}#
    bool hash_found = false;
    unsigned j = i;
    for (; j < str.size(); j++)
    {
      if (!isdigit(str[j]))
      {
        if (str[j] == '#')
          hash_found = true;
        break;
      }
    }
    if (hash_found)
    {
      // use the hash prefix to define the radix
      // i points to the start of the radix and j points to the # character
      std::string slice = str.substr(i, j-i);
      radix = to_uint(slice);
      i = j+1;
    }
  }
  if (radix == 0)
    radix = 10;
  if (radix < 2 || radix > 36)
    throw std::invalid_argument("invalid radix value " + to_string(radix));
  T val(0);
  if (c_style)
  {
    // the C style formats are bit patterns not integer values - these need to be sign-extended to get the right value
    std::string binary;
    DEBUG_ASSERT(radix == 2 || radix == 8 || radix == 16);
    if (radix == 2)
    {
      for (unsigned j = i; j < str.size(); j++)
      {
        switch(str[j])
        {
        case '0':
          binary += '0';
          break;
        case '1':
          binary += '1';
          break;
        default:
          throw std::invalid_argument("invalid character in string " + str + " for radix " + to_string(radix));
          break;
        }
      }
    }
    else if (radix == 8)
    {
      for (unsigned j = i; j < str.size(); j++)
      {
        switch(str[j])
        {
        case '0':
          binary += "000";
          break;
        case '1':
          binary += "001";
          break;
        case '2':
          binary += "010";
          break;
        case '3':
          binary += "011";
          break;
        case '4':
          binary += "100";
          break;
        case '5':
          binary += "101";
          break;
        case '6':
          binary += "110";
          break;
        case '7':
          binary += "111";
          break;
        default:
          throw std::invalid_argument("invalid character in string " + str + " for radix " + to_string(radix));
          break;
        }
      }
    }
    else
    {
      for (unsigned j = i; j < str.size(); j++)
      {
        switch(tolower(str[j]))
        {
        case '0':
          binary += "0000";
          break;
        case '1':
          binary += "0001";
          break;
        case '2':
          binary += "0010";
          break;
        case '3':
          binary += "0011";
          break;
        case '4':
          binary += "0100";
          break;
        case '5':
          binary += "0101";
          break;
        case '6':
          binary += "0110";
          break;
        case '7':
          binary += "0111";
          break;
        case '8':
          binary += "1000";
          break;
        case '9':
          binary += "1001";
          break;
        case 'a':
          binary += "1010";
          break;
        case 'b':
          binary += "1011";
          break;
        case 'c':
          binary += "1100";
          break;
        case 'd':
          binary += "1101";
          break;
        case 'e':
          binary += "1110";
          break;
        case 'f':
          binary += "1111";
          break;
        default:
          throw std::invalid_argument("invalid character in string " + str + " for radix " + to_string(radix));
          break;
        }
      }
    }
    // now zero-extend to the right number of bits for the type
    while (binary.size() < sizeof(T)*8)
      binary.insert((std::string::size_type)0, 1, '0');
    // now convert the value
    for (unsigned j = 0; j < binary.size(); j++)
    {
      val *= 2;
      int ch = from_char[(unsigned char)binary[j]] ;
      DEBUG_ASSERT(ch != -1);
      val += T(ch);
    }
  }
  else
  {
    // now scan for a sign and find whether this is a negative number
    if (i < str.size())
    {
      switch (str[i])
      {
      case '-':
        throw std::invalid_argument("invalid sign character in string " + str + " for unsigned value");
        i++;
        break;
      case '+':
        i++;
        break;
      }
    }
    for (; i < str.size(); i++)
    {
      val *= T(radix);
      int ch = from_char[(unsigned char)str[i]] ;
      if (ch == -1 || (unsigned)ch >= radix)
      {
        throw std::invalid_argument("invalid character in string " + str + " for radix " + to_string(radix));
      }
      val += T(ch);
    }
  }
  return val;
}

////////////////////////////////////////////////////////////////////////////////
// exported functions

bool to_bool(const std::string& str, unsigned radix)
  throw(std::invalid_argument)
{
  return uvalue<unsigned char>(str, radix) != 0;
}

short to_short(const std::string& str, unsigned radix)
  throw(std::invalid_argument)
{
  return svalue<short>(str, radix);
}

unsigned short to_ushort(const std::string& str, unsigned radix)
  throw(std::invalid_argument)
{
  return uvalue<unsigned short>(str, radix);
}

int to_int(const std::string& str, unsigned radix)
  throw(std::invalid_argument)
{
  return svalue<int>(str, radix);
}

unsigned int to_uint(const std::string& str, unsigned radix)
  throw(std::invalid_argument)
{
  return uvalue<unsigned>(str, radix);
}

long to_long(const std::string& str, unsigned radix)
  throw(std::invalid_argument)
{
  return svalue<long>(str, radix);
}

unsigned long to_ulong(const std::string& str, unsigned radix)
  throw(std::invalid_argument)
{
  return uvalue<unsigned long>(str, radix);
}

void* to_void_star(const std::string& str, unsigned radix)
  throw(std::invalid_argument)
{
  return (void*)uvalue<unsigned long>(str, radix);
}

////////////////////////////////////////////////////////////////////////////////

float to_float(const std::string& value)
  throw(std::invalid_argument)
{
  return (float)to_double(value);
}

double to_double(const std::string& value)
  throw(std::invalid_argument)
{
  // TODO - error checking
  return strtod(value.c_str(), 0);
}

////////////////////////////////////////////////////////////////////////////////

std::string pad(const std::string& str, alignment_t alignment, unsigned width, char padch)
  throw(std::invalid_argument)
{
  std::string result = str;
  switch(alignment)
  {
  case align_left:
  {
    unsigned padding = width>str.size() ? width - str.size() : 0;
    unsigned i = 0;
    while (i++ < padding)
      result.insert(result.end(), padch);
    break;
  }
  case align_right:
  {
    unsigned padding = width>str.size() ? width - str.size() : 0;
    unsigned i = 0;
    while (i++ < padding)
      result.insert(result.begin(), padch);
    break;
  }
  case align_centre:
  {
    unsigned padding = width>str.size() ? width - str.size() : 0;
    unsigned i = 0;
    while (i++ < padding/2)
      result.insert(result.end(), padch);
    i--;
    while (i++ < padding)
      result.insert(result.begin(), padch);
    break;
  }
  default:
    throw std::invalid_argument("invalid alignment value");
  }
  return result;
}

std::string trim_left(const std::string& val)
{
  std::string result = val;
  while (!result.empty() && isspace(result[0]))
    result.erase(result.begin());
  return result;
}

std::string trim_right(const std::string& val)
{
  std::string result = val;
  while (!result.empty() && isspace(result[result.size()-1]))
    result.erase(result.end()-1);
  return result;
}

std::string trim(const std::string& val)
{
  std::string result = val;
  while (!result.empty() && isspace(result[0]))
    result.erase(result.begin());
  while (!result.empty() && isspace(result[result.size()-1]))
    result.erase(result.end()-1);
  return result;
}

std::string lowercase(const std::string& val)
{
  std::string text = val;
  for (unsigned i = 0; i < text.size(); i++)
    text[i] = tolower(text[i]);
  return text;
}

std::string uppercase(const std::string& val)
{
  std::string text = val;
  for (unsigned i = 0; i < text.size(); i++)
    text[i] = toupper(text[i]);
  return text;
}

std::string translate(const std::string& input, const std::string& from_set, const std::string& to_set)
{
  std::string result;
  for (unsigned i = 0; i < input.size(); i++)
  {
    char ch = input[i];
    // check to see if the character is in the from set
    std::string::size_type found = from_set.find(ch);
    if (found == std::string::npos)
    {
      // not found so just copy across
      result += ch;
    }
    else if (found < to_set.size())
    {
      // found and in range so translate
      result += to_set[found];
    }
  }
  return result;
}

////////////////////////////////////////////////////////////////////////////////
// WARNING: new wheel invention follows
// Given that all shells perform wildcard matching, why don't the library writers put it in the C run-time????????
// The problem:
//   *  matches any number of characters - this is achieved by matching 1 and seeing if the remainder matches
//      if not, try 2 characters and see if the remainder matches etc.
//      this must be recursive, not iterative, so that multiple *s can appear in the same wildcard expression
//   ?  matches exactly one character so doesn't need the what-if approach
//   \  escapes special characters such as *, ? and [
//   [] matches exactly one character in the set - the difficulty is the set can contain ranges, e.g [a-zA-Z0-9]
//      a set cannot be empty and the ] character can be included by making it the first character

// function for testing whether a character matches a set
// I can't remember the exact rules and I have no definitive references but:
// a set contains characters, escaped characters (I think) and ranges in the form a-z
// The character '-' can only appear at the start of the set where it is not interpreted as a range
// This is a horrible mess - blame the Unix folks for making a hash of wildcards
// first expand any ranges and remove escape characters to make life more palatable

static bool match_set (const std::string& set, char match)
{
  std::string simple_set;
  for (std::string::const_iterator i = set.begin(); i != set.end(); ++i)
  {
    switch(*i)
    {
    case '-':
    {
      if (i == set.begin())
      {
	simple_set += *i;
      }
      else if (i+1 == set.end())
      {
	return false;
      }
      else
      {
	// found a set. The first character is already in the result, so first remove it (the set might be empty)
	simple_set.erase(simple_set.end()-1);
	char last = *++i;
	for (char ch = *(i-2); ch <= last; ch++)
	{
	  simple_set += ch;
	}
      }
      break;
    }
    case '\\':
      if (i+1 == set.end()) {return false;}
      simple_set += *++i;
      break;
    default:
      simple_set += *i;
      break;
    }
  }
  std::string::size_type result = simple_set.find(match);
  return result != std::string::npos;
}

// the recursive bit - basically whenever a * is found you recursively call this for each candidate substring match
// until either it succeeds or you run out of string to match
// for each * in the wildcard another level of recursion is created

static bool match_remainder (const std::string& wild, std::string::const_iterator wildi, const std::string& match, std::string::const_iterator matchi)
{
  //cerr << "match_remainder called at " << *matchi << " with wildcard " << *wildi << endl;
  while (wildi != wild.end() && matchi != match.end())
  {
    //cerr << "trying to match " << *matchi << " with wildcard " << *wildi << endl;
    switch(*wildi)
    {
    case '*':
    {
      ++wildi;
      ++matchi;
      for (std::string::const_iterator i = matchi; i != match.end(); ++i)
      {
	// deal with * at the end of the wildcard - there is no remainder then
	if (wildi == wild.end())
	{
	  if (i == match.end()-1)
	    return true;
	}
	else if (match_remainder(wild, wildi, match, i))
	{
	  return true;
	}
      }
      return false;
    }
    case '[':
    {
      // scan for the end of the set using a similar method for avoiding escaped characters
      bool found = false;
      std::string::const_iterator end = wildi + 1;
      for (; !found && end != wild.end(); ++end)
      {
	switch(*end)
	{
	case ']':
	{
	  // found the set, now match with its contents excluding the brackets
	  if (!match_set(wild.substr(wildi - wild.begin() + 1, end - wildi - 1), *matchi))
	    return false;
	  found = true;
	  break;
	}
	case '\\':
	  if (end == wild.end()-1)
	    return false;
	  ++end;
	  break;
	default:
	  break;
	}
      }
      if (!found)
	return false;
      ++matchi;
      wildi = end;
      break;
    }
    case '?':
      ++wildi;
      ++matchi;
      break;
    case '\\':
      if (wildi == wild.end()-1)
	return false;
      ++wildi;
      if (*wildi != *matchi)
	return false;
      ++wildi;
      ++matchi;
      break;
    default:
      if (*wildi != *matchi)
	return false;
      ++wildi;
      ++matchi;
      break;
    }
  }
  bool result = wildi == wild.end() && matchi == match.end();
  return result;
}

// like all recursions the exported function has a simpler interface than the recursive function and is just a 'seed' to
// the recursion itself

bool match_wildcard(const std::string& wild, const std::string& match)
{
  return match_remainder(wild, wild.begin(), match, match.begin());
}

////////////////////////////////////////////////////////////////////////////////

std::vector<std::string> split(const std::string& str, const std::string& splitter)
{
  std::vector<std::string> result;
  if (!str.empty())
  {
    for(std::string::size_type offset = 0;;)
    {
      std::string::size_type found = str.find(splitter, offset);
      if (found != std::string::npos)
      {
        result.push_back(str.substr(offset, found-offset));
        offset = found + splitter.size();
      }
      else
      {
        result.push_back(str.substr(offset, str.size()-offset));
        break;
      }
    }
  }
  return result;
}

std::string join (const std::vector<std::string>& str, const std::string& joiner, const std::string& prefix, const std::string& suffix)
{
  std::string result = prefix;
  for (unsigned i = 0; i < str.size(); i++)
  {
    if (i) result += joiner;
    result += str[i];
  }
  result += suffix;
  return result;
}

////////////////////////////////////////////////////////////////////////////////

std::string display_bytes(long bytes)
{
  std::string result;
  if (bytes < 0)
  {
    result += '-';
    bytes = -bytes;
  }
  static const long kB = 1024l;
  static const long MB = kB * kB;
  static const long GB = MB * kB;
  if (bytes < kB)
    result += dformat("%iB", bytes);
  else if (bytes < (10l * kB))
    result += dformat("%.2fkB", ((float)bytes / (float)kB));
  else if (bytes < (100l * kB))
    result += dformat("%.1fkB", ((float)bytes / (float)kB));
  else if (bytes < MB)
    result += dformat("%.0fkB", ((float)bytes / (float)kB));
  else if (bytes < (10l * MB))
    result += dformat("%.2fMB", ((float)bytes / (float)MB));
  else if (bytes < (100l * MB))
    result += dformat("%.1fMB", ((float)bytes / (float)MB));
  else if (bytes < GB)
    result += dformat("%.0fMB", ((float)bytes / (float)MB));
  else
    result += dformat("%.2fGB", ((float)bytes / (float)GB));
  return result;
}

std::string display_time(unsigned seconds)
{
  unsigned minutes = seconds / 60;
  seconds %= 60;
  unsigned hours = minutes / 60;
  minutes %= 60;
  unsigned days = hours / 24;
  hours %= 24;
  unsigned weeks = days / 7;
  days %= 7;
  std::string result;
  if (weeks > 0)
  {
    result += to_string(weeks, 10, radix_none, 1);
    result += "w ";
  }
  if (!result.empty() || days > 0)
  {
    result += to_string(days, 10, radix_none, 1);
    result += "d ";
  }
  if (!result.empty() || hours > 0)
  {
    result += to_string(hours, 10, radix_none, 1);
    result += ":";
  }
  if (!result.empty() || minutes > 0)
  {
    if (!result.empty())
      result += to_string(minutes, 10, radix_none, 2);
    else
      result += to_string(minutes, 10, radix_none, 1);
    result += ":";
  }
  if (!result.empty())
    result += to_string(seconds, 10, radix_none, 2);
  else
  {
    result += to_string(seconds, 10, radix_none, 1);
    result += "s";
  }
  return result;
}

////////////////////////////////////////////////////////////////////////////////
