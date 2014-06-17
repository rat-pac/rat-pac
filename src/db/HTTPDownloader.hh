/** @class HTTPDownloader
 *  Utility class for fetching files from a HTTP/HTTPS server using libcurl
 *
 *  @author Stan Seibert <sseibert@lanl.gov>
 *
 *  HTTPDownload uses libcurl to open a persistent connection with an HTTP or HTTPS server
 *  to download files.  File contents are returned as std::string objects.
 */
 
#ifndef __RAT_HTTPDownloader__
#define __RAT_HTTPDownloader__

#include <string>
#include <sstream>

typedef void CURL;

namespace RAT {
  
  class HTTPDownloader {
  public:
    HTTPDownloader();
    ~HTTPDownloader();
    
    /** This object will maintain a persistent connection to the HTTP server after a fetch.
     *  Call this method to force a disconnect; */
    void Disconnect();
    
    /** Fetch the file at the given url */
    std::string Fetch(const std::string &url);
    
  protected:
    void SetCurlOpts();
    
    CURL *handle;
    bool noproxy_set;
    std::ostringstream contents;
    
  };
  
  
} // namespace RAT

#endif
