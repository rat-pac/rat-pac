#ifndef SMART_PTR_HPP
#define SMART_PTR_HPP
/*------------------------------------------------------------------------------

  Author:    Andy Rushton and Daniel Milton
  Copyright: (c) Andy Rushton, 2004; Daniel Milton 2005
  License:   BSD License, see ../docs/license.html

  Dan Milton: three simple pointer containers with single level access:
    - simple_ptr for simple types and classes
    - simple_ptr_clone for polymorphic class hierarchies
    - simple_ptr_nocopy for any class that cannot or should no be copied

  Andy Rushton: three smart pointer containers with two-layer access:
    - smart_ptr for simple types and classes
    - smart_ptr_clone for polymorphic class hierarchies
    - smart_ptr_nocopy for any class that cannot or should no be copied

  ------------------------------------------------------------------------------*/
#include "os_fixes.hpp"
#include "exceptions.hpp"
#include "persistent.hpp"
#include "textio.hpp"
#include <map>
#include <string>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Simple pointer class
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template<typename T>
class simple_ptr
{
public:
  //////////////////////////////////////////////////////////////////////////////
  // member type definitions

  typedef T value_type;
  typedef T& reference;
  typedef const T& const_reference;

  //////////////////////////////////////////////////////////////////////////////
  // constructors, assignments and destructors

  // create a null pointer
  simple_ptr(void);

  // create a pointer containing a *copy* of the object
  // this copy is taken because the pointer class maintains a dynamically allocated object 
  // and the T& may not be (usually is not) dynamically allocated
  // constructor form
  simple_ptr(const T& data);
  // assignment form for an already-constructed smart-pointer
  simple_ptr<T>& operator=(const T& data);

  // copy constructor implements counted referencing - no copy is made
  simple_ptr(const simple_ptr<T>& r);
  // assignment of smart pointers implement counted referencing - no copy is made
  simple_ptr<T>& operator=(const simple_ptr<T>&);

  // create a pointer containing a dynamically created object
  // Note: the object must be allocated *by the user* with new
  // constructor form - must be called in the form simple_ptr<type> x(new type(args))
  explicit simple_ptr(T* data);
  // assignment form
  simple_ptr<T>& operator= (T* data);

  // destructor decrements the reference count and delete only when the last reference is destroyed
  ~simple_ptr(void);

  //////////////////////////////////////////////////////////////////////////////
  // logical tests to see if there is anything contained in the pointer since it can be null

  // there are two forms:explicit and implicit
  // implicit: if(!r) or if(r)
  // explicit: if(r.null()) or if(r.present())
  operator bool(void) const;
  bool operator!(void) const;
  bool present(void) const;
  bool null(void) const;

  //////////////////////////////////////////////////////////////////////////////
  // dereference operators and functions

  // dereference the smart pointer to get the object - use in the form *p1
  T& operator*(void) throw(null_dereference);
  const T& operator*(void) const throw(null_dereference);

  // used as a prefix to a member access to the contained object e.g. p1->print() calls T::print()
  T* operator->(void) throw(null_dereference);
  const T* operator->(void) const throw(null_dereference);

  //////////////////////////////////////////////////////////////////////////////
  // explicit function forms of the above assignment and dereference operators

  // set the value
  void set_value(const T& data);
  // get the value
  T& value(void) throw(null_dereference);
  const T& value(void) const throw(null_dereference);

  // set the pointer
  // deletes the previous pointer and adopts the passed pointer instead
  // Note: the object must be allocated *by the user* with new
  // Warning: it is very easy to break the memory management with this operation
  void set(T* data = 0);
  // get the pointer
  T* pointer(void);
  const T* pointer(void) const;

  //////////////////////////////////////////////////////////////////////////////
  // functions to manage counted referencing

  // test whether two pointers point to the same object(known as aliasing the object)
  // used in the form if(a.aliases(b))
  bool aliases(const simple_ptr<T>&) const;

  // find the number of aliases - used when you need to know whether an object is still referred to from elsewhere (rare!)
  unsigned alias_count(void) const;

