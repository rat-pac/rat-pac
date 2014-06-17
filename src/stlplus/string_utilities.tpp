/*------------------------------------------------------------------------------

  Author:    Andy Rushton
  Copyright: (c) Andy Rushton, 2004
  License:   BSD License, see ../docs/license.html

  template implementations

  ------------------------------------------------------------------------------*/

////////////////////////////////////////////////////////////////////////////////
// pointers

template <typename T>
std::string pointer_to_string(const T* value,
                              const std::string& null_string, const std::string& prefix, const std::string& suffix)
{
  return value ? (prefix + to_string(*value) + suffix) : null_string;
}

template <typename T>
otext& print_pointer(otext& str, const T* value,
                     const std::string& null_string, const std::string& prefix, const std::string& suffix)
{
  if (!value)
    str << null_string;
  else
  {
    str << prefix;
    print(str, *value);
    str << suffix;
  }
  return str;
}

template <typename T>
otext& print_pointer(otext& str, const T* value, unsigned indent,
                     const std::string& null_string, const std::string& prefix, const std::string& suffix)
{
  print_indent(str, indent);
  print_pointer(str, value, null_string, prefix, suffix);
  return str << endl;
}

////////////////////////////////////////////////////////////////////////////////
// bitset

template<size_t N>
std::string bitset_to_string(const std::bitset<N>& data)
{
  std::string result;
  for (unsigned i = data.size(); i--; )
    result += data[i] ? '1' : '0';
  return result;
}

template<size_t N>
otext& print_bitset(otext& str, const std::bitset<N>& value)
{
  return str << bitset_to_string(value);
}

template<size_t N>
otext& print_bitset(otext& str, const std::bitset<N>& value, unsigned indent)
{
  print_indent(str, indent);
  return str << bitset_to_string(value) << endl;
}

////////////////////////////////////////////////////////////////////////////////
// list

template<typename T>
std::string list_to_string(const std::list<T>& values, const std::string& separator)
{
  std::string result;
  for (typename std::list<T>::const_iterator i = values.begin(); i != values.end(); i++)
  {
    if (i != values.begin()) result += separator;
    result += to_string(*i);
  }
  return result;
}

template<typename T>
otext& print_list(otext& str, const std::list<T>& values, const std::string& separator)
{
  for (typename std::list<T>::const_iterator i = values.begin(); i != values.end(); i++)
  {
    if (i != values.begin()) str << separator;
    print(str, *i);
  }
  return str;
}

template<typename T>
otext& print_list(otext& str, const std::list<T>& values, unsigned indent)
{
  for (typename std::list<T>::const_iterator i = values.begin(); i != values.end(); i++)
    print(str, *i, indent);
  return str;
}

////////////////////////////////////////////////////////////////////////////////
// pair

template<typename L, typename R>
std::string pair_to_string(const std::pair<L,R>& values, const std::string& separator)
{
  return to_string(values.first) + separator + to_string(values.second);
}

template<typename L, typename R>
otext& print_pair(otext& str, const std::pair<L,R>& values, const std::string& separator)
{
  print(str, values.first);
  str << separator;
  print(str, values.second);
  return str;
}

template<typename L, typename R>
otext& print_pair(otext& str, const std::pair<L,R>& values, const std::string& separator, unsigned indent)
{
  print_indent(str, indent);
  print_pair(str, values, separator);
  return str << endl;
}

////////////////////////////////////////////////////////////////////////////////
// map

template<typename K, typename T, typename P>
std::string map_to_string(const std::map<K,T,P>& values,
                                   const std::string& pair_separator, const std::string& separator)
{
  std::string result;
  for (typename std::map<K,T,P>::const_iterator i = values.begin(); i != values.end(); i++)
  {
    if (i != values.begin()) result += separator;
    result += pair_to_string(*i, pair_separator);
  }
  return result;
}

template<typename K, typename T, typename P>
otext& print_map(otext& str, const std::map<K,T,P>& values,
                                   const std::string& pair_separator, const std::string& separator)
{
  for (typename std::map<K,T,P>::const_iterator i = values.begin(); i != values.end(); i++)
  {
    if (i != values.begin()) str << separator;
    print_pair(str, *i, pair_separator);
  }
  return str;
}

