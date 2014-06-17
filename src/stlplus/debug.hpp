#ifndef DEBUG_HPP
#define DEBUG_HPP
/*------------------------------------------------------------------------------

  Author:    Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License, see ../docs/license.html

  Set of simple debug utilities, all of which are switched off by the
  NDEBUG compiler directive

  ------------------------------------------------------------------------------*/
#include "os_fixes.hpp"
#include "exceptions.hpp"
#include <assert.h>
#include <stdio.h>
#include <string>

////////////////////////////////////////////////////////////////////////////////
// Exception thrown if an assertion fails

class assert_failed : public std::logic_error
{
public:
  assert_failed(const char* file, int line, const char* function, const std::string& message) throw();
  ~assert_failed(void) throw();
};

////////////////////////////////////////////////////////////////////////////////
// The macros used in debugging

#ifndef NDEBUG

#define DEBUG_TRACE _debug_trace _debug_trace_(__FILE__,__LINE__,__FUNCTION__)
#define IF_DEBUG(stmts) {if (_debug_trace_.debug()){_debug_trace_.prefix(__LINE__);stmts;}}
#define DEBUG_REPORT(str) IF_DEBUG(_debug_trace_.report(__LINE__,str))
#define DEBUG_ERROR(str) _debug_trace_.error(__LINE__,str)
#define DEBUG_STACKDUMP(str) _debug_trace_.stackdump(__LINE__,str)
#define DEBUG_ON _debug_trace_.debug_on(__LINE__,true)
#define DEBUG_ON_LOCAL _debug_trace_.debug_on(__LINE__,false)
#define DEBUG_ON_GLOBAL _debug_global(__FILE__,__LINE__,__FUNCTION__,true)
#define DEBUG_OFF_GLOBAL _debug_global(__FILE__,__LINE__,__FUNCTION__,false)
#define DEBUG_OFF _debug_trace_.debug_off(__LINE__)
#define DEBUG_ASSERT(test) if (!(test))_debug_assert_fail(__FILE__,__LINE__,__FUNCTION__,#test)

#else

#define DEBUG_TRACE
#define IF_DEBUG(stmts)
#define DEBUG_REPORT(str)
#define DEBUG_ERROR(str)
#define DEBUG_STACKDUMP(str)
#define DEBUG_ON
#define DEBUG_ON_LOCAL
#define DEBUG_ON_GLOBAL
#define DEBUG_OFF_GLOBAL
#define DEBUG_OFF
#define DEBUG_ASSERT(test)

#endif

////////////////////////////////////////////////////////////////////////////////
// infrastructure - don't use directly

void _debug_global(const char* file, int line, const char* function, bool state = true);
void _debug_assert_fail(const char* file, int line, const char* function, const char* test) throw(assert_failed);

class _debug_trace
{
public:
  _debug_trace(const char* f, int l, const char* fn);
  ~_debug_trace(void);
  const char* file(void) const;
  int line(void) const;
  bool debug(void) const;
  void debug_on(int l, bool recurse);
  void debug_off(int l);
  void prefix(int l) const;
  void report(int l, const std::string& message) const;
  void report(const std::string& message) const;
  void error(int l, const std::string& message) const;
  void error(const std::string& message) const;
  void stackdump(int l, const std::string& message) const;
  void stackdump(const std::string& message) const;
  void stackdump(void) const;

private:
  const char* m_file;
  int m_line;
  const char* m_function;
  unsigned m_depth;
  const _debug_trace* m_last;
  bool m_dbg;
  bool m_old;
  void do_report(int l, const std::string& message) const;
  void do_report(const std::string& message) const;

  // make this class uncopyable
  _debug_trace(const _debug_trace&);
  _debug_trace& operator = (const _debug_trace&);
};

////////////////////////////////////////////////////////////////////////////////
#endif