  // make this pointer unique with respect to any other references to the same object
  // if this pointer is already unique, it does nothing - otherwise it copies the object
  void make_unique(void);

  // delete the object and make the pointer null - does not make it unique first, so all other pointers to this will be null too
  void clear(void);

  // make the pointer unique and null in one step - does not affect other pointers that were pointing to the same object
  void clear_unique(void);

  // make this pointer a unique copy of the parameter
  // useful for expressions like p1.copy(p2) which makes p1 a pointer to a unique copy of the contents of p2
  void copy(const simple_ptr<T>&);
  // alternate form used in assignments: p1 = ps.copy()
  simple_ptr<T> copy(void) const;

  // persistence functions
  void dump(dump_context& str) const throw(persistent_dump_failed);
  void restore(restore_context& str) throw(persistent_restore_failed);

protected:
  T* m_pointer;
  unsigned* m_count;
};

////////////////////////////////////////////////////////////////////////////////
// comparisons required for using this class in an STL container
// These require == and < operator in the contained type T
// the remaining relational operators are provided by template functions
// a null pointer is less-than a non-null, two nulls are equal
// these funcions are defined as non-members so that you only need provide
// the underlying T::operator< and == if you are going to use these functions

template<typename T>
bool operator==(const simple_ptr<T>&, const simple_ptr<T>&);

template<typename T>
bool operator<(const simple_ptr<T>&, const simple_ptr<T>&);

////////////////////////////////////////////////////////////////////////////////
// string/print utilities

template<typename T>
std::string simple_ptr_to_string(const simple_ptr<T>& ptr, std::string null_string);

template<typename T>
otext& print_simple_ptr(otext& str, const simple_ptr<T>& ptr, std::string null_string);

template<typename T>
otext& print_simple_ptr(otext& str, const simple_ptr<T>& ptr, unsigned indent, std::string null_string);

////////////////////////////////////////////////////////////////////////////////
// persistence - call these rather than the methods
// the dump routine dumps simple_ptr-specific information and then calls dump_pointer on the contents
// similarly the restore routine calls restore_pointer
// so therefore the class T should have non-member dump/restore functions

template<typename T>
void dump_simple_ptr(dump_context& str, const simple_ptr<T>& data) throw(persistent_dump_failed);

template<typename T>
void restore_simple_ptr(restore_context& str, simple_ptr<T>& data) throw(persistent_restore_failed);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Cloning simple pointer class for polymorphic class hierarchies
// The contained class T should implement the clonable interface defined in clonable.hpp
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template<typename T>
class simple_ptr_clone
{
public:
  //////////////////////////////////////////////////////////////////////////////
  // member type definitions

  typedef T value_type;
  typedef T& reference;
  typedef const T& const_reference;

  //////////////////////////////////////////////////////////////////////////////
  // constructors, assignments and destructors

  // create a null pointer
  simple_ptr_clone(void);

  // create a pointer containing a *copy* of the object
  // this copy is taken because the pointer class maintains a dynamically allocated object 
  // and the T& may not be (usually is not) dynamically allocated
  // constructor form
  simple_ptr_clone(const T& data);
  // assignment form for an already-constructed smart-pointer
  simple_ptr_clone<T>& operator=(const T& data);

  // copy constructor implements counted referencing - no copy is made
  simple_ptr_clone(const simple_ptr_clone<T>& r);
  // assignment of smart pointers implement counted referencing - no copy is made
  simple_ptr_clone<T>& operator=(const simple_ptr_clone<T>&);

  // create a pointer containing a dynamically created object
  // Note: the object must be allocated *by the user* with new
  // constructor form - must be called in the form simple_ptr_clone<type> x(new type(args))
  explicit simple_ptr_clone(T* data);
  // assignment form
  simple_ptr_clone<T>& operator= (T* data);

  // destructor decrements the reference count and delete only when the last reference is destroyed
  ~simple_ptr_clone(void);

  //////////////////////////////////////////////////////////////////////////////
  // logical tests to see if there is anything contained in the pointer since it can be null

