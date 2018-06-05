/*------------------------------------------------------------------------------

  Author:    Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License, see ../docs/license.html

  ------------------------------------------------------------------------------*/
#include "os_fixes.hpp"
#include "persistent.hpp"
#include "textio.hpp"
#include "string_utilities.hpp"
#include "smart_ptr.hpp"

////////////////////////////////////////////////////////////////////////////////
// File format version
// This relates to the layout of basic types - if I change the file layout of, say, int or vector, then this will change
// Early versions of the persistence routines did not have this - they are no longer supported
// - Change from version 1 to 2: changed the persistent representation of inf

unsigned char PersistentVersion = 2;

////////////////////////////////////////////////////////////////////////////////
// exceptions

persistent_illegal_type::persistent_illegal_type(const std::string& type) throw() : 
  std::logic_error(std::string("illegal type: ") + type)
{
}

persistent_illegal_type::persistent_illegal_type(unsigned short key) throw() : 
  std::logic_error(std::string("illegal key: ") + to_string(key))
{
}

persistent_illegal_type::~persistent_illegal_type(void) throw()
{
}

////////////////////////////////////////////////////////////////////////////////

persistent_dump_failed::persistent_dump_failed(const std::string& message) throw() :
  std::runtime_error(std::string("dump failed: ") + message)
{
}

persistent_dump_failed::~persistent_dump_failed(void) throw()
{
}

////////////////////////////////////////////////////////////////////////////////

persistent_restore_failed::persistent_restore_failed(const std::string& message) throw() :
  std::runtime_error(std::string("restore failed: ") + message)
{
}

persistent_restore_failed::~persistent_restore_failed(void) throw()
{
}

////////////////////////////////////////////////////////////////////////////////
// dump context classes
////////////////////////////////////////////////////////////////////////////////

class dump_context_body
{
public:
  typedef std::map<const void*,unsigned> magic_map;
  typedef std::map<std::string,dump_context::callback_data> callback_map;
  typedef std::map<std::string,unsigned short> interface_map;

  unsigned short m_max_key;
  unsigned char m_version;
  bool m_little_endian;
  otext m_device;
  magic_map m_pointers;
  callback_map m_callbacks;
  interface_map m_interfaces;

  dump_context_body(const otext& _device, unsigned char _version) throw(persistent_dump_failed) : 
    m_max_key(0), m_version(_version), m_little_endian(::little_endian()), m_device(_device)
    {
      m_device.set_binary_mode();
      put(_version);
      // map a null pointer onto magic number zero
      m_pointers[0] = 0;
      if (m_version != 1 && m_version != 2)
        throw persistent_dump_failed(std::string("wrong version: ") + to_string(m_version));
    }

  void put(unsigned char data) throw(persistent_dump_failed)
    {
      if (!m_device.put(data))
      {
        if (m_device.error())
          throw persistent_dump_failed(std::string("output device error: ") + m_device.error_string());
        else
          throw persistent_dump_failed(std::string("output device error: unknown"));
      }
    }

  const otext& device(void) const
    {
      return m_device;
    }

  unsigned char version(void) const
    {
      return m_version;
    }

  bool little_endian(void) const
    {
      return m_little_endian;
    }

  std::pair<bool,unsigned> pointer_map(const void* const pointer)
    {
      magic_map::iterator found = m_pointers.find(pointer);
      if (found == m_pointers.end())
      {
        // add a new mapping
        unsigned magic = m_pointers.size();
        m_pointers[pointer] = magic;
        return std::pair<bool,unsigned>(false,magic);
      }
      // return the old mapping
      return std::pair<bool,unsigned>(true,found->second);
    }

  unsigned short register_type(const std::type_info& info, dump_context::dump_callback callback)
    {
      std::string key = info.name();
      unsigned short data = ++m_max_key;
      m_callbacks[key] = std::make_pair(data,callback);
      return data;
    }

  bool is_callback(const std::type_info& info) const
    {
      return m_callbacks.find(info.name()) != m_callbacks.end();
    }

