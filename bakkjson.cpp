#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <memory>
#include <utility>
#include <cmath>

#include "bakkjson.hpp"

namespace {
  std::map<char, std::string> trans = {
    {'"', "\\\""},
    {'\\', "\\\\"},
    {'/', "\\/"},
    {'\b', "\\b"},
    {'\f', "\\f"},
    {'\n', "\\n"},
    {'\r', "\\r"},
    {'\t', "\\t"}
  };

  std::map<char, char> rev_trans = {
    {'\"', '"'},
    {'\\', '\\'},
    {'/', '/'},
    {'b', '\b'},
    {'f', '\f'},
    {'n', '\n'},
    {'r', '\r'},
    {'t', '\t'}
  };

  std::string escape(const std::string& s) {
    std::stringstream ss;
    ss << "\"";
    for(char c : s) {
      if(trans.count(c)) ss << trans[c];
      else ss << c;
    }
    ss << "\"";
    return ss.str();
  }

  char next_token(std::istream& is) {
    char c;
    while(true) {
      c = is.peek();
      if(isspace(c)) is.get();
      else break;
    }
    return c;
  }

  std::istream& parse_true(std::istream& is, json::value& v) {
    std::string s = "true";
    for(char c : s) if(is.get() != c) throw json::PARSE_ERROR(c);
    v = json::value(true);
    return is;
  }

  std::istream& parse_false(std::istream& is, json::value& v) {
    std::string s = "false";
    for(char c : s) if(is.get() != c) throw json::PARSE_ERROR(c);
    v = json::value(false);
    return is;
  }

  std::istream& parse_null(std::istream& is, json::value& v) {
    std::string s = "null";
    for(char c : s) if(is.get() != c) throw json::PARSE_ERROR(c);
    v = json::value();
    return is;
  }

  std::istream& parse_number(std::istream& is, json::value& v) {

    int sign = 1;
    int i = 0;
    double d = 0.0;
    double f = 0.0;
    double exp_sign = 10;
    double exp = 0;
    bool integer = true;
    char c;

    c = is.peek();
    if(c == '+' || c == '-') {
      is.get();
      if(c == '-') {
        sign = -1;
      }
    }

    c = is.get();
    if(c == '0') {
      i = 0;
    }
    else if(isdigit(c)) {

      i = c - '0';
      d = c - '0';

      while((c = is.get())) {
        if(!isdigit(c)) {
          is.unget();
          break;
        }
        i = i * 10 + (c - '0');
        d = d * 10 + (c - '0');
      }
    
    }
    else throw json::PARSE_ERROR(c);

    if(is.peek() == '.') {
      is.get();
      integer = false;
	  
      int decimal_place = 1;
      while((c = is.get())) {
        if(!isdigit(c)) {
          is.unget();
          break;
        }
        f = f + pow(0.1, decimal_place++) * (c - '0');
      }
    }

    c = is.peek();
    if(c == 'e' || c == 'E') {
      is.get();
      integer = false;

      c = is.peek();
      if(c == '+' || c == '-') {
        is.get();
        if(c == '-') exp_sign = 0.1;
      }

      while((c = is.get())) {
        if(!isdigit(c)) {
          is.unget();
          break;
        }
        exp = exp * 10 + (c - '0');
      }
    }
  
    if(integer) v = sign * i;
    else v = sign * (d + f) * std::pow(exp_sign, exp);
    return is;
  }

  std::istream& parse_string(std::istream& is, json::value& v) {
    char c;
    std::stringstream ss;
    if ((c = is.get()) != '"') throw json::PARSE_ERROR(c);

    while((c = is.get())) {
      if(c == EOF) throw json::PARSE_ERROR(c);
      if(c == '"') break;
      if(c == '\\') {
        c = is.get();
        if(c == EOF) throw json::PARSE_ERROR(c);
        if(rev_trans.count(c)) ss << rev_trans[c];
        else throw json::PARSE_ERROR(c);
      }
      else ss << c;        
    }
    v = json::value(ss.str());
    return is;
  }

  std::istream& parse_object(std::istream& is, json::value& v) {
    char c;
    if((c = is.get()) != '{') throw json::PARSE_ERROR(c);
    json::value::object obj;

    c = next_token(is);
    if(c == EOF) throw json::PARSE_ERROR(c);
    if(c == '}') {
      is.get();
      v = obj;
      return is;
    }
    if(c != '"') throw json::PARSE_ERROR(c);
    json::value key;
    is >> key;
    if(key.get_type() != json::string_t) throw json::PARSE_ERROR(c);
    if(next_token(is) != ':') throw json::PARSE_ERROR(c);
    is.get();
    json::value val;
    is >> val;
    obj[(std::string)key] = val;

    while((c = next_token(is))) {
      if(c == EOF) throw json::PARSE_ERROR(c);
      if(c == '}') {
        is.get();
        break;
      }
      if(c != ',') throw json::PARSE_ERROR(c);
      is.get();
      json::value key;
      is >> key;
      if(key.get_type() != json::string_t) throw json::PARSE_ERROR(c);
      if(next_token(is) != ':') throw json::PARSE_ERROR(c);
      is.get();
      json::value val;
      is >> val;
      obj[(std::string)key] = val;
    }
    v = obj;
    return is;
  }

