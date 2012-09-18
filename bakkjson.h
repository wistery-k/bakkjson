#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <memory>
#include <initializer_list>
#include <utility>
#include <cmath>

namespace json {

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

  enum type {
    int_t, double_t, string_t, object_t, array_t, true_t, false_t, null_t
  };

  using TYPE_ERROR = std::pair<type, type>;
  
  class value {
  public:
    using object = std::map<std::string, value>;
    using array = std::vector<value>;

    type typ;

    int i;
    double d;
    std::shared_ptr<std::string> s;
    std::shared_ptr<object> o;
    std::shared_ptr<array> a;
    
    void type_check(type _typ) { if(_typ != typ) throw TYPE_ERROR(_typ, typ); }

  public:

    // constructors
    value(){ typ = null_t; }
    value(int _i){ typ = int_t; i = _i; }
    value(double _d){ typ = double_t, d = _d; }
    value(const std::string& _s){ typ = string_t, s = std::shared_ptr<std::string>(new std::string(_s)); }
    value(const object& _o) { typ = object_t, o = std::shared_ptr<object>(new object(_o)); }
    value(const array& _a) { typ = array_t, a = std::shared_ptr<array>(new array(_a)); }

    operator int() { type_check(int_t); return i; }
    operator double() { type_check(double_t); return d; }
    operator std::string() { type_check(string_t); return *s; }
    operator object() { type_check(object_t); return *o; }
    operator array() { type_check(array_t); return *a; }

    std::string dump() {
      std::stringstream ss;
      ss << *this;
      return ss.str();
    }

    friend std::ostream& operator<<(std::ostream& os, const value& v) {
      
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

    friend std::ostream& operator<<(std::ostream& os, const object& o) {
      if (o.empty()) { os << "{}"; return os; }
      os << "{";
      object::const_iterator it = o.begin();
      os << escape(it->first) << ": " << it->second;
      for(++it; it != o.end(); ++it) os << ", " << escape(it->first) << ": " << it->second;
      os << "}";
      return os;
    }
  
    friend std::ostream& operator<<(std::ostream& os, const array& a) {
      if (a.empty()) { os << "[]"; return os; }
      os << "[";
      array::const_iterator it = a.begin();
      os << *it;
      for(++it; it != a.end(); ++it) { os << ", " << *it; }
      os << "]";
      return os;
    }

  };

  struct PARSE_ERROR {
    char c;
    PARSE_ERROR(char _c) { c = _c; }
  };

  std::istream& operator>>(std::istream&, value&);

  char next_token(std::istream& is) {
    char c;
    while(true) {
      c = is.peek();
      if(isspace(c)) is.get();
      else break;
    }
    return c;
  }

  std::istream& parse_true(std::istream& is, value& v) {
    std::string s = "true";
    for(char c : s) if(is.get() != c) throw PARSE_ERROR(c);
    v = value(true);
    return is;
  }

  std::istream& parse_false(std::istream& is, value& v) {
    std::string s = "false";
    for(char c : s) if(is.get() != c) throw PARSE_ERROR(c);
    v = value(false);
    return is;
  }

  std::istream& parse_null(std::istream& is, value& v) {
    std::string s = "null";
    for(char c : s) if(is.get() != c) throw PARSE_ERROR(c);
    v = value();
    return is;
  }

  std::istream& parse_number(std::istream& is, value& v) {

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
      if(c == '-') sign = -1;
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
    else throw PARSE_ERROR(c);

    if(is.peek() == '.') {
      is.get();
      integer = false;
      while((c = is.get())) {
        if(!isdigit(c)) {
          is.unget();
          break;
        }
        f = 0.1 * f + 0.1 * (c - '0');
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

  std::istream& parse_string(std::istream& is, value& v) {
    char c;
    std::stringstream ss;
    if ((c = is.get()) != '"') throw PARSE_ERROR(c);

    while((c = is.get())) {
      if(c == EOF) throw PARSE_ERROR(c);
      if(c == '"') break;
      if(c == '\\') {
        c = is.get();
        if(c == EOF) throw PARSE_ERROR(c);
        if(rev_trans.count(c)) ss << rev_trans[c];
        else throw PARSE_ERROR(c);
      }
      else ss << c;        
    }
    v = value(ss.str());
    return is;
  }

  std::istream& parse_object(std::istream& is, value& v) {
    char c;
    if((c = is.get()) != '{') throw PARSE_ERROR(c);
    value::object obj;

    c = next_token(is);
    if(c == EOF) throw PARSE_ERROR(c);
    if(c == '}') {
      is.get();
      v = obj;
      return is;
    }
    if(c != '"') throw PARSE_ERROR(c);
    json::value key;
    is >> key;
    if(key.typ != string_t) throw PARSE_ERROR(c);
    if(next_token(is) != ':') throw PARSE_ERROR(c);
    is.get();
    json::value val;
    is >> val;
    obj[(std::string)key] = val;

    while((c = next_token(is))) {
      if(c == EOF) throw PARSE_ERROR(c);
      if(c == '}') {
        is.get();
        break;
      }
      if(c != ',') throw PARSE_ERROR(c);
      is.get();
      json::value key;
      is >> key;
      if(key.typ != string_t) throw PARSE_ERROR(c);
      if(next_token(is) != ':') throw PARSE_ERROR(c);
      is.get();
      json::value val;
      is >> val;
      obj[(std::string)key] = val;
    }
    v = obj;
    return is;
  }

  std::istream& parse_array(std::istream& is, value& v) {
    char c;
    value::array arr;

    if((c = is.get()) != '[') throw PARSE_ERROR(c);
    if(next_token(is) == ']') {
      is.get();
      v = arr;
      return is;
    }
    value vv;
    is >> vv;
    arr.push_back(vv);
    while((c = next_token(is))) {
      if(c == EOF) throw PARSE_ERROR(c);
      if(c == ']') {
        is.get();
        break;
      }
      if(c != ',') throw PARSE_ERROR(c);
      is.get();
      is >> vv;
      arr.push_back(vv);
    }
    v = arr;
    return is;
  }

  std::istream& operator>>(std::istream& is, value& v) {
    char c = next_token(is);
    
    if (c == 't') parse_true(is, v);
    else if (c == 'f') parse_false(is, v);
    else if (c == 'n') parse_null(is, v);
    else if (c == '{') parse_object(is, v);
    else if (c == '[') parse_array(is, v);
    else if (c == '"') parse_string(is, v);
    else if (isdigit(c)) parse_number(is, v);
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
