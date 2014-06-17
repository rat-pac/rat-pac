/*------------------------------------------------------------------------------

  Author:    Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License, see ../docs/license.html

  ------------------------------------------------------------------------------*/
#include "debug.hpp"
#include "string_utilities.hpp"
#include "file_system.hpp"
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////

static const char* unknown_function = "";

////////////////////////////////////////////////////////////////////////////////

assert_failed::assert_failed(const char* file, int line, const char* function, const std::string& message) throw() : 
  std::logic_error(std::string(file) + std::string(":") + to_string(line) + std::string(":") + 
                   std::string(function ? function : unknown_function) + std::string(": assertion failed: ") + message)
{
}

assert_failed::~assert_failed(void) throw()
{
}

////////////////////////////////////////////////////////////////////////////////

static unsigned debug_trace_depth = 0;
static bool debug_global = false;
static bool debug_set = false;
static bool debug_recurse = false;
static bool debug_read = false;
static char* debug_match = 0;
static const _debug_trace* debug_last = 0;

void _debug_global(const char* file, int line, const char* function, bool state)
{
  debug_global = state;
  fprintf(stderr, "%s:%i:[%i]%s ", filename_part(file).c_str(), line, debug_trace_depth, function ? function : unknown_function);
  fprintf(stderr, "debug global : %s\n", debug_global ? "on" : "off");
}

void _debug_assert_fail(const char* file, int line, const char* function, const char* test) throw(assert_failed)
{
  fprintf(stderr, "%s:%i:[%i]%s: assertion failed: %s\n", 
          filename_part(file).c_str(), line, debug_trace_depth, function ? function : unknown_function, test);
  if (debug_last) debug_last->stackdump();
  throw assert_failed (file, line, function, std::string(test));
}

////////////////////////////////////////////////////////////////////////////////

_debug_trace::_debug_trace(const char* f, int l, const char* fn) :
  m_file(f), m_line(l), m_function(fn ? fn : unknown_function), 
  m_depth(0), m_last(debug_last), m_dbg(false), m_old(false)
{
  if (!debug_read)
  {
    debug_read = true;
    debug_match = getenv("DEBUG");
    debug_recurse = getenv("DEBUG_LOCAL") == 0;
  }
  m_dbg = debug_set || (debug_match && (!debug_match[0] || match_wildcard(debug_match, filename_part(m_file))));
  m_old = debug_set;
  if (m_dbg && debug_recurse)
    debug_set = true;
  m_depth = ++debug_trace_depth;
  debug_last = this;
  if (debug()) report(std::string("entering ") + (m_function ? m_function : unknown_function));
}

_debug_trace::~_debug_trace(void)
{
  if (debug()) report("leaving");
  --debug_trace_depth;
  debug_set = m_old;
  debug_last = m_last;
}

const char* _debug_trace::file(void) const
{
  return m_file;
}

int _debug_trace::line(void) const
{
  return m_line;
}

bool _debug_trace::debug(void) const
{
  return m_dbg || debug_global;
}

void _debug_trace::debug_on(int l, bool recurse)
{
  m_dbg = true;
  m_old = debug_set;
  if (recurse)
    debug_set = true;
  report(l, std::string("debug on") + (recurse ? " recursive" : ""));
}

void _debug_trace::debug_off(int l)
{
  if (debug()) report(l, std::string("debug off"));
  m_dbg = false;
  debug_set = m_old;
}

void _debug_trace::prefix(int l) const
{
  fprintf(stderr, "%s:%i:[%i]%s ", filename_part(m_file).c_str(), l, m_depth, m_function ? m_function : unknown_function);
}

void _debug_trace::do_report(int l, const std::string& message) const
{
  prefix(l);
  fprintf(stderr, "%s\n", message.c_str());
  fflush(stderr);
}

void _debug_trace::do_report(const std::string& message) const
{
  do_report(m_line, message);
}

void _debug_trace::report(int l, const std::string& message) const
{
  do_report(l, message);
}

void _debug_trace::report(const std::string& message) const
{
  report(m_line, message);
}

void _debug_trace::error(int l, const std::string& message) const
{
  do_report(l, "ERROR: " + message);
}

void _debug_trace::error(const std::string& message) const
{
  error(m_line, message);
}

void _debug_trace::stackdump(int l, const std::string& message) const
{
  do_report(l, message);
  stackdump();
}

void _debug_trace::stackdump(const std::string& message) const
{
  stackdump(m_line, message);
}

void _debug_trace::stackdump(void) const
{
  for (const _debug_trace* item = this; item; item = item->m_last)
    item->do_report("...called from here");
}
