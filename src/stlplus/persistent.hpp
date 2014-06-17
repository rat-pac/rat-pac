#ifndef PERSISTENT_HPP
#define PERSISTENT_HPP
/*------------------------------------------------------------------------------

  Author:    Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License, see ../docs/license.html

  ------------------------------------------------------------------------------*/
#include "os_fixes.hpp"
#include "textio.hpp"
#include "clonable.hpp"
#include <stdlib.h>
#include <complex>
#include <deque>
#include <bitset>
#include <set>
#include <map>
#include <list>
#include <vector>
#include <string>
#include <typeinfo>
#include <stdexcept>

////////////////////////////////////////////////////////////////////////////////
// The format version number currently supported
////////////////////////////////////////////////////////////////////////////////

extern unsigned char PersistentVersion;

////////////////////////////////////////////////////////////////////////////////
// Exceptions thrown by the persistence functions

// exception thrown if you try to dump or restore an illegal polymorphic type
class persistent_illegal_type : public std::logic_error
{
public:
  persistent_illegal_type(const std::string& type) throw();
  persistent_illegal_type(unsigned short key) throw();
  ~persistent_illegal_type(void) throw();
};

// exception thrown if a dump fails for any reason - but typically because the output stream couldn't take the data
class persistent_dump_failed : public std::runtime_error
{
public:
  persistent_dump_failed(const std::string& message) throw();
  ~persistent_dump_failed(void) throw();
};

// exception thrown if you try to restore from an out of date or unrecognised byte stream
class persistent_restore_failed : public std::runtime_error
{
public:
  persistent_restore_failed(const std::string& message) throw();
  ~persistent_restore_failed(void) throw();
};

////////////////////////////////////////////////////////////////////////////////
// dump_context controls the formatting of a persistent dump
////////////////////////////////////////////////////////////////////////////////

class dump_context_body;

class dump_context
{
public:
  // types used in making polymorphous classes persistent using the callback approach

  // callback function for dumping the class
  typedef void (*dump_callback)(dump_context&,const void*);
  // data stored per class registered
  typedef std::pair<unsigned short,dump_callback> callback_data;

  // type of callback function used to install all polymorphic classes for either approach
  typedef void (*installer)(dump_context&);

  //////////////////////////////////////////////////////////////////////////////

  dump_context(const otext& device, unsigned char version = PersistentVersion) throw(persistent_dump_failed);
  ~dump_context(void);

  // low level output used to dump a byte
  void put(unsigned char data) throw(persistent_dump_failed);

  // access the device, for example to check the error status
  const otext& device(void) const;

  // recover the version number of the dumped output
  unsigned char version(void) const;

  // test whether the current platform uses little-endian or big-endian addressing of bytes
  // this is used in dump/restore of integers
  bool little_endian(void) const;

  // Assist functions for Pointers
  // the return pair is a flag saying whether this is a new pointer and the magic key to dump to file
  std::pair<bool,unsigned> pointer_map(const void* const pointer);

  // Assist functions for Polymorphous classes (i.e. subclasses) using callback approach
  unsigned short register_type(const std::type_info& info, dump_callback);
  bool is_callback(const std::type_info& info) const;
  callback_data lookup_type(const std::type_info&) const throw(persistent_illegal_type);

  // Assist functions for Polymorphous classes (i.e. subclasses) using interface approach
  unsigned short register_interface(const std::type_info& info);
  bool is_interface(const std::type_info& info) const;
  unsigned short lookup_interface(const std::type_info&) const throw(persistent_illegal_type);

  // Register all Polymorphous classes using either approach by calling an installer callback
  void register_all(installer);

private:
  friend class dump_context_body;
  dump_context_body* m_body;

  // disallow copying by making assignment and copy constructor private
  dump_context(const dump_context&);
  dump_context& operator=(const dump_context&);
};

////////////////////////////////////////////////////////////////////////////////
// restore_context controls the reading of the persistent data during a restore

class persistent;
class restore_context_body;

class restore_context
{
public:
  // types used in making polymorphous classes persistent using the callback approach

