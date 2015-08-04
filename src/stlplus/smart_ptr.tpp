/*------------------------------------------------------------------------------

  Author:    Andy Rushton and Daniel Milton
  Copyright: (c) Andy Rushton, 2004; Daniel Milton 2005
  License:   BSD License, see ../docs/license.html

------------------------------------------------------------------------------*/
#include "string_utilities.hpp"
#include "debug.hpp"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Simple Pointer Classes
// (c) Dan Milton
// based on smart_ptr classes
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// simple_ptr class
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// constructors, assignments and destructors

// create a null pointer
template<typename T>
simple_ptr<T>::simple_ptr(void) :
  m_pointer(0), m_count(new unsigned(1))
{
}

// create a pointer containing a *copy* of the object pointer
template<typename T>
simple_ptr<T>::simple_ptr(const T& data) :
  m_pointer(0), m_count(new unsigned(1))
{
  set_value(data);
}

// delete any old value in the pointer and assign to it a new *copy* of the argument
template<typename T>
simple_ptr<T>& simple_ptr<T>::operator=(const T& data)
{
  set_value(data);
  return *this;
}

// copy constructor implements aliasing
template<typename T>
simple_ptr<T>::simple_ptr(const simple_ptr<T>& r) :
  m_pointer(r.m_pointer), m_count(r.m_count)
{
  ++(*m_count);
}

// assignment of smart pointers implements aliasing
template<typename T>
simple_ptr<T>& simple_ptr<T>::operator=(const simple_ptr<T>& r)
{
  // make it alias-copy safe - this means that I don't try to do the
  // assignment if r is either the same object or an alias of it
  if (m_count == r.m_count) return *this;
  // dealias this object
  if((--(*m_count)) == 0)
  {
    if (m_pointer) delete m_pointer;
    delete m_count;
  }
  // make this object an alias of r
  m_pointer = r.m_pointer;
  m_count = r.m_count;
  ++(*m_count);
  return *this;
}

// create a pointer containing a dynamically created object
// Note: the object must be allocated *by the user* with new
// constructor form - must be called in the form simple_ptr<type> x(new type(args))
template<typename T>
simple_ptr<T>::simple_ptr(T* data) :
  m_pointer(0), m_count(new unsigned(1))
{
  set(data);
}

// assignment form
template<typename T>
simple_ptr<T>& simple_ptr<T>::operator=(T* data)
{
  set(data);
  return *this;
}

// destructor decrements the reference count and delete only when the last reference is destroyed
template<typename T>
simple_ptr<T>::~simple_ptr(void)
{
  // dealias this object
  if((--(*m_count)) == 0)
  {
    if (m_pointer) delete m_pointer;
    delete m_count;
  }
}

//////////////////////////////////////////////////////////////////////////////
// logical tests to see if there is anything contained in the pointer since it can be null

template<typename T>
bool simple_ptr<T>::null(void) const
{
  return m_pointer == 0;
}

template<typename T>
bool simple_ptr<T>::present(void) const
{
  return m_pointer != 0;
}

template<typename T>
bool simple_ptr<T>::operator!(void) const
{
  return m_pointer == 0;
}

template<typename T>
simple_ptr<T>::operator bool(void) const
{
  return m_pointer != 0;
}

//////////////////////////////////////////////////////////////////////////////
// dereference operators and functions

template<typename T>
T& simple_ptr<T>::operator*(void) throw(null_dereference)
{
  if (!m_pointer) throw null_dereference("null pointer dereferenced in simple_ptr::operator*");
  return *m_pointer;
}

template<typename T>
const T& simple_ptr<T>::operator*(void) const throw(null_dereference)
{
  if (!m_pointer) throw null_dereference("null pointer dereferenced in simple_ptr::operator*");
  return *m_pointer;
}

template<typename T>
T* simple_ptr<T>::operator->(void) throw(null_dereference)
{
  if (!m_pointer) throw null_dereference("null pointer dereferenced in simple_ptr::operator->");
  return m_pointer;
}

template<typename T>
const T* simple_ptr<T>::operator->(void) const throw(null_dereference)
{
  if (!m_pointer) throw null_dereference("null pointer dereferenced in simple_ptr::operator->");
  return m_pointer;
}

//////////////////////////////////////////////////////////////////////////////
// explicit function forms of the above assignment dereference operators

template<typename T>
void simple_ptr<T>::set_value(const T& data)
{
  set(new T(data));
}

template<typename T>
T& simple_ptr<T>::value(void) throw(null_dereference)
{
  if (!m_pointer) throw null_dereference("null pointer dereferenced in simple_ptr::value");
  return *m_pointer;
}

template<typename T>
const T& simple_ptr<T>::value(void) const throw(null_dereference)
{
  if (!m_pointer) throw null_dereference("null pointer dereferenced in simple_ptr::value");
  return *m_pointer;
}

template<typename T>
void simple_ptr<T>::set(T* data)
{
  // if this is the only object, delete the old value, else dealias
  if (*m_count == 1)
  {
    if (m_pointer) delete m_pointer;
  }
  else
  {
    --(*m_count);
    m_count = new unsigned(1);
  }
  m_pointer = data;
}

template<typename T>
T* simple_ptr<T>::pointer(void)
{
  return m_pointer;
}

template<typename T>
const T* simple_ptr<T>::pointer(void) const
{
  return m_pointer;
}

////////////////////////////////////////////////////////////////////////////////
// functions to manage counted referencing

template<typename T>
bool simple_ptr<T>::aliases(const simple_ptr<T>& r) const
{
  // aliases have the same counter
  return m_count == r.m_count;
}

template<typename T>
unsigned simple_ptr<T>::alias_count(void) const
{
  return *m_count;
}

template<typename T>
void simple_ptr<T>::make_unique(void)
{
  // if already unique, do nothing
  if (*m_count == 1) return;
  // dealias this and copy the old contents
  --(*m_count);
  if (m_pointer) m_pointer = new T(*m_pointer);
  m_count = new unsigned(1);
}

// clear sets this alias to null, dealiasing if necessary
template<typename T>
void simple_ptr<T>::clear(void)
{
  set(0);
}

template<typename T>
void simple_ptr<T>::copy(const simple_ptr<T>& data)
{
  // first make this an alias of the data, then make it a copy by making it unique
  *this = data;
  make_unique();
}

template<typename T>
simple_ptr<T> simple_ptr<T>::copy(void) const
{
  simple_ptr<T> result;
  result.copy(*this);
  return result;
}

////////////////////////////////////////////////////////////////////////////////
// persistence methods
// see also - external persistence functions

// Like ordinary pointers they are dumped as a magic key and then, on the first
// alias only, the contents. Subsequent aliases to the same object are dumped
// as just a magic key. I use the address of the counter to generate the key
// since that is always there and is unique. Using the address of the object
// would make all null pointers aliases of each other which may not be
// correct.

// I've split these into the exported non-member functions and internal member
// functions because I need access to the data structure but couldn't get
// friend templates to work on VC

// a simple_ptr is used to point to non-clonable objects, so use the standard dump/restore for pointers

template<typename T>
void simple_ptr<T>::dump(dump_context& context) const throw(persistent_dump_failed)
{
  // get a magic key for the count - this also returns a flag saying whether its been seen before
  std::pair<bool,unsigned> mapping = context.pointer_map(m_count);
  // dump the magic key
  ::dump(context,mapping.second);
  // dump the contents but only if this is the first time this object has been seen
  // use the existing routines for ordinary pointers to dump the contents
  if (!mapping.first)
    dump_pointer(context,m_pointer);
}