  dump_context::callback_data lookup_type(const std::type_info& info) const throw(persistent_illegal_type)
    {
      std::string key = info.name();
      callback_map::const_iterator found = m_callbacks.find(key);
      if (found == m_callbacks.end())
        throw persistent_illegal_type(key);
      return found->second;
    }

  unsigned short register_interface(const std::type_info& info)
    {
      std::string key = info.name();
      unsigned short data = ++m_max_key;
      m_interfaces[key] = data;
      return data;
    }

  bool is_interface(const std::type_info& info) const
    {
      return m_interfaces.find(info.name()) != m_interfaces.end();
    }

  unsigned short lookup_interface(const std::type_info& info) const throw(persistent_illegal_type)
    {
      std::string key = info.name();
      interface_map::const_iterator found = m_interfaces.find(key);
      if (found == m_interfaces.end())
        throw persistent_illegal_type(key);
      return found->second;
    }
};

////////////////////////////////////////////////////////////////////////////////

dump_context::dump_context(const otext& _device, unsigned char _version) throw(persistent_dump_failed) : 
  m_body(0)
{
  m_body = new dump_context_body(_device,_version);
}

dump_context::~dump_context(void)
{
  delete m_body;
}

void dump_context::put(unsigned char data) throw(persistent_dump_failed)
{
  DEBUG_ASSERT(m_body);
  m_body->put(data);
}

const otext& dump_context::device(void) const
{
  DEBUG_ASSERT(m_body);
  return m_body->device();
}

unsigned char dump_context::version(void) const
{
  DEBUG_ASSERT(m_body);
  return m_body->version();
}

bool dump_context::little_endian(void) const
{
  DEBUG_ASSERT(m_body);
  return m_body->little_endian();
}

std::pair<bool,unsigned> dump_context::pointer_map(const void* const pointer)
{
  DEBUG_ASSERT(m_body);
  return m_body->pointer_map(pointer);
}

unsigned short dump_context::register_type(const std::type_info& info, dump_context::dump_callback callback)
{
  DEBUG_ASSERT(m_body);
  return m_body->register_type(info,callback);
}

bool dump_context::is_callback(const std::type_info& info) const
{
  DEBUG_ASSERT(m_body);
  return m_body->is_callback(info);
}

dump_context::callback_data dump_context::lookup_type(const std::type_info& info) const throw(persistent_illegal_type)
{
  DEBUG_ASSERT(m_body);
  return m_body->lookup_type(info);
}

unsigned short dump_context::register_interface(const std::type_info& info)
{
  DEBUG_ASSERT(m_body);
  return m_body->register_interface(info);
}

bool dump_context::is_interface(const std::type_info& info) const
{
  DEBUG_ASSERT(m_body);
  return m_body->is_interface(info);
}

unsigned short dump_context::lookup_interface(const std::type_info& info) const throw(persistent_illegal_type)
{
  DEBUG_ASSERT(m_body);
  return m_body->lookup_interface(info);
}

void dump_context::register_all(dump_context::installer _installer)
{
  DEBUG_ASSERT(m_body);
  if (_installer) _installer(*this);
}

////////////////////////////////////////////////////////////////////////////////
// restore context classes
////////////////////////////////////////////////////////////////////////////////

class restore_context_body
{
public:
  typedef smart_ptr_clone<persistent> persistent_ptr;
  typedef std::map<unsigned,void*> magic_map;
  typedef std::map<unsigned short,restore_context::callback_data> callback_map;
  typedef std::map<unsigned short,persistent_ptr> interface_map;

  unsigned short m_max_key;
  unsigned char m_version;
  bool m_little_endian;
  itext m_device;
  magic_map m_pointers;
  callback_map m_callbacks;
  interface_map m_interfaces;