  // there are two forms:explicit and implicit
  // implicit: if(!r) or if(r)
  // explicit: if(r.null()) or if(r.present())
  operator bool(void) const;
  bool operator!(void) const;
  bool present(void) const;
  bool null(void) const;

  //////////////////////////////////////////////////////////////////////////////
  // dereference operators and functions

  // dereference the smart pointer to get the object - use in the form *p1
  T& operator*(void) throw(null_dereference);
  const T& operator*(void) const throw(null_dereference);

  // used as a prefix to a member access to the contained object e.g. p1->print() calls T::print()
  T* operator->(void) throw(null_dereference);
  const T* operator->(void) const throw(null_dereference);

  //////////////////////////////////////////////////////////////////////////////
  // explicit function forms of the above assignment and dereference operators

  // set the value
  void set_value(const T& data);
  // get the value
  T& value(void) throw(null_dereference);
  const T& value(void) const throw(null_dereference);

  // set the pointer
  // deletes the previous pointer and adopts the passed pointer instead
  // Note: the object must be allocated *by the user* with new
  // Warning: it is very easy to break the memory management with this operation
  void set(T* data = 0);
  // get the pointer
  T* pointer(void);
  const T* pointer(void) const;

  //////////////////////////////////////////////////////////////////////////////
  // functions to manage counted referencing

  // test whether two pointers point to the same object(known as aliasing the object)
  // used in the form if(a.aliases(b))
  bool aliases(const simple_ptr_clone<T>&) const;

  // find the number of aliases - used when you need to know whether an object is still referred to from elsewhere (rare!)
  unsigned alias_count(void) const;

  // make this pointer unique with respect to any other references to the same object
  // if this pointer is already unique, it does nothing - otherwise it copies the object
  void make_unique(void);

  // delete the object and make the pointer null - does not make it unique first, so all other pointers to this will be null too
  void clear(void);

  // make the pointer unique and null in one step - does not affect other pointers that were pointing to the same object
  void clear_unique(void);

  // make this pointer a unique copy of the parameter
  // useful for expressions like p1.copy(p2) which makes p1 a pointer to a unique copy of the contents of p2
  void copy(const simple_ptr_clone<T>&);
  // alternate form used in assignments: p1 = ps.copy()
  simple_ptr_clone<T> copy(void) const;

  // persistence functions
  void dump(dump_context& str) const throw(persistent_dump_failed);
  void restore(restore_context& str) throw(persistent_restore_failed);

protected:
  T* m_pointer;
  unsigned* m_count;
};

////////////////////////////////////////////////////////////////////////////////
// comparisons required for using this class in an STL container
// These require == and < operator in the contained type T
// the remaining relational operators are provided by template functions
// a null pointer is less-than a non-null, two nulls are equal
// these funcions are defined as non-members so that you only need provide
// the underlying T::operator< and == if you are going to use these functions

template<typename T>
bool operator==(const simple_ptr_clone<T>&, const simple_ptr_clone<T>&);

template<typename T>
bool operator<(const simple_ptr_clone<T>&, const simple_ptr_clone<T>&);

////////////////////////////////////////////////////////////////////////////////
// string/print utilities

template<typename T>
std::string simple_ptr_clone_to_string(const simple_ptr_clone<T>& ptr, std::string null_string);

template<typename T>
otext& print_simple_ptr_clone(otext& str, const simple_ptr_clone<T>& ptr, std::string null_string);

template<typename T>
otext& print_simple_ptr_clone(otext& str, const simple_ptr_clone<T>& ptr, unsigned indent, std::string null_string);

////////////////////////////////////////////////////////////////////////////////
// persistence - call these rather than the methods
// the dump routine dumps simple_ptr_clone-specific information and then calls dump_interface
// similarly the restore routine calls restore_interface
// so therefore the class T should implement the persistent interface defined by the class persistent in persistent.hpp

template<typename T>
void dump_simple_ptr_clone(dump_context& str, const simple_ptr_clone<T>& data) throw(persistent_dump_failed);