template<typename T>
void simple_ptr<T>::restore(restore_context& context) throw(persistent_restore_failed)
{
  // restore the magic key
  unsigned magic = 0;
  ::restore(context,magic);
  // lookup this magic number to see if we have seen this already
  std::pair<bool,void*> mapping = context.pointer_map(magic);
  if (mapping.first)
  {
    // this object has already been restored
    // dealias the existing object and make it an alias of the existing object
    if (--(*m_count) == 0)
    {
      if (m_pointer) delete m_pointer;
      delete m_count;
    }
    m_pointer = ((simple_ptr<T>*)mapping.second)->m_pointer;
    m_count = ((simple_ptr<T>*)mapping.second)->m_count;
    ++(*m_count);
  }
  else
  {
    // this is the first contact with this object
    // make sure this pointer is unique to prevent side-effects
    if (--(*m_count) == 0)
    {
      if (m_pointer) delete m_pointer;
      delete m_count;
    }
    m_count = new unsigned(1);
    // restore the contents
    restore_pointer(context,m_pointer);
    // map the magic key onto this object
    context.pointer_add(magic,this);
  }
}

////////////////////////////////////////////////////////////////////////////////
// comparisons required for using this class in an STL container

template<typename T>
bool operator==(const simple_ptr<T>& left, const simple_ptr<T>& right)
{
  // a null is not equal to a non-null but equal to another null
  if(!left || !right) return left.pointer() == right.pointer();
  // shortcut - if the two pointers are equal then the objects must be equal
  if (left.pointer() == right.pointer()) return true;
  // otherwise compare the objects themselves
  return *left == *right;
}

template<typename T>
bool operator<(const simple_ptr<T>& left, const simple_ptr<T>& right)
{
  // a null pointer is less than a non-null but equal to another null
  if(!left || !right) return left.pointer() < right.pointer();
  // shortcut - if the two pointers are equal then the comparison must be false
  if (left.pointer() == right.pointer()) return false;
  // otherwise, compare the objects
  return *left < *right;
}

////////////////////////////////////////////////////////////////////////////////
// string/print utilities

template<typename T>
std::string simple_ptr_to_string(const simple_ptr<T>& ptr, std::string null_string)
{
  if (!ptr) return null_string;
  return "*(" + to_string(*ptr) + ")";
}

template<typename T>
otext& print_simple_ptr(otext& str, const simple_ptr<T>& ptr, std::string null_string)
{
  if (!ptr) return str << null_string;
  str << "*(";
  print(str, *ptr);
  str << ")";
  return str;
}

template<typename T>
otext& print_simple_ptr(otext& str, const simple_ptr<T>& ptr, unsigned indent, std::string null_string)
{
  print_indent(str, indent);
  print_simple_ptr(str, ptr, null_string);
  return str << endl;
}

////////////////////////////////////////////////////////////////////////////////
// Persistence - non-member functions actually called by the user

template<typename T>
void dump_simple_ptr(dump_context& context, const simple_ptr<T>& data) throw(persistent_dump_failed)
{
  data.dump(context);
}

template<typename T>
void restore_simple_ptr(restore_context& context, simple_ptr<T>& data) throw(persistent_restore_failed)
{
  data.restore(context);
}

////////////////////////////////////////////////////////////////////////////////
// simple_ptr_clone class
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// constructors, assignments and destructors

// create a null pointer
template<typename T>
simple_ptr_clone<T>::simple_ptr_clone(void) :
  m_pointer(0), m_count(new unsigned(1))
{
}

// create a pointer containing a *copy* of the object pointer
template<typename T>
simple_ptr_clone<T>::simple_ptr_clone(const T& data) :
  m_pointer(0), m_count(new unsigned(1))
{
  set_value(data);
}

// delete any old value in the pointer and assign to it a new *copy* of the argument
template<typename T>
simple_ptr_clone<T>& simple_ptr_clone<T>::operator=(const T& data)
{
  set_value(data);
  return *this;
}

// copy constructor implements counted referencing - no copy is made
template<typename T>
simple_ptr_clone<T>::simple_ptr_clone(const simple_ptr_clone<T>& r) :
  m_pointer(r.m_pointer), m_count(r.m_count)
{
  ++(*m_count);
}

// assignment of smart pointers implement counted referencing - no copy is made
template<typename T>
simple_ptr_clone<T>& simple_ptr_clone<T>::operator=(const simple_ptr_clone<T>& r)
{
  // make it alias-copy safe - this means that I don't try to do the
  // assignment if r is either the same object or an alias of it
  if (m_count == r.m_count) return *this;
  // dealias this object
  if((--(*m_count)) == 0)
  {
    if (m_pointer) delete m_pointer;
    delete m_count;
  }
  // make this object an alias of r
  m_pointer = r.m_pointer;
  m_count = r.m_count;
  ++(*m_count);
  return *this;
}

// create a pointer containing a dynamically created object
// Note: the object must be allocated *by the user* with new
// constructor form - must be called in the form simple_ptr_clone<type> x(new type(args))
template<typename T>
simple_ptr_clone<T>::simple_ptr_clone(T* data) :
  m_pointer(0), m_count(new unsigned(1))
{
  set(data);
}

// assignment form
template<typename T>
simple_ptr_clone<T>& simple_ptr_clone<T>::operator=(T* data)
{
  set(data);
  return *this;
}

// destructor decrements the reference count and delete only when the last reference is destroyed
template<typename T>
simple_ptr_clone<T>::~simple_ptr_clone(void)
{
  // dealias this object
  if((--(*m_count)) == 0)
  {
    if (m_pointer) delete m_pointer;
    delete m_count;
  }
}

//////////////////////////////////////////////////////////////////////////////
// logical tests to see if there is anything contained in the pointer since it can be null

template<typename T>
bool simple_ptr_clone<T>::null(void) const
{
  return m_pointer == 0;
}

template<typename T>
bool simple_ptr_clone<T>::present(void) const
{
  return m_pointer != 0;
}

template<typename T>
bool simple_ptr_clone<T>::operator!(void) const
{
  return m_pointer == 0;
}

template<typename T>
simple_ptr_clone<T>::operator bool(void) const
{
  return m_pointer != 0;
}

//////////////////////////////////////////////////////////////////////////////
// dereference operators and functions

template<typename T>
T& simple_ptr_clone<T>::operator*(void) throw(null_dereference)
{
  if (!m_pointer) throw null_dereference("null pointer dereferenced in simple_ptr_clone::operator*");
  return *m_pointer;
}

template<typename T>
const T& simple_ptr_clone<T>::operator*(void) const throw(null_dereference)
{
  if (!m_pointer) throw null_dereference("null pointer dereferenced in simple_ptr_clone::operator*");
  return *m_pointer;
}

template<typename T>
T* simple_ptr_clone<T>::operator->(void) throw(null_dereference)
{
  if (!m_pointer) throw null_dereference("null pointer dereferenced in simple_ptr_clone::operator->");
  return m_pointer;
}

template<typename T>
const T* simple_ptr_clone<T>::operator->(void) const throw(null_dereference)
{
  if (!m_pointer) throw null_dereference("null pointer dereferenced in simple_ptr_clone::operator->");
  return m_pointer;
}