  restore_context_body(const itext& _device) throw(persistent_restore_failed) : 
    m_max_key(0), m_little_endian(::little_endian()), m_device(_device)
    {
      m_device.set_binary_mode();
      // map a null pointer onto magic number zero
      m_pointers[0] = 0;
      // get the dump version and see if we support it
      m_version = (unsigned char)get();
      if (m_version != 1 && m_version != 2)
        throw persistent_restore_failed(std::string("wrong version: ") + to_string(m_version));
    }

  ~restore_context_body(void)
    {
    }

  const itext& device(void) const
    {
      return m_device;
    }

  unsigned char version(void) const
    {
      return m_version;
    }

  bool little_endian(void) const
    {
      return m_little_endian;
    }

  int get(void) throw(persistent_restore_failed)
    {
      int result = m_device.get();
      if (result < 0)
      {
        if (m_device.error()) throw persistent_restore_failed(std::string("input device error: ") + m_device.error_string());
        throw persistent_restore_failed(std::string("premature end of file"));
      }
      return result;
    }

  std::pair<bool,void*> pointer_map(unsigned magic)
    {
      magic_map::iterator found = m_pointers.find(magic);
      if (found == m_pointers.end())
      {
        // this magic number has never been seen before
        return std::pair<bool,void*>(false,0);
      }
      return std::pair<bool,void*>(true,found->second);
    }

  void pointer_add(unsigned magic, void* new_pointer)
    {
      m_pointers[magic] = new_pointer;
    }

  unsigned short register_type(restore_context::create_callback create, restore_context::restore_callback restore)
    {
      unsigned short key = ++m_max_key;
      m_callbacks[key] = std::make_pair(create,restore);
      return key;
    }

  bool is_callback(unsigned short key) const
    {
      return m_callbacks.find(key) != m_callbacks.end();
    }

  restore_context::callback_data lookup_type(unsigned short key) const throw(persistent_illegal_type)
    {
      callback_map::const_iterator found = m_callbacks.find(key);
      if (found == m_callbacks.end())
        throw persistent_illegal_type(key);
      return found->second;
    }

  unsigned short register_interface(const persistent& sample)
    {
      unsigned short key = ++m_max_key;
      m_interfaces[key] = persistent_ptr(sample);
      return key;
    }

  bool is_interface(unsigned short key) const
    {
      return m_interfaces.find(key) != m_interfaces.end();
    }

  const persistent& lookup_interface(unsigned short key) const throw(persistent_illegal_type)
    {
      interface_map::const_iterator found = m_interfaces.find(key);
      if (found == m_interfaces.end())
        throw persistent_illegal_type(key);
      return *(found->second);
    }
};

////////////////////////////////////////////////////////////////////////////////

restore_context::restore_context(const itext& _device) throw(persistent_restore_failed) : 
  m_body(0)
{
  m_body = new restore_context_body(_device);
}

restore_context::~restore_context(void)
{
  delete m_body;
}

const itext& restore_context::device(void) const
{
  DEBUG_ASSERT(m_body);
  return m_body->device();
}

unsigned char restore_context::version(void) const
{
  DEBUG_ASSERT(m_body);
  return m_body->version();
}

bool restore_context::little_endian(void) const
{
  DEBUG_ASSERT(m_body);
  return m_body->little_endian();
}

int restore_context::get(void) throw(persistent_restore_failed)
{
  DEBUG_ASSERT(m_body);
  return m_body->get();
}

std::pair<bool,void*> restore_context::pointer_map(unsigned magic)
{
  DEBUG_ASSERT(m_body);
  return m_body->pointer_map(magic);
}

void restore_context::pointer_add(unsigned magic, void* new_pointer)
{
  DEBUG_ASSERT(m_body);
  m_body->pointer_add(magic,new_pointer);
}

unsigned short restore_context::register_type(restore_context::create_callback create, restore_context::restore_callback restore)
{
  DEBUG_ASSERT(m_body);
  return m_body->register_type(create,restore);
}

bool restore_context::is_callback(unsigned short key) const
{
  DEBUG_ASSERT(m_body);
  return m_body->is_callback(key);
}

