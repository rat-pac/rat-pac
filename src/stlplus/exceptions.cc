/*------------------------------------------------------------------------------

  Author:    Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License, see ../docs/license.html

  ------------------------------------------------------------------------------*/

#include "exceptions.hpp"
#include "debug.hpp"
#include "fileio.hpp"

////////////////////////////////////////////////////////////////////////////////

null_dereference::null_dereference(const std::string& description) throw() :
  std::logic_error("null dereference: " + description)
{
  DEBUG_TRACE;
  DEBUG_STACKDUMP(std::string(what()));
}

null_dereference::~null_dereference(void) throw()
{
}

////////////////////////////////////////////////////////////////////////////////

end_dereference::end_dereference(const std::string& description) throw() :
  std::logic_error("end dereference: " + description)
{
  DEBUG_TRACE;
  DEBUG_STACKDUMP(std::string(what()));
}

end_dereference::~end_dereference(void) throw()
{
}

////////////////////////////////////////////////////////////////////////////////

wrong_object::wrong_object(const std::string& description) throw() :
  std::logic_error("wrong object: " + description)
{
  DEBUG_TRACE;
  DEBUG_STACKDUMP(std::string(what()));
}

wrong_object::~wrong_object(void) throw()
{
}

////////////////////////////////////////////////////////////////////////////////
