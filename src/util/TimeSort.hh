#ifndef __RAT_TimeSort__
#define __RAT_TimeSort__

#include <utility>
#include <vector>
#include <algorithm>

using namespace std;

namespace RAT{
  /** Function to sort time intervals. */
  template <typename T>
  inline bool TimeSort(const pair<T,T>& firstPair,
		const pair<T,T>& secondPair){
    return (firstPair.first < secondPair.first);
  }
  
  /* Functions to sort and merge overlapping time intervals. */
  inline void TimeSortMerge(vector<pair<int,int> >& pulseTimes){
    sort(pulseTimes.begin(), pulseTimes.end(), TimeSort<int>);
    
    if (pulseTimes.size() == 0)
      return;
    
    vector<pair<int,int> > merged;
    pair<int, int> current = pulseTimes[0];
    
    for (unsigned int i=1; i < pulseTimes.size(); i++) {
      // offset -1 to fuse adjacent integers
      if (pulseTimes[i].first - 1 <= current.second) {
	if (current.second < pulseTimes[i].second)
	  current.second = pulseTimes[i].second;
      } else {
	merged.push_back(current);
	current = pulseTimes[i];
      }
    }
    merged.push_back(current);
    
    // Exchange storage
    pulseTimes.swap(merged);
  }
  
  inline void TimeSortMerge(vector<pair<double,double> >& pulseTimes){
    sort(pulseTimes.begin(), pulseTimes.end(), TimeSort<double>);
    
    if (pulseTimes.size() == 0)
      return;
    
    vector<pair<double,double> > merged;
    pair<double, double> current = pulseTimes[0];
    
    for (unsigned int i=1; i < pulseTimes.size(); i++) {
      if (pulseTimes[i].first <= current.second) {
	if (current.second < pulseTimes[i].second)
	  current.second = pulseTimes[i].second;
      } else {
	merged.push_back(current);
	current = pulseTimes[i];
      }
    }
    merged.push_back(current);
    
    // Exchange storage
    pulseTimes.swap(merged);
  }

}

#endif