restore_context::callback_data restore_context::lookup_type(unsigned short key) const throw(persistent_illegal_type)
{
  DEBUG_ASSERT(m_body);
  return m_body->lookup_type(key);
}

unsigned short restore_context::register_interface(const persistent& sample)
{
  DEBUG_ASSERT(m_body);
  return m_body->register_interface(sample);
}

bool restore_context::is_interface(unsigned short key) const
{
  DEBUG_ASSERT(m_body);
  return m_body->is_interface(key);
}

const persistent& restore_context::lookup_interface(unsigned short key) const throw(persistent_illegal_type)
{
  DEBUG_ASSERT(m_body);
  return m_body->lookup_interface(key);
}

void restore_context::register_all(restore_context::installer _installer)
{
  DEBUG_ASSERT(m_body);
  if (_installer) _installer(*this);
}

////////////////////////////////////////////////////////////////////////////////
// Macro for mapping either endian data onto little-endian addressing to make
// my life easier in writing this code! I think better in little-endian mode
// so the macro does nothing in that mode but maps little-endian onto
// big-endian addressing in big-endian mode

#define INDEX(index) ((context.little_endian()) ? (index) : ((bytes) - (index) - 1))

////////////////////////////////////////////////////////////////////////////////
// Integer types
// format: {size}{byte}*size
// size can be zero!
//
// A major problem is that integer types may be different sizes on different
// machines or even with different compilers on the same machine (though I
// haven't come across that yet). Neither the C nor the C++ standards specify
// the size of integer types. Dumping an int on one machine might dump 16
// bits. Restoring it on another machine might try to restore 32 bits. With
// version 0 of these persistence routines, this was not dealt with and so the
// restore function could fail. Version 1 handles different type sizes. It
// does this by writing the size to the file as well as the data, so the
// restore can therefore know how many bytes to restore independent of the
// type size.
//
// In fact, the standard does not even specify the size of char (true! And
// mind-numbingly stupid...). However, to be able to do anything at all, I've
// had to assume that a char is 1 byte.

static void dump_unsigned(dump_context& context, size_t bytes, unsigned char* data) throw(persistent_dump_failed)
{
  // first skip zero bytes - this may reduce the data to zero bytes long
  size_t i = bytes;
  while(i >= 1 && data[INDEX(i-1)] == 0)
    i--;
  // put the remaining size
  context.put((unsigned char)i);
  // and put the bytes
  while(i--)
    context.put(data[INDEX(i)]);
}

static void dump_signed(dump_context& context, size_t bytes, unsigned char* data) throw(persistent_dump_failed)
{
  // first skip all-zero or all-one bytes but only if doing so does not change the sign
  size_t i = bytes;
  if (data[INDEX(i-1)] < 128)
  {
    // positive number so discard leading zeros but only if the following byte is positive
    while(i >= 2 && data[INDEX(i-1)] == 0 && data[INDEX(i-2)] < 128)
      i--;
  }
  else
  {
    // negative number so discard leading ones but only if the following byte is negative
    while(i >= 2 && data[INDEX(i-1)] == 255 && data[INDEX(i-2)] >= 128)
      i--;
  }
  // put the remaining size
  context.put((unsigned char)i);
  // and put the bytes
  while(i--)
    context.put(data[INDEX(i)]);
}

static void restore_unsigned(restore_context& context, size_t bytes, unsigned char* data) throw(persistent_restore_failed)
{
  // get the dumped size from the file
  size_t dumped_bytes = (size_t)context.get();
  // zero fill any empty space
  size_t i = bytes;
  for (; i > dumped_bytes; i--)
    data[INDEX(i-1)] = 0;
  // restore the dumped bytes but discard any that don't fit
  // TODO - could detect overflow and throw an exception here
  while(i--)
  {
    int ch = context.get();
    if (i < bytes)
      data[INDEX(i)] = (unsigned char)ch;
    else
      throw persistent_restore_failed(std::string("integer overflow: restoring byte ") + to_string(i) + " of " + to_string(bytes));
  }
}

