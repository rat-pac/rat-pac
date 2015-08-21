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

#include "json.hh"

#include <cstdlib>
#include <cstring>
#include <sstream>
#include <limits>

namespace json {
    
    void Value::reset(Type _type) {
        decref();
        this->type = _type;
        switch (_type) {
            case TSTRING:
                data._string = new TString();
                refcount = new TUInteger(0);
                return;
            case TOBJECT:
                data.object = new TObject();
                refcount = new TUInteger(0);
                return;
            case TARRAY:
                data.array = new TArray();
                refcount = new TUInteger(0);
                return;
            default:
                refcount = NULL;
        }
    }
    
    void Value::clean() {
        if (refcount) delete refcount;
        switch (type) {
            case TSTRING:
                delete data._string;
                break;
            case TOBJECT:
                delete data.object;
                break;   
            case TARRAY: 
                delete data.array;
                break;
            default:
                break;
        }
        type = TNULL;
        refcount = NULL;
    }
    
    std::vector<std::string> Value::getMembers() const {
        checkType(TOBJECT);
        std::vector<std::string> keys(data.object->size());
        size_t i = 0;
        for (TObject::iterator pair = data.object->begin(); pair != data.object->end(); ++pair) {
            keys[i++] = pair->first;
        }
        return keys;
    }
    
    bool Value::isMember(std::string key) const {
        checkType(TOBJECT);
        return (data.object->find(key) != data.object->end());
    }
    
    std::string Value::prettyType(Type type) {
        switch (type) {
            case TOBJECT:
                return "TObject";
            case TARRAY:
                return "TArray";
            case TSTRING:
                return "TString";
            case TBOOL:
                return "TBool";
            case TREAL:
                return "TReal";
            case TINTEGER:
                return "TInteger";
            case TUINTEGER:
                return "TUInteger";
            case TNULL:
                return "TNULL";
            default:
                return "UNKNOWN";
        }
    }
    
    void Value::wrongType(Type actual, Type requested) {
        std::stringstream pretty;
        pretty << "JSON Value of type " << prettyType(actual) << " is not type " << prettyType(requested);
        throw std::runtime_error(pretty.str());
    }
    
    parser_error::parser_error(const int line_, const int pos_, std::string desc_) : line(line_), pos(pos_), desc(desc_)  {
        std::stringstream prettyss;
        prettyss << '[' << line << ':' << pos << "] " << desc;
        this->pretty = prettyss.str();
    }
    
    parser_error::~parser_error() throw () { }
    
    const char* parser_error::what() const throw () {
        return pretty.c_str();
    }
    
    Reader::Reader(std::istream &in) {
        std::string ret;
        char buffer[4096];
        while (in.read(buffer, sizeof(buffer)))
            ret.append(buffer, sizeof(buffer));
        ret.append(buffer, in.gcount());
        data = new char[ret.length()+1];
        cur = data;
        memcpy(data,ret.c_str(),ret.length());
        data[ret.length()] = '\0';
        line = 1;
        lastbr = cur;
    }
    
    Reader::Reader(const std::string &str) {
        data = new char[str.length()+1];
        cur = data;
        memcpy(data,str.c_str(),str.length());
        data[str.length()] = '\0';
        line = 1;
        lastbr = cur;
    }
    
    Reader::~Reader() {
        delete [] data;
    }

