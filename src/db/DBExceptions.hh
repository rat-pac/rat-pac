#ifndef __RAT_DBExceptions__
#define __RAT_DBExceptions__

namespace RAT {
  
/** Exception: Base class for all errors loading RATDB text files. */
class TextLoaderError { };

/** Exception: Error parsing RATDB text file */
class ParseError : public TextLoaderError {
public:
  /** Create new parse error.
   *
   *  @param lnum      Line number in text file where error occurred.
   *  @param colnum    Column number in text file where error occurred.
   *  @param l         Contents of erroneous line from file.
   *  @param mes       Message explaining specific error.
   */
  ParseError(int lnum, int cnum, std::string l, std::string mes) :
    linenum(lnum), colnum(cnum), line(l), message(mes) { };
    
  /** Return formatted string explaining parse error.
   *
   *  This method creates a user-readable string which shows where the
   *  error ocurred and the problem line.
   */
  std::string GetFull() const {
    std::ostringstream s;
    s << "Error on line " << linenum << ": " << message << std::endl;
    s << line;
    for (int i=0; i < colnum; i++)
      s << " ";
    s << "^" << std::endl;
    return s.str();
  };

  /** Compare equality between two ParseError objects by comparing all fields. */
  bool operator== (const ParseError &other) const {
    return linenum == other.linenum && colnum == other.colnum
      && line == other.line && message == other.message;
  };

  int linenum; /**< Line number of error. */
  int colnum;  /**< Column number of error. */
  std::string line; /**< Line containing error. */
  std::string message; /**< Message explaining error. */
};

class ProbablyJSONParseError : public ParseError {
public:
  ProbablyJSONParseError(int lnum, int cnum, std::string l, std::string mes) :
    ParseError(lnum, cnum, l, mes) {};
};


/** Exception: Base class of file errors in RATDB. */
class FileError : public TextLoaderError {
public: 
  FileError(std::string _filename) : filename(_filename) { };
  bool operator== (const FileError &other) const {
    return filename == other.filename;
  };

  std::string filename;
};

/** Exception: Error finding RATDB file. */
class FileNotFoundError : public FileError { 
public :
  FileNotFoundError(std::string filename) : FileError(filename) { };
};

/** Exception: Insufficient access privileges to read file. */
class FileAccessError : public FileError { 
public :
  FileAccessError(std::string filename) : FileError(filename) { };
};

} // namespace RAT

#endif
