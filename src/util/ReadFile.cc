#include <RAT/ReadFile.hh>
#include <stdio.h>
#if __HAS_BZ2
#include <bzlib.h>
#endif

namespace RAT {
  
  int ReadFile(const std::string &filename, std::string &filecontents)
  {
    bool bzip2 = true;

    const int nmax = 4096;
    char buffer[nmax];
    int nread = 0;

#if __HAS_BZ2
    // Try bzip2 first
    BZFILE *bf = BZ2_bzopen(filename.c_str(), "r");

    if (bf == NULL)
      return -1;

    filecontents = "";
    while( (nread = BZ2_bzread(bf, buffer, nmax)) ){

      // Did the read fail?
      if (nread < 0) {
        bzip2 = false;
        break;
      }
        
      filecontents.append(buffer, nread);
    }
    BZ2_bzclose(bf);
#else
    bzip2 = false;
#endif
    
    if (!bzip2) {
      // Not BZIP2 format, so just dump the file contents raw into the string
      FILE *f = fopen(filename.c_str(), "r");
      
      if (f == NULL)
        return -1;
        
      filecontents = "";
      while ( (nread = fread(buffer, sizeof(char), nmax, f)) )
        filecontents.append(buffer, nread);
      
      fclose(f);
    }
    
    return 0;
  }
  
} // namespace RAT