    bool Reader::getValue(Value &result) {
        for (;;) {
            switch (*cur) {
                case '\n':
                    line++;
                case '\r':
                    lastbr = cur+1;
                case ' ':
                case '\t':
                    cur++;
                    break;
                case '-':
                case '+':
                case '.':
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    result = readNumber();
                    return true;
                case '{':
                    result = readObject();
                    return true;
                case '[':
                    result = readArray();
                    return true;
                case '"':
                    result = readString();
                    return true;
                case 'n': //https://tools.ietf.org/rfc/rfc7159.txt
                    if (cur[1] == 'u' && cur[2] == 'l' && cur[3] == 'l') {
                        cur+=4;
                        result = Value();
                        return true;
                    }
                    throw parser_error(line,cur-lastbr,"Unexpected character");
                case 't': //https://tools.ietf.org/rfc/rfc7159.txt
                    if (cur[1] == 'r' && cur[2] == 'u' && cur[3] == 'e') {
                        cur+=4;
                        result = Value(true);
                        return true;
                    }
                    throw parser_error(line,cur-lastbr,"Unexpected character");
                case 'f': //https://tools.ietf.org/rfc/rfc7159.txt
                    if (cur[1] == 'a' && cur[2] == 'l' && cur[3] == 's' && cur[4] == 'e') {
                        cur+=5;
                        result = Value(false);
                        return true;
                    }
                    throw parser_error(line,cur-lastbr,"Unexpected character");
                case '/': //non-json comment
                    skipComment();
                    break;
                case '\0': //EOF
                    return false;
                default:
                    throw parser_error(line,cur-lastbr,"Unexpected character");
            }
        }
        throw parser_error(line,cur-lastbr,"Should never reach here. Probably hardware error.");
    }
    
    void Reader::skipComment() {
        if (cur[1] == '/') {
            cur++;
            while ((*cur) && *cur != '\n') { cur++; }
            line++;
            lastbr = cur+1;
            if (*cur++) return;
        } else if (cur[1] == '*') {
            cur++;
            for ( ; *cur; cur++) {
                switch (*cur) {
                    case '*':
                        if (cur[1] == '/') {
                            cur += 2;
                            return;
                        }
                        break;
                    case '\n':
                        line++;
                    case '\r':
                        lastbr = cur+1;
                }
            }
            if (*cur++) return;
        }
        throw parser_error(line,cur-lastbr,"Malformed comment");
    }
    
    Value Reader::readNumber() {
        bool real = false;
        bool exp = false;
        char *start = cur;
        for (;;) {
            switch (*cur) {
                case 'x': //non-json hex
                    if (cur-start == 1 && start[0] == '0') {
                        cur++;
                        start = cur;
                        for (;;) {
                            switch (*cur) {
                                case 'A':
                                case 'a':
                                case 'B':
                                case 'b':
                                case 'C':
                                case 'c':
                                case 'D':
                                case 'd':
                                case 'E':
                                case 'e':
                                case 'F':
                                case 'f':
                                case '0':
                                case '1':
                                case '2':
                                case '3':
                                case '4':
                                case '5':
                                case '6':
                                case '7':
                                case '8':
                                case '9':
                                    cur++;
                                    break;
                                default: {
                                    char next = *cur;
                                    *cur = '\0';
                                    TUInteger hex;
                                    std::stringstream temp;
                                    temp << std::hex << start;
                                    temp >> hex;
                                    *cur = next;
                                    return Value(hex);
                                }
                            }
                        }
                    } else {
                        throw parser_error(line,cur-lastbr,"Malformed hex number");
                    }
                case 'e': //exponential
                    exp = true;
                    cur++;
                    break;
                case 'u': //non-json explicit unsigned
                    *cur = '\0';
                    cur++;
                    return Value((TUInteger)atoi(start));
                case 'd': //non-json explicit real OR strange exponential
                    switch (cur[1]) {
                        case '+':
                        case '-':
                        case '0':
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                        case '8':
                        case '9':
                            if (exp) throw parser_error(line,cur-lastbr,"Malformed exponential");
                            exp = true;
                    }
                    if (exp) {
                        *cur = 'e'; //this is ugly but the syntax I'm trying to parse is also ugly
                        cur++;
                        break;
                    }
                    //intentional fallthrough
                case 'f': //non-json explicit real
                    *cur = '\0';
                    cur++;
                    return Value((TReal)atof(start));
                case '.': //real
                    real = true;
                case '+':
                case '-':
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    cur++;
                    break;
                default: { //any other character is end of number
                    char next = *cur;
                    *cur = '\0';
                    Value val;
                    if (real || exp) {
                        val = Value((TReal)atof(start));
                    } else {
                        val = Value((TInteger)atoi(start));
                    }
                    *cur = next;
                    return val;
                }
            }
        }
        throw parser_error(line,cur-lastbr,"Should never reach here. Probably hardware error.");
    }
    
