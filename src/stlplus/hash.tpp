/*------------------------------------------------------------------------------

  Author:    Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License, see ../docs/license.html

  ------------------------------------------------------------------------------*/
#include "string_utilities.hpp"
#include "dprintf.hpp"
#include "debug.hpp"
#include "stringio.hpp"

namespace stlplus {
////////////////////////////////////////////////////////////////////////////////
// the element stored in the hash

template<typename K, typename T>
class hash_element
{
public:
  std::pair<const K, T> m_value;
  hash_element<K,T>* m_next;
  unsigned m_hash;

  hash_element(const K& key, const T& data, unsigned hash) : m_value(key,data), m_next(0), m_hash(hash) {}
  hash_element(const std::pair<const K,T>& value, unsigned hash) : m_value(value), m_next(0), m_hash(hash) {}
};

////////////////////////////////////////////////////////////////////////////////
// iterator

// checks

template<typename K, typename T, class H, class E, typename V>
void hash_iterator<K,T,H,E,V>::check_owner(const hash<K,T,H,E>* _owner) const
  throw(wrong_object)
{
  if (_owner != m_owner)
    throw wrong_object("hash iterator");
}

template<typename K, typename T, class H, class E, typename V>
void hash_iterator<K,T,H,E,V>::check_non_null(void) const
  throw(null_dereference)
{
  if (null())
    throw null_dereference("hash iterator");
}

template<typename K, typename T, class H, class E, typename V>
void hash_iterator<K,T,H,E,V>::check_non_end(void) const
  throw(end_dereference)
{
  if (end())
    throw end_dereference("hash iterator");
}

template<typename K, typename T, class H, class E, typename V>
void hash_iterator<K,T,H,E,V>::check_valid(void) const
  throw(null_dereference,end_dereference)
{
  check_non_null();
  check_non_end();
}

template<typename K, typename T, class H, class E, typename V>
void hash_iterator<K,T,H,E,V>::check(const hash<K,T,H,E>* _owner) const
  throw(wrong_object,null_dereference,end_dereference)
{
  check_valid();
  if (_owner) check_owner(_owner);
}

// null constructor

template<typename K, typename T, class H, class E, typename V>
hash_iterator<K,T,H,E,V>::hash_iterator(void) :
  m_owner(0), m_bin(0), m_element(0) 
{
}

// this iterator points to a specific element and so allows an iterator to be constructed from a list node
// if the bin is set to bins and the element to null, then this is an end() iterator
// if the element parameter is null it scans for the first element so implements the begin() behaviour
// if the hash is empty, m_bin gets set to m_bins, so this becomes an end() iterator

template<typename K, typename T, class H, class E, typename V>
hash_iterator<K,T,H,E,V>::hash_iterator(const hash<K,T,H,E>* _owner, unsigned bin, hash_element<K,T>* element) :
  m_owner(_owner), m_bin(bin), m_element(element)
{
  DEBUG_ASSERT(_owner);
  if (!element)
  {
    for (; m_bin < m_owner->m_bins; m_bin++)
    {
      if (m_owner->m_values[m_bin])
      {
        m_element = m_owner->m_values[m_bin];
        break;
      }
    }
  }
}

// destructor

template<typename K, typename T, class H, class E, typename V>
hash_iterator<K,T,H,E,V>::~hash_iterator(void)
{
}

// validity tests

template<typename K, typename T, class H, class E, typename V>
bool hash_iterator<K,T,H,E,V>::null(void) const
{
  return m_owner == 0;
}

template<typename K, typename T, class H, class E, typename V>
bool hash_iterator<K,T,H,E,V>::end(void) const
{
  if (null()) return false;
  return (m_bin == m_owner->m_bins) && m_element == 0;
}

template<typename K, typename T, class H, class E, typename V>
bool hash_iterator<K,T,H,E,V>::valid(void) const
{
  return !null() && !end();
}

// owner method

template<typename K, typename T, class H, class E, typename V>
const hash<K,T,H,E>* hash_iterator<K,T,H,E,V>::owner(void) const
{
  return m_owner;
}

// mode conversions

template<typename K, typename T, class H, class E, typename V>
typename hash_iterator<K,T,H,E,V>::const_iterator hash_iterator<K,T,H,E,V>::constify(void) const
{
  return hash_iterator<K,T,H,E,V>::const_iterator(m_owner, m_bin, m_element);
}

template<typename K, typename T, class H, class E, typename V>
typename hash_iterator<K,T,H,E,V>::iterator hash_iterator<K,T,H,E,V>::deconstify(void) const
{
  return hash_iterator<K,T,H,E,V>::iterator(m_owner, m_bin, m_element);
}

// increment operator looks for the next element in the table
// if there isn't one, then this becomes an end() iterator - m_bin = m_bins and m_element = null

template<typename K, typename T, class H, class E, typename V>
typename hash_iterator<K,T,H,E,V>::this_iterator& hash_iterator<K,T,H,E,V>::operator ++ (void)
  throw(null_dereference,end_dereference)
{
  check_valid();
  if (m_element->m_next)
    m_element = m_element->m_next;
  else
  {
    // failing that, subsequent hash values are tried until either an element is found or there are no more bins
    // in which case it becomes an end() iterator (bin == bins and element = null)
    m_element = 0;
    if (m_bin < m_owner->m_bins)
    {
      for(m_bin++; m_bin < m_owner->m_bins; m_bin++)
      {
        if (m_owner->m_values[m_bin])
        {
          m_element = m_owner->m_values[m_bin];
          break;
        }
      }
    }
  }
  return *this;
}

// post-increment is defined in terms of pre-increment

template<typename K, typename T, class H, class E, typename V>
typename hash_iterator<K,T,H,E,V>::this_iterator hash_iterator<K,T,H,E,V>::operator ++ (int)
  throw(null_dereference,end_dereference)
{
  typename hash_iterator<K,T,H,E,V>::this_iterator old = *this;
  ++(*this);
  return old;
}

// two iterators are equal if they point to the same element
// both iterators must be non-null and belong to the same table

template<typename K, typename T, class H, class E, typename V>
bool hash_iterator<K,T,H,E,V>::operator == (const hash_iterator<K,T,H,E,V>& r) const
{
  return m_element == r.m_element;
}

template<typename K, typename T, class H, class E, typename V>
bool hash_iterator<K,T,H,E,V>::operator != (const hash_iterator<K,T,H,E,V>& r) const
{
  return !operator==(r);
}

template<typename K, typename T, class H, class E, typename V>
bool hash_iterator<K,T,H,E,V>::operator < (const hash_iterator<K,T,H,E,V>& r) const
{
  return m_element < r.m_element;
}

// iterator dereferencing is only legal on a non-null iterator

template<typename K, typename T, class H, class E, typename V>
V& hash_iterator<K,T,H,E,V>::operator*(void) const
  throw(null_dereference,end_dereference)
{
  check_valid();
  return m_element->m_value;
}

template<typename K, typename T, class H, class E, typename V>
V* hash_iterator<K,T,H,E,V>::operator->(void) const
  throw(null_dereference,end_dereference)
{
  check_valid();
  return &(m_element->m_value);
}

////////////////////////////////////////////////////////////////////////////////
// hash

// totally arbitrary initial size used for auto-rehashed tables
// Stan: Changed to #define because static doesn't actually allocate space
// for the variable on Sun CC with the optimizer turned on
//static unsigned hash_default_bins = 127;
#define hash_default_bins 127

// constructor
// tests whether the user wants auto-rehash
// sets the rehash point to be a loading of 1.0 by setting it to the number of bins
// uses the user's size unless this is zero, in which case implement the default

template<typename K, typename T, class H, class E>
hash<K,T,H,E>::hash(unsigned bins) :
  m_rehash(bins), m_bins(bins > 0 ? bins : hash_default_bins), m_size(0), m_values(0)
{
  m_values = new hash_element<K,T>*[m_bins];
  for (unsigned i = 0; i < m_bins; i++)
    m_values[i] = 0;
}

template<typename K, typename T, class H, class E>
hash<K,T,H,E>::~hash(void)
{
  // delete all the elements
  clear();
  // and delete the data structure
  delete[] m_values;
  m_values = 0;
}

// as usual, implement the copy constructor i.t.o. the assignment operator

template<typename K, typename T, class H, class E>
hash<K,T,H,E>::hash(const hash<K,T,H,E>& right) :
  m_rehash(right.m_rehash), m_bins(right.m_bins), m_size(0), m_values(0)
{
  m_values = new hash_element<K,T>*[right.m_bins];
  // copy the rehash behaviour as well as the size
  for (unsigned i = 0; i < m_bins; i++)
    m_values[i] = 0;
  *this = right;
}

// assignment operator
// this is done by copying the elements
// the source and target hashes can be different sizes
// the hash is self-copy safe, i.e. it is legal to say x = x;

template<typename K, typename T, class H, class E>
hash<K,T,H,E>& hash<K,T,H,E>::operator = (const hash<K,T,H,E>& r)
{
  // make self-copy safe
  if (&r == this) return *this;
  // remove all the existing elements
  clear();
  // copy the elements across - remember that this is rehashing because the two
  // tables can be different sizes so there is no quick way of doing this by
  // copying the lists
  for (typename hash<K,T,H,E>::const_iterator i = r.begin(); i != r.end(); ++i)
    insert(i->first, i->second);
  return *this;
}

// number of values in the hash

template<typename K, typename T, class H, class E>
bool hash<K,T,H,E>::empty(void) const
{
  return m_size == 0;
}

template<typename K, typename T, class H, class E>
unsigned hash<K,T,H,E>::size(void) const
{
  return m_size;
}

// equality

template<typename K, typename T, class H, class E>
bool hash<K,T,H,E>::operator == (const hash<K,T,H,E>& right) const
{
  // this table is the same as the right table if they are the same table!
  if (&right == this) return true;
  // they must be the same size to be equal
  if (m_size != right.m_size) return false;
  // now every key in this must be in right and have the same data
  for (typename hash<K,T,H,E>::const_iterator i = begin(); i != end(); i++)
  {
    typename hash<K,T,H,E>::const_iterator found = right.find(i->first);
    if (found == right.end()) return false;
    if (!(i->second == found->second)) return false;
  }
  return true;
}

template<typename K, typename T, class H, class E>
bool hash<K,T,H,E>::operator != (const hash<K,T,H,E>& right) const
{
  return !operator==(right);
}

// set up the hash to auto-rehash at a specific size
// setting the rehash size to 0 forces manual rehashing

template<typename K, typename T, class H, class E>
void hash<K,T,H,E>::auto_rehash(void)
{
  m_rehash = m_bins;
}

template<typename K, typename T, class H, class E>
void hash<K,T,H,E>::manual_rehash(void)
{
  m_rehash = 0;
}

// the rehash function
// builds a new hash table and moves the elements (without copying) from the old to the new
// I store the un-modulused hash value in the element for more efficient rehashing
// passing 0 to the bins parameter does auto-rehashing
// passing any other value forces the number of bins

template<typename K, typename T, class H, class E>
void hash<K,T,H,E>::rehash(unsigned bins)
{
  // user specified size: just take the user's value
  // auto calculate: if the load is high, increase the size; else do nothing
  unsigned new_bins = bins ? bins : m_bins;
  if (bins == 0 && m_size > 0)
  {
    // these numbers are pretty arbitrary
    // TODO - make them user-customisable?
    float load = loading();
    if (load > 2.0)
      new_bins = (unsigned)(m_bins * load);
    else if (load > 1.0)
      new_bins = m_bins * 2;
  }
  if (new_bins == m_bins) return;
  // set the new rehashing point if auto-rehashing is on
  if (m_rehash) m_rehash = new_bins;
  // move aside the old structure
  hash_element<K,T>** old_values = m_values;
  unsigned old_bins = m_bins;
  // create a replacement structure
  m_values = new hash_element<K,T>*[new_bins];
  for (unsigned i = 0; i < new_bins; i++)
    m_values[i] = 0;
  m_bins = new_bins;
  // move all the old elements across, rehashing each one
  for (unsigned j = 0; j < old_bins; j++)
  {
    while(old_values[j])
    {
      // unhook from the old structure
      hash_element<K,T>* current = old_values[j];
      old_values[j] = current->m_next;
      // rehash using the stored un-modulused hash value
      unsigned hash_value = current->m_hash % m_bins;
      // hook it into the new structure
      current->m_next = m_values[hash_value];
      m_values[hash_value] = current;
    }
  }
  // now delete the old structure
  delete[] old_values;
}

// the loading is the average number of elements per bin
// this simplifies to the total elements divided by the number of bins

template<typename K, typename T, class H, class E>
float hash<K,T,H,E>::loading(void) const
{
  return (float)m_size / (float)m_bins;
}

// remove all elements from the table

template<typename K, typename T, class H, class E>
void hash<K,T,H,E>::erase(void)
{
  // unhook the list elements and destroy them
  for (unsigned i = 0; i < m_bins; i++)
  {
    hash_element<K,T>* current = m_values[i];
    while(current)
    {
      hash_element<K,T>* next = current->m_next;
      delete current;
      current = next;
    }
    m_values[i] = 0;
  }
  m_size = 0;
}

// test for whether a key is present in the table

template<typename K, typename T, class H, class E>
bool hash<K,T,H,E>::present(const K& key) const
{
  return find(key) != end();
}

template<typename K, typename T, class H, class E>
typename hash<K,T,H,E>::size_type hash<K,T,H,E>::count(const K& /*key*/) const
{
  return present() ? 1 : 0;
}

// add a key and data element to the table - defined in terms of the general-purpose pair insert function

template<typename K, typename T, class H, class E>
typename hash<K,T,H,E>::iterator hash<K,T,H,E>::insert(const K& key, const T& data)
{
  return insert(std::pair<const K,T>(key,data)).first;
}

// insert a key/data pair into the table
// this removes any old value with the same key since there is no multihash functionality

template<typename K, typename T, class H, class E>
std::pair<TEMPLATE_TYPENAME hash<K,T,H,E>::iterator, bool> hash<K,T,H,E>::insert(const std::pair<const K,T>& value)
{
  // if auto-rehash is enabled, implement the auto-rehash before inserting the new value
  // the table is rehashed if this insertion makes the loading exceed 1.0
  if (m_rehash && (m_size >= m_rehash)) rehash();
  // calculate the new hash value
  unsigned hash_value_full = H()(value.first);
  unsigned hash_value = hash_value_full % m_bins;
  bool inserted = true;
  // unhook any previous value with this key
  // this has been inlined from erase(key) so that the hash value is not calculated twice
  hash_element<K,T>* previous = 0;
  for (hash_element<K,T>* current = m_values[hash_value]; current; previous = current, current = current->m_next)
  {
    // first check the full stored hash value
    if (current->m_hash != hash_value_full) continue;

    // next try the equality operator
    if (!E()(current->m_value.first, value.first)) continue;

    // unhook this value and destroy it
    if (previous)
      previous->m_next = current->m_next;
    else
      m_values[hash_value] = current->m_next;
    delete current;
    m_size--;

    // we've overwritten a previous value
    inserted = false;
    
    // assume there can only be one match so we can give up now
    break;
  }
  // now hook in a new list element at the start of the list for this hash value
  hash_element<K,T>* new_item = new hash_element<K,T>(value, hash_value_full);
  new_item->m_next = m_values[hash_value];
  m_values[hash_value] = new_item;
  // remember to increment the size count
  m_size++;
  // construct an iterator from the list node, and return whether inserted
  return make_pair(SUN_TYPENAME_HACK hash<K,T,H,E>::iterator(this,hash_value,new_item), inserted);
}

// insert a key with an empty data field ready to be filled in later

template<typename K, typename T, class H, class E>
typename hash<K,T,H,E>::iterator hash<K,T,H,E>::insert(const K& key)
{
  return insert(key,T());
}

// remove a key from the table - return true if the key was found and removed, false if it wasn't present

template<typename K, typename T, class H, class E>
bool hash<K,T,H,E>::erase(const K& key)
{
  unsigned hash_value_full = H()(key);
  unsigned hash_value = hash_value_full % m_bins;
  // scan the list for an element with this key
  // need to keep a previous pointer because the lists are single-linked
  hash_element<K,T>* previous = 0;
  for (hash_element<K,T>* current = m_values[hash_value]; current; previous = current, current = current->m_next)
  {
    // first check the full stored hash value
    if (current->m_hash != hash_value_full) continue;
    
    // next try the equality operator
    if (!E()(current->m_value.first, key)) continue;

    // found this key, so unhook the element from the list
    if (previous)
      previous->m_next = current->m_next;
    else
      m_values[hash_value] = current->m_next;
    // destroy it
    delete current;
    // remember to maintain the size count
    m_size--;
    return true;
  }
  return false;
}

template<typename K, typename T, class H, class E>
void hash<K,T,H,E>::clear(void)
{
  erase();
}

// search for a key in the table and return an iterator to it
// if the search fails, returns an end() iterator

template<typename K, typename T, class H, class E>
typename hash<K,T,H,E>::const_iterator hash<K,T,H,E>::find(const K& key) const
{
  // scan the list for this key's hash value for the element with a matching key
  unsigned hash_value_full = H()(key);
  unsigned hash_value = hash_value_full % m_bins;
  for (hash_element<K,T>* current = m_values[hash_value]; current; current = current->m_next)
  {
    if (current->m_hash == hash_value_full && E()(current->m_value.first, key))
      return typename hash<K,T,H,E>::const_iterator(this, hash_value, current);
  }
  return end();
}

template<typename K, typename T, class H, class E>
typename hash<K,T,H,E>::iterator hash<K,T,H,E>::find(const K& key)
{
  // scan the list for this key's hash value for the element with a matching key
  unsigned hash_value_full = H()(key);
  unsigned hash_value = hash_value_full % m_bins;
  for (hash_element<K,T>* current = m_values[hash_value]; current; current = current->m_next)
  {
    if (current->m_hash == hash_value_full && E()(current->m_value.first, key))
      return typename hash<K,T,H,E>::iterator(this, hash_value, current);
  }
  return end();
}

// table lookup by key using the index operator[], returning a reference to the data field, not an iterator
// this is rather like the std::map's [] operator
// the difference is that I have a const and non-const version
// the const version will not create the element if not present already, but the non-const version will
// the non-const version is compatible with the behaviour of the map

template<typename K, typename T, class H, class E>
const T& hash<K,T,H,E>::operator[] (const K& key) const
{
  // this const version cannot change the hash, so has to raise an exception if the key is missing
  // TODO make this a proper exception with a throw declaration
  typename hash<K,T,H,E>::const_iterator found = find(key);
  DEBUG_ASSERT(found != end());
  return found->second;
}

template<typename K, typename T, class H, class E>
T& hash<K,T,H,E>::operator[] (const K& key)
{
  // this non-const version can change the hash, so creates a new element if the key is missing
  typename hash<K,T,H,E>::iterator found = find(key);
  if (found == end())
    found = insert(key);
  return found->second;
}

// iterators

template<typename K, typename T, class H, class E>
typename hash<K,T,H,E>::const_iterator hash<K,T,H,E>::begin(void) const
{
  return typename hash<K,T,H,E>::const_iterator(this,0,0);
}

template<typename K, typename T, class H, class E>
typename hash<K,T,H,E>::iterator hash<K,T,H,E>::begin(void)
{
  return hash<K,T,H,E>::iterator(this,0,0);
}

template<typename K, typename T, class H, class E>
typename hash<K,T,H,E>::const_iterator hash<K,T,H,E>::end(void) const
{
  return typename hash<K,T,H,E>::const_iterator(this,m_bins,0);
}

template<typename K, typename T, class H, class E>
typename hash<K,T,H,E>::iterator hash<K,T,H,E>::end(void)
{
  return typename hash<K,T,H,E>::iterator(this,m_bins,0);
}

// persistence

template<typename K, typename T, class H, class E>
void hash<K,T,H,E>::dump(dump_context& context) const
  throw(persistent_dump_failed)
{
  ::dump(context,size());
  for (typename hash<K,T,H,E>::const_iterator i = begin(); i != end(); i++)
  {
    ::dump(context,i->first);
    ::dump(context,i->second);
  }
}

template<typename K, typename T, class H, class E>
void hash<K,T,H,E>::restore(restore_context& context)
  throw(persistent_restore_failed)
{
  erase();
  size_t _size = 0;
  ::restore(context,_size);
  for (size_t j = 0; j < _size; j++)
  {
    K key;
    ::restore(context,key);
    ::restore(context,operator[](key));
  }
}

// diagnostic report shows the number of elements in each bin so that inefficient hash functions can be identified
// (they tend to cluster keys in a few bins)

template<typename K, typename T, class H, class E>
void hash<K,T,H,E>::debug_report(otext& str, unsigned indent) const
{
  // calculate some stats first
  unsigned occupied = 0;
  unsigned min_in_bin = m_size;
  unsigned max_in_bin = 0;
  for (unsigned i = 0; i < m_bins; i++)
  {
    if (m_values[i]) occupied++;
    unsigned _count = 0;
    for (hash_element<K,T>* item = m_values[i]; item; item = item->m_next) _count++;
    if (_count > max_in_bin) max_in_bin = _count;
    if (_count < min_in_bin) min_in_bin = _count;
  }
  // now print the table
  print_indent(str, indent); str << "------------------------------------------------------------------------" << endl;
  print_indent(str, indent); str << "| size:     " << m_size << endl;
  print_indent(str, indent); str << "| bins:     " << m_bins << endl;
  print_indent(str, indent); str << "| loading:  " << loading() << " ";
  if (m_rehash)
    str << "auto-rehash at " << m_rehash << endl;
  else
    str << "manual rehash" << endl;
  print_indent(str, indent); str << "| occupied: " << occupied << dformat(" (%2.1f%%)", 100.0*(float)occupied/(float)m_bins)
                                 << ", min = " << min_in_bin << ", max = " << max_in_bin << endl;
  print_indent(str, indent); str << "|-----------------------------------------------------------------------" << endl;
  print_indent(str, indent); str << "|  bin         0     1     2     3     4     5     6     7     8     9" << endl;
  print_indent(str, indent); str << "|        ---------------------------------------------------------------";
  for (unsigned j = 0; j < m_bins; j++)
  {
    if (j % 10 == 0)
    {
      str << endl;
      print_indent(str, indent); 
      str << "| " << pad(::to_string(j/10*10), align_right, 6) << " |";
    }
    unsigned _count = 0;
    for (hash_element<K,T>* item = m_values[j]; item; item = item->m_next) _count++;
    if (!_count)
      str << "     .";
    else
      str << pad(::to_string(_count), align_right, 6);
  }
  str << endl;
  print_indent(str, indent); str << "------------------------------------------------------------------------" << endl;
}

template<typename K, typename T, class H, class E>
std::string hash<K,T,H,E>::debug_report(unsigned indent) const
{
  ostext o;
  debug_report(o, indent);
  return o.get_string();
}

////////////////////////////////////////////////////////////////////////////////
// global print routines

template<typename K, typename T, class H, class E>
otext& print(otext& str, const hash<K,T,H,E>& table, unsigned indent)
{
  table.debug_report(str,indent);
  return str;
}

template<typename K, typename T, class H, class E>
otext& operator << (otext& str, const hash<K,T,H,E>& table)
{
  return print(str,table);
}

////////////////////////////////////////////////////////////////////////////////
// persistence

template<typename K, typename T, class H, class E>
void dump_hash(dump_context& context, const hash<K,T,H,E>& data)

  throw(persistent_dump_failed)
{
  data.dump(context);
}

template<typename K, typename T, class H, class E>
void restore_hash(restore_context& context, hash<K,T,H,E>& data) throw(persistent_restore_failed)
{
  data.restore(context);
}

////////////////////////////////////////////////////////////////////////////////
}
