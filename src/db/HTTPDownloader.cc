#include <RAT/HTTPDownloader.hh>
#include <RAT/Log.hh>

#if __HAS_CURL
#include <curl/curl.h>
#endif

namespace RAT {
  
  HTTPDownloader::HTTPDownloader() : handle(0), noproxy_set(false)
  {
    
  }
  
  HTTPDownloader::~HTTPDownloader()
  {
    Disconnect();
  }
  
  void HTTPDownloader::Disconnect()
  {
#if __HAS_CURL
    if (handle)
      curl_easy_cleanup(handle);
#endif
      
    handle = 0;
  }


#ifdef __HAS_CURL
  /* Callback function for curl */
  static size_t curl_write_func(void *ptr, size_t size, size_t nmemb, void *stream)
  {
    std::ostringstream *doc = static_cast<std::ostringstream *>(stream);
    char *data = static_cast<char *>(ptr);

    for (size_t i=0; i < size * nmemb; i++)
      (*doc) << data[i];

    return size * nmemb;
  }
#endif
    
  std::string HTTPDownloader::Fetch(const std::string &url)
  {
#if __HAS_CURL
    if (!handle) {
      handle = curl_easy_init();
      Log::Assert(handle, "RATDB: Could not init curl library!");

      SetCurlOpts();
    }
    
    // Hack, but should work for LANL folks
    // Disable externally-set proxy if fetching from localhost
    // Get rid of this once everyone has CURL >= 7.19.4
    /////////
    if (url.find("://localhost") != std::string::npos) {
      if (!noproxy_set) {
        curl_easy_setopt(handle, CURLOPT_PROXY, "");
        noproxy_set = true;
      }
    } else if (noproxy_set) {
      // need to clear CURLOPT_PROXY option, but no way to do that
      curl_easy_reset(handle);
      SetCurlOpts();
      noproxy_set = false;
    }
    /////////
    
    curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
    contents.str(""); // clear contents
    
    curl_easy_perform(handle);
    
    return contents.str();
#else
    Log::Die("RATDB: Attempt to use HTTPDownloader without libcurl support.");
    return "";
#endif
  }
  
  void HTTPDownloader::SetCurlOpts()
  {
#if __HAS_CURL
    if (Log::GetDisplayLevel() >= Log::DEBUG)
      curl_easy_setopt(handle, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1);
    curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, &curl_write_func);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &contents);
    // Damn, can't do this until CURL 7.19.4, which is too new for OS X 10.5
    //curl_easy_setopt(handle, CURLOPT_NOPROXY, "localhost,127.0.0.1");
#endif
  }
  
} // namespace RAT