    Value Reader::readString() {
        char *start = ++cur;
        for (;;) {
            switch (*(cur++)) {
                case '\\': 
                    cur++; //definitely an escape, so skip next character
                    break;
                case '\"':
                    cur[-1] = '\0';
                    return Value(unescapeString(std::string(start)));
                case '\0':
                    throw parser_error(line,cur-lastbr,"Reached EOF while parsing string");
            }
        }
        throw parser_error(line,cur-lastbr,"Should never reach here. Probably hardware error.");
    }
    
    Value Reader::readObject() {
        Value object = Value();
        object.reset(TOBJECT);
        char *key = NULL;
        bool keyfound = false;
        Value val = Value();
        cur++;
        for (;;) {
            switch (*cur) {
                case '/': //non-json comment
                    skipComment();
                    break;
                case '\n':
                    line++;
                case '\r':
                    lastbr = cur+1;
                case ' ':
                case '\t':
                    if (key && !keyfound) {
                        *cur = '\0';
                        keyfound = true;
                    }
                    cur++;
                    break;
                case '}':
                    cur++;
                    if (key) {
                        throw parser_error(line,cur-lastbr,"} found where value expected");
                    }
                    return object;
                case ',':
                    cur++;
                    if (key) {
                        throw parser_error(line,cur-lastbr,", found where value expected");
                    }
                    break;
                case ':':
                    if (!key) {
                        throw parser_error(line,cur-lastbr,": found where field expected");
                    }
                    if (key && !keyfound) *cur = '\0';
                    cur++;
                    if (!getValue(val)) {
                        throw parser_error(line,cur-lastbr,"EOF reached while parsing object");
                    }
                    object.setMember(std::string(key),val);
                    key = NULL;
                    keyfound = false;
                    break;
                case '\"':
                    cur++;
                    key = cur;
                    readString();
                    keyfound = true;
                    break;
                case '\0':
                    throw parser_error(line,cur-lastbr,"Reached EOF while parsing object");
                default:
                    if (keyfound) {
                        throw parser_error(line,cur-lastbr,"Unexpected character where value expected");
                    }
                    if (!key) key = cur;
                    cur++;
            }
        }
        throw parser_error(line,cur-lastbr,"Should never reach here. Probably hardware error.");
    }
    
    Value Reader::readArray() {
        Value array = Value();
        array.reset(TARRAY);
        Value next = Value();
        cur++;
        for (;;) {
            switch (*cur) {
                case '/': //non-json comment
                    skipComment();
                    break;
                case '\n':
                    line++;
                case '\r':
                    lastbr = cur+1;
                case ' ':
                case '\t':
                case ',':
                    cur++;
                    break;
                case ':':  { //non-json value repetition
                    cur++;
                    Value reps;
                    if (!getValue(reps) || reps.getType() != TINTEGER || reps.getInteger() < 0) {
                        throw parser_error(line,cur-lastbr,"Array value repetition syntax error");
                    }    
                    const int nreps = reps.getInteger();
                    // The value to be repeated has already been pushed once
                    if (nreps == 0) { 
                        array.data.array->pop_back();
                    } else {
                        array.data.array->reserve(array.data.array->size() + nreps - 1);
                        for (int i = 1; i < nreps; i++) {
                            array.data.array->push_back(next);
                        }
                    }
                    break;
                }
                case ']':
                    cur++;
                    return array;
                case '\0':
                    throw parser_error(line,cur-lastbr,"Reached EOF while parsing array");
                default:
                    if (!getValue(next)) {
                        throw parser_error(line,cur-lastbr,"EOF reached while parsing array");
                    }
                    array.data.array->push_back(next);
            }
        }
        throw parser_error(line,cur-lastbr,"Should never reach here. Probably hardware error.");
    }

    Writer::Writer(std::ostream &stream) : out(stream) {
        
    }
    
    Writer::~Writer() {
        
    }

