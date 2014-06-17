#ifndef STRINGIO_HPP
#define STRINGIO_HPP
/*------------------------------------------------------------------------------

  Author:    Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License, see ../docs/license.html

  Classes for redirecting I/O to/from a string

  ------------------------------------------------------------------------------*/
#include "os_fixes.hpp"
#include "textio.hpp"
#include <string>

////////////////////////////////////////////////////////////////////////////////
// string Output

class ostext : public otext
{
public:
  ostext(void);
  std::string& get_string(void);
  const std::string& get_string(void) const;
};

class osbuff : public obuff
{
protected:
  friend class ostext;
  std::string m_data;
public:
  osbuff(void);
protected:
  virtual unsigned put (unsigned char);
private:
  // make this class uncopyable
  osbuff(const osbuff&);
  osbuff& operator = (const osbuff&);
};

////////////////////////////////////////////////////////////////////////////////
// string Input

class istext : public itext
{
public:
  istext(const std::string& data);
  std::string& get_string(void);
  const std::string& get_string(void) const;
};

class isbuff : public ibuff
{
protected:
  friend class istext;
  std::string m_data;
  unsigned m_index;
public:
  isbuff(const std::string& data);
protected:
  virtual int peek (void);
  virtual int get (void);
private:
  // make this class uncopyable
  isbuff(const isbuff&);
  isbuff& operator = (const isbuff&);
};

////////////////////////////////////////////////////////////////////////////////
#endif