template<typename T>
void restore_simple_ptr_clone(restore_context& str, simple_ptr_clone<T>& data) throw(persistent_restore_failed);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// No-copy simple pointer class for managing objects that cannot be copied
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template<typename T>
class simple_ptr_nocopy
{
public:
  //////////////////////////////////////////////////////////////////////////////
  // member type definitions

  typedef T value_type;
  typedef T& reference;
  typedef const T& const_reference;

  //////////////////////////////////////////////////////////////////////////////
  // constructors, assignments and destructors

  // create a null pointer
  simple_ptr_nocopy(void);

  // copy constructor implements counted referencing - no copy is made
  simple_ptr_nocopy(const simple_ptr_nocopy<T>& r);
  // assignment of smart pointers implement counted referencing - no copy is made
  simple_ptr_nocopy<T>& operator=(const simple_ptr_nocopy<T>&);

  // create a pointer containing a dynamically created object
  // Note: the object must be allocated *by the user* with new
  // constructor form - must be called in the form simple_ptr_nocopy<type> x(new type(args))
  explicit simple_ptr_nocopy(T* data);
  // assignment form
  simple_ptr_nocopy<T>& operator= (T* data);

  // destructor decrements the reference count and delete only when the last reference is destroyed
  ~simple_ptr_nocopy(void);

  //////////////////////////////////////////////////////////////////////////////
  // logical tests to see if there is anything contained in the pointer since it can be null

  // there are two forms:explicit and implicit
  // implicit: if(!r) or if(r)
  // explicit: if(r.null()) or if(r.present())
  operator bool(void) const;
  bool operator!(void) const;
  bool present(void) const;
  bool null(void) const;

  //////////////////////////////////////////////////////////////////////////////
  // dereference operators and functions

  // dereference the smart pointer to get the object - use in the form *p1
  T& operator*(void) throw(null_dereference);
  const T& operator*(void) const throw(null_dereference);

  // used as a prefix to a member access to the contained object e.g. p1->print() calls T::print()
  T* operator->(void) throw(null_dereference);
  const T* operator->(void) const throw(null_dereference);

  //////////////////////////////////////////////////////////////////////////////
  // explicit function forms of the above assignment and dereference operators

  // get the value
  T& value(void) throw(null_dereference);
  const T& value(void) const throw(null_dereference);

  // set the pointer
  // deletes the previous pointer and adopts the passed pointer instead
  // Note: the object must be allocated *by the user* with new
  // Warning: it is very easy to break the memory management with this operation
  void set(T* data = 0);
  // get the pointer
  T* pointer(void);
  const T* pointer(void) const;

  //////////////////////////////////////////////////////////////////////////////
  // functions to manage counted referencing

  // test whether two pointers point to the same object(known as aliasing the object)
  // used in the form if(a.aliases(b))
  bool aliases(const simple_ptr_nocopy<T>&) const;

  // find the number of aliases - used when you need to know whether an object is still referred to from elsewhere (rare!)
  unsigned alias_count(void) const;

  // delete the object and make the pointer null - does not make it unique first, so all other pointers to this will be null too
  void clear(void);

  // make the pointer unique and null in one step - does not affect other pointers that were pointing to the same object
  //void clear_unique(void); FIXME

protected:
  T* m_pointer;
  unsigned* m_count;
};

////////////////////////////////////////////////////////////////////////////////
// comparisons required for using this class in an STL container
// These require == and < operator in the contained type T
// the remaining relational operators are provided by template functions
// a null pointer is less-than a non-null, two nulls are equal
// these funcions are defined as non-members so that you only need provide
// the underlying T::operator< and == if you are going to use these functions

template<typename T>
bool operator==(const simple_ptr_nocopy<T>&, const simple_ptr_nocopy<T>&);

template<typename T>
bool operator<(const simple_ptr_nocopy<T>&, const simple_ptr_nocopy<T>&);

////////////////////////////////////////////////////////////////////////////////
// string/print utilities

template<typename T>
std::string simple_ptr_nocopy_to_string(const simple_ptr_nocopy<T>& ptr, std::string null_string);

template<typename T>
otext& print_simple_ptr_nocopy(otext& str, const simple_ptr_nocopy<T>& ptr, std::string null_string);

