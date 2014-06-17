/*------------------------------------------------------------------------------

  Author:    Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License, see ../docs/license.html

  ------------------------------------------------------------------------------*/
#include "stringio.hpp"
#include "fileio.hpp"

////////////////////////////////////////////////////////////////////////////////
// enumeration types

template<typename T>
void dump_enum(dump_context& context, const T& data) throw(persistent_dump_failed)
{
  dump(context, (unsigned)data);
}

template<typename T>
void restore_enum(restore_context& context, T& data) throw(persistent_restore_failed)
{
  unsigned value = 0;
  restore(context, value);
  data = (T)value;
}

////////////////////////////////////////////////////////////////////////////////
// STL strings

template<typename charT, typename traits, typename allocator>
void dump_basic_string(dump_context& context, const std::basic_string<charT,traits,allocator>& data) throw(persistent_dump_failed)
{
  dump(context, data.size());
  for (size_t i = 0; i < data.size(); i++)
    dump(context,data[i]);
}

template<typename charT, typename traits, typename allocator>
void restore_basic_string(restore_context& context, std::basic_string<charT,traits,allocator>& data) throw(persistent_restore_failed)
{
  data.erase();
  size_t size = 0;
  restore(context, size);
  for (size_t i = 0; i < size; i++)
  {
    charT ch;
    restore(context,ch);
    data += ch;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Pointers
// format: magic_key [ data ]

template<typename T>
void dump_pointer(dump_context& context, const T* const data) throw(persistent_dump_failed)
{
  // register the address and get the magic key for it
  std::pair<bool,unsigned> mapping = context.pointer_map(data);
  dump(context,mapping.second);
  // if the address is null, then that is all that we need to do
  // however, if it is non-null and this is the first sight of the address, dump the contents
  if (data && !mapping.first)
    dump(context,*data);
}

template<typename T>
void restore_pointer(restore_context& context, T*& data) throw(persistent_restore_failed)
{
  if (data)
  {
    delete data;
    data = 0;
  }
  // get the magic key
  unsigned magic = 0;
  restore(context,magic);
  // now lookup the magic key to see if this pointer has already been restored
  // null pointers are always flagged as already restored
  std::pair<bool,void*> address = context.pointer_map(magic);
  if (address.first)
  {
    // seen before, so simply assign the old address
    data = (T*)address.second;
  }
  else
  {
    // this pointer has never been seen before and is non-null
    data = new T();
    restore(context,*data);
    // add this pointer to the set of already seen objects
    context.pointer_add(magic,data);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Cross-reference pointers
// format: magic_key [ data ]

template<typename T>
void dump_xref(dump_context& context, const T* const data) throw(persistent_dump_failed)
{
  // register the address and get the magic key for it
  std::pair<bool,unsigned> mapping = context.pointer_map(data);
  // if this is the first view of this pointer, simply throw an exception
  if (!mapping.first) throw persistent_dump_failed("tried to dump a cross-reference not seen before");
  // otherwise, just dump the magic key
  dump(context,mapping.second);
}

template<typename T>
void restore_xref(restore_context& context, T*& data) throw(persistent_restore_failed)
{
  // Note: I do not try to delete the old data because this is a cross-reference
  // get the magic key
  unsigned magic = 0;
  restore(context,magic);
  // now lookup the magic key to see if this pointer has already been restored
  // null pointers are always flagged as already restored
  std::pair<bool,void*> address = context.pointer_map(magic);
  // if this is the first view of this pointer, simply throw an exception
  if (!address.first) throw persistent_restore_failed("tried to restore a cross-reference not seen before");
  // seen before, so simply assign the old address
  data = (T*)address.second;
}

////////////////////////////////////////////////////////////////////////////////
// Polymorphous classes using the callback approach
// format: address [ key data ]

template<typename T>
void dump_polymorph(dump_context& context, const T* data) throw(persistent_dump_failed)
{
  try
  {
    // register the address and get the magic key for it
    std::pair<bool,unsigned> mapping = context.pointer_map(data);
    dump(context,mapping.second);
    // if the address is null, then that is all that we need to do
    // however, if it is non-null and this is the first sight of the address, dump the contents
    if (data && !mapping.first)
    {
      // callback method - get the callback data and perform the dump
      // this will throw persistent_illegal_type if not recognised, thus the try block
      dump_context::callback_data callback = context.lookup_type(typeid(*data));
      // dump the magic key for the type
      dump(context, callback.first);
      // now call the callback that dumps the subclass
      callback.second(context,data);
    }
  }
  catch (const persistent_illegal_type& except)
  {
    // convert this to a simpler dump failed exception
    throw persistent_dump_failed(except.what());
  }
}

template<typename T>
void restore_polymorph(restore_context& context, T*& data) throw(persistent_restore_failed)
{
  try
  {
    // first delete any previous object pointed to since the restore creates the object of the right subclass
    if (data)
    {
      delete data;
      data = 0;
    }
    // get the magic key
    unsigned magic = 0;
    restore(context,magic);
    // now lookup the magic key to see if this pointer has already been restored
    // null pointers are always flagged as already restored
    std::pair<bool,void*> address = context.pointer_map(magic);
    if (address.first)
    {
      // seen before, so simply map it to the existing address
      data = (T*)address.second;
    }
    else
    {
      // now restore the magic key that denotes the particular subclass
      unsigned short key = 0;
      restore(context, key);
      // callback approach
      // call the create callback to create an object of the right type
      // then call the restore callback to get the contents
      // this will throw persistent_illegal_type if not recognised - this is caught below
      restore_context::callback_data callbacks = context.lookup_type(key);
      void* void_data = callbacks.first();
      data = (T*)void_data;
      try
      {
        callbacks.second(context,void_data);
      }
      catch(...)
      {
        // tidy up any memory allocated in this function
        if (data)
        {
          delete data;
          data = 0;
        }
        throw;
      }
      // add this pointer to the set of already seen objects
      context.pointer_add(magic,data);
    }
  }
  catch (const persistent_illegal_type& exception)
  {
    // convert this to a simpler dump failed exception
    throw persistent_restore_failed(exception.what());
  }
}

////////////////////////////////////////////////////////////////////////////////
// Polymorphous classes using the interface approach
// format: address [ key data ]

template<typename T>
void dump_interface(dump_context& context, const T* data) throw(persistent_dump_failed)
{
  try
  {
    // register the address and get the magic key for it
    std::pair<bool,unsigned> mapping = context.pointer_map(data);
    dump(context,mapping.second);
    // if the address is null, then that is all that we need to do
    // however, if it is non-null and this is the first sight of the address, dump the contents
    if (data && !mapping.first)
    {
      // interface method
      // the lookup just finds the magic key and the type has a dump method
      // this will throw persistent_illegal_type if the type is not registered
      unsigned short key = context.lookup_interface(typeid(*data));
      // dump the magic key for the type
      dump(context, key);
      // now call the dump method defined by the interface
      data->dump(context);
    }
  }
  catch (const persistent_illegal_type& exception)
  {
    // convert this to a simpler dump failed exception
    throw persistent_dump_failed(exception.what());
  }
}

template<typename T>
void restore_interface(restore_context& context, T*& data) throw(persistent_restore_failed)
{
  try
  {
    // first delete any previous object pointed to since the restore creates the object of the right subclass
    if (data)
    {
      delete data;
      data = 0;
    }
    // get the magic key
    unsigned magic = 0;
    restore(context,magic);
    // now lookup the magic key to see if this pointer has already been restored
    // null pointers are always flagged as already restored
    std::pair<bool,void*> address = context.pointer_map(magic);
    if (address.first)
    {
      // seen this address before, so simply assign it
      data = (T*)address.second;
    }
    else
    {
      // now restore the magic key that denotes the particular subclass
      unsigned short key = 0;
      restore(context, key);
      // interface approach
      // first clone the sample object stored in the map - lookup_interface can throw persistent_illegal_type
      data = (T*)(context.lookup_interface(key).clone());
      // now restore the contents using the object's method
      try
      {
        data->restore(context);
      }
      catch(...)
      {
        if (data)
        {
          delete data;
          data = 0;
        }
        throw;
      }
      // add this pointer to the set of already seen objects
      context.pointer_add(magic,data);
    }
  }
  catch (const persistent_illegal_type& exception)
  {
    // convert this to a simpler dump failed exception
    throw persistent_restore_failed(exception.what());
  }
}

////////////////////////////////////////////////////////////////////////////////
// format: data msB first, packed into bytes with lowest index at the byte's lsb

// Note: the interface does not provide access to the internal storage and yet
// to be efficient the std::bitset must be packed as bytes. Thus I have to do it the
// hard way.

template<size_t N>
void dump_bitset(dump_context& context, const std::bitset<N>& data) throw(persistent_dump_failed)
{
  size_t bits = data.size();
  size_t bytes = (bits+7)/8;
  for (size_t B = bytes; B--; )
  {
    unsigned char ch = 0;
    for (size_t b = 0; b < 8; b++)
    {
      size_t bit = B*8+b;
      if (bit < bits && data[bit])
        ch |= (0x01 << b);
    }
    dump(context,ch);
  }
}

template<size_t N>
void restore_bitset(restore_context& context, std::bitset<N>& data) throw(persistent_restore_failed)
{
  size_t bits = data.size();
  size_t bytes = (bits+7)/8;
  for (size_t B = bytes; B--; )
  {
    unsigned char ch = 0;
    restore(context,ch);
    for (size_t b = 0; b < 8; b++)
    {
      size_t bit = B*8+b;
      if (bit >= bits) break;
      data.set(bit, ch & (0x01 << b) ? true : false);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// complex

template<typename T>
void dump_complex(dump_context& context, const std::complex<T>& data) throw(persistent_dump_failed)
{
  dump(context,data.real());
  dump(context,data.imag());
}

template<typename T>
void restore_complex(restore_context& context, std::complex<T>& data) throw(persistent_restore_failed)
{
  T re, im;
  restore(context,re);
  restore(context,im);
  data = std::complex<T>(re, im);
}

////////////////////////////////////////////////////////////////////////////////
// deque

template<typename T>
void dump_deque(dump_context& context, const std::deque<T>& data) throw(persistent_dump_failed)
{
  dump(context,data.size());
  for (typename std::deque<T>::const_iterator i = data.begin(); i != data.end(); i++)
    dump(context,*i);
}

template<typename T>
void restore_deque(restore_context& context, std::deque<T>& data) throw(persistent_restore_failed)
{
  data.clear();
  size_t size = 0;
  restore(context,size);
  for (size_t i = 0; i < size; i++)
  {
    data.push_back(T());
    restore(context,data.back());
  }
}

////////////////////////////////////////////////////////////////////////////////
// list

template<typename T>
void dump_list(dump_context& context, const std::list<T>& data) throw(persistent_dump_failed)
{
  dump(context,data.size());
  for (typename std::list<T>::const_iterator i = data.begin(); i != data.end(); i++)
    dump(context,*i);
}

template<typename T>
void restore_list(restore_context& context, std::list<T>& data) throw(persistent_restore_failed)
{
  data.clear();
  size_t size = 0;
  restore(context,size);
  for (size_t i = 0; i < size; i++)
  {
    data.push_back(T());
    restore(context,data.back());
  }
}

////////////////////////////////////////////////////////////////////////////////
// pair

template<typename K, typename T>
void dump_pair(dump_context& context, const std::pair<K,T>& data) throw(persistent_dump_failed)
{
  dump(context,data.first);
  dump(context,data.second);
}

template<typename K, typename T>
void restore_pair(restore_context& context, std::pair<K,T>& data) throw(persistent_restore_failed)
{
  restore(context,data.first);
  restore(context,data.second);
}

////////////////////////////////////////////////////////////////////////////////
// Map

template<typename K, typename T, typename P>
void dump_map(dump_context& context, const std::map<K,T,P>& data) throw(persistent_dump_failed)
{
  dump(context,data.size());
  for (typename std::map<K,T,P>::const_iterator i = data.begin(); i != data.end(); i++)
  {
    dump(context,i->first);
    dump(context,i->second);
  }
}

template<typename K, typename T, typename P>
void restore_map(restore_context& context, std::map<K,T,P>& data) throw(persistent_restore_failed)
{
  data.clear();
  size_t size = 0;
  restore(context,size);
  for (size_t j = 0; j < size; j++)
  {
    K key;
    restore(context,key);
    restore(context,data[key]);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Multimap

template<typename K, typename T, typename P>
void dump_multimap(dump_context& context, const std::multimap<K,T,P>& data) throw(persistent_dump_failed)
{
  dump(context,data.size());
  for (typename std::multimap<K,T,P>::const_iterator i = data.begin(); i != data.end(); i++)
  {
    dump(context,i->first);
    dump(context,i->second);
  }
}

template<typename K, typename T, typename P>
void restore_multimap(restore_context& context, std::multimap<K,T,P>& data) throw(persistent_restore_failed)
{
  data.clear();
  size_t size = 0;
  restore(context,size);
  for (size_t j = 0; j < size; j++)
  {
    K key;
    restore(context,key);
    typename std::multimap<K,T,P>::iterator v = data.insert(std::pair<K,T>(key,T()));
    restore(context,v->second);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Set

template<typename K, typename P>
void dump_set(dump_context& context, const std::set<K,P>& data) throw(persistent_dump_failed)
{
  dump(context,data.size());
  for (typename std::set<K,P>::const_iterator i = data.begin(); i != data.end(); i++)
    dump(context,*i);
}

template<typename K, typename P>
void restore_set(restore_context& context, std::set<K,P>& data) throw(persistent_restore_failed)
{
  data.clear();
  size_t size = 0;
  restore(context,size);
  typename std::set<K,P>::iterator i = data.begin();
  for (size_t j = 0; j < size; j++)
  {
    K key;
    restore(context,key);
    // inserting using an iterator is O(n) rather than O(n*log(n))
    data.insert(i, key);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Multiset
// Shame that a multiset is not just a derivative of set since it has the same interface

template<typename K, typename P>
void dump_multiset(dump_context& context, const std::multiset<K,P>& data) throw(persistent_dump_failed)
{
  dump(context,data.size());
  for (typename std::multiset<K,P>::const_iterator i = data.begin(); i != data.end(); i++)
    dump(context,*i);
}

template<typename K, typename P>
void restore_multiset(restore_context& context, std::multiset<K,P>& data) throw(persistent_restore_failed)
{
  data.clear();
  size_t size = 0;
  restore(context,size);
  typename std::multiset<K,P>::iterator i = data.begin();
  for (size_t j = 0; j < size; j++)
  {
    K key;
    restore(context,key);
    // inserting using an iterator is O(n) rather than O(n*log(n))
    data.insert(i, key);
  }
}

////////////////////////////////////////////////////////////////////////////////
//vector

template<typename T>
void dump_vector(dump_context& context, const std::vector<T>& data) throw(persistent_dump_failed)
{
  dump(context,data.size());
  for (size_t i = 0; i < data.size(); i++)
    dump(context,data[i]);
}

template<typename T>
void restore_vector(restore_context& context, std::vector<T>& data) throw(persistent_restore_failed)
{
  size_t size = 0;
  restore(context,size);
  data.resize(size);
  for (size_t i = 0; i < size; i++)
    restore(context,data[i]);
}

////////////////////////////////////////////////////////////////////////////////

template<typename T>
void dump_to_device(const T& source, otext& result, dump_context::installer installer)
  throw(persistent_dump_failed)
{
  dump_context context(result);
  context.register_all(installer);
  dump(context, source);
}

template<typename T>
void restore_from_device(itext& source, T& result, restore_context::installer installer)
  throw(persistent_restore_failed)
{
  restore_context context(source);
  context.register_all(installer);
  restore(context, result);
}

template<typename T>
void dump_to_string(const T& source, std::string& result, dump_context::installer installer)
  throw(persistent_dump_failed)
{
  ostext output;
  dump_to_device(source, output, installer);
  result = output.get_string();
}

template<typename T>
void restore_from_string(const std::string& source, T& result, restore_context::installer installer)
  throw(persistent_restore_failed)
{
  istext input(source);
  restore_from_device(input, result, installer);
}

template<typename T>
void dump_to_file(const T& source, const std::string& filename, dump_context::installer installer)
  throw(persistent_dump_failed)
{
  oftext output(filename);
  dump_to_device(source, output, installer);
}

template<typename T>
void restore_from_file(const std::string& filename, T& result, restore_context::installer installer)
  throw(persistent_restore_failed)
{
  iftext input(filename);
  restore_from_device(input, result, installer);
}

////////////////////////////////////////////////////////////////////////////////
