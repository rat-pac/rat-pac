/*------------------------------------------------------------------------------

  Author:    Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License, see ../docs/license.html

  ------------------------------------------------------------------------------*/
#include "dprintf.hpp"
#include "debug.hpp"
#include "string_utilities.hpp"
#include <stdio.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>

/*--------------------------------------------------------------------------------

  I have chosen to partially re-invent the wheel here. This is because the obvious
  solution to the problem of in-memory formatted output is to use sprintf(), but
  this is a potentially dangerous operation since it will quite happily charge off
  the end of the string it is printing to and thereby corrupt memory. Building in
  potential bear-traps by using arbitrary-sized internal buffers is not part of
  any quality-orientated design philosophy. Simply buggering around with huge
  buffers is not a solution to this problem, it just pushes the problem into a
  different shape.

  However, sprintf() is acceptable if used in strictly controlled conditions that
  make overflow impossible. This is what I do here. I break the format string up
  to get the individual formatting codes for each argument and use sprintf() to
  format just the numeric substitutions. String substitutions are handled
  directly.

  Notes:

  serious problems apparently with unsigned short - getting the argument value doesn't work
  at least on SunOS4. Therefore I will pass this value as an unsigned int and see if that fixes it.

--------------------------------------------------------------------------------*/

static const int max_int_length = 20;  // allow for up to 64 bits;
static const int max_mantissa_length = (DBL_MAX_EXP*4/10);

////////////////////////////////////////////////////////////////////////////////

int vdprintf(std::string& formatted, const char* format, va_list args)
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
        element_format += to_string(field);
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
          element_format += to_string(precision);
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

int dprintf(std::string& formatted, const char* format, ...)
{
  va_list args;
  va_start(args, format);
  int result = vdprintf(formatted, format, args);
  va_end(args);
  return result;
}

std::string vdformat(const char* format, va_list args)
{
  std::string formatted;
  vdprintf(formatted, format, args);
  return formatted;
}

std::string dformat(const char* format, ...)
{
  std::string formatted;
  va_list args;
  va_start(args, format);
  vdprintf(formatted, format, args);
  va_end(args);
  return formatted;
}
