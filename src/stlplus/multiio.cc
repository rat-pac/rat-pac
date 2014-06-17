/*------------------------------------------------------------------------------

  Author:    Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License, see ../docs/license.html

------------------------------------------------------------------------------*/
#include "multiio.hpp"
#include "debug.hpp"
#include <algorithm>

////////////////////////////////////////////////////////////////////////////////
// Output

omtext::omtext(void) : 
  otext(new ombuff())
{
}

omtext::omtext(const otext& str) :
  otext(new ombuff())
{
  add(str);
}

omtext::omtext(const otext& str1, const otext& str2) :
  otext(new ombuff())
{
  add(str1);
  add(str2);
}

void omtext::open(const otext& str)
{
  otext::open(new ombuff());
  add(str);
}

void omtext::open(const otext& str1, const otext& str2)
{
  otext::open(new ombuff());
  add(str1);
  add(str2);
}

unsigned omtext::add(const otext& str)
{
  ombuff* buf = dynamic_cast<ombuff*>(m_buffer);
  DEBUG_ASSERT(buf);
  return buf ? buf->add(str) : (unsigned)-1;
}

void omtext::remove(unsigned offset)
{
  ombuff* buf = dynamic_cast<ombuff*>(m_buffer);
  DEBUG_ASSERT(buf);
  if (buf) buf->remove(offset);
}  

unsigned omtext::device_count(void) const
{
  const ombuff* buf = dynamic_cast<const ombuff*>(m_buffer);
  DEBUG_ASSERT(buf);
  return buf ? buf->count() : 0;
}

otext& omtext::device_get(unsigned i)
{
  ombuff* buf = dynamic_cast<ombuff*>(m_buffer);
  DEBUG_ASSERT(buf);
  return buf->device(i);
}

const otext& omtext::device_get(unsigned i) const
{
  const ombuff* buf = dynamic_cast<const ombuff*>(m_buffer);
  DEBUG_ASSERT(buf);
  return buf->device(i);
}

////////////////////////////////////////////////////////////////////////////////
// Input

imtext::imtext(void) : itext(new imbuff())
{
}

imtext::imtext(const itext& str) : itext(new imbuff())
{
  add(str);
}

imtext::imtext(const itext& str1, const itext& str2) : itext(new imbuff())
{
  add(str1);
  add(str2);
}

void imtext::open(const itext& str)
{
  itext::open(new imbuff());
  add(str);
}

void imtext::open(const itext& str1, const itext& str2)
{
  itext::open(new imbuff());
  add(str1);
  add(str2);
}

unsigned imtext::add(const itext& str)
{
  imbuff* buf = dynamic_cast<imbuff*>(m_buffer);
  DEBUG_ASSERT(buf);
  return buf ? buf->add(str) : (unsigned)-1;
}

void imtext::remove(unsigned offset)
{
  imbuff* buf = dynamic_cast<imbuff*>(m_buffer);
  DEBUG_ASSERT(buf);
  if (buf) buf->remove(offset);
}  

unsigned imtext::device_count(void) const
{
  const imbuff* buf = dynamic_cast<const imbuff*>(m_buffer);
  DEBUG_ASSERT(buf);
  return buf ? buf->count() : 0;
}

itext& imtext::device_get(unsigned i)
{
  imbuff* buf = dynamic_cast<imbuff*>(m_buffer);
  DEBUG_ASSERT(buf);
  return buf->device(i);
}

const itext& imtext::device_get(unsigned i) const
{
  const imbuff* buf = dynamic_cast<const imbuff*>(m_buffer);
  DEBUG_ASSERT(buf);
  return buf->device(i);
}

////////////////////////////////////////////////////////////////////////////////
// Output Buffer

ombuff::ombuff(void) :
  // each sub-device should define its own line-buffer and newline modes
  obuff(false,otext::binary_mode)
{
}

unsigned ombuff::add(const otext& str)
{
  m_devices.push_back(str);
  return m_devices.size()-1;
}

void ombuff::remove(unsigned offset)
{
  DEBUG_ASSERT(offset < m_devices.size());
  m_devices.erase(m_devices.begin()+offset);
}

unsigned ombuff::put (unsigned char ch)
{
  unsigned result = 0;
  for (unsigned i = 0; i < m_devices.size(); i++)
    result = std::max(result,m_devices[i].put(ch)?1u:0u);
  return result;
}

unsigned ombuff::count(void) const
{
  return m_devices.size();
}

otext& ombuff::device(unsigned i)
{
  return m_devices[i];
}

const otext& ombuff::device(unsigned i) const
{
  return m_devices[i];
}

////////////////////////////////////////////////////////////////////////////////
// Input Buffer

imbuff::imbuff(void) :
  // the individual devices define their own conversion modes
  ibuff(itext::binary_mode)
{
}

unsigned imbuff::add(const itext& str)
{
  m_devices.push_back(str);
  return m_devices.size()-1;
}

void imbuff::remove(unsigned offset)
{
  DEBUG_ASSERT(offset < m_devices.size());
  m_devices.erase(m_devices.begin()+offset);
}

int imbuff::peek (void)
{
  while (!m_devices.empty() && m_devices.begin()->eof())
    m_devices.erase(m_devices.begin());
  return m_devices.empty() ? -1 : m_devices.begin()->peek();
}

int imbuff::get (void)
{
  while (!m_devices.empty() && m_devices.begin()->eof())
    m_devices.erase(m_devices.begin());
  return m_devices.empty() ? -1 : m_devices.begin()->get();
}

unsigned imbuff::count(void) const
{
  return m_devices.size();
}

itext& imbuff::device(unsigned i)
{
  return m_devices[i];
}

const itext& imbuff::device(unsigned i) const
{
  return m_devices[i];
}

////////////////////////////////////////////////////////////////////////////////
