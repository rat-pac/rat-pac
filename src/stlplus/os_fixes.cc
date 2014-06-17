/*------------------------------------------------------------------------------

  Author:    Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License, see ../docs/license.html

  ------------------------------------------------------------------------------*/
#include "os_fixes.hpp"
#include "debug.hpp"
#include "string_utilities.hpp"

#if defined(_MSC_VER)
#include "windows.h"
#endif

////////////////////////////////////////////////////////////////////////////////
// problems with missing functions
////////////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER)
unsigned sleep(unsigned seconds)
{
  Sleep(1000*seconds);
  // should return remaining time if interrupted - however Windoze Sleep cannot be interrupted
  return 0;
}
#endif

////////////////////////////////////////////////////////////////////////////////
// Function for establishing endian-ness
////////////////////////////////////////////////////////////////////////////////

bool little_endian(void)
{
//  DEBUG_TRACE;
  int sample = 1;
  char* sample_bytes = (char*)&sample;
  char byte0 = sample_bytes[0];
  bool result = byte0 != 0;
//  DEBUG_REPORT("little endian = " + to_string(result));
  return result;
}

////////////////////////////////////////////////////////////////////////////////