//////////////////////////////////////////////////////////////////////////////
// explicit function forms of the above assignment dereference operators

template<typename T>
void simple_ptr_clone<T>::set_value(const T& data)
{
  set((T*)data.clone());
}

template<typename T>
T& simple_ptr_clone<T>::value(void) throw(null_dereference)
{
  if (!m_pointer) throw null_dereference("null pointer dereferenced in simple_ptr_clone::value");
  return *m_pointer;
}

template<typename T>
const T& simple_ptr_clone<T>::value(void) const throw(null_dereference)
{
  if (!m_pointer) throw null_dereference("null pointer dereferenced in simple_ptr_clone::value");
  return *m_pointer;
}

template<typename T>
void simple_ptr_clone<T>::set(T* data)
{
  // if this is the only object, delete the old value, else dealias
  if (*m_count == 1)
  {
    if (m_pointer) delete m_pointer;
  }
  else
  {
    --(*m_count);
    m_count = new unsigned(1);
  }
  m_pointer = data;
}

template<typename T>
T* simple_ptr_clone<T>::pointer(void)
{
  return m_pointer;
}

template<typename T>
const T* simple_ptr_clone<T>::pointer(void) const
{
  return m_pointer;
}

////////////////////////////////////////////////////////////////////////////////
// functions to manage counted referencing

template<typename T>
bool simple_ptr_clone<T>::aliases(const simple_ptr_clone<T>& r) const
{
  // aliases have the same counter
  return m_count == r.m_count;
}

template<typename T>
unsigned simple_ptr_clone<T>::alias_count(void) const
{
  return *m_count;
}

template<typename T>
void simple_ptr_clone<T>::make_unique(void)
{
  // if already unique, do nothing
  if (*m_count == 1) return;
  // dealias this and copy the old contents
  --(*m_count);
  if (m_pointer) m_pointer = (T*)m_pointer->clone();
  m_count = new unsigned(1);
}

// this version of the smart pointer only clears unique
template<typename T>
void simple_ptr_clone<T>::clear(void)
{
  set(0);
}

template<typename T>
void simple_ptr_clone<T>::copy(const simple_ptr_clone<T>& data)
{
  // first make this an alias of the data, then make it a copy by making it unique
  *this = data;
  make_unique();
}

template<typename T>
simple_ptr_clone<T> simple_ptr_clone<T>::copy(void) const
{
  simple_ptr_clone<T> result;
  result.copy(*this);
  return result;
}

////////////////////////////////////////////////////////////////////////////////
// persistence methods
// see also - external persistence functions

// Like ordinary pointers they are dumped as a magic key and then, on the first
// alias only, the contents. Subsequent aliases to the same object are dumped
// as just a magic key. I use the address of the counter to generate the key
// since that is always there and is unique. Using the address of the object
// would make all null pointers aliases of each other which may not be
// correct.

// I've split these into the exported non-member functions and internal member
// functions because I need access to the data structure but couldn't get
// friend templates to work on VC

// a simple_ptr_clone is used to point to non-clonable objects, so use the standard dump/restore for pointers

template<typename T>
void simple_ptr_clone<T>::dump(dump_context& context) const throw(persistent_dump_failed)
{
  // get a magic key for the count - this also returns a flag saying whether its been seen before
  std::pair<bool,unsigned> mapping = context.pointer_map(m_count);
  // dump the magic key
  ::dump(context,mapping.second);
  // dump the contents but only if this is the first time this object has been seen
  // use the existing routines for ordinary pointers to dump the contents
  if (!mapping.first)
    dump_interface(context,m_pointer);
}

template<typename T>
void simple_ptr_clone<T>::restore(restore_context& context) throw(persistent_restore_failed)
{
  // restore the magic key
  unsigned magic = 0;
  ::restore(context,magic);
  // lookup this magic number to see if we have seen this already
  std::pair<bool,void*> mapping = context.pointer_map(magic);
  if (mapping.first)
  {
    // this object has already been restored
    // dealias the existing object and make it an alias of the existing object
    if (--(*m_count) == 0)
    {
      if (m_pointer) delete m_pointer;
      delete m_count;
    }
    m_pointer = ((simple_ptr_clone<T>*)mapping.second)->m_pointer;
    m_count = ((simple_ptr_clone<T>*)mapping.second)->m_count;
    ++(*m_count);
  }
  else
  {
    // this is the first contact with this object
    // make sure this pointer is unique to prevent side-effects
    if (--(*m_count) == 0)
    {
      if (m_pointer) delete m_pointer;
      delete m_count;
    }
    m_count = new unsigned(1);
    // restore the contents
    restore_interface(context,m_pointer);
    // map the magic key onto this object
    context.pointer_add(magic,this);
  }
}

////////////////////////////////////////////////////////////////////////////////
// comparisons required for using this class in an STL container

template<typename T>
bool operator==(const simple_ptr_clone<T>& left, const simple_ptr_clone<T>& right)
{
  // a null is not equal to a non-null but equal to another null
  if(!left || !right) return left.pointer() == right.pointer();
  // shortcut - if the two pointers are equal then the objects must be equal
  if (left.pointer() == right.pointer()) return true;
  // otherwise compare the objects themselves
  return *left == *right;
}

template<typename T>
bool operator<(const simple_ptr_clone<T>& left, const simple_ptr_clone<T>& right)
{
  // a null pointer is less than a non-null but equal to another null
  if(!left || !right) return left.pointer() < right.pointer();
  // shortcut - if the two pointers are equal then the comparison must be false
  if (left.pointer() == right.pointer()) return false;
  // otherwise, compare the objects
  return *left < *right;
}

////////////////////////////////////////////////////////////////////////////////
// string/print utilities

template<typename T>
std::string simple_ptr_clone_to_string(const simple_ptr_clone<T>& ptr, std::string null_string)
{
  if (!ptr) return null_string;
  return "*(" + to_string(*ptr) + ")";
}

template<typename T>
otext& print_simple_ptr_clone(otext& str, const simple_ptr_clone<T>& ptr, std::string null_string)
{
  if (!ptr) return str << null_string;
  str << "*(";
  print(str, *ptr);
  str << ")";
  return str;
}

template<typename T>
otext& print_simple_ptr_clone(otext& str, const simple_ptr_clone<T>& ptr, unsigned indent, std::string null_string)
{
  print_indent(str, indent);
  print_simple_ptr_clone(str, ptr, null_string);
  return str << endl;
}

////////////////////////////////////////////////////////////////////////////////
// Persistence - non-member functions actually called by the user

template<typename T>
void dump_simple_ptr_clone(dump_context& context, const simple_ptr_clone<T>& data) throw(persistent_dump_failed)
{
  data.dump(context);
}

template<typename T>
void restore_simple_ptr_clone(restore_context& context, simple_ptr_clone<T>& data) throw(persistent_restore_failed)
{
  data.restore(context);
}

////////////////////////////////////////////////////////////////////////////////
// simple_ptr_nocopy class
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// constructors, assignments and destructors

// create a null pointer
template<typename T>
simple_ptr_nocopy<T>::simple_ptr_nocopy(void) :
  m_pointer(0), m_count(new unsigned(1))
{
}