template<typename T>
otext& print_simple_ptr_nocopy(otext& str, const simple_ptr_nocopy<T>& ptr, unsigned indent, std::string null_string);

////////////////////////////////////////////////////////////////////////////////
// there's no persistence on simple_ptr_nocopy because the whole point is that
// it stores an uncopyable object and persistence is a form of copying

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Simple smart pointer class
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// internals

template<typename T> class smart_ptr_holder;

////////////////////////////////////////////////////////////////////////////////

template<typename T>
class smart_ptr
{
public:
  //////////////////////////////////////////////////////////////////////////////
  // member type definitions

  typedef T value_type;
  typedef T& reference;
  typedef const T& const_reference;

  //////////////////////////////////////////////////////////////////////////////
  // constructors, assignments and destructors

  // create a null pointer
  smart_ptr(void);

  // create a pointer containing a *copy* of the object
  // this copy is taken because the pointer class maintains a dynamically allocated object 
  // and the T& may not be (usually is not) dynamically allocated
  // constructor form
  smart_ptr(const T& data);
  // assignment form for an already-constructed smart-pointer
  smart_ptr<T>& operator=(const T& data);

  // copy constructor implements counted referencing - no copy is made
  smart_ptr(const smart_ptr<T>& r);
  // assignment of smart pointers implement counted referencing - no copy is made
  smart_ptr<T>& operator=(const smart_ptr<T>&);

  // create a pointer containing a dynamically created object
  // Note: the object must be allocated *by the user* with new
  // constructor form - must be called in the form smart_ptr<type> x(new type(args))
  explicit smart_ptr(T* data);
  // assignment form
  smart_ptr<T>& operator= (T* data);

  // destructor decrements the reference count and delete only when the last reference is destroyed
  ~smart_ptr(void);

  //////////////////////////////////////////////////////////////////////////////
  // logical tests to see if there is anything contained in the pointer since it can be null

  // there are two forms:explicit and implicit
  // implicit: if(!r) or if(r)
  // explicit: if(r.null()) or if(r.present())
  operator bool(void) const;
  bool operator!(void) const;
  bool present(void) const;
  bool null(void) const;

  //////////////////////////////////////////////////////////////////////////////
  // dereference operators and functions

  // dereference the smart pointer to get the object - use in the form *p1
  T& operator*(void) throw(null_dereference);
  const T& operator*(void) const throw(null_dereference);

  // used as a prefix to a member access to the contained object e.g. p1->print() calls T::print()
  T* operator->(void) throw(null_dereference);
  const T* operator->(void) const throw(null_dereference);

  //////////////////////////////////////////////////////////////////////////////
  // explicit function forms of the above assignment and dereference operators

  // set the value
  void set_value(const T& data);
  // get the value
  T& value(void) throw(null_dereference);
  const T& value(void) const throw(null_dereference);

  // set the pointer
  // deletes the previous pointer and adopts the passed pointer instead
  // Note: the object must be allocated *by the user* with new
  // Warning: it is very easy to break the memory management with this operation
  void set(T* data = 0);
  // get the pointer
  T* pointer(void);
  const T* pointer(void) const;

  //////////////////////////////////////////////////////////////////////////////
  // functions to manage counted referencing

  // test whether two pointers point to the same object(known as aliasing the object)
  // used in the form if(a.aliases(b))
  bool aliases(const smart_ptr<T>&) const;

  // find the number of aliases - used when you need to know whether an object is still referred to from elsewhere (rare!)
  unsigned alias_count(void) const;

  // make this pointer unique with respect to any other references to the same object
  // if this pointer is already unique, it does nothing - otherwise it copies the object
  void make_unique(void);

  // delete the object and make the pointer null - does not make it unique first, so all other pointers to this will be null too
  void clear(void);

  // make the pointer unique and null in one step - does not affect other pointers that were pointing to the same object
  void clear_unique(void);

