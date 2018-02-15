#include <RAT/DBTextLoader.hh>
#include <RAT/DBTable.hh>
#include <RAT/Log.hh>
#include <RAT/ReadFile.hh>
#include <TSystem.h>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

using namespace std;

namespace RAT {


/*************************** Text Parser *******************************/

const string Tokenizer::symbol_char = "{}[],:#";
const string Tokenizer::quote_char = "\"'";
const string Tokenizer::number_char = "0123456789+-.ed";
const string Tokenizer::hex_char = "0123456789abcdefABCDEF";

Tokenizer::Tokenizer(std::string thedoc, std::string _filename) : doc(thedoc), filename(_filename) 
{
  len = doc.size();
  // End with newline if not already terminated with whitespace
  // this simplifies EOF handling
  if (len > 0 && !isspace(doc[len-1])) {
    doc += '\n';
    len++;
  }

  pos = 0; 
  linenum = 1;
  linestart = 0; 
  eof = false;
}


void Tokenizer::RaiseError(string message)
{
  // Find end of line
  size_t eol = doc.find('\n', linestart);
  if (eol == string::npos)
    eol = doc.size();

  throw ParseError(linenum, pos - linestart, 
		   doc.substr(linestart, eol-linestart)+"\n", message);
}

void Tokenizer::RaiseProbablyJSONError(string message)
{
  // Find end of line
  size_t eol = doc.find('\n', linestart);
  if (eol == string::npos)
    eol = doc.size();

  throw ProbablyJSONParseError(linenum, pos - linestart, 
		   doc.substr(linestart, eol-linestart)+"\n", message);
}

std::string PickFile(std::string name, std::string enclosing_file)
{
  struct stat s;
  string newname;

  // First try file relative to current directory (absolute path or relative path will work)
  if (stat(name.c_str(), &s) == 0)
    return name;

  // Then try file in same directory as file which included this one
  if (enclosing_file != "") {
    newname = std::string(gSystem->DirName(enclosing_file.c_str())) + "/" + name;
    if (stat(newname.c_str(), &s) == 0)
      return newname;
  }

  // Finally try file in data directory
  newname = string(getenv("GLG4DATA")) + "/" + name;
  if (stat(newname.c_str(), &s) == 0)
    return newname;

  // Give up
  throw FileNotFoundError(name);
  return std::string(""); // never get here
}

Tokenizer::Type Tokenizer::Next()
{
  char quote = 0;
  bool is_double = false;
  string double_token;

  State state = STATE_START;
  token = "";
  while (pos < len) {
    char c = doc[pos];

    switch (state) {
    case STATE_START:
      if(isspace(c)) {
	pos++;
      } else if (c == '/') {
	state = STATE_COMMENT;
	pos++;
      } else if (symbol_char.find(c) != string::npos) {
	token = c;
	pos++;
	return TYPE_SYMBOL;
      } else if (quote_char.find(c) != string::npos) {
	token = "";
	state = STATE_STRING;
	quote = c;
	pos++;
      } else if (isalpha(c) || c == '_') {
	token = c;
	state = STATE_IDENTIFIER;
	pos++;
      } else if (number_char.find(c) != string::npos) {
	token = c;
	double_token = c;
	state = STATE_NUMBER;
	is_double = false;
	pos++;
	
	// Detect hex string
	if (pos < len && c == '0' && (doc[pos] == 'x' || doc[pos] == 'X')) {
	  token += doc[pos];
	  state = STATE_HEX_NUMBER;
	  pos++;
	}

      } else {
	RaiseError("Illegal character");
	return TYPE_ERROR;
      }
      break;

    case STATE_COMMENT:
      if (c == '/') {
	state = STATE_COMMENT_LINE;
	pos++;
      } else if (c == '*') {
	state = STATE_COMMENT_LONG;
	pos++;
      } else {
	RaiseError("Incorrect comment syntax");
	return TYPE_ERROR;
      }
      break;
      
    case STATE_COMMENT_LINE:
      if (c == '\n') {
	state = STATE_START;
	pos++;
      } else
	pos++;
      break;

    case STATE_COMMENT_LONG:
      if (c == '*') {
	pos++;
	if (pos < len && doc[pos] == '/') {
	  state = STATE_START;
	  pos++;
	}
      } else
	pos++;
      break;

    case STATE_IDENTIFIER:
      if (isalpha(c) || isdigit(c) || c == '_') {
	token += c;
	pos++;
      } else {
	return TYPE_IDENTIFIER;
      }
      break;

    case STATE_NUMBER:
      if (number_char.find(c) != string::npos) {
	char translated_c = c;
	if (c == 'e' || c == '.')
	  is_double = true;
	else if (c == 'd') {
	  is_double = true;
	  translated_c = 'e'; // Convert to normal syntax
	}
	
	token += c;
	double_token += translated_c;
	pos++;
      } else {
	if (is_double) {
	  if (sscanf(double_token.c_str(), "%lf", &doubleval) == 1)
	    return TYPE_DOUBLE;
	  else
	    return TYPE_ERROR;
	} else {
	  if (sscanf(token.c_str(), "%d", &intval) == 1)
	    return TYPE_INTEGER;
	  else {
	    RaiseError("Unable to parse number");
	    return TYPE_ERROR; // never get here
	  }
	}
      }
      break;
     
    case STATE_HEX_NUMBER:
      if (hex_char.find(c) != string::npos) {
	token += c;
	pos++;
      } else {
	unsigned int temp;
	if (sscanf(token.c_str(), "%x", &temp) == 1) {
	  intval = static_cast<int>(temp); // to keep compiler happy
	  return TYPE_INTEGER;
	} else {
	  RaiseError("Unable to parse hex number");
	  return TYPE_ERROR;
	}
      }
      break;

    case STATE_STRING:
      if (c == quote) {
	pos++;
	return TYPE_STRING;
      } else if (c == '\\') {
	state = STATE_STRING_ESC;
	pos++;
      } else {
	token += c;
	pos++;
      }
      break;

    case STATE_STRING_ESC:
      state = STATE_STRING;
      switch (c) {
      case '"':
      case '/':
      case '\\': token += c; break;
      case 'b':  token += '\b'; break;
      case 'f':  token += '\f'; break;
      case 'n': token += '\n'; break;
      case 'r': token += '\r'; break;
      case 't': token += '\t'; break;
      default:
	return TYPE_ERROR;
      }

      pos++;
      break;

    }

    if (c == '\n') {
      linenum++;
      linestart = pos;
    }
  }

  eof = true;

  if (token == "")
    return TYPE_EOF;
  else
    return TYPE_EOF_ERROR;
}

Tokenizer::Type TokenizerStack::Next()
{
  Tokenizer::Type toktype = stack.top().Next();
  
  // Keep popping tokenizer until we get to something, or give up at the last one
  while (toktype == Tokenizer::TYPE_EOF && stack.size() > 1) {
    stack.pop();
    toktype = stack.top().Next();
  }
  
  return toktype;
}


DBTable *Parser::Next()
{
  State state = STATE_START;
  Tokenizer::Type toktype = tokenizer.Next();
  // This fanciness is to ensure tbl deleted if we throw an exception
  // or leave this method for any reason.
  auto_ptr<DBTable> tbl(new DBTable());
  string identifier;

  Tokenizer::Type array_type = Tokenizer::TYPE_ERROR;
  vector<int>    integer_array;
  vector<double> double_array;
  vector<string> string_array;

  if (tokenizer.Eof())
    return 0;

  while (toktype != Tokenizer::TYPE_EOF && state != STATE_END) {
    switch (state) {
    case STATE_START:
      if (toktype == Tokenizer::TYPE_SYMBOL 
	  && tokenizer.Token() == "{")
	state = STATE_TABLE;
      else if (toktype == Tokenizer::TYPE_SYMBOL
          && tokenizer.Token() == "#") {
        // Eat include token
        toktype = tokenizer.Next();
        if (toktype != Tokenizer::TYPE_IDENTIFIER && tokenizer.Token() != "include")
          tokenizer.RaiseError("Syntax error");
          
	// Get filename
	toktype = tokenizer.Next();
	if (toktype != Tokenizer::TYPE_STRING)
	  tokenizer.RaiseError("#include requires a filename in quotes");
	 
	string filename = PickFile(tokenizer.Token(), tokenizer.GetCurrentFilename());
	string contents;
	
	if (ReadFile(filename, contents) < 0)
    throw FileNotFoundError(filename);
	
	info << "DBTextLoader: #including " << filename << newline;
	tokenizer.Push(Tokenizer(contents, filename));
      } else
	tokenizer.RaiseError("Must start table with '{'");
      break;

    case STATE_TABLE:
      if (toktype == Tokenizer::TYPE_IDENTIFIER) {
	identifier = tokenizer.Token();
	state = STATE_FIELD;
      } else if (toktype == Tokenizer::TYPE_SYMBOL
		 && tokenizer.Token() == "}")
	state = STATE_END;
	    else if (toktype == Tokenizer::TYPE_STRING)
        tokenizer.RaiseProbablyJSONError("Syntax errorrrr");
      else
	tokenizer.RaiseError("Syntax error");
      break;

    case STATE_FIELD:
      if (toktype == Tokenizer::TYPE_SYMBOL
	  && tokenizer.Token() == ":")
	state = STATE_VAL_SEP;
      else
	tokenizer.RaiseError("Syntax error");
      break;

    case STATE_VAL_SEP:
      if (Tokenizer::IsValType(toktype)) {
	state = STATE_VAL;

	switch (toktype) {
	case Tokenizer::TYPE_STRING: tbl->Set(identifier, tokenizer.Token());
	  break;
	case Tokenizer::TYPE_INTEGER: tbl->Set(identifier, tokenizer.AsInt());
	  break;
	case Tokenizer::TYPE_DOUBLE: tbl->Set(identifier, tokenizer.AsDouble());
	  break;
	default:
	  assert(0); // should never get here
	}

      } else if (toktype == Tokenizer::TYPE_SYMBOL
		 && tokenizer.Token() == "[") {
	  state = STATE_ARRAY;
	  array_type = Tokenizer::TYPE_ERROR; // array type not known yet
	  integer_array.clear();
	  double_array.clear();
	  string_array.clear();
      } else
	tokenizer.RaiseError("Invalid value");
      break;

    case STATE_VAL:
      if (toktype == Tokenizer::TYPE_SYMBOL
	  && tokenizer.Token() == "}")
	state = STATE_END;
      else if (toktype == Tokenizer::TYPE_SYMBOL
	  && tokenizer.Token() == ",")
	state = STATE_TABLE;
      else
	tokenizer.RaiseError("Syntax error");
      break;

    case STATE_ARRAY:
      if (Tokenizer::IsValType(toktype)) {
	if (array_type == Tokenizer::TYPE_ERROR)
	  array_type = toktype;
	else if (array_type != toktype)
	  tokenizer.RaiseError("Mixed types in array not allowed");

	state = STATE_ARRAY_VAL;
	
	switch (toktype) {
	case Tokenizer::TYPE_STRING: string_array.push_back(tokenizer.Token());
	  break;
	case Tokenizer::TYPE_INTEGER: integer_array.push_back(tokenizer.AsInt());
	  break;
	case Tokenizer::TYPE_DOUBLE: double_array.push_back(tokenizer.AsDouble());
	  break;
	default:
	  assert(0); // Should never get here
	}
      } else if (toktype == Tokenizer::TYPE_SYMBOL 
		 && tokenizer.Token() == "]") {
	// Allow array to be closed after a comma (to be nice)
	state = STATE_VAL;
	switch (array_type) {
	case Tokenizer::TYPE_STRING: tbl->Set(identifier, string_array); break;
	case Tokenizer::TYPE_INTEGER: tbl->Set(identifier, integer_array); break;
	case Tokenizer::TYPE_DOUBLE: tbl->Set(identifier, double_array); break;
	case Tokenizer::TYPE_ERROR:
	  tokenizer.RaiseError("Empty arrays not allowed");
	  break;
	default:
	  assert(0); // should never get here
	}
      } else {
        tokenizer.RaiseError(string("Unexpected symbol in array: ") + tokenizer.Token());
        return 0; // should never get here because above raises exception
      }
      break;

    case STATE_ARRAY_VAL:
      if (toktype == Tokenizer::TYPE_SYMBOL
	  && tokenizer.Token() == "]") {
	state = STATE_VAL;
	switch (array_type) {
	case Tokenizer::TYPE_STRING: tbl->Set(identifier, string_array); break;
	case Tokenizer::TYPE_INTEGER: tbl->Set(identifier, integer_array); break;
	case Tokenizer::TYPE_DOUBLE: tbl->Set(identifier, double_array); break;
	case Tokenizer::TYPE_ERROR:
	  tokenizer.RaiseError("Empty array not allowed");
	  break;
	default:
	  assert(0); // should never get here
	}
      } else if (toktype == Tokenizer::TYPE_SYMBOL
		 && tokenizer.Token() == ":") {

	// Repeat last value in array
	toktype = tokenizer.Next();
	if (toktype == Tokenizer::TYPE_INTEGER) {
	  int repeat = tokenizer.AsInt() - 1;
	  if (repeat < 0)
	    tokenizer.RaiseError("Value repetition must be greater than 0");

	  for (int i=0; i < repeat; i++) {
	    switch (array_type) {
	    case Tokenizer::TYPE_STRING: string_array.push_back(string_array.back());
	      break;
	    case Tokenizer::TYPE_INTEGER: integer_array.push_back(integer_array.back());
	      break;
	    case Tokenizer::TYPE_DOUBLE: double_array.push_back(double_array.back());;
	      break;
	    default:
	      assert(0); // Should never get here
	    }
	  }
	} else
	  tokenizer.RaiseError("Repetition count for value must be positive integer.");
	
      } else if (toktype == Tokenizer::TYPE_SYMBOL
		 && tokenizer.Token() == ":") {

	// Repeat last value in array
	toktype = tokenizer.Next();
	if (toktype == Tokenizer::TYPE_INTEGER) {
	  int repeat = tokenizer.AsInt() - 1;
	  if (repeat < 0)
	    tokenizer.RaiseError("Value repetition must be greater than 0");

	  for (int i=0; i < repeat; i++) {
	    switch (array_type) {
	    case Tokenizer::TYPE_STRING: string_array.push_back(string_array.back());
	      break;
	    case Tokenizer::TYPE_INTEGER: integer_array.push_back(integer_array.back());
	      break;
	    case Tokenizer::TYPE_DOUBLE: double_array.push_back(double_array.back());;
	      break;
	    default:
	      assert(0); // Should never get here
	    }
	  }
	} else
	  tokenizer.RaiseError("Repetition count for value must be positive integer.");
	
      } else if (toktype == Tokenizer::TYPE_SYMBOL
	  && tokenizer.Token() == ",")
	state = STATE_ARRAY;
      else
	tokenizer.RaiseError("Syntax error");
      break;

    case STATE_END:
      assert(0); // should never get here
      break;
    } // end of switch: parser state

    if (state != STATE_END)
      toktype = tokenizer.Next();

  } // end of while: token read loop

  if (state != STATE_END && state != STATE_START) {
    tokenizer.RaiseError("Unexpected end of file");
    return 0;  // Never get here since previous call always throws exception
  } else
    return tbl.release();  // Extract object from auto_ptr and prevent
                           // it from being automatically deleted.
}


/************************* Loader ******************************/

std::vector<DBTable *> DBTextLoader::parse(std::string filename)
{
  std::vector<DBTable *> tables;

  filename = PickFile(filename, "");

  string contents;
  if(ReadFile(filename, contents) < 0)
    throw FileNotFoundError(filename);

  Parser parser(contents, filename);

  try {
    DBTable *table = parser.Next();
    while (table != 0) {
      bool bad = false;
      
      // Figure out table name
      if (table->GetFieldType("name") == DBTable::STRING)
	      table->SetName(table->GetS("name"));
      else {
	      cerr << "Unnamed table in " << filename << endl;
	      bad = true;
      }

      // Set index if present
      if (table->GetFieldType("index") == DBTable::STRING)
	      table->SetIndex(table->GetS("index"));

      if (table->GetFieldType("run_range") == DBTable::INTEGER_ARRAY
	        && table->GetIArray("run_range").size() == 2) {	

	        const vector<int> &run_range = table->GetIArray("run_range");
          table->SetRunRange(run_range[0], run_range[1]);
      } else if (table->GetFieldType("valid_begin") == DBTable::INTEGER_ARRAY
                 && table->GetIArray("valid_begin").size() == 2
                 && table->GetFieldType("valid_end") == DBTable::INTEGER_ARRAY
                 && table->GetIArray("valid_end").size() == 2) {
        // Backward compatibility
        const vector<int> &valid_begin = table->GetIArray("valid_begin");
        const vector<int> &valid_end = table->GetIArray("valid_end");
        vector<int> run_range(2);
        
        if (valid_begin[0] == 0 && valid_begin[1] == 0 && valid_end[0] == 0 && valid_end[1] == 0) {
          run_range[0] = run_range[1] = 0;

          // Add modern run_range field
          table->Set("run_range", run_range);
          table->SetRunRange(run_range[0], run_range[1]);
        } else if (valid_begin[0] == -1 && valid_begin[1] == -1 && valid_end[0] == -1 && valid_end[1] == -1) {
          run_range[0] = run_range[1] = -1;
          
          // Add modern run_range field
          table->Set("run_range", run_range);
          table->SetRunRange(run_range[0], run_range[1]);          
        } else {
          cerr << "Table has old-style valid_begin/valid_end arrays not set to default or user plane.  Discarding..."
               << endl;
          bad = true;
        }
        
      } else {
	      cerr << "Table " << table->GetName() 
	        << " has bad/missing validity information." << endl
	        << "Discarding..." << endl;
	      bad = true;
      }

      if (bad)
	      delete table;
      else
	      tables.push_back(table);

      table = parser.Next();
    }
  } catch (ProbablyJSONParseError &jp) {
    // Say nothing because this is probably a JSON format file
    throw;
  } catch (ParseError &p) {
    // Inform user of parse failure and keep handing the exception up
    // the call stack (presumably we will exit the entire program)
    cerr << p.GetFull();
    throw;
  }
  
  return tables;
}


} // namespace RAT
