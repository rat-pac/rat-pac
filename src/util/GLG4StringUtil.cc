#include "GLG4StringUtil.hh"
#include <stdio.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>

/*------------------------------------------------------------------------------

  Author:    Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License

  ------------------------------------------------------------------------------*/

// Turn off debug asserts
#define DEBUG_ASSERT(test)


////////////////////////////////////////////////////////////////////////////////
// character mappings
// Note: this has been copied and modified for the inf class - so any changes here must be made there too

char util_to_char [] = "0123456789abcdefghijklmnopqrstuvwxyz";
int util_from_char [] = 
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
static std::string util_simage (T i, unsigned radix, radix_display_t display, unsigned width)
  throw(std::invalid_argument)
{
  if (radix < 2 || radix > 36)
    throw std::invalid_argument("invalid radix value " + util_to_string(radix));
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
        unsigned value = util_to_uint(slice, 2);
        octal_result += util_to_char[value];
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
        unsigned value = util_to_uint(slice, 2);
        hex_result += util_to_char[value];
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
      result.insert((std::string::size_type)0, 1, util_to_char[ch]);
    }
    while(i != t_zero || result.size() < width);
    // add the prefixes
    // add a sign only for negative values
    if (negative)
      result.insert((std::string::size_type)0, 1, '-');
    // then prefix everything with the radix if the hashed representation was requested
    if (hashed)
      result.insert((std::string::size_type)0, util_to_string(radix) + "#");
  }
  return result;
}

// unsigned version
template<typename T>
static std::string util_uimage (T i, unsigned radix, radix_display_t display, unsigned width)
  throw(std::invalid_argument)
{
  if (radix < 2 || radix > 36)
    throw std::invalid_argument("invalid radix value " + util_to_string(radix));
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
        unsigned value = util_to_uint(slice, 2);
        octal_result += util_to_char[value];
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
        unsigned value = util_to_uint(slice, 2);
        hex_result += util_to_char[value];
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
      result.insert((std::string::size_type)0, 1, util_to_char[(int)ch]);
    }
    while(i != t_zero || result.size() < width);
    // prefix everything with the radix if the hashed representation was requested
    if (hashed)
      result.insert((std::string::size_type)0, util_to_string(radix) + "#");
  }
  return result;
}

////////////////////////////////////////////////////////////////////////////////
// exported conversions to string

// Integer types

std::string util_to_string(bool i, unsigned radix, radix_display_t display, unsigned width)
  throw(std::invalid_argument)
{
  // use the char representation for bool
  return util_uimage<char>(i, radix, display, width);
}

std::string util_to_string(short i, unsigned radix, radix_display_t display, unsigned width)
  throw(std::invalid_argument)
{
  return util_simage(i, radix, display, width);
}

std::string util_to_string(unsigned short i, unsigned radix, radix_display_t display, unsigned width)
  throw(std::invalid_argument)
{
  return util_uimage(i, radix, display, width);
}

std::string util_to_string(int i, unsigned radix, radix_display_t display, unsigned width)
  throw(std::invalid_argument)
{
  return util_simage(i, radix, display, width);
}

std::string util_to_string(unsigned i, unsigned radix, radix_display_t display, unsigned width)
  throw(std::invalid_argument)
{
  return util_uimage(i, radix, display, width);
}

std::string util_to_string(long i, unsigned radix, radix_display_t display, unsigned width)
  throw(std::invalid_argument)
{
  return util_simage(i, radix, display, width);
}

std::string util_to_string(unsigned long i, unsigned radix, radix_display_t display, unsigned width)
  throw(std::invalid_argument)
{
  return util_uimage(i, radix, display, width);
}

// floating-point types

std::string util_to_string(float f, real_display_t display, unsigned width, unsigned precision)
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
  return util_dformat(format.c_str(), width, precision, f);
}

std::string util_to_string(double f, real_display_t display, unsigned width, unsigned precision)
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
  return util_dformat(format.c_str(), width, precision, f);
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
    throw std::invalid_argument("invalid radix value " + util_to_string(radix));
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
      radix = util_to_uint(slice);
      i = j+1;
    }
  }
  if (radix == 0)
    radix = 10;
  if (radix < 2 || radix > 36)
    throw std::invalid_argument("invalid radix value " + util_to_string(radix));
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
          throw std::invalid_argument("invalid character in string " + str + " for radix " + util_to_string(radix));
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
          throw std::invalid_argument("invalid character in string " + str + " for radix " + util_to_string(radix));
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
          throw std::invalid_argument("invalid character in string " + str + " for radix " + util_to_string(radix));
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
      int ch = util_from_char[(unsigned char)binary[j]] ;
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
      int ch = util_from_char[(unsigned char)str[i]] ;
      if (ch == -1 || (unsigned)ch >= radix)
      {
        throw std::invalid_argument("invalid character in string " + str + " for radix " + util_to_string(radix));
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
    throw std::invalid_argument("invalid radix value " + util_to_string(radix));
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
      radix = util_to_uint(slice);
      i = j+1;
    }
  }
  if (radix == 0)
    radix = 10;
  if (radix < 2 || radix > 36)
    throw std::invalid_argument("invalid radix value " + util_to_string(radix));
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
          throw std::invalid_argument("invalid character in string " + str + " for radix " + util_to_string(radix));
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
          throw std::invalid_argument("invalid character in string " + str + " for radix " + util_to_string(radix));
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
          throw std::invalid_argument("invalid character in string " + str + " for radix " + util_to_string(radix));
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
      int ch = util_from_char[(unsigned char)binary[j]] ;
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
      int ch = util_from_char[(unsigned char)str[i]] ;
      if (ch == -1 || (unsigned)ch >= radix)
      {
        throw std::invalid_argument("invalid character in string " + str + " for radix " + util_to_string(radix));
      }
      val += T(ch);
    }
  }
  return val;
}