static void restore_signed(restore_context& context, size_t bytes, unsigned char* data) throw(persistent_restore_failed)
{
  // get the dumped size from the file
  size_t dumped_bytes = (size_t)context.get();
  // restore the dumped bytes but discard any that don't fit
  size_t i = dumped_bytes;
  while(i--)
  {
    int ch = context.get();
    if (i < bytes)
      data[INDEX(i)] = (unsigned char)ch;
    else
      throw persistent_restore_failed(std::string("integer overflow: restoring byte ") + to_string(i) + " of " + to_string(bytes));
  }
  // sign extend if the dumped integer was smaller
  if (dumped_bytes < bytes)
  {
    if (data[INDEX(dumped_bytes-1)] < 128)
    {
      // positive so zero fill
      for (i = dumped_bytes; i < bytes; i++)
        data[INDEX(i)] = 0;
    }
    else
    {
      // negative so one fill
      for (i = dumped_bytes; i < bytes; i++)
        data[INDEX(i)] = 0xff;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// exported functions

// bool is dumped and restored as an unsigned char
void dump(dump_context& context, const bool& data) throw(persistent_dump_failed)
{
  context.put((unsigned char)data);
}

void restore(restore_context& context, bool& data) throw(persistent_restore_failed)
{
  data = context.get() != 0;
}

// char is dumped and restored as an unsigned char because the signedness of char is not defined and can vary
void dump(dump_context& context, const char& data) throw(persistent_dump_failed)
{
  context.put((unsigned char)data);
}

void restore(restore_context& context, char& data) throw(persistent_restore_failed)
{
  data = (char)(unsigned char)context.get();
}

void dump(dump_context& context, const signed char& data) throw(persistent_dump_failed)
{
  context.put((unsigned char)data);
}

void restore(restore_context& context, signed char& data) throw(persistent_restore_failed)
{
  data = (signed char)(unsigned char)context.get();
}

void dump(dump_context& context, const unsigned char& data) throw(persistent_dump_failed)
{
  context.put((unsigned char)data);
}

void restore(restore_context& context, unsigned char& data) throw(persistent_restore_failed)
{
  data = (signed char)(unsigned char)context.get();
}

void dump(dump_context& context, const short& data) throw(persistent_dump_failed)
{
  dump_signed(context, sizeof(short), (unsigned char*)&data);
}

void restore(restore_context& context, short& data) throw(persistent_restore_failed)
{
  restore_signed(context, sizeof(short),(unsigned char*)&data);
}

void dump(dump_context& context, const unsigned short& data) throw(persistent_dump_failed)
{
  dump_unsigned(context, sizeof(unsigned short), (unsigned char*)&data);
}

void restore(restore_context& context, unsigned short& data) throw(persistent_restore_failed)
{
  restore_unsigned(context, sizeof(unsigned short),(unsigned char*)&data);
}

void dump(dump_context& context, const int& data) throw(persistent_dump_failed)
{
  dump_signed(context, sizeof(int), (unsigned char*)&data);
}

void restore(restore_context& context, int& data) throw(persistent_restore_failed)
{
  restore_signed(context, sizeof(int),(unsigned char*)&data);
}

void dump(dump_context& context, const unsigned& data) throw(persistent_dump_failed)
{
  dump_unsigned(context, sizeof(unsigned), (unsigned char*)&data);
}

void restore(restore_context& context, unsigned& data) throw(persistent_restore_failed)
{
  restore_unsigned(context, sizeof(unsigned),(unsigned char*)&data);
}

void dump(dump_context& context, const long& data) throw(persistent_dump_failed)
{
  dump_signed(context, sizeof(long), (unsigned char*)&data);
}

void restore(restore_context& context, long& data) throw(persistent_restore_failed)
{
  restore_signed(context, sizeof(long),(unsigned char*)&data);
}

void dump(dump_context& context, const unsigned long& data) throw(persistent_dump_failed)
{
  dump_unsigned(context, sizeof(unsigned long), (unsigned char*)&data);
}

void restore(restore_context& context, unsigned long& data) throw(persistent_restore_failed)
{
  restore_unsigned(context, sizeof(unsigned long),(unsigned char*)&data);
}

/////////////////////////////////////////////////////////////////////
// floating point types
// format: {size}{byte}*size
// ordering is msB first

// this uses a similar mechanism to integer dumps. However, it is not clear how
// the big-endian and little-endian argument applies to multi-word data so
// this may need reworking by splitting into words and then bytes.

static void dump_float(dump_context& context, size_t bytes, unsigned char* data) throw(persistent_dump_failed)
{
  size_t i = bytes;
  // put the size
  context.put((unsigned char)i);
  // and put the bytes
  while(i--)
    context.put(data[INDEX(i)]);
}

static void restore_float(restore_context& context, size_t bytes, unsigned char* data) throw(persistent_restore_failed)
{
  // get the dumped size from the file
  size_t dumped_bytes = (size_t)context.get();
  // get the bytes from the file
  size_t i = dumped_bytes;
  while(i--)
  {
    int ch = context.get();
    if (i < bytes)
      data[INDEX(i)] = (unsigned char)ch;
  }
  // however, if the dumped size was different I don't know how to map the formats, so give an error
  if (dumped_bytes != bytes)
    throw persistent_restore_failed(std::string("size mismatch: dumped ") + to_string(dumped_bytes) + std::string(" bytes, restored ") + 
                                    to_string(bytes) + std::string(" bytes"));
}

////////////////////////////////////////////////////////////////////////////////
// exported functions which simply call the low-levl byte-dump and byte-restore routines above

void dump(dump_context& context, const float& data) throw(persistent_dump_failed)
{
  dump_float(context, sizeof(float), (unsigned char*)&data);
}

void restore(restore_context& context, float& data) throw(persistent_restore_failed)
{
  restore_float(context, sizeof(float), (unsigned char*)&data);
}

void dump(dump_context& context, const double& data) throw(persistent_dump_failed)
{
  dump_float(context, sizeof(double), (unsigned char*)&data);
}

void restore(restore_context& context, double& data) throw(persistent_restore_failed)
{
  restore_float(context, sizeof(double), (unsigned char*)&data);
}

////////////////////////////////////////////////////////////////////////////////
// Null-terminated char arrays
// Format: address [ size data ]

void dump(dump_context& context, char*& data) throw(persistent_dump_failed)
{
  // register the address and get the magic key for it
  std::pair<bool,unsigned> mapping = context.pointer_map(data);
  dump(context,mapping.second);
  // if the address is null, then that is all that we need to do
  // however, if it is non-null and this is the first sight of the address, dump the contents
  if (data && !mapping.first)
  {
    unsigned size = strlen(data);
    dump(context,size);
    for (unsigned i = 0; i < size; i++)
      dump(context,data[i]);
  }
}

void restore(restore_context& context, char*& data) throw(persistent_restore_failed)
{
  // destroy any previous contents
  if (data)
  {
    delete[] data;
    data = 0;
  }
  // get the magic key
  unsigned magic = 0;
  restore(context,magic);
  // now lookup the magic key to see if this pointer has already been restored
  // null pointers are always flagged as already restored
  std::pair<bool,void*> address = context.pointer_map(magic);
  if (!address.first)
  {
    // this pointer has never been seen before and is non-null
    // restore the string
    size_t size = 0;
    restore(context,size);
    data = new char[size+1];
    for (size_t i = 0; i < size; i++)
      restore(context,data[i]);
    data[size] = '\0';
    // add this pointer to the set of already seen objects
    context.pointer_add(magic,data);
  }
}

////////////////////////////////////////////////////////////////////////////////

void dump(dump_context& context, const std::string& data) throw(persistent_dump_failed)
{
  dump_basic_string(context, data);
}

void restore(restore_context& context, std::string& data) throw(persistent_restore_failed)
{
  restore_basic_string(context, data);
}

////////////////////////////////////////////////////////////////////////////////