  // callback function for creating a new object of the class and returning the pointer
  typedef void* (*create_callback)(void);
  // callback for restoring the contents of a new object created by the create_callback
  // and passed as the second argument
  typedef void (*restore_callback)(restore_context&,void*);
  // data stored per class registered
  typedef std::pair<create_callback, restore_callback> callback_data;

  // types used in making polymorphous classes persistent using the interface approach

  friend class persistent;
  typedef std::pair<unsigned short,persistent*> interface_data;

  // type of callback function used to install all polymorphic classes for either approach
  typedef void (*installer)(restore_context&);

  //////////////////////////////////////////////////////////////////////////////

  restore_context(const itext& device) throw(persistent_restore_failed);
  ~restore_context(void);

  // low level input used to restore a byte
  int get(void) throw(persistent_restore_failed);

  // access the device, for example to check the error status
  const itext& device(void) const;

  // access the version number of the input being restored
  unsigned char version(void) const;

  // test whether the current platform uses little-endian or big-endian addressing of bytes
  // this is used in dump/restore of integers
  bool little_endian(void) const;

  // Assist functions for Pointers
  std::pair<bool,void*> pointer_map(unsigned magic);
  void pointer_add(unsigned magic, void* new_pointer);

  // Assist functions for Polymorphous classes using the callback approach
  unsigned short register_type(create_callback,restore_callback);
  bool is_callback(unsigned short) const;
  callback_data lookup_type(unsigned short) const throw(persistent_illegal_type);

  // Assist functions for Polymorphous classes using the interface approach
  // the object class must be a derivative of class persistent - i.e. it must implement the persistent interface
  unsigned short register_interface(const persistent&);
  bool is_interface(unsigned short) const;
  const persistent& lookup_interface(unsigned short) const throw(persistent_illegal_type);

  // Register all Polymorphous classes using either approach by calling an installer callback
  void register_all(installer);

private:
  friend class restore_context_body;
  restore_context_body* m_body;

  // disallow copying by making assignment and copy constructor private
  restore_context(const restore_context&);
  restore_context& operator=(const restore_context&);
};

////////////////////////////////////////////////////////////////////////////////
// Class defines an interface for use in making polymorphous classes persistent
////////////////////////////////////////////////////////////////////////////////
// Note that it is derived from the clonable interface - so you must provide
// that interface too

class persistent : public clonable
{
public:
  virtual void dump(dump_context&) const throw(persistent_dump_failed) = 0;
  virtual void restore(restore_context&)  throw(persistent_restore_failed) = 0;
  virtual ~persistent() {};
};

////////////////////////////////////////////////////////////////////////////////
// Integers

void dump(dump_context&, const bool& data) throw(persistent_dump_failed);
void restore(restore_context&, bool& data) throw(persistent_restore_failed);
void dump(dump_context&, const char& data) throw(persistent_dump_failed);
void restore(restore_context&, char& data) throw(persistent_restore_failed);
void dump(dump_context&, const signed char& data) throw(persistent_dump_failed);
void restore(restore_context&, signed char& data) throw(persistent_restore_failed);
void dump(dump_context&, const unsigned char& data) throw(persistent_dump_failed);
void restore(restore_context&, unsigned char& data) throw(persistent_restore_failed);
void dump(dump_context&, const short& data) throw(persistent_dump_failed);
void restore(restore_context&, short& data) throw(persistent_restore_failed);
void dump(dump_context&, const unsigned short& data) throw(persistent_dump_failed);
void restore(restore_context&, unsigned short& data) throw(persistent_restore_failed);
void dump(dump_context&, const int& data) throw(persistent_dump_failed);
void restore(restore_context&, int& data) throw(persistent_restore_failed);
void dump(dump_context&, const unsigned& data) throw(persistent_dump_failed);
void restore(restore_context&, unsigned& data) throw(persistent_restore_failed);
void dump(dump_context&, const long& data) throw(persistent_dump_failed);
void restore(restore_context&, long& data) throw(persistent_restore_failed);
void dump(dump_context&, const unsigned long& data) throw(persistent_dump_failed);
void restore(restore_context&, unsigned long& data) throw(persistent_restore_failed);

