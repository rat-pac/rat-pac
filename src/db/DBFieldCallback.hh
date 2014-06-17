#ifndef __RAT_DBFieldCallback__
#define __RAT_DBFieldCallback__

#include <vector>

namespace RAT {
  
  class DBFieldCallback {
  public:
    virtual ~DBFieldCallback() { };
    virtual std::vector<int> FetchIArray(const std::string &tableID, const std::string &fieldname) = 0;
    virtual std::vector<double> FetchDArray(const std::string &tableID, const std::string &fieldname) = 0;
  };
  
}// namespace RAT

#endif