    void Writer::putValue(Value value) {
        writeValue(value);
        out << '\n';
    }
    
    //This could make prettier output
    void Writer::writeValue(Value value) {
        switch (value.type) {
            case TINTEGER:
                out << value.data.integer;
                break;
            case TUINTEGER:
                out << value.data.uinteger << 'u';
                break;
            case TREAL:
                out.precision(std::numeric_limits<double>::digits10);
                out << value.data.real;
                break;
            case TSTRING:
                out << '"' << escapeString(*(value.data._string)) << '"';
                break;
            case TOBJECT: {
                    TObject::iterator it = value.data.object->begin();
                    TObject::iterator end = value.data.object->end();
                    out << "{\n";
                    for ( ; it != end; ++it) {
                        out << '\"' << it->first << "\" : ";
                        writeValue(it->second);
                        out << ",\n";
                    }
                    out << '}';
                }
                break;   
            case TARRAY: {
                    TArray::iterator it = value.data.array->begin();
                    TArray::iterator end = value.data.array->end();
                    out << '[';
                    for ( ; it != end; ++it) {
                        writeValue(*it);
                        out << ", ";
                    }
                    out << ']';
                }
                break;
            case TNULL:
                out << "null";
                break;
            case TBOOL:
                out << (value.data.boolean ? "true" : "false");
        }
    }

    //https://tools.ietf.org/rfc/rfc7159.txt
    std::string Writer::escapeString(std::string unescaped) {
        std::stringstream escaped;
        size_t last = 0, pos = 0, len = unescaped.length();
        while (pos < len) {
            switch (unescaped[pos]) {
                case '"':
                case '\\':
                case '/':
                    escaped << unescaped.substr(last,pos-last) << '\\' << unescaped[pos];
                    last = pos+1;
                    break;
                case '\b':
                    escaped << unescaped.substr(last,pos-last) << "\\b";
                    last = pos+1;
                    break;
                case '\f':
                    escaped << unescaped.substr(last,pos-last) << "\\f";
                    last = pos+1;
                    break;
                case '\n':
                    escaped << unescaped.substr(last,pos-last) << "\\n";
                    last = pos+1;
                    break;
                case '\r': 
                    escaped << unescaped.substr(last,pos-last) << "\\r";
                    last = pos+1;
                    break;
                case '\t':
                    escaped << unescaped.substr(last,pos-last) << "\\t";
                    last = pos+1;
                    break;
                default:
                    if (unescaped[pos] < 0x20) throw parser_error(0,0,"Arbitrary unicode escapes not yet supported"); //FIXME
            }
            pos++;
        }
        escaped << unescaped.substr(last,pos-last);
        return escaped.str();
    }
    
    //https://tools.ietf.org/rfc/rfc7159.txt
    std::string Reader::unescapeString(std::string escaped) {
        if (escaped.find("\\") != std::string::npos) {
            size_t last = 0, pos = 0;
            std::stringstream unescaped;
            while ((pos = escaped.find("\\",pos)) != std::string::npos) {
                unescaped << escaped.substr(last,pos-last);
                switch (escaped[pos+1]) {
                    case '"':
                    case '\\':
                    case '/':
                        unescaped << escaped[pos+1];
                        last = pos = pos+2;
                        break;
                    case 'b':
                        unescaped << '\b';
                        last = pos = pos+2;
                        break;
                    case 'f':
                        unescaped << '\f';
                        last = pos = pos+2;
                        break;
                    case 'n':
                        unescaped << '\n';
                        last = pos = pos+2;
                        break;
                    case 'r':
                        unescaped << '\r';
                        last = pos = pos+2;
                        break;
                    case 't':
                        unescaped << '\t';
                        last = pos = pos+2;
                        break;
                    case 'u':
                        throw parser_error(line,cur-lastbr,"Arbitrary unicode escapes not yet supported"); //FIXME
                    default:
                        throw parser_error(line,cur-lastbr,"Invalid escape sequence in string");
                }
            }
            unescaped << escaped.substr(last);
            return unescaped.str();
        } else {
            return escaped;
        }
    }
    
}