  // make this pointer a unique copy of the parameter
  // useful for expressions like p1.copy(p2) which makes p1 a pointer to a unique copy of the contents of p2
  void copy(const smart_ptr<T>&);
  // alternate form used in assignments: p1 = ps.copy()
  smart_ptr<T> copy(void) const;

  // persistence functions
  void dump(dump_context& str) const throw(persistent_dump_failed);
  void restore(restore_context& str) throw(persistent_restore_failed);

protected:
  smart_ptr_holder<T>* m_holder;
};

////////////////////////////////////////////////////////////////////////////////
// comparisons required for using this class in an STL container
// These require == and < operator in the contained type T
// the remaining relational operators are provided by template functions
// a null pointer is less-than a non-null, two nulls are equal
// these funcions are defined as non-members so that you only need provide
// the underlying T::operator< and == if you are going to use these functions

template<typename T>
bool operator==(const smart_ptr<T>&, const smart_ptr<T>&);

template<typename T>
bool operator<(const smart_ptr<T>&, const smart_ptr<T>&);

////////////////////////////////////////////////////////////////////////////////
// string/print utilities

template<typename T>
std::string smart_ptr_to_string(const smart_ptr<T>& ptr, std::string null_string);

template<typename T>
otext& print_smart_ptr(otext& str, const smart_ptr<T>& ptr, std::string null_string);

template<typename T>
otext& print_smart_ptr(otext& str, const smart_ptr<T>& ptr, unsigned indent, std::string null_string);

////////////////////////////////////////////////////////////////////////////////
// persistence - call these rather than the methods
// the dump routine dumps smart_ptr-specific information and then calls dump_pointer on the contents
// similarly the restore routine calls restore_pointer
// so therefore the class T should have non-member dump/restore functions

template<typename T>
void dump_smart_ptr(dump_context& str, const smart_ptr<T>& data) throw(persistent_dump_failed);

template<typename T>
void restore_smart_ptr(restore_context& str, smart_ptr<T>& data) throw(persistent_restore_failed);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Cloning smart pointer class for polymorphic class hierarchies
// The contained class T should implement the clonable interface defined in clonable.hpp
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template<typename T>
class smart_ptr_clone
{
public:
  //////////////////////////////////////////////////////////////////////////////
  // member type definitions

  typedef T value_type;
  typedef T& reference;
  typedef const T& const_reference;

  //////////////////////////////////////////////////////////////////////////////
  // constructors, assignments and destructors

  // create a null pointer
  smart_ptr_clone(void);

  // create a pointer containing a *copy* of the object
  // this copy is taken because the pointer class maintains a dynamically allocated object 
  // and the T& may not be (usually is not) dynamically allocated
  // constructor form
  smart_ptr_clone(const T& data);
  // assignment form for an already-constructed smart-pointer
  smart_ptr_clone<T>& operator=(const T& data);

  // copy constructor implements counted referencing - no copy is made
  smart_ptr_clone(const smart_ptr_clone<T>& r);
  // assignment of smart pointers implement counted referencing - no copy is made
  smart_ptr_clone<T>& operator=(const smart_ptr_clone<T>&);

  // create a pointer containing a dynamically created object
  // Note: the object must be allocated *by the user* with new
  // constructor form - must be called in the form smart_ptr_clone<type> x(new type(args))
  explicit smart_ptr_clone(T* data);
  // assignment form
  smart_ptr_clone<T>& operator= (T* data);

  // destructor decrements the reference count and delete only when the last reference is destroyed
  ~smart_ptr_clone(void);

  //////////////////////////////////////////////////////////////////////////////
  // logical tests to see if there is anything contained in the pointer since it can be null

  // there are two forms:explicit and implicit
  // implicit: if(!r) or if(r)
  // explicit: if(r.null()) or if(r.present())
  operator bool(void) const;
  bool operator!(void) const;
  bool present(void) const;
  bool null(void) const;

  //////////////////////////////////////////////////////////////////////////////
  // dereference operators and functions

  // dereference the smart pointer to get the object - use in the form *p1
  T& operator*(void) throw(null_dereference);
  const T& operator*(void) const throw(null_dereference);