  std::istream& parse_array(std::istream& is, json::value& v) {
    char c;
    json::value::array arr;

    if((c = is.get()) != '[') throw json::PARSE_ERROR(c);
    if(next_token(is) == ']') {
      is.get();
      v = arr;
      return is;
    }
    json::value vv;
    is >> vv;
    arr.push_back(vv);
    while((c = next_token(is))) {
      if(c == EOF) throw json::PARSE_ERROR(c);
      if(c == ']') {
        is.get();
        break;
      }
      if(c != ',') throw json::PARSE_ERROR(c);
      is.get();
      is >> vv;
      arr.push_back(vv);
    }
    v = arr;
    return is;
  }


}

namespace json {

  void value::type_check(type _typ) { 
    if(_typ != typ) throw TYPE_ERROR(_typ, typ); 
  }

  // constructors
  value::value(){ typ = null_t; }
  value::value(bool _b){ typ = (_b) ? true_t : false_t; }
  value::value(int _i){ typ = int_t; i = _i; }
  value::value(double _d){ typ = double_t, d = _d; }
  value::value(const char* _s){ typ = string_t, s = std::shared_ptr<std::string>(new std::string(_s)); }
  value::value(const std::string& _s){ typ = string_t, s = std::shared_ptr<std::string>(new std::string(_s)); }
  value::value(const object& _o) { typ = object_t, o = std::shared_ptr<object>(new object(_o)); }
  value::value(const array& _a) { typ = array_t, a = std::shared_ptr<array>(new array(_a)); }

  type value::get_type(){ return typ; }

  value::operator bool() { 
    if(typ == true_t) return true;
    else if(typ == false_t) return false;
    else throw TYPE_ERROR(true_t, typ);
  }
  value::operator int() { type_check(int_t); return i; }
  value::operator double() { type_check(double_t); return d; }
  value::operator std::string() { type_check(string_t); return *s; }
  value::operator object() { type_check(object_t); return *o; }
  value::operator array() { type_check(array_t); return *a; }

  value& value::operator[](const char* key) { return (*o)[std::string(key)]; }
  value& value::operator[](const std::string& key) { type_check(object_t); return (*o)[key]; }  

  value& value::operator[](int index) { type_check(array_t); return (*a)[index]; }

  std::string value::dump() {
    std::stringstream ss;
    ss << *this;
    return ss.str();
  }

  std::ostream& operator<<(std::ostream& os, const value& v) {
      
    switch(v.typ) {
    case int_t:    os << v.i;          break;
    case double_t: os << v.d;          break;
    case string_t: os << escape(*v.s); break;
    case object_t: os << *v.o;         break;
    case array_t:  os << *v.a;         break;
    case true_t:   os << "true";       break;
    case false_t:  os << "false";      break;
    case null_t:   os << "null";       break;
    }
    
    return os;
  }

  std::ostream& operator<<(std::ostream& os, const value::object& o) {
    if (o.empty()) { os << "{}"; return os; }
    os << "{";
    value::object::const_iterator it = o.begin();
    os << escape(it->first) << ": " << it->second;
    for(++it; it != o.end(); ++it) os << ", " << escape(it->first) << ": " << it->second;
    os << "}";
    return os;
  }
  
  std::ostream& operator<<(std::ostream& os, const value::array& a) {
    if (a.empty()) { os << "[]"; return os; }
    os << "[";
    value::array::const_iterator it = a.begin();
    os << *it;
    for(++it; it != a.end(); ++it) { os << ", " << *it; }
    os << "]";
    return os;
  }
  
  std::istream& operator>>(std::istream& is, value& v) {
    char c = next_token(is);
    
    if (c == 't') parse_true(is, v);
    else if (c == 'f') parse_false(is, v);
    else if (c == 'n') parse_null(is, v);
    else if (c == '{') parse_object(is, v);
    else if (c == '[') parse_array(is, v);
    else if (c == '"') parse_string(is, v);
    else if (isdigit(c) || c == '+' || c == '-') parse_number(is, v);
    else throw PARSE_ERROR(c);
    
    return is;
  }

  value parse(const std::string& s) {
    std::stringstream ss(s);
    value val;
    ss >> val;
    return val;
  }

};

