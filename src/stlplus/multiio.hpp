#ifndef MULTIIO_HPP
#define MULTIIO_HPP
/*------------------------------------------------------------------------------

  Author:    Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License, see ../docs/license.html

  Classes for redirecting I/O to/from multiple devices

  ------------------------------------------------------------------------------*/
#include "os_fixes.hpp"
#include "textio.hpp"

////////////////////////////////////////////////////////////////////////////////
// Output

class omtext : public otext
{
public:
  omtext(void);
  omtext(const otext&);
  omtext(const otext&, const otext&);
  void open(const otext&);
  void open(const otext&, const otext&);
  unsigned add(const otext&);
  void remove(unsigned);

  unsigned device_count(void) const;
  otext& device_get(unsigned);
  const otext& device_get(unsigned) const;
};

////////////////////////////////////////////////////////////////////////////////
// Input

class imtext : public itext
{
public:
  imtext(void);
  imtext(const itext&);
  imtext(const itext&, const itext&);
  void open(const itext&);
  void open(const itext&, const itext&);
  unsigned add(const itext&);
  void remove(unsigned);

  unsigned device_count(void) const;
  itext& device_get(unsigned);
  const itext& device_get(unsigned) const;
};

////////////////////////////////////////////////////////////////////////////////
// Internals

class ombuff : public obuff
{
  friend class omtext;
public:
  ombuff(void);
  unsigned add(const otext&);
  void remove(unsigned);

  virtual unsigned put (unsigned char);

  unsigned count(void) const;
  otext& device(unsigned);
  const otext& device(unsigned) const;

private:
  std::vector<otext> m_devices;

  // make this class uncopyable
  ombuff(const ombuff&);
  ombuff& operator = (const ombuff&);
};

class imbuff : public ibuff
{
  friend class imtext;
public:
  imbuff(void);
  unsigned add(const itext&);
  void remove(unsigned);

  virtual int peek (void);
  virtual int get (void);

  unsigned count(void) const;
  itext& device(unsigned);
  const itext& device(unsigned) const;

private:
  std::vector<itext> m_devices;

  // make this class uncopyable
  imbuff(const imbuff&);
  imbuff& operator = (const imbuff&);
};

////////////////////////////////////////////////////////////////////////////////

#endif