////////////////////////////////////////////////////////////////////////////////
// Floating point types
//
// Note: despite years and years of IEEE standardisation, not all
// architectures use IEEE-standard representations of floating-point numbers.
// Therefore a binary dump is not necessarily portable between platforms.
// Solving this is (currently) beyond the scope of the STLplus project.

void dump(dump_context&, const float& data) throw(persistent_dump_failed);
void restore(restore_context&, float& data) throw(persistent_restore_failed);
void dump(dump_context&, const double& data) throw(persistent_dump_failed);
void restore(restore_context&, double& data) throw(persistent_restore_failed);

////////////////////////////////////////////////////////////////////////////////
// enumeration types

template<typename T>
void dump_enum(dump_context&, const T& data) throw(persistent_dump_failed);
template<typename T>
void restore_enum(restore_context&, T& data) throw(persistent_restore_failed);

////////////////////////////////////////////////////////////////////////////////
// C-style char arrays.
// These are handled differently to other pointer types as below

// Warning! This means that pointers to char cannot be supported, since there
// is no type difference between a pointer to char and a C-style array of char

// Warning! The restore deletes any old value of the data parameter and
// allocates a new char* which is (just) big enough and assigns it to the data
// field. This is because there is no way of knowing how long a char* is so
// the passed parameter is not safe to use. The allocation is done using
// standard new. If the data field is non-null on entry it will be deleted by
// standard delete. Best to make it null in the first place.

void dump(dump_context&, char*& data) throw(persistent_dump_failed);
void restore(restore_context&, char*& data) throw(persistent_restore_failed);

////////////////////////////////////////////////////////////////////////////////
// STL strings

template<typename charT, typename traits, typename allocator>
void dump_basic_string(dump_context&, const std::basic_string<charT,traits,allocator>& data) throw(persistent_dump_failed);
template<typename charT, typename traits, typename allocator>
void restore_basic_string(restore_context&, std::basic_string<charT,traits,allocator>& data) throw(persistent_restore_failed);

void dump(dump_context&, const std::string& data) throw(persistent_dump_failed);
void restore(restore_context&, std::string& data) throw(persistent_restore_failed);

////////////////////////////////////////////////////////////////////////////////
// Pointers

// Supports null pointers too!

// Warning! The pointer must be a dynamically-allocated type, since the implementation uses new/delete
// If the data field to restore is null and the file format non-null, allocates a new T()
// If the data field is non-null and the file format is null, deletes it and sets it null

// Multiple pointers to the same object *will* be restored as multiple
// pointers to the same object. The object is dumped only the first time it is
// encountered along with a "magic key". Subsequent pointers to the same
// object cause only the magic key to be dumped. On restore, the object is
// only restored once and the magic keys are matched up so that the other
// pointers now pojnt to the restored object.

template<typename T>
void dump_pointer(dump_context&, const T* const data) throw(persistent_dump_failed);

template<typename T>
void restore_pointer(restore_context&, T*& data) throw(persistent_restore_failed);

////////////////////////////////////////////////////////////////////////////////
// Cross-references
// A cross-reference is a pointer to an object that has definitely been dumped
// already by one of dump_pointer, dump_interface or dump_polymorph, i.e. by
// one of the dump routines for pointers to objects.
//
// These are typically used in data structures as back-pointers or pointers
// between nodes.
//
// For example, you may have a tree with cross links. Dump the tree as the
// primary data structure and then dump the cross links as cross-references.
// These functions will throw an exception if the cross-reference points to
// something not dumped before.

template<typename T>
void dump_xref(dump_context&, const T* const data) throw(persistent_dump_failed);

template<typename T>
void restore_xref(restore_context&, T*& data) throw(persistent_restore_failed);

////////////////////////////////////////////////////////////////////////////////
// Polymorphous types using the callback approach
// These are always dumped/restored as pointers to the superclass T.

// Multiple pointers to the same object are handled in the same way as above
// for simple pointers

// Only classes registered with the context can be dumped and restored as
// polymorphic types - see dump_context::register_type and restore_context::register_type.

template<typename T>
void dump_polymorph(dump_context&, const T* const data) throw(persistent_dump_failed);

template<typename T>
void restore_polymorph(restore_context&, T*& data) throw(persistent_restore_failed);