template<typename K, typename T, typename P>
otext& print_map(otext& str, const std::map<K,T,P>& values,
                                   const std::string& pair_separator, unsigned indent)
{
  for (typename std::map<K,T,P>::const_iterator i = values.begin(); i != values.end(); i++)
    print_pair(str, *i, pair_separator, indent);
  return str;
}

////////////////////////////////////////////////////////////////////////////////
// multimap

template<typename K, typename T, typename P>
std::string multimap_to_string(const std::multimap<K,T,P>& values,
                                        const std::string& pair_separator, const std::string& separator)
{
  std::string result;
  for (typename std::multimap<K,T,P>::const_iterator i = values.begin(); i != values.end(); i++)
  {
    if (i != values.begin()) result += separator;
    result += pair_to_string(*i, pair_separator);
  }
  return result;
}

template<typename K, typename T, typename P>
otext& print_multimap(otext& str, const std::multimap<K,T,P>& values,
                                        const std::string& pair_separator, const std::string& separator)
{
  for (typename std::multimap<K,T,P>::const_iterator i = values.begin(); i != values.end(); i++)
  {
    if (i != values.begin()) str << separator;
    print_pair(str, *i, pair_separator);
  }
  return str;
}

template<typename K, typename T, typename P>
otext& print_multimap(otext& str, const std::multimap<K,T,P>& values,
                                        const std::string& pair_separator, unsigned indent)
{
  for (typename std::multimap<K,T,P>::const_iterator i = values.begin(); i != values.end(); i++)
    print_pair(str, *i, pair_separator, indent);
  return str;
}

////////////////////////////////////////////////////////////////////////////////
// set

template<typename K, typename P>
std::string set_to_string(const std::set<K,P>& values, const std::string& separator)
{
  std::string result;
  for (typename std::set<K,P>::const_iterator i = values.begin(); i != values.end(); i++)
  {
    if (i != values.begin()) result += separator;
    result += to_string(*i);
  }
  return result;
}

template<typename K, typename P>
otext& print_set(otext& str, const std::set<K,P>& values, const std::string& separator)
{
  for (typename std::set<K,P>::const_iterator i = values.begin(); i != values.end(); i++)
  {
    if (i != values.begin()) str << separator;
    print(str, *i);
  }
  return str;
}

template<typename K, typename P>
otext& print_set(otext& str, const std::set<K,P>& values, unsigned indent)
{
  for (typename std::set<K,P>::const_iterator i = values.begin(); i != values.end(); i++)
    print(str, *i, indent);
  return str;
}

////////////////////////////////////////////////////////////////////////////////
// multiset

template<typename K, typename P>
std::string multiset_to_string(const std::multiset<K,P>& values, const std::string& separator)
{
  std::string result;
  for (typename std::multiset<K,P>::const_iterator i = values.begin(); i != values.end(); i++)
  {
    if (i != values.begin()) result += separator;
    result += to_string(*i);
  }
  return result;
}

template<typename K, typename P>
otext& print_multiset(otext& str, const std::multiset<K,P>& values, const std::string& separator)
{
  for (typename std::multiset<K,P>::const_iterator i = values.begin(); i != values.end(); i++)
  {
    if (i != values.begin()) str << separator;
    print(str, *i);
  }
  return str;
}

template<typename K, typename P>
otext& print_multiset(otext& str, const std::multiset<K,P>& values, unsigned indent)
{
  for (typename std::multiset<K,P>::const_iterator i = values.begin(); i != values.end(); i++)
    print(str, *i, indent);
  return str;
}

////////////////////////////////////////////////////////////////////////////////
// vector

template<typename T>
std::string vector_to_string(const std::vector<T>& values, const std::string& separator)
{
  std::string result;
  for (unsigned i = 0; i < values.size(); i++)
  {
    if (i > 0) result += separator;
    result += to_string(values[i]);
  }
  return result;
}

template<typename T>
otext& print_vector(otext& str, const std::vector<T>& values, const std::string& separator)
{
  for (unsigned i = 0; i < values.size(); i++)
  {
    if (i > 0) str << separator;
    print(str, values[i]);
  }
  return str;
}

template<typename T>
otext& print_vector(otext& str, const std::vector<T>& values, unsigned indent)
{
  for (unsigned i = 0; i < values.size(); i++)
    print(str, values[i], indent);
  return str;
}
