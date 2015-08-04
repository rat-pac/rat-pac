/**
 *  Copyright 2014 by Benjamin Land (a.k.a. BenLand100)
 *
 *  fastjson is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  fastjson is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with fastjson. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _JSON
#define _JSON

#include <vector>
#include <map>
#include <stdexcept>
#include <string>
#include <sstream>

namespace json {
    
    class Value;
    class Reader;
    class Writer;

    //types used by Value
    typedef int TInteger;
    typedef unsigned int TUInteger;
    typedef double TReal;
    typedef bool TBool;
    typedef std::string TString;
    typedef std::map<TString,Value> TObject;
    typedef std::vector<Value> TArray;
    
    typedef union {
        //basic types by value
        TInteger integer;
        TUInteger uinteger;
        TReal real;
        TBool boolean;
        //structured types by reference
        TString *_string;
        TObject *object;
        TArray *array;
    } TData;

    //type ids used by Value
    enum Type {
        TINTEGER,
        TUINTEGER,
        TREAL,
        TBOOL,
        TSTRING,
        TOBJECT,
        TARRAY,
        TNULL
    };
    
    //JSON Value container class. Basic types (int,uint,real,bool) are stored by value, and structured types are stored by reference.
    class Value {
    
        friend class Reader;
        friend class Writer;
        
        public:
        
            // Default constructs null Value (this is fast)
            inline Value() : refcount(NULL), type(TNULL) { }
            
            // Construct values directly from basic types. These are passed by value and have no refcount.
            explicit inline Value(TInteger integer) : refcount(NULL), type(TINTEGER) { data.integer = integer; }
            explicit inline Value(TUInteger uinteger) : refcount(NULL), type(TUINTEGER) { data.uinteger = uinteger; }
            explicit inline Value(TReal real) : refcount(NULL), type(TREAL) { data.real = real; }
            explicit inline Value(TBool boolean) : refcount(NULL), type(TBOOL) { data.boolean = boolean; }
            
            // Construct structured types. These values are copied into the Value and subsequently passed by reference with refcount.
            explicit inline Value(TString string) : refcount(new TUInteger(0)), type(TSTRING) { data._string = new TString(string); }
            explicit inline Value(TObject object) : refcount(new TUInteger(0)), type(TOBJECT) { data.object = new TObject(object); }
            explicit inline Value(TArray array) : refcount(new TUInteger(0)), type(TARRAY) { data.array = new TArray(array); }
            
            // Constructs a JSON array from a vector (assuming the compile type conversions are possible)
            template <typename T> Value(const std::vector<T> &ref) : refcount(new TUInteger(0)), type(TARRAY) {
                const size_t size = ref.size();
                data.array = new TArray(size);
                for (size_t i = 0; i < size; i++) {
                    (*data.array)[i] = Value(ref[i]);
                }
            }
            
            // Copy constructor - preserves structured types and refcount tracking
            inline Value(const Value &other) : refcount(other.refcount), type(other.type), data(other.data) { incref(); }
            
            // Destructor handles refcount tracking of structured types
            inline ~Value() { decref(); }
            
            // Sets the lhs equal to the value (for base types) or reference (for structured types)
            inline Value& operator=(const Value& other) { decref(); data = other.data; type = other.type; refcount = other.refcount; incref(); return *this; }
            template <typename T> inline Value& operator=(const T& val) { return operator=(Value(val)); }
            
            inline Value& operator[](const std::string &key) const { return getMember(key); }
            inline Value& operator[](const size_t index) const { return getIndex(index); }
            
            // Initializes the state of the Value to the default for structured types or unspecified for basic types
            void reset(Type type);
            
            // Sets the value to null
            inline void reset() { reset(TNULL); }
            
            // Returns the type of the Value
            inline Type getType() const { return type; }
            
            // Getters will throw a parser_error if the type of the Value is not the requested type
            inline TInteger getInteger() const { checkType(TINTEGER); return data.integer; }
            inline TUInteger getUInteger() const { checkType(TUINTEGER); return data.uinteger; }
            inline TReal getReal() const { checkType(TREAL); return data.real; }
            inline TBool getBool() const { checkType(TBOOL); return data.boolean; }
            inline TString getString() const { checkType(TSTRING); return *data._string; }
            
            // Returns a member of a JSON object
            inline Value& getMember(TString key) const { checkType(TOBJECT); return (*data.object)[key]; }
            
            // Returns the size of a JSON array
            inline size_t getArraySize() const { checkType(TARRAY); return data.array->size(); }
            
            // Returns the Value at an index in a JSON array
            inline Value& getIndex(size_t index) const { checkType(TARRAY); return (*data.array)[index]; }
            
#ifndef __CINT__

            // Templated casting functions (use these when possible / see below for default specializations) 
            template <typename T> inline T cast() const {
                throw std::runtime_error("Cannot cast Value to desired type"); // Arbitrary type casts are impossible
            }
            
            // Templated vector constructing method (uses templated casters to convert types)
            template <typename T> inline std::vector<T> toVector() const {
                const size_t size = getArraySize(); //will check that we are an array
                std::vector<T> result(size);
                for (size_t  i = 0; i < size; i++) {
                    result[i] = (*data.array)[i].cast<T>();
                }
                return result;
            }

#endif
            
            // Returns a vector of all the keys in the JSON object
            std::vector<std::string> getMembers() const;
            
            // Returns true if the key exists in the JSON object
            bool isMember(std::string key) const;
            
            // Setters will reset the type if necessary
            inline void setInteger(TInteger integer)  { checkTypeReset(TINTEGER); data.integer = integer; }
            inline void setUINteger(TUInteger uinteger) { checkTypeReset(TUINTEGER); data.uinteger = uinteger; }
            inline void setReal(TReal real) { checkTypeReset(TREAL); data.real = real; }
            inline void setReal(TBool boolean) { checkTypeReset(TBOOL); data.boolean = boolean; }
            inline void setString(TString string) { checkTypeReset(TSTRING); *data._string = string; }
            
            // Sets a member of a JSON object 
            inline void setMember(TString key, Value value) { checkTypeReset(TOBJECT); (*data.object)[key] = value; }
            
            // Sets the size of a JSON array 
            inline void setArraySize(size_t size) { checkTypeReset(TARRAY); data.array->resize(size); }
            
            // Sets the Value at an index in a JSON array
            inline void setIndex(size_t index, Value value) { checkTypeReset(TARRAY); (*data.array)[index] = value; }
            
        protected:
            
            // Returns a string representing the given type
            static std::string prettyType(Type type);
            
            // Throws a nice error message for trying to get the wrong type
            static void wrongType(Type actual, Type requested);
        
            // Throws a runtime_error if the type of the Value does not match the given Type
            inline void checkType(Type _type) const { if (this->type != _type) { wrongType(this->type, _type); } }
            
            // Resets the type of Value of the current type does not match the given Type
            inline void checkTypeReset(Type _type) { if (this->type != _type) reset(TOBJECT); }
            
            // Decreases the refcount of the Value and cleans up if necessary
            inline void decref() { if (refcount && !((*refcount)--)) clean(); }
            
            // Increases the refcount of the Value if necessary
            inline void incref() { if (refcount) (*refcount)++; }
            
            // Frees any allocated memory for this object and resets to null
            void clean();
        
            // Pointer to the number of references of a structured type
            TUInteger *refcount;
            
            // The current type of the Value
            Type type;
            
            // Union to hold the data with minimal space requirements
            TData data;
    };
    
#ifndef __CINT__
    
    // Everything can be cast to a string in one way or another
    template <> inline std::string Value::cast<std::string>() const {
        switch (type) {
            case TINTEGER: {
                std::stringstream out; out << data.integer;
                return out.str();
            }
            case TUINTEGER: {
                std::stringstream out; out << data.uinteger;
                return out.str();
            }
            case TREAL: {
                std::stringstream out; out << data.real;
                return out.str();
            }
            case TBOOL:
                return data.boolean ? "true" : "false";
            case TNULL:
                return "null";
            case TSTRING:
                return *(data._string);
            case TARRAY: {
                std::stringstream out; out << "ARR{" << (void*)data.array << '}';
                return out.str();
            }
            case TOBJECT: {
                std::stringstream out; out << "ARR{" << (void*)data.object << '}';
                return out.str();
            }
            default:
                throw std::runtime_error("Value could not be cast to string (forgotten?)");
        }
    }
    
    // Only integer Values can be cast as ints
    template <> inline int Value::cast<int>() const {
        switch (type) {
            case TUINTEGER:
                return data.uinteger; //strictly speaking this is unsafe, but does not lose precision
            case TINTEGER:
                return data.integer;
            default:
                throw std::runtime_error("Cannot cast " + prettyType(type) + " to integer");
        }
    }
    
    // All numerics can be cast to doubles
    template <> inline double Value::cast<double>() const { 
        switch (type) {
            case TUINTEGER:
                return data.uinteger;
            case TINTEGER:
                return data.integer;
            case TREAL:
                return data.real;
            default:
                throw std::runtime_error("Cannot cast " + prettyType(type) + " to double");
        }
    }
    
    // All Values are true except zero, false, and null
    template <> inline bool Value::cast<bool>() const { 
        switch (type) {
            case TUINTEGER:
                return data.uinteger != 0;
            case TINTEGER:
                return data.integer != 0;
            case TREAL:
                return data.real != 0.0;
            case TNULL:
                return false;
            case TBOOL:
                return data.boolean;
            default:
                return true;
        }
    }

#endif
    
    //represents errors in parsing JSON values
    class parser_error : public std::exception {
        public:
            parser_error(const int line_, const int pos_, std::string desc);
            virtual ~parser_error() throw ();
            virtual const char* what() const throw ();
        protected:
            const int line, pos;
            std::string desc, pretty;
    };
    
    //parses JSON values from a stream
    class Reader {
        public:
            //Reads the entire stream immediately
            Reader(std::istream &stream);
            
            //Copies the string into the internal buffer
            Reader(const std::string &str);
            
            ~Reader();
            
            //Returns the next value in the stream
            bool getValue(Value &result);
            
        protected:
            //Positional data in the stream data (gets garbled during parsing)
            char *data,*cur,*lastbr;
            int line;
            
            //Converts an escaped JSON string into its literal representation
            std::string unescapeString(std::string string);
            
            //Helpers to read JSON types
            Value readNumber();
            Value readString();
            Value readObject();
            Value readArray();
            
            void skipComment();
    
    };
    
    //writes JSON values to a stream
    class Writer {
        public:
            //Only writes to the stream when requested
            Writer(std::ostream &stream);
            
            ~Writer();
            
            //Writes a value to the stream
            void putValue(Value value);
            
        protected:
            //The stream to write to
            std::ostream &out;
            
            //Converts a literal string to its escaped representation
            std::string escapeString(std::string string);
            
            //Helper to write a value to the stream
            void writeValue(Value value);
    
    };  
    
}

#endif
