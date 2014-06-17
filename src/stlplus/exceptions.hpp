#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP
/*------------------------------------------------------------------------------

  Author: Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License, see ../docs/license.html

  The set of general exceptions thrown by STLplus components

  ------------------------------------------------------------------------------*/
#include "os_fixes.hpp"
#include <stdexcept>

////////////////////////////////////////////////////////////////////////////////
// Thrown if a pointer or an iterator is dereferenced when it is null

class null_dereference : public std::logic_error
{
public:
  null_dereference(const std::string& description) throw();
  ~null_dereference(void) throw();
};

////////////////////////////////////////////////////////////////////////////////
// Thrown if an iterator is dereferenced when it is pointing to the end element

class end_dereference : public std::logic_error
{
public:
  end_dereference(const std::string& description) throw();
  ~end_dereference(void) throw();
};

////////////////////////////////////////////////////////////////////////////////
// Thrown if an iterator is used with the wrong container. In other words, an
// iterator is created as a pointer to a sub-object within a container. If
// that iterator is then used with a different container, this exception is
// thrown.

class wrong_object : public std::logic_error
{
public:
  wrong_object(const std::string& description) throw();
  ~wrong_object(void) throw();
};

////////////////////////////////////////////////////////////////////////////////
#endif
