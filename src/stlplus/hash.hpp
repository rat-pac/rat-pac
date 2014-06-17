#ifndef HASH_HPP
#define HASH_HPP
/*----------------------------------------------------------------------------

  Author:    Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License, see ../docs/license.html

  A chained hash table using STL semantics

------------------------------------------------------------------------------*/
#include "os_fixes.hpp"
#include "textio.hpp"
#include "persistent.hpp"
#include "exceptions.hpp"
#include <map>

namespace stlplus {
////////////////////////////////////////////////////////////////////////////////
// internals

template<typename K, typename T, class H, class E> class hash;
template<typename K, typename T> class hash_element;

////////////////////////////////////////////////////////////////////////////////
// iterator class

template<typename K, typename T, class H, class E, typename V>
class hash_iterator
{
public:
  friend class hash<K,T,H,E>;

  // local type definitions
  // an iterator points to a value whilst a const_iterator points to a const value
  typedef V                                                  value_type;
  typedef hash_iterator<K,T,H,E,std::pair<const K,T> >       iterator;
  typedef hash_iterator<K,T,H,E,const std::pair<const K,T> > const_iterator;
  typedef hash_iterator<K,T,H,E,V>                           this_iterator;
  typedef V&                                                 reference;
  typedef V*                                                 pointer;

  // constructor to create a null iterator - you must assign a valid value to this iterator before using it
  // any attempt to dereference or use a null iterator is an error
  // the only valid thing you can do is assign an iterator to it
  hash_iterator(void);
  ~hash_iterator(void);

  // tests
  // a null iterator is one that has not been initialised with a value yet
  // i.e. you just declared it but didn't assign to it
  bool null(void) const;
  // an end iterator is one that points to the end element of the list of nodes
  // in STL conventions this is one past the last valid element and must not be dereferenced
  bool end(void) const;
  // a valid iterator is one that can be dereferenced
  // i.e. non-null and non-end
  bool valid(void) const;

  // get the hash container that created this iterator
  // a null iterator doesn't have an owner so returns a null pointer
  const hash<K,T,H,E>* owner(void) const;

  // Type conversion methods allow const_iterator and iterator to be converted
  // convert an iterator/const_iterator to a const_iterator
  const_iterator constify(void) const;
  // convert an iterator/const_iterator to an iterator
  iterator deconstify(void) const;

  // increment operators used to step through the set of all values in a hash
  // it is only legal to increment a valid iterator
  // there's no decrement - I've only implemented this as a unidirectional iterator
  // pre-increment
  this_iterator& operator ++ (void)
    throw(null_dereference,end_dereference);
  // post-increment
  this_iterator operator ++ (int)
    throw(null_dereference,end_dereference);

  // tests useful for putting iterators into other STL structures and for testing whether iteration has completed
  bool operator == (const this_iterator& r) const;
  bool operator != (const this_iterator& r) const;
  bool operator < (const this_iterator& r) const;

  // access the value - a const_iterator gives you a const value, an iterator a non-const value
  // it is illegal to dereference an invalid (i.e. null or end) iterator
  reference operator*(void) const
    throw(null_dereference,end_dereference);
  pointer operator->(void) const
    throw(null_dereference,end_dereference);

  // Note: hash iterators are not persistent for a good reason: they are
  // invalidated by rehashing and so it is not a good idea to build data
  // structures containing hash iterators in the first place

private:
  friend class hash_element<K,T>;

  const hash<K,T,H,E>* m_owner;
  unsigned m_bin;
  hash_element<K,T>* m_element;

  void check_owner(const hash<K,T,H,E>* owner) const
    throw(wrong_object);
  void check_non_null(void) const
    throw(null_dereference);
  void check_non_end(void) const
    throw(end_dereference);
  void check_valid(void) const
    throw(null_dereference,end_dereference);
  void check(const hash<K,T,H,E>* owner) const
    throw(wrong_object,null_dereference,end_dereference);

  // constructor used by hash to create a non-null iterator
  // you cannot create a valid iterator except by calling a hash method that returns one
  hash_iterator(const hash<K,T,H,E>* owner, unsigned bin, hash_element<K,T>* element);
};

////////////////////////////////////////////////////////////////////////////////
// Hash class
// K = key type
// T = value type
// H = hash function object with the profile 'unsigned H(const K&)'
// E = equal function object with the profile 'bool E(const K&, const K&)' defaults to equal_to which in turn calls '=='

template<typename K, typename T, class H, class E = std::equal_to<K> >
class hash
{
public:
  typedef unsigned                                size_type;
  typedef K                                       key_type;
  typedef T                                       data_type;
  typedef T                                       mapped_type;
  typedef std::pair<const K, T>                   value_type;
  typedef hash_iterator<K,T,H,E,value_type>       iterator;
  typedef hash_iterator<K,T,H,E,const value_type> const_iterator;