  // used as a prefix to a member access to the contained object e.g. p1->print() calls T::print()
  T* operator->(void) throw(null_dereference);
  const T* operator->(void) const throw(null_dereference);

  //////////////////////////////////////////////////////////////////////////////
  // explicit function forms of the above assignment and dereference operators

  // set the value
  void set_value(const T& data);
  // get the value
  T& value(void) throw(null_dereference);
  const T& value(void) const throw(null_dereference);

  // set the pointer
  // deletes the previous pointer and adopts the passed pointer instead
  // Note: the object must be allocated *by the user* with new
  // Warning: it is very easy to break the memory management with this operation
  void set(T* data = 0);
  // get the pointer
  T* pointer(void);
  const T* pointer(void) const;

  //////////////////////////////////////////////////////////////////////////////
  // functions to manage counted referencing

  // test whether two pointers point to the same object(known as aliasing the object)
  // used in the form if(a.aliases(b))
  bool aliases(const smart_ptr_clone<T>&) const;

  // find the number of aliases - used when you need to know whether an object is still referred to from elsewhere (rare!)
  unsigned alias_count(void) const;

  // make this pointer unique with respect to any other references to the same object
  // if this pointer is already unique, it does nothing - otherwise it copies the object
  void make_unique(void);

  // delete the object and make the pointer null - does not make it unique first, so all other pointers to this will be null too
  void clear(void);

  // make the pointer unique and null in one step - does not affect other pointers that were pointing to the same object
  void clear_unique(void);

  // make this pointer a unique copy of the parameter
  // useful for expressions like p1.copy(p2) which makes p1 a pointer to a unique copy of the contents of p2
  void copy(const smart_ptr_clone<T>&);
  // alternate form used in assignments: p1 = ps.copy()
  smart_ptr_clone<T> copy(void) const;

  // persistence functions
  void dump(dump_context& str) const throw(persistent_dump_failed);
  void restore(restore_context& str) throw(persistent_restore_failed);

protected:
  smart_ptr_holder<T>* m_holder;
};

////////////////////////////////////////////////////////////////////////////////
// comparisons required for using this class in an STL container
// These require == and < operator in the contained type T
// the remaining relational operators are provided by template functions
// a null pointer is less-than a non-null, two nulls are equal
// these funcions are defined as non-members so that you only need provide
// the underlying T::operator< and == if you are going to use these functions

template<typename T>
bool operator==(const smart_ptr_clone<T>&, const smart_ptr_clone<T>&);

template<typename T>
bool operator<(const smart_ptr_clone<T>&, const smart_ptr_clone<T>&);

////////////////////////////////////////////////////////////////////////////////
// string/print utilities

template<typename T>
std::string smart_ptr_clone_to_string(const smart_ptr_clone<T>& ptr, std::string null_string);

template<typename T>
otext& print_smart_ptr_clone(otext& str, const smart_ptr_clone<T>& ptr, std::string null_string);

template<typename T>
otext& print_smart_ptr_clone(otext& str, const smart_ptr_clone<T>& ptr, unsigned indent, std::string null_string);

////////////////////////////////////////////////////////////////////////////////
// persistence - call these rather than the methods
// the dump routine dumps smart_ptr_clone-specific information and then calls dump_interface
// similarly the restore routine calls restore_interface
// so therefore the class T should implement the persistent interface defined by the class persistent in persistent.hpp

template<typename T>
void dump_smart_ptr_clone(dump_context& str, const smart_ptr_clone<T>& data) throw(persistent_dump_failed);

template<typename T>
void restore_smart_ptr_clone(restore_context& str, smart_ptr_clone<T>& data) throw(persistent_restore_failed);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// No-copy smart pointer class for managing objects that cannot be copied
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template<typename T>
class smart_ptr_nocopy
{
public:
  //////////////////////////////////////////////////////////////////////////////
  // member type definitions

  typedef T value_type;
  typedef T& reference;
  typedef const T& const_reference;

  //////////////////////////////////////////////////////////////////////////////
  // constructors, assignments and destructors

  // create a null pointer
  smart_ptr_nocopy(void);