////////////////////////////////////////////////////////////////////////////////
// Polymorphous types using the interface approach
// These are always dumped/restored as pointers to the superclass T.

// Multiple pointers to the same object are handled in the same way as above
// for simple pointers

// Only classes registered with the context can be dumped and restored as
// polymorphic types - see dump_context::register_interface and restore_context::register_interface

template<typename T>
void dump_interface(dump_context&, const T* const data) throw(persistent_dump_failed);

template<typename T>
void restore_interface(restore_context&, T*& data) throw(persistent_restore_failed);

////////////////////////////////////////////////////////////////////////////////
// STL Containers

template<size_t N>
void dump_bitset(dump_context&, const std::bitset<N>& data) throw(persistent_dump_failed);
template<size_t N>
void restore_bitset(restore_context&, std::bitset<N>& data) throw(persistent_restore_failed);

template<typename T>
void dump_complex(dump_context&, const std::complex<T>& data) throw(persistent_dump_failed);
template<typename T>
void restore_complex(restore_context&, std::complex<T>& data) throw(persistent_restore_failed);

template<typename T>
void dump_deque(dump_context&, const std::deque<T>& data) throw(persistent_dump_failed);
template<typename T>
void restore_deque(restore_context&, std::deque<T>& data) throw(persistent_restore_failed);

template<typename T>
void dump_list(dump_context&, const std::list<T>& data) throw(persistent_dump_failed);
template<typename T>
void restore_list(restore_context&, std::list<T>& data) throw(persistent_restore_failed);

template<typename K, typename T>
void dump_pair(dump_context&, const std::pair<K,T>& data) throw(persistent_dump_failed);
template<typename K, typename T>
void restore_pair(restore_context&, std::pair<K,T>& data) throw(persistent_restore_failed);

template<typename K, typename T, typename P>
void dump_map(dump_context&, const std::map<K,T,P>& data) throw(persistent_dump_failed);
template<typename K, typename T, typename P>
void restore_map(restore_context&, std::map<K,T,P>& data) throw(persistent_restore_failed);

template<typename K, typename T, typename P>
void dump_multimap(dump_context&, const std::multimap<K,T,P>& data) throw(persistent_dump_failed);
template<typename K, typename T, typename P>
void restore_multimap(restore_context&, std::multimap<K,T,P>& data) throw(persistent_restore_failed);

template<typename K, typename P>
void dump_set(dump_context&, const std::set<K,P>& data) throw(persistent_dump_failed);
template<typename K, typename P>
void restore_set(restore_context&, std::set<K,P>& data) throw(persistent_restore_failed);

template<typename K, typename P>
void dump_multiset(dump_context&, const std::multiset<K,P>& data) throw(persistent_dump_failed);
template<typename K, typename P>
void restore_multiset(restore_context&, std::multiset<K,P>& data) throw(persistent_restore_failed);

template<typename T>
void dump_vector(dump_context&, const std::vector<T>& data) throw(persistent_dump_failed);
template<typename T>
void restore_vector(restore_context&, std::vector<T>& data) throw(persistent_restore_failed);

////////////////////////////////////////////////////////////////////////////////
// short-cut functions for dumping and restoring to common targets
//
// These functions use the installer callback function to install any
// polymorphic type handlers required. If there are no polymorphic types used
// in the data structure, then the callback can be set to null (i.e. 0).

template<typename T>
void dump_to_device(const T& source, otext& result, dump_context::installer installer)
  throw(persistent_dump_failed);
template<typename T>
void restore_from_device(itext& source, T& result, restore_context::installer installer)
  throw(persistent_restore_failed);

template<typename T>
void dump_to_string(const T& source, std::string& result, dump_context::installer installer)
  throw(persistent_dump_failed);
template<typename T>
void restore_from_string(const std::string& source, T& result, restore_context::installer installer)
  throw(persistent_restore_failed);

template<typename T>
void dump_to_file(const T& source, const std::string& filename, dump_context::installer installer)
  throw(persistent_dump_failed);
template<typename T>
void restore_from_file(const std::string& filename, T& result, restore_context::installer installer)
  throw(persistent_restore_failed);

////////////////////////////////////////////////////////////////////////////////
#include "persistent.tpp"
#endif