  // construct a hash table with specified number of bins
  // the default 0 bins means leave it to the table to decide
  // specifying 0 bins also enables auto-rehashing, otherwise auto-rehashing defaults off
  hash(unsigned bins = 0);
  ~hash(void);

  // copy and equality copy the data elements but not the size of the copied table
  hash(const hash&);
  hash& operator = (const hash&);

  // test for an empty table and for the size of a table - efficient because the size is stored separately from the table contents
  bool empty(void) const;
  unsigned size(void) const;

  // test for equality - two hashes are equal if they contain equal values
  bool operator == (const hash&) const;
  bool operator != (const hash&) const;

  // switch auto-rehash on
  void auto_rehash(void);
  // switch auto-rehash off
  void manual_rehash(void);
  // force a rehash now
  // default of 0 means implement built-in size calculation for rehashing (recommended - it doubles the number of bins)
  void rehash(unsigned bins = 0);
  // test the loading ratio, which is the size divided by the number of bins
  // use this if you are doing your own rehashing
  // the recommendation is to double the bins when the loading exceeds 0.5 which is what auto-rehashing does
  float loading(void) const;

  // test for the presence of a key
  bool present(const K& key) const;
  // provide map equivalent key count function (0 or 1, as not a multimap)
  size_type count(const K& key) const;

  // insert a new key/data pair - replaces any previous value for this key
  iterator insert(const K& key, const T& data);
  // insert a copy of the pair into the table (std::map compatible)
  std::pair<iterator, bool> insert(const value_type& value);
  // insert a new key and return the iterator so that the data can be filled in
  iterator insert(const K& key);

  // remove a key/data pair from the hash table
  bool erase(const K& key);
  // remove all elements from the hash table
  void erase(void);
  // provide the std::map equivalent clear function
  void clear(void);

  // find a key and return an iterator to it
  // The iterator is like a pointer to a pair<const K,T>
  // end() is returned if the find fails
  const_iterator find(const K& key) const;
  iterator find(const K& key);
  // returns the data corresponding to the key
  // the const version is used by the compiler on const hashes and cannot change the hash, so find failure causes an exception
  // the non-const version is used by the compiler on non-const hashes and is like map - it creates a new key/data pair if find fails
  const T& operator[] (const K& key) const;
  T& operator[] (const K& key);

  // iterators allow the hash table to be traversed
  // iterators remain valid unless an item is removed or unless a rehash happens
  const_iterator begin(void) const;
  iterator begin(void);
  const_iterator end(void) const;
  iterator end(void);

  // diagnostic report shows the number of items in each bin so can be used to diagnose effectiveness of hash functions
  void debug_report(otext&, unsigned indent = 0) const;
  // the same diagnostic function that returns a string, rather than use an otext stream
  std::string debug_report(unsigned indent = 0) const;

  // persistence methods
  void dump(dump_context&) const
    throw(persistent_dump_failed);
  void restore(restore_context&)
    throw(persistent_restore_failed);

  // internals
private:
  friend class hash_element<K,T>;
  friend class hash_iterator<K,T,H,E,std::pair<const K,T> >;
  friend class hash_iterator<K,T,H,E,const std::pair<const K,T> >;

  unsigned m_rehash;
  unsigned m_bins;
  unsigned m_size;
  hash_element<K,T>** m_values;
};

////////////////////////////////////////////////////////////////////////////////

template<typename K, typename T, class H, class E>
otext& print(otext& str, const hash<K,T,H,E>& table, unsigned indent = 0);

template<typename K, typename T, class H, class E>
otext& operator << (otext& str, const hash<K,T,H,E>& table);

////////////////////////////////////////////////////////////////////////////////

template<typename K, typename T, class H, class E>
void dump_hash(dump_context& str, const hash<K,T,H,E>& data)
  throw(persistent_dump_failed);

template<typename K, typename T, class H, class E>
void restore_hash(restore_context& str, hash<K,T,H,E>& data)
  throw(persistent_restore_failed);

////////////////////////////////////////////////////////////////////////////////
}

#include "hash.tpp"
#endif