  // copy constructor implements counted referencing - no copy is made
  smart_ptr_nocopy(const smart_ptr_nocopy<T>& r);
  // assignment of smart pointers implement counted referencing - no copy is made
  smart_ptr_nocopy<T>& operator=(const smart_ptr_nocopy<T>&);

  // create a pointer containing a dynamically created object
  // Note: the object must be allocated *by the user* with new
  // constructor form - must be called in the form smart_ptr_nocopy<type> x(new type(args))
  explicit smart_ptr_nocopy(T* data);
  // assignment form
  smart_ptr_nocopy<T>& operator= (T* data);

  // destructor decrements the reference count and delete only when the last reference is destroyed
  ~smart_ptr_nocopy(void);

  //////////////////////////////////////////////////////////////////////////////
  // logical tests to see if there is anything contained in the pointer since it can be null

  // there are two forms:explicit and implicit
  // implicit: if(!r) or if(r)
  // explicit: if(r.null()) or if(r.present())
  operator bool(void) const;
  bool operator!(void) const;
  bool present(void) const;
  bool null(void) const;

  //////////////////////////////////////////////////////////////////////////////
  // dereference operators and functions

  // dereference the smart pointer to get the object - use in the form *p1
  T& operator*(void) throw(null_dereference);
  const T& operator*(void) const throw(null_dereference);

  // used as a prefix to a member access to the contained object e.g. p1->print() calls T::print()
  T* operator->(void) throw(null_dereference);
  const T* operator->(void) const throw(null_dereference);

  //////////////////////////////////////////////////////////////////////////////
  // explicit function forms of the above assignment and dereference operators

  // get the value
  T& value(void) throw(null_dereference);
  const T& value(void) const throw(null_dereference);

  // set the pointer
  // deletes the previous pointer and adopts the passed pointer instead
  // Note: the object must be allocated *by the user* with new
  // Warning: it is very easy to break the memory management with this operation
  void set(T* data = 0);
  // get the pointer
  T* pointer(void);
  const T* pointer(void) const;

  //////////////////////////////////////////////////////////////////////////////
  // functions to manage counted referencing

  // test whether two pointers point to the same object(known as aliasing the object)
  // used in the form if(a.aliases(b))
  bool aliases(const smart_ptr_nocopy<T>&) const;

  // find the number of aliases - used when you need to know whether an object is still referred to from elsewhere (rare!)
  unsigned alias_count(void) const;

  // delete the object and make the pointer null - does not make it unique first, so all other pointers to this will be null too
  void clear(void);

  // make the pointer unique and null in one step - does not affect other pointers that were pointing to the same object
  void clear_unique(void);

protected:
  smart_ptr_holder<T>* m_holder;
};

////////////////////////////////////////////////////////////////////////////////
// comparisons required for using this class in an STL container
// These require == and < operator in the contained type T
// the remaining relational operators are provided by template functions
// a null pointer is less-than a non-null, two nulls are equal
// these funcions are defined as non-members so that you only need provide
// the underlying T::operator< and == if you are going to use these functions

template<typename T>
bool operator==(const smart_ptr_nocopy<T>&, const smart_ptr_nocopy<T>&);

template<typename T>
bool operator<(const smart_ptr_nocopy<T>&, const smart_ptr_nocopy<T>&);

////////////////////////////////////////////////////////////////////////////////
// string/print utilities

template<typename T>
std::string smart_ptr_nocopy_to_string(const smart_ptr_nocopy<T>& ptr, std::string null_string);

template<typename T>
otext& print_smart_ptr_nocopy(otext& str, const smart_ptr_nocopy<T>& ptr, std::string null_string);

template<typename T>
otext& print_smart_ptr_nocopy(otext& str, const smart_ptr_nocopy<T>& ptr, unsigned indent, std::string null_string);

////////////////////////////////////////////////////////////////////////////////
// there's no persistence on smart_ptr_nocopy because the whole point is that
// it stores an uncopyable object and persistence is a form of copying

////////////////////////////////////////////////////////////////////////////////
#include "smart_ptr.tpp"
#endif
