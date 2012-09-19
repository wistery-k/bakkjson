#ifndef BAKKJSON_HPP
#define BAKKJSON_HPP

#include <iostream>
#include <map>
#include <vector>
#include <utility>
#include <memory>

namespace json {

  enum type {
    int_t, double_t, string_t, object_t, array_t, true_t, false_t, null_t
  };

  typedef std::pair<type, type> TYPE_ERROR;
  
  class value {
  public:
    typedef std::map<std::string, value> object;
    typedef std::vector<value> array;
  private:
    type typ;

    int i;
    double d;
    std::shared_ptr<std::string> s;
    std::shared_ptr<object> o;
    std::shared_ptr<array> a;
    
    void type_check(type _typ);

  public:
    // constructors
    value();
    value(bool _b);
    value(int _i);
    value(double _d);
    value(const char* _s);
    value(const std::string& _s);
    value(const object& _o);
    value(const array& _a);
    type get_type();

    operator bool();
    operator int();
    operator double();
    operator std::string();
    operator object();
    operator array();

    value& operator[](const char* key);
    value& operator[](const std::string& key);

    value& operator[](int index);

    std::string dump();

    friend std::ostream& operator<<(std::ostream& os, const value& v);

  };

  std::ostream& operator<<(std::ostream& os, const value::object& o);
  
  std::ostream& operator<<(std::ostream& os, const value::array& a);

  struct PARSE_ERROR {
    char c;
    PARSE_ERROR(char _c) { c = _c; }
  };

  std::istream& operator>>(std::istream& is, value& v);

  value parse(const std::string& s);

};

#endif
