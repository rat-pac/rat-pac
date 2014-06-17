#ifndef CLONABLE_HPP
#define CLONABLE_HPP
/*------------------------------------------------------------------------------

  Author:    Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License, see ../docs/license.html

  This abstract class defines the clonable interface

  The clonable interface is used by smart_ptr_clone and by the persistence
  functions for polymorphic types. 

  If you make any class a subclass of clonable, then you must provide a
  clone() method with the type profile specified here. This method must copy
  the object (i.e. *this), returning a new-allocated object with the same
  contents.

  If you provide this interface, all systems requiring a clonable object will
  work correctly.

  ------------------------------------------------------------------------------*/

class clonable
{
public:
  virtual clonable* clone(void) const = 0;
};

#endif
