#include <iostream>
#include <iomanip>
#include <vector>

#include "bakkjson.h"

using namespace std;

int main() {

  typedef json::value::object obj;
  typedef json::value::array arr;

  obj o;
  o["hoge"] = 3.0;
  o["moja"] = arr({3.0, arr({3}), 7});
  o["doya"] = obj({{"piyo\n", 5}});
  o["poyo"] = true;

  cout << fixed << setprecision(1) << o << endl;

  json::value val;

  try {
    while(cin >> val) cout << fixed << setprecision(5) << val << endl;
  }
  catch(json::PARSE_ERROR e) {
    cerr << "PARSE_ERROR" << endl;
    cerr << e.c << endl;
    return 0;
  }

  return 0;
}