// copy constructor implements counted referencing - no copy is made
template<typename T>
simple_ptr_nocopy<T>::simple_ptr_nocopy(const simple_ptr_nocopy<T>& r) :
  m_pointer(r.m_pointer), m_count(r.m_count)
{
  ++(*m_count);
}

// assignment of smart pointers implement counted referencing - no copy is made
template<typename T>
simple_ptr_nocopy<T>& simple_ptr_nocopy<T>::operator=(const simple_ptr_nocopy<T>& r)
{
  // make it alias-copy safe - this means that I don't try to do the
  // assignment if r is either the same object or an alias of it
  if (m_count == r.m_count) return *this;
  // dealias this object
  if((--(*m_count)) == 0)
  {
    if (m_pointer) delete m_pointer;
    delete m_count;
  }
  // make this object an alias of r
  m_pointer = r.m_pointer;
  m_count = r.m_count;
  ++(*m_count);
  return *this;
}

// create a pointer containing a dynamically created object
// Note: the object must be allocated *by the user* with new
// constructor form - must be called in the form simple_ptr_nocopy<type> x(new type(args))
template<typename T>
simple_ptr_nocopy<T>::simple_ptr_nocopy(T* data) :
  m_pointer(0), m_count(new unsigned(1))
{
  set(data);
}

// assignment form
template<typename T>
simple_ptr_nocopy<T>& simple_ptr_nocopy<T>::operator=(T* data)
{
  set(data);
  return *this;
}

// destructor decrements the reference count and delete only when the last reference is destroyed
template<typename T>
simple_ptr_nocopy<T>::~simple_ptr_nocopy(void)
{
  // dealias this object
  if((--(*m_count)) == 0)
  {
    if (m_pointer) delete m_pointer;
    delete m_count;
  }
}

//////////////////////////////////////////////////////////////////////////////
// logical tests to see if there is anything contained in the pointer since it can be null

template<typename T>
bool simple_ptr_nocopy<T>::null(void) const
{
  return m_pointer == 0;
}

template<typename T>
bool simple_ptr_nocopy<T>::present(void) const
{
  return m_pointer != 0;
}

template<typename T>
bool simple_ptr_nocopy<T>::operator!(void) const
{
  return m_pointer == 0;
}

template<typename T>
simple_ptr_nocopy<T>::operator bool(void) const
{
  return m_pointer != 0;
}

//////////////////////////////////////////////////////////////////////////////
// dereference operators and functions

template<typename T>
T& simple_ptr_nocopy<T>::operator*(void) throw(null_dereference)
{
  if (!m_pointer) throw null_dereference("null pointer dereferenced in simple_ptr_nocopy::operator*");
  return *m_pointer;
}

template<typename T>
const T& simple_ptr_nocopy<T>::operator*(void) const throw(null_dereference)
{
  if (!m_pointer) throw null_dereference("null pointer dereferenced in simple_ptr_nocopy::operator*");
  return *m_pointer;
}

template<typename T>
T* simple_ptr_nocopy<T>::operator->(void) throw(null_dereference)
{
  if (!m_pointer) throw null_dereference("null pointer dereferenced in simple_ptr_nocopy::operator->");
  return m_pointer;
}

template<typename T>
const T* simple_ptr_nocopy<T>::operator->(void) const throw(null_dereference)
{
  if (!m_pointer) throw null_dereference("null pointer dereferenced in simple_ptr_nocopy::operator->");
  return m_pointer;
}

//////////////////////////////////////////////////////////////////////////////
// explicit function forms of the above assignment dereference operators

template<typename T>
T& simple_ptr_nocopy<T>::value(void) throw(null_dereference)
{
  if (!m_pointer) throw null_dereference("null pointer dereferenced in simple_ptr_nocopy::value");
  return *m_pointer;
}

template<typename T>
const T& simple_ptr_nocopy<T>::value(void) const throw(null_dereference)
{
  if (!m_pointer) throw null_dereference("null pointer dereferenced in simple_ptr_nocopy::value");
  return *m_pointer;
}

template<typename T>
void simple_ptr_nocopy<T>::set(T* data)
{
  // if this is the only object, delete the old value, else dealias
  if (*m_count == 1)
  {
    if (m_pointer) delete m_pointer;
  }
  else
  {
    --(*m_count);
    m_count = new unsigned(1);
  }
  m_pointer = data;
}

template<typename T>
T* simple_ptr_nocopy<T>::pointer(void)
{
  return m_pointer;
}

template<typename T>
const T* simple_ptr_nocopy<T>::pointer(void) const
{
  return m_pointer;
}

////////////////////////////////////////////////////////////////////////////////
// functions to manage counted referencing

template<typename T>
bool simple_ptr_nocopy<T>::aliases(const simple_ptr_nocopy<T>& r) const
{
  // aliases have the same counter
  return m_count == r.m_count;
}

template<typename T>
unsigned simple_ptr_nocopy<T>::alias_count(void) const
{
  return *m_count;
}

// this version of the smart pointer only clears unique
template<typename T>
void simple_ptr_nocopy<T>::clear(void)
{
  set(0);
}

////////////////////////////////////////////////////////////////////////////////
// persistence methods
// a smart_ptr_nocopy is used to point to non-copyable objects, so it doesn't make sense to provide peersistence!

////////////////////////////////////////////////////////////////////////////////
// comparisons required for using this class in an STL container

template<typename T>
bool operator==(const simple_ptr_nocopy<T>& left, const simple_ptr_nocopy<T>& right)
{
  // a null is not equal to a non-null but equal to another null
  if(!left || !right) return left.pointer() == right.pointer();
  // shortcut - if the two pointers are equal then the objects must be equal
  if (left.pointer() == right.pointer()) return true;
  // otherwise compare the objects themselves
  return *left == *right;
}

template<typename T>
bool operator<(const simple_ptr_nocopy<T>& left, const simple_ptr_nocopy<T>& right)
{
  // a null pointer is less than a non-null but equal to another null
  if(!left || !right) return left.pointer() < right.pointer();
  // shortcut - if the two pointers are equal then the comparison must be false
  if (left.pointer() == right.pointer()) return false;
  // otherwise, compare the objects
  return *left < *right;
}

////////////////////////////////////////////////////////////////////////////////
// string/print utilities

template<typename T>
std::string simple_ptr_nocopy_to_string(const simple_ptr_nocopy<T>& ptr, std::string null_string)
{
  if (!ptr) return null_string;
  return "*(" + to_string(*ptr) + ")";
}

template<typename T>
otext& print_simple_ptr_nocopy(otext& str, const simple_ptr_nocopy<T>& ptr, std::string null_string)
{
  if (!ptr) return str << null_string;
  str << "*(";
  print(str, *ptr);
  str << ")";
  return str;
}

