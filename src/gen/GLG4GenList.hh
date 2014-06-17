#ifndef __GLG4GenList_h__
#define __GLG4GenList_h__

#include <queue>
#include "GLG4Gen.hh"

class TimeOrder_GLG4Gen 
{
public:
  bool operator() (const GLG4Gen *a, const GLG4Gen *b) const {
    // use greater than because priority queue top() returns the 
    // element x for which Compare(x, y) == false for all y
    return a->NextTime() > b->NextTime();
  };
};


class GLG4GenList : public std::priority_queue<GLG4Gen*, std::vector<GLG4Gen*>, 
					       TimeOrder_GLG4Gen>
{
public:
  inline void SubtractTime(double time) {
    for (std::vector<GLG4Gen*>::iterator i=c.begin(); i != c.end(); i++)
      (*i)->SubtractTime(time);
  };
  inline void clear(void) { c.clear();					       
  };
};

#endif