////////////////////////////////////////////////////////////////////////////////
// exported functions

bool util_to_bool(const std::string& str, unsigned radix)
  throw(std::invalid_argument)
{
  return uvalue<unsigned char>(str, radix) != 0;
}

short util_to_short(const std::string& str, unsigned radix)
  throw(std::invalid_argument)
{
  return svalue<short>(str, radix);
}

unsigned short util_to_ushort(const std::string& str, unsigned radix)
  throw(std::invalid_argument)
{
  return uvalue<unsigned short>(str, radix);
}

int util_to_int(const std::string& str, unsigned radix)
  throw(std::invalid_argument)
{
  return svalue<int>(str, radix);
}

unsigned int util_to_uint(const std::string& str, unsigned radix)
  throw(std::invalid_argument)
{
  return uvalue<unsigned>(str, radix);
}

long util_to_long(const std::string& str, unsigned radix)
  throw(std::invalid_argument)
{
  return svalue<long>(str, radix);
}

unsigned long util_to_ulong(const std::string& str, unsigned radix)
  throw(std::invalid_argument)
{
  return uvalue<unsigned long>(str, radix);
}


////////////////////////////////////////////////////////////////////////////////

float util_to_float(const std::string& value)
  throw(std::invalid_argument)
{
  return (float)util_to_double(value);
}

double util_to_double(const std::string& value)
  throw(std::invalid_argument)
{
  // TODO - error checking
  return strtod(value.c_str(), 0);
}

////////////////////////////////////////////////////////////////////////////////

std::vector<std::string> util_split(const std::string& str, const std::string& splitter)
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

std::string util_join (const std::vector<std::string>& str, const std::string& joiner, const std::string& prefix, const std::string& suffix)
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

std::string util_strip(const std::string &s, const std::string &stripchars)
{
  std::string result = s;
  int i= result.find_first_not_of(stripchars);
  if (i < 0 || i >= (int)result.length()) {
    return std::string("");
  }
  result = result.substr(i);
  i = result.find_last_not_of(stripchars);
  if (i < 0 || i >= (int)result.length()) {
    return std::string("");
  }
  result.resize(i+1);
  return result;
}

std::string util_strip_default(const std::string &s)
{
  return util_strip(s, " \t\"");
}

////////////////////////////////////////////////////////////////////////////////


static const int max_int_length = 20;  // allow for up to 64 bits;
static const int max_mantissa_length = (DBL_MAX_EXP*4/10);