template<typename T>
otext& print_simple_ptr_nocopy(otext& str, const simple_ptr_nocopy<T>& ptr, unsigned indent, std::string null_string)
{
  print_indent(str, indent);
  print_simple_ptr_nocopy(str, ptr, null_string);
  return str << endl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Smart Pointer Classes - Andy Rushton
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// internal holder data structure
////////////////////////////////////////////////////////////////////////////////

template<typename T>
class smart_ptr_holder
{
private:
  unsigned m_count;
  T* m_data;
  
  // disallow copying because the holder doesn't know how to copy
  smart_ptr_holder(const smart_ptr_holder& /*s*/) :
    m_count(0), m_data(0)
    {
      DEBUG_ASSERT(false);
    }

  smart_ptr_holder& operator=(const smart_ptr_holder& /*s*/)
    {
      DEBUG_ASSERT(false);
      return *this;
    }

public:
  smart_ptr_holder(T* p = 0) : 
    m_count(1), m_data(p)
    {
    }

  ~smart_ptr_holder(void)
    {
      clear();
    }

  unsigned count(void) const
    {
      return m_count;
    }

  void increment(void)
    {
      ++m_count;
    }

  bool decrement(void)
    {
      --m_count;
      return m_count == 0;
    }

  bool null(void)
    {
      return m_data == 0;
    }

  void clear(void)
    {
      if(m_data)
        delete m_data;
      m_data = 0;
    }

  void set(T* p = 0)
    {
      clear();
      m_data = p;
    }

  T*& pointer(void)
    {
      return m_data;
    }

  const T* pointer(void) const
    {
      return m_data;
    }

  T& value(void)
    {
      return *m_data;
    }

  const T& value(void) const
    {
      return *m_data;
    }
};

////////////////////////////////////////////////////////////////////////////////
// smart_ptr class
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// constructors, assignments and destructors

// create a null pointer
template<typename T>
smart_ptr<T>::smart_ptr(void) :
  m_holder(new smart_ptr_holder<T>)
{
}

// create a pointer containing a *copy* of the object pointer
template<typename T>
smart_ptr<T>::smart_ptr(const T& data) :
  m_holder(new smart_ptr_holder<T>)
{
  m_holder->set(new T(data));
}

// delete any old value in the pointer and assign to it a new *copy* of the argument
template<typename T>
smart_ptr<T>& smart_ptr<T>::operator=(const T& data)
{
  m_holder->set(new T(data));
  return *this;
}

// copy constructor implements counted referencing - no copy is made
template<typename T>
smart_ptr<T>::smart_ptr(const smart_ptr<T>& r) :
  m_holder(0)
{
  m_holder = r.m_holder;
  m_holder->increment();
}

// assignment of smart pointers implement counted referencing - no copy is made
template<typename T>
smart_ptr<T>& smart_ptr<T>::operator=(const smart_ptr<T>& r)
{
//   // make it self-copy safe
//   if (this == &r) return *this;
  // make it alias-copy safe - this means that I don't try to do the
  // assignment if r is either the same object or an alias of it
  if (m_holder == r.m_holder) return *this;
  if(m_holder->decrement())
    delete m_holder;
  m_holder = r.m_holder;
  m_holder->increment();
  return *this;
}

// create a pointer containing a dynamically created object
// Note: the object must be allocated *by the user* with new
// constructor form - must be called in the form smart_ptr<type> x(new type(args))
template<typename T>
smart_ptr<T>::smart_ptr(T* data) :
  m_holder(new smart_ptr_holder<T>)
{
  m_holder->set(data);
}

// assignment form
template<typename T>
smart_ptr<T>& smart_ptr<T>::operator=(T* data)
{
  m_holder->set(data);
  return *this;
}

// destructor decrements the reference count and delete only when the last reference is destroyed
template<typename T>
smart_ptr<T>::~smart_ptr(void)
{
  if(m_holder->decrement())
    delete m_holder;
}

//////////////////////////////////////////////////////////////////////////////
// logical tests to see if there is anything contained in the pointer since it can be null

template<typename T>
bool smart_ptr<T>::null(void) const
{
  return m_holder->null();
}

template<typename T>
bool smart_ptr<T>::present(void) const
{
  return !m_holder->null();
}

template<typename T>
bool smart_ptr<T>::operator!(void) const
{
  return m_holder->null();
}

template<typename T>
smart_ptr<T>::operator bool(void) const
{
  return !m_holder->null();
}

//////////////////////////////////////////////////////////////////////////////
// dereference operators and functions

template<typename T>
T& smart_ptr<T>::operator*(void) throw(null_dereference)
{
  if (m_holder->null()) throw null_dereference("null pointer dereferenced in smart_ptr::operator*");
  return m_holder->value();
}

template<typename T>
const T& smart_ptr<T>::operator*(void) const throw(null_dereference)
{
  if (m_holder->null()) throw null_dereference("null pointer dereferenced in smart_ptr::operator*");
  return m_holder->value();
}

template<typename T>
T* smart_ptr<T>::operator->(void) throw(null_dereference)
{
  if (m_holder->null()) throw null_dereference("null pointer dereferenced in smart_ptr::operator->");
  return m_holder->pointer();
}

template<typename T>
const T* smart_ptr<T>::operator->(void) const throw(null_dereference)
{
  if (m_holder->null()) throw null_dereference("null pointer dereferenced in smart_ptr::operator->");
  return m_holder->pointer();
}

//////////////////////////////////////////////////////////////////////////////
// explicit function forms of the above assignment dereference operators

template<typename T>
void smart_ptr<T>::set_value(const T& data)
{
  m_holder->set(new T(data));
}

template<typename T>
T& smart_ptr<T>::value(void) throw(null_dereference)
{
  if (m_holder->null()) throw null_dereference("null pointer dereferenced in smart_ptr::value");
  return m_holder->value();
}

template<typename T>
const T& smart_ptr<T>::value(void) const throw(null_dereference)
{
  if (m_holder->null()) throw null_dereference("null pointer dereferenced in smart_ptr::value");
  return m_holder->value();
}

template<typename T>
void smart_ptr<T>::set(T* data)
{
  m_holder->set(data);
}

template<typename T>
T* smart_ptr<T>::pointer(void)
{
  return m_holder->pointer();
}

template<typename T>
const T* smart_ptr<T>::pointer(void) const
{
  return m_holder->pointer();
}

////////////////////////////////////////////////////////////////////////////////
// functions to manage counted referencing

template<typename T>
bool smart_ptr<T>::aliases(const smart_ptr<T>& r) const
{
  return !m_holder->null() && m_holder->pointer() == r.m_holder->pointer();
}

template<typename T>
unsigned smart_ptr<T>::alias_count(void) const
{
  return m_holder->count();
}

template<typename T>
void smart_ptr<T>::make_unique(void)
{
  if(m_holder->count() > 1)
  {
    smart_ptr_holder<T>* old_holder = m_holder;
    m_holder = new smart_ptr_holder<T>;
    old_holder->decrement();
    if (old_holder->pointer())
      m_holder->set(new T(old_holder->value()));
  }
}

template<typename T>
void smart_ptr<T>::clear(void)
{
  m_holder->clear();
}

template<typename T>
void smart_ptr<T>::clear_unique(void)
{
  if (m_holder->count() == 1)
    m_holder->clear();
  else
  {
    m_holder->decrement();
    m_holder = new smart_ptr_holder<T>;
  }
}

template<typename T>
void smart_ptr<T>::copy(const smart_ptr<T>& data)
{
  *this = data;
  make_unique();
}

template<typename T>
smart_ptr<T> smart_ptr<T>::copy(void) const
{
  smart_ptr<T> result;
  result.copy(*this);
  return result;
}

////////////////////////////////////////////////////////////////////////////////
// persistence methods
// see also - external persistence functions

// Like ordinary pointers they are dumped as an address and then, on the first
// alias only, the contents. Subsequent aliases to the same object are dumped
// as just an address. I use the address of the holder within smart pointer
// since that is always there and is unique. Using the address of the object
// would make all null pointers aliases of each other which may not be
// correct. This requires burglarisation of the smart pointer class.

// I've split these into the exported non-member functions and internal member
// functions because I need access to the data structure but couldn't get
// friend templates to work on VC

// a smart_ptr is used to point to non-clonable objects, so use the standard dump/restore for pointers

template<typename T>
void smart_ptr<T>::dump(dump_context& context) const throw(persistent_dump_failed)
{
  // Many smart pointers can point to the same object.
  // I could have used the address of the object to differentiate, 
  // but that would not have differentiated between different null smart pointers
  // so I use the address of the substructure to differentiate between different objects.
  // get a magic key for the substructure - this also returns a flag saying whether its been seen before
  std::pair<bool,unsigned> mapping = context.pointer_map(m_holder);
  // dump the magic key
  ::dump(context,mapping.second);
  // dump the contents but only if this is the first time this object has been seen
  // use the existing routines for ordinary pointers to dump the contents
  if (!mapping.first)
    dump_pointer(context,m_holder->pointer());
}

template<typename T>
void smart_ptr<T>::restore(restore_context& context) throw(persistent_restore_failed)
{
  // get the old substructure magic key
  unsigned magic = 0;
  ::restore(context,magic);
  // lookup this magic number to see if we have seen this already
  std::pair<bool,void*> mapping = context.pointer_map(magic);
  if (mapping.first)
  {
    // this holder has already been restored
    // dealias the existing holder and replace it with the seen-before holder to make this object an alias of it
    if (m_holder->decrement())
      delete m_holder;
    m_holder = (smart_ptr_holder<T>*)mapping.second;
    m_holder->increment();
  }
  else
  {
    // this is the first contact with this holder
    // make sure this smart pointer is unique to prevent side-effects
    clear_unique();
    // map the magic key onto this structure's holder and then restore the contents
    context.pointer_add(magic,m_holder);
    restore_pointer(context,m_holder->pointer());
  }
}

////////////////////////////////////////////////////////////////////////////////
// comparisons required for using this class in an STL container

template<typename T>
bool operator==(const smart_ptr<T>& left, const smart_ptr<T>& right)
{
  // a null is not equal to a non-null but equal to another null
  if(!left || !right) return left.pointer() == right.pointer();
  // shortcut - if the two pointers are equal then the objects must be equal
  if (left.pointer() == right.pointer()) return true;
  // otherwise compare the objects themselves
  return *left == *right;
}

template<typename T>
bool operator<(const smart_ptr<T>& left, const smart_ptr<T>& right)
{
  // a null pointer is less than a non-null but equal to another null
  if(!left || !right) return left.pointer() < right.pointer();
  // shortcut - if the two pointers are equal then the comparison must be false
  if (left.pointer() == right.pointer()) return false;
  // otherwise, compare the objects
  return *left < *right;
}

////////////////////////////////////////////////////////////////////////////////
// string/print utilities

template<typename T>
std::string smart_ptr_to_string(const smart_ptr<T>& ptr, std::string null_string)
{
  if (!ptr) return null_string;
  return "*(" + to_string(*ptr) + ")";
}

template<typename T>
otext& print_smart_ptr(otext& str, const smart_ptr<T>& ptr, std::string null_string)
{
  if (!ptr) return str << null_string;
  str << "*(";
  print(str, *ptr);
  str << ")";
  return str;
}

template<typename T>
otext& print_smart_ptr(otext& str, const smart_ptr<T>& ptr, unsigned indent, std::string null_string)
{
  print_indent(str, indent);
  print_smart_ptr(str, ptr, null_string);
  return str << endl;
}

////////////////////////////////////////////////////////////////////////////////
// Persistence - non-member functions actually called by the user

template<typename T>
void dump_smart_ptr(dump_context& context, const smart_ptr<T>& data) throw(persistent_dump_failed)
{
  data.dump(context);
}

template<typename T>
void restore_smart_ptr(restore_context& context, smart_ptr<T>& data) throw(persistent_restore_failed)
{
  data.restore(context);
}

////////////////////////////////////////////////////////////////////////////////
// smart_ptr_clone class
////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// constructors, assignments and destructors

// create a null pointer
template<typename T>
smart_ptr_clone<T>::smart_ptr_clone(void) :
  m_holder(new smart_ptr_holder<T>)
{
}

// create a pointer containing a *copy* of the object pointer
template<typename T>
smart_ptr_clone<T>::smart_ptr_clone(const T& data) :
  m_holder(new smart_ptr_holder<T>)
{
  m_holder->set((T*)data.clone());
}

// delete any old value in the pointer and assign to it a new *copy* of the argument
template<typename T>
smart_ptr_clone<T>& smart_ptr_clone<T>::operator=(const T& data)
{
  m_holder->set((T*)data.clone());
  return *this;
}

// copy constructor implements counted referencing - no copy is made
template<typename T>
smart_ptr_clone<T>::smart_ptr_clone(const smart_ptr_clone<T>& r) :
  m_holder(0)
{
  m_holder = r.m_holder;
  m_holder->increment();
}

// assignment of smart pointers implement counted referencing - no copy is made
template<typename T>
smart_ptr_clone<T>& smart_ptr_clone<T>::operator=(const smart_ptr_clone<T>& r)
{
//   // make it self-copy safe
//   if (this == &r) return *this;
  // make it alias-copy safe - this means that I don't try to do the
  // assignment if r is either the same object or an alias of it
  if (m_holder == r.m_holder) return *this;
  if(m_holder->decrement())
    delete m_holder;
  m_holder = r.m_holder;
  m_holder->increment();
  return *this;
}

// create a pointer containing a *copy* of the object
// this copy is taken because the pointer class maintains a dynamically allocated object 
// and the T& may not be (usually is not) dynamically allocated
// constructor form
template<typename T>
smart_ptr_clone<T>::smart_ptr_clone(T* data) :
  m_holder(new smart_ptr_holder<T>)
{
  m_holder->set(data);
}

// assignment form for an already-constructed smart-pointer
template<typename T>
smart_ptr_clone<T>& smart_ptr_clone<T>::operator=(T* data)
{
  m_holder->set(data);
  return *this;
}

// destructor decrements the reference count and delete only when the last reference is destroyed
template<typename T>
smart_ptr_clone<T>::~smart_ptr_clone(void)
{
  if(m_holder->decrement())
    delete m_holder;
}

//////////////////////////////////////////////////////////////////////////////
// logical tests to see if there is anything contained in the pointer since it can be null

template<typename T>
bool smart_ptr_clone<T>::null(void) const
{
  return m_holder->null();
}

template<typename T>
bool smart_ptr_clone<T>::present(void) const
{
  return !m_holder->null();
}

template<typename T>
bool smart_ptr_clone<T>::operator!(void) const
{
  return m_holder->null();
}

template<typename T>
smart_ptr_clone<T>::operator bool(void) const
{
  return !m_holder->null();
}

//////////////////////////////////////////////////////////////////////////////
// dereference operators and functions

template<typename T>
T& smart_ptr_clone<T>::operator*(void) throw(null_dereference)
{
  if (m_holder->null()) throw null_dereference("null pointer dereferenced in smart_ptr_clone::operator*");
  return m_holder->value();
}

template<typename T>
const T& smart_ptr_clone<T>::operator*(void) const throw(null_dereference)
{
  if (m_holder->null()) throw null_dereference("null pointer dereferenced in smart_ptr_clone::operator*");
  return m_holder->value();
}

template<typename T>
T* smart_ptr_clone<T>::operator->(void) throw(null_dereference)
{
  if (m_holder->null()) throw null_dereference("null pointer dereferenced in smart_ptr_clone::operator->");
  return m_holder->pointer();
}

template<typename T>
const T* smart_ptr_clone<T>::operator->(void) const throw(null_dereference)
{
  if (m_holder->null()) throw null_dereference("null pointer dereferenced in smart_ptr_clone::operator->");
  return m_holder->pointer();
}

//////////////////////////////////////////////////////////////////////////////
// explicit function forms of the above assignment dereference operators

template<typename T>
void smart_ptr_clone<T>::set_value(const T& data)
{
  m_holder->set((T*)data.clone());
}

template<typename T>
T& smart_ptr_clone<T>::value(void) throw(null_dereference)
{
  if (m_holder->null()) throw null_dereference("null pointer dereferenced in smart_ptr_clone::value");
  return m_holder->value();
}

template<typename T>
const T& smart_ptr_clone<T>::value(void) const throw(null_dereference)
{
  if (m_holder->null()) throw null_dereference("null pointer dereferenced in smart_ptr_clone::value");
  return m_holder->value();
}

template<typename T>
void smart_ptr_clone<T>::set(T* data)
{
  m_holder->set(data);
}

template<typename T>
T* smart_ptr_clone<T>::pointer(void)
{
  return m_holder->pointer();
}

template<typename T>
const T* smart_ptr_clone<T>::pointer(void) const
{
  return m_holder->pointer();
}

////////////////////////////////////////////////////////////////////////////////
// functions to manage counted referencing

template<typename T>
bool smart_ptr_clone<T>::aliases(const smart_ptr_clone<T>& r) const
{
  return !m_holder->null() && m_holder->pointer() == r.m_holder->pointer();
}

template<typename T>
unsigned smart_ptr_clone<T>::alias_count(void) const
{
  return m_holder->count();
}

template<typename T>
void smart_ptr_clone<T>::make_unique(void)
{
  if(m_holder->count() > 1)
  {
    smart_ptr_holder<T>* old_holder = m_holder;
    m_holder = new smart_ptr_holder<T>;
    old_holder->decrement();
    if (old_holder->pointer())
      m_holder->set((T*)(old_holder->pointer()->clone()));
  }
}

template<typename T>
void smart_ptr_clone<T>::clear(void)
{
  m_holder->clear();
}

template<typename T>
void smart_ptr_clone<T>::clear_unique(void)
{
  if (m_holder->count() == 1)
    m_holder->clear();
  else
  {
    m_holder->decrement();
    m_holder = new smart_ptr_holder<T>;
  }
}

template<typename T>
void smart_ptr_clone<T>::copy(const smart_ptr_clone<T>& data)
{
  *this = data;
  make_unique();
}

template<typename T>
smart_ptr_clone<T> smart_ptr_clone<T>::copy(void) const
{
  smart_ptr_clone<T> result;
  result.copy(*this);
  return result;
}

////////////////////////////////////////////////////////////////////////////////
// persistence methods
// see also - external persistence functions

// Like ordinary pointers they are dumped as an address and then, on the first
// alias only, the contents. Subsequent aliases to the same object are dumped
// as just an address. I use the address of the holder within smart pointer
// since that is always there and is unique. Using the address of the object
// would make all null pointers aliases of each other which may not be
// correct. This requires burglarisation of the smart pointer class.

// I've split these into the exported non-member functions and internal member
// functions because I need access to the data structure but couldn't get
// friend templates to work on VC

// a smart_ptr_clone is used to point to clonable objects, so use the dump/restore for the clonable interface

template<typename T>
void smart_ptr_clone<T>::dump(dump_context& context) const throw(persistent_dump_failed)
{
  // Many smart pointers can point to the same object.
  // I could have used the address of the object to differentiate, 
  // but that would not have differentiated between different null smart pointers
  // so I use the address of the substructure to differentiate between different objects.
  // get a magic key for the substructure - this also returns a flag saying whether its been seen before
  std::pair<bool,unsigned> mapping = context.pointer_map(m_holder);
  // dump the magic key
  ::dump(context,mapping.second);
  // dump the contents but only if this is the first time this object has been seen
  // use the existing routines for ordinary pointers to dump the contents
  if (!mapping.first)
    dump_interface(context,m_holder->pointer());
}

template<typename T>
void smart_ptr_clone<T>::restore(restore_context& context) throw(persistent_restore_failed)
{
  // get the old substructure magic key
  unsigned magic = 0;
  ::restore(context,magic);
  // lookup this magic number to see if we have seen this already
  std::pair<bool,void*> mapping = context.pointer_map(magic);
  if (mapping.first)
  {
    // this holder has already been restored
    // dealias the existing holder and replace it with the seen-before holder to make this object an alias of it
    if (m_holder->decrement())
      delete m_holder;
    m_holder = (smart_ptr_holder<T>*)mapping.second;
    m_holder->increment();
  }
  else
  {
    // this is the first contact with this holder
    // make sure this smart pointer is unique to prevent side-effects
    clear_unique();
    // map the magic key onto this structure's holder and then restore the contents
    context.pointer_add(magic,m_holder);
    restore_interface(context,m_holder->pointer());
  }
}

////////////////////////////////////////////////////////////////////////////////
// comparisons required for using this class in an STL container

template<typename T>
bool operator==(const smart_ptr_clone<T>& left, const smart_ptr_clone<T>& right)
{
  // a null is not equal to a non-null but equal to another null
  if(!left || !right) return left.pointer() == right.pointer();
  // shortcut - if the two pointers are equal then the objects must be equal
  if (left.pointer() == right.pointer()) return true;
  // otherwise compare the objects themselves
  return *left == *right;
}

template<typename T>
bool operator<(const smart_ptr_clone<T>& left, const smart_ptr_clone<T>& right)
{
  // a null pointer is less than a non-null but equal to another null
  if(!left || !right) return left.pointer() < right.pointer();
  // shortcut - if the two pointers are equal then the comparison must be false
  if (left.pointer() == right.pointer()) return false;
  // otherwise, compare the objects
  return *left < *right;
}

////////////////////////////////////////////////////////////////////////////////
// string/print utilities

template<typename T>
std::string smart_ptr_clone_to_string(const smart_ptr_clone<T>& ptr, std::string null_string)
{
  if (!ptr) return null_string;
  return "*(" + to_string(*ptr) + ")";
}

template<typename T>
otext& print_smart_ptr_clone(otext& str, const smart_ptr_clone<T>& ptr, std::string null_string)
{
  if (!ptr) return str << null_string;
  str << "*(";
  print(str, *ptr);
  str << ")";
  return str;
}

template<typename T>
otext& print_smart_ptr_clone(otext& str, const smart_ptr_clone<T>& ptr, unsigned indent, std::string null_string)
{
  print_indent(str, indent);
  print_smart_ptr_clone(str, ptr, null_string);
  return str << endl;
}

////////////////////////////////////////////////////////////////////////////////
// Persistence - non-member functions actually called by the user

template<typename T>
void dump_smart_ptr_clone(dump_context& context, const smart_ptr_clone<T>& data) throw(persistent_dump_failed)
{
  data.dump(context);
}

template<typename T>
void restore_smart_ptr_clone(restore_context& context, smart_ptr_clone<T>& data) throw(persistent_restore_failed)
{
  data.restore(context);
}

////////////////////////////////////////////////////////////////////////////////
// smart_ptr_nocopy class
////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// constructors, assignments and destructors

// create a null pointer
template<typename T>
smart_ptr_nocopy<T>::smart_ptr_nocopy(void) :
  m_holder(new smart_ptr_holder<T>)
{
}

// copy constructor implements counted referencing - no copy is made
template<typename T>
smart_ptr_nocopy<T>::smart_ptr_nocopy(const smart_ptr_nocopy<T>& r) :
  m_holder(0)
{
  m_holder = r.m_holder;
  m_holder->increment();
}

// assignment of smart pointers implement counted referencing - no copy is made
template<typename T>
smart_ptr_nocopy<T>& smart_ptr_nocopy<T>::operator=(const smart_ptr_nocopy<T>& r)
{
//   // make it self-copy safe
//   if (this == &r) return *this;
  // make it alias-copy safe - this means that I don't try to do the
  // assignment if r is either the same object or an alias of it
  if (m_holder == r.m_holder) return *this;
  if(m_holder->decrement())
    delete m_holder;
  m_holder = r.m_holder;
  m_holder->increment();
  return *this;
}

// constructor form
template<typename T>
smart_ptr_nocopy<T>::smart_ptr_nocopy(T* data) :
  m_holder(new smart_ptr_holder<T>)
{
  m_holder->set(data);
}

// assignment form
template<typename T>
smart_ptr_nocopy<T>& smart_ptr_nocopy<T>::operator=(T* data)
{
  m_holder->set(data);
  return *this;
}

// destructor decrements the reference count and delete only when the last reference is destroyed
template<typename T>
smart_ptr_nocopy<T>::~smart_ptr_nocopy(void)
{
  if(m_holder->decrement())
    delete m_holder;
}

//////////////////////////////////////////////////////////////////////////////
// logical tests to see if there is anything contained in the pointer since it can be null

template<typename T>
bool smart_ptr_nocopy<T>::null(void) const
{
  return m_holder->null();
}

template<typename T>
bool smart_ptr_nocopy<T>::present(void) const
{
  return !m_holder->null();
}

template<typename T>
bool smart_ptr_nocopy<T>::operator!(void) const
{
  return m_holder->null();
}

template<typename T>
smart_ptr_nocopy<T>::operator bool(void) const
{
  return !m_holder->null();
}

//////////////////////////////////////////////////////////////////////////////
// dereference operators and functions

template<typename T>
T& smart_ptr_nocopy<T>::operator*(void) throw(null_dereference)
{
  if (m_holder->null()) throw null_dereference("null pointer dereferenced in smart_ptr_nocopy::operator*");
  return m_holder->value();
}

template<typename T>
const T& smart_ptr_nocopy<T>::operator*(void) const throw(null_dereference)
{
  if (m_holder->null()) throw null_dereference("null pointer dereferenced in smart_ptr_nocopy::operator*");
  return m_holder->value();
}

template<typename T>
T* smart_ptr_nocopy<T>::operator->(void) throw(null_dereference)
{
  if (m_holder->null()) throw null_dereference("null pointer dereferenced in smart_ptr_nocopy::operator->");
  return m_holder->pointer();
}

template<typename T>
const T* smart_ptr_nocopy<T>::operator->(void) const throw(null_dereference)
{
  if (m_holder->null()) throw null_dereference("null pointer dereferenced in smart_ptr_nocopy::operator->");
  return m_holder->pointer();
}

//////////////////////////////////////////////////////////////////////////////
// explicit function forms of the above assignment dereference operators

template<typename T>
T& smart_ptr_nocopy<T>::value(void) throw(null_dereference)
{
  if (m_holder->null()) throw null_dereference("null pointer dereferenced in smart_ptr_nocopy::value");
  return m_holder->value();
}

template<typename T>
const T& smart_ptr_nocopy<T>::value(void) const throw(null_dereference)
{
  if (m_holder->null()) throw null_dereference("null pointer dereferenced in smart_ptr_nocopy::value");
  return m_holder->value();
}

template<typename T>
void smart_ptr_nocopy<T>::set(T* data)
{
  m_holder->set(data);
}

template<typename T>
T* smart_ptr_nocopy<T>::pointer(void)
{
  return m_holder->pointer();
}

template<typename T>
const T* smart_ptr_nocopy<T>::pointer(void) const
{
  return m_holder->pointer();
}

////////////////////////////////////////////////////////////////////////////////
// functions to manage counted referencing

template<typename T>
bool smart_ptr_nocopy<T>::aliases(const smart_ptr_nocopy<T>& r) const
{
  // aliases have the same holder
  return m_holder == r.m_holder();
}

template<typename T>
unsigned smart_ptr_nocopy<T>::alias_count(void) const
{
  return m_holder->count();
}

template<typename T>
void smart_ptr_nocopy<T>::clear(void)
{
  m_holder->clear();
}

template<typename T>
void smart_ptr_nocopy<T>::clear_unique(void)
{
  if (m_holder->count() == 1)
    m_holder->clear();
  else
  {
    m_holder->decrement();
    m_holder = new smart_ptr_holder<T>;
  }
}

////////////////////////////////////////////////////////////////////////////////
// persistence methods
// a smart_ptr_nocopy is used to point to non-copyable objects, so it doesn't make sense to provide peersistence!

////////////////////////////////////////////////////////////////////////////////
// comparisons required for using this class in an STL container

template<typename T>
bool operator==(const smart_ptr_nocopy<T>& left, const smart_ptr_nocopy<T>& right)
{
  // a null is not equal to a non-null but equal to another null
  if(!left || !right) return left.pointer() == right.pointer();
  // shortcut - if the two pointers are equal then the objects must be equal
  if (left.pointer() == right.pointer()) return true;
  // otherwise compare the objects themselves
  return *left == *right;
}

template<typename T>
bool operator<(const smart_ptr_nocopy<T>& left, const smart_ptr_nocopy<T>& right)
{
  // a null pointer is less than a non-null but equal to another null
  if(!left || !right) return left.pointer() < right.pointer();
  // shortcut - if the two pointers are equal then the comparison must be false
  if (left.pointer() == right.pointer()) return false;
  // otherwise, compare the objects
  return *left < *right;
}

////////////////////////////////////////////////////////////////////////////////
// string/print utilities

template<typename T>
std::string smart_ptr_nocopy_to_string(const smart_ptr_nocopy<T>& ptr, std::string null_string)
{
  if (!ptr) return null_string;
  return "*(" + to_string(*ptr) + ")";
}

template<typename T>
otext& print_smart_ptr_nocopy(otext& str, const smart_ptr_nocopy<T>& ptr, std::string null_string)
{
  if (!ptr) return str << null_string;
  str << "*(";
  print(str, *ptr);
  str << ")";
  return str;
}

template<typename T>
otext& print_smart_ptr_nocopy(otext& str, const smart_ptr_nocopy<T>& ptr, unsigned indent, std::string null_string)
{
  print_indent(str, indent);
  print_smart_ptr_nocopy(str, ptr, null_string);
  return str << endl;
}

////////////////////////////////////////////////////////////////////////////////
