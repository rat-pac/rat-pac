#ifndef __RAT_Delete__
#define __RAT_Delete__

#include <vector>
#include <functional>

namespace RAT {

  template <typename T>
  void deepdelete_vector(std::vector<T*> &vect) 
  {
    typename std::vector<T*>::iterator i;
    for (i=vect.begin(); i != vect.end(); i++)
      delete (*i);
    vect.resize(0);
  }
  
  template <typename T>
  void deepcopy_vector(std::vector<T*> &dest, const std::vector<T*> &src) {
    deepdelete_vector(dest);
    dest.resize(src.size());
    for (unsigned i=0; i < src.size(); i++) {
      if (src[i])
	dest[i] = dynamic_cast<T*>(src[i]->Clone()); // Use Clone() for proper polymorphism
      else
	dest[i] = 0;
    }
  }
  
  template <typename T>
  void endcopy_vector(std::vector<T*> &dest, const std::vector<T*> &src) {
    deepdelete_vector(dest);
    dest.resize(2);
    if(src.size() >= 2){        //
      dest[0] = dynamic_cast<T*>(src.front()->Clone()); // Use Clone() for proper polymorphism
      dest[1] = dynamic_cast<T*>(src.back()->Clone()); 
    }
  }
  
  
  
  template <class T>
  class CompareFunc: public std::binary_function<T*, T*, bool> {
  public:
    virtual bool operator() (T*, T*) = 0;
  };

} // namespace RAT

#endif