int util_vdprintf(std::string& formatted, const char* format, va_list args)
{
  int start_length = formatted.size();
  while (*format)
  {
    switch (*format)
    {
    case '\\':
    {
      format++;
      switch (*format)
      {
      case 'b': formatted += '\b'; format++; break;
      case 'f': formatted += '\f'; format++; break;
      case 'n': formatted += '\n'; format++; break;
      case 'r': formatted += '\r'; format++; break;
      case 't': formatted += '\t'; format++; break;
      case 'v': formatted += '\v'; format++; break;
      case '\\': formatted += '\\'; format++; break;
      case '\?': formatted += '\?'; format++; break;
      case '\'': formatted += '\''; format++; break;
      case '\"': formatted += '\"'; format++; break;
      default: break;
      }
      break;
    }
    case '%':
    {
      std::string element_format(1,*format++);
      bool left_justified = false;
      for(bool found = true; found && *format; )
      {
        switch (*format)
        {
        case '-': 
          left_justified = true;
        case '+':
        case ' ':
        case '0':
        case '#':
          element_format += *format++;
          break;
        default: 
          found = false;
          break;
        }
      }
      int field = 0;
      if (*format == '*')
      {
        format++;
        field = va_arg(args, int);
        element_format += util_to_string(field);
        if (field < 0)
        {
          left_justified = true;
          field = -field;
        }
      }
      else
      {
        while (isdigit(*format))
        {
          field *= 10;
          field +=(*format - '0');
          element_format += *format++;
        }
      }
      int precision = -1;
      if (*format == '.')
      {
        element_format += *format++;
        if (*format == '*')
        {
          format++;
          precision = va_arg(args, int);
          element_format += util_to_string(precision);
          if (precision < 0)
          {
            left_justified = true;
            precision = -precision;
          }
        }
        else
        {
          precision = 0;
          while (isdigit(*format))
          {
            precision *= 10;
            precision +=(*format - '0');
            element_format += *format++;
          }
        }
      }
      char modifier = '\0';
      switch (*format)
      {
      case 'h': case 'l': case 'L':
        modifier = *format++;
        element_format += modifier;
        break;
      default:
        break;
      }
      char conversion = *format;
      if (conversion) element_format += *format++;
      switch (conversion)
      {
      case 'd': case 'i':
      {
        int length = max_int_length;
        if (precision > length) length = precision;
        if (field > length) length = field;
        length += 2; // for possible prefix sign/0x etc;
        char* element_result = new char[length+1];
        switch (modifier)
        {
        case 'h':
        {
          short value =(short)va_arg(args, int);
          sprintf(element_result,element_format.c_str(), value);
        }
        break;
        case 'l':
        {
          long value = va_arg(args, long);
          sprintf(element_result,element_format.c_str(), value);
        }
        break;
        default:
        {
          int value = va_arg(args, int);
          sprintf(element_result,element_format.c_str(), value);
        }
        break;
        }
        formatted += element_result;
        delete[] element_result;
        break;
      }
      case 'u': case 'o': case 'X': case 'x':
      {
        int length = max_int_length;
        if (precision > length) length = precision;
        if (field > length) length = field;
        length += 2; // for possible prefix sign/0x etc;
        char* element_result = new char[length+1];
        switch (modifier)
        {
        case 'h':
        {
          unsigned /*short*/ value = va_arg(args, unsigned /*short*/);
          sprintf(element_result,element_format.c_str(), value);
        }
        break;
        case 'l':
        {
          unsigned long value = va_arg(args, unsigned long);
          sprintf(element_result,element_format.c_str(), value);
        }
        break;
        default:
        {
          unsigned int value = va_arg(args, unsigned int);
          sprintf(element_result,element_format.c_str(), value);
        }
        break;
        }
        formatted += element_result;
        delete[] element_result;
        break;
      }
      case 'p':
      {
        int length = max_int_length;
        if (precision > length) length = precision;
        if (field > length) length = field;
        length += 2; // for possible prefix sign/0x etc;
        char* element_result = new char[length+1];
        void* value = va_arg(args, void*);
        sprintf(element_result,element_format.c_str(), value);
        formatted += element_result;
        delete[] element_result;
        break;
      }
      case 'f':
      case 'E': case 'e':
      case 'G': case 'g':
      {
        if (precision == -1) precision = 6;
        int length = max_mantissa_length + precision;
        if (field > length) length = field;
        length += 2; // for punctuation;
        char* element_result = new char[length+1];
        if (modifier == 'L')
        {
          long double value = va_arg(args, long double);
          sprintf(element_result,element_format.c_str(), value);
        }
        else
        {
          double value = va_arg(args, double);
          sprintf(element_result,element_format.c_str(), value);
        }
        formatted += element_result;
        delete[] element_result;
        break;
      }
      case 'c':
      {
        char value =(char)va_arg(args, int);
        if (!left_justified) for(int i = 1; i < field; i++) formatted += ' ';
        formatted += value;
        if (left_justified) for(int i = 1; i < field; i++) formatted += ' ';
        break;
      }
      case 's':
      {
        char* value = va_arg(args, char*);
        int length = strlen(value);
        if (precision != -1 && length > precision) length = precision;
        if (!left_justified) for(int i = length; i < field; i++) formatted += ' ';
        for(int i = 0; i < length; i++)
          formatted += value[i];
        if (left_justified) for(int i = length; i < field; i++) formatted += ' ';
        break;
      }
      case 'n':
      {
        int* result = va_arg(args, int*);
        *result = formatted.size() - start_length;
        break;
      }
      case '%':
      default:
        formatted += conversion;
        break;
      }
      break;
    }
    default:
      formatted += *format++;
      break;
    }
  }
  return formatted.size() - start_length;
}

int util_dprintf(std::string& formatted, const char* format, ...)
{
  va_list args;
  va_start(args, format);
  int result = util_vdprintf(formatted, format, args);
  va_end(args);
  return result;
}

std::string util_vdformat(const char* format, va_list args)
{
  std::string formatted;
  util_vdprintf(formatted, format, args);
  return formatted;
}

std::string util_dformat(const char* format, ...)
{
  std::string formatted;
  va_list args;
  va_start(args, format);
  util_vdprintf(formatted, format, args);
  va_end(args);
  return formatted;
}

