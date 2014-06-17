#include <RAT/DSReader.hh>
#include <iostream>

ClassImp(RAT::DSReader)

namespace RAT {

#undef DEBUG

DSReader::DSReader(const char *filename) : T("T")
{  
  T.Add(filename);

  next = 0;
  total = T.GetEntries();

#ifdef DEBUG
  std::cout << "DSReader::DSReader - "
	    << "filename='" << filename
	    << "', total=" << total << std::endl;
#endif

  ds = new DS::Root();
  T.SetBranchAddress("ds", &ds);
}

DSReader::~DSReader()
{
  delete ds;
}

void DSReader::Add(const char* filename) 
{ 
  T.Add(filename); 
  total = T.GetEntries(); 

#ifdef DEBUG
  std::cout << "DSReader::Add - "
	    << "filename='" << filename
	    << "', total=" << total << std::endl;
#endif
}

DS::Root *DSReader::NextEvent()
{ 
  if (next < total) {
    T.GetEntry(next); 
    next++;
    return ds;
  } else
    return 0;
}


} // namespace RAT
