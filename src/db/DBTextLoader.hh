#ifndef __RAT_DBTextLoader__
#define __RAT_DBTextLoader__

#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <sstream>
#include <RAT/DBExceptions.hh>

namespace RAT {

class DBTable;

/** Tokenizer for RATDB text format. */
class Tokenizer {
public:
  enum Type { TYPE_IDENTIFIER, TYPE_INTEGER, TYPE_DOUBLE,
	      TYPE_STRING, TYPE_SYMBOL, TYPE_EOF, TYPE_ERROR, TYPE_EOF_ERROR };

  Tokenizer(std::string doc, std::string _filename="");
  Tokenizer::Type Next();
  std::string Token() { return token; };
  int AsInt() { return intval; };
  double AsDouble() { return doubleval; };
  
  void RaiseError(std::string message);
  void RaiseProbablyJSONError(std::string message);

  std::string GetFilename() const { return filename; };

  inline bool Eof() { return eof; };

  static bool IsValType(Tokenizer::Type type) {
    return type == TYPE_STRING || type == TYPE_INTEGER ||
      type == TYPE_DOUBLE;
  };

protected:
  enum State { STATE_START, 
	       STATE_COMMENT, STATE_COMMENT_LONG, STATE_COMMENT_LINE,
	       STATE_IDENTIFIER, STATE_NUMBER, STATE_HEX_NUMBER, 
	       STATE_STRING, STATE_STRING_ESC };

  bool eof;
  std::string doc;
  std::string filename;

  int pos, len;
  std::string token;
  int intval;
  double doubleval;
  
  int linenum;
  int linestart;

  static const std::string number_char;
  static const std::string symbol_char;
  static const std::string quote_char;
  static const std::string hex_char;
};

/** Tokenizer stack to support #include of other files */
class TokenizerStack {
public:
  TokenizerStack(Tokenizer tokenizer) { stack.push(tokenizer); };
  
  void Push(Tokenizer tokenizer) { stack.push(tokenizer); };
  Tokenizer &Top() { return stack.top(); };
  
  Tokenizer::Type Next();
  std::string Token() { return stack.top().Token(); };
  int AsInt() { return stack.top().AsInt(); };
  double AsDouble() { return stack.top().AsDouble(); };
  
  void RaiseError(std::string message) { stack.top().RaiseError(message); };
  void RaiseProbablyJSONError(std::string message) { stack.top().RaiseProbablyJSONError(message); };

  bool Eof() { return stack.top().Eof(); };
  
  std::string GetCurrentFilename() const { return stack.top().GetFilename(); };

protected:
  std::stack<Tokenizer> stack;
};


/** Parser of RATDB text format. */
class Parser {
public:
  Parser(std::string str, std::string filename) : tokenizer(Tokenizer(str, filename)) { };

  enum State { STATE_START, STATE_TABLE, STATE_FIELD,
	       STATE_VAL_SEP, STATE_VAL, 
	       STATE_ARRAY, STATE_ARRAY_VAL, STATE_END};

  DBTable *Next();

protected:
  TokenizerStack tokenizer;
};


/* A $#%!! class with one static function to make rootcint happy */
class DBTextLoader
{
public:
  /** Returns a list of all tables found in RATDB text file @p filename. */
  static std::vector<RAT::DBTable *> parse(std::string filename);
};

} // namespace RAT

#endif
