/*------------------------------------------------------------------------------

  Author:    Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License, see ../docs/license.html

  ------------------------------------------------------------------------------*/
#include "os_fixes.hpp"
#include "stringio.hpp"
#include "debug.hpp"

////////////////////////////////////////////////////////////////////////////////

ostext::ostext(void) : otext(new osbuff)
{
}

std::string& ostext::get_string(void)
{
  osbuff* buf = dynamic_cast<osbuff*>(m_buffer);
  DEBUG_ASSERT(buf);
  return buf->m_data;
}

const std::string& ostext::get_string(void) const
{
  const osbuff* buf = dynamic_cast<const osbuff*>(m_buffer);
  DEBUG_ASSERT(buf);
  return buf->m_data;
}

////////////////////////////////////////////////////////////////////////////////

osbuff::osbuff(void)
{
}

unsigned osbuff::put(unsigned char ch)
{
  m_data += (char)ch;
  return 1;
}

////////////////////////////////////////////////////////////////////////////////

istext::istext(const std::string& data) : itext(new isbuff(data))
{
}

std::string& istext::get_string(void)
{
  isbuff* buf = dynamic_cast<isbuff*>(m_buffer);
  DEBUG_ASSERT(buf);
  return buf->m_data;
}

const std::string& istext::get_string(void) const
{
  const isbuff* buf = dynamic_cast<const isbuff*>(m_buffer);
  DEBUG_ASSERT(buf);
  return buf->m_data;
}

////////////////////////////////////////////////////////////////////////////////

isbuff::isbuff(const std::string& d) : m_data(d), m_index(0)
{
}

int isbuff::peek(void)
{
  return m_index >= m_data.size() ? -1 : (int)(unsigned char)m_data[m_index];
}

int isbuff::get(void)
{
  return m_index >= m_data.size() ? -1 : (int)(unsigned char)m_data[m_index++];
}

////////////////////////////////////////////////////////////////////////////////
