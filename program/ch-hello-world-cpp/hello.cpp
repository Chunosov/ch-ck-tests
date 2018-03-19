#include <iostream>
#include <cstdlib>

using namespace std;

void print_env(const char* name) {
  auto var = getenv(name);
  cout << name << "=" << (var ? string(var) : string()) << endl;
}

int main() {
  cout << endl;
  cout << "Hello, world!" << endl;
  cout << endl;
  print_env("CK_CXX");
  // These vars can be empty when compiling on Linux.
  // In this case g++ founds headers and libs at standard locations.
  // But they are set when cross-compilling for Android.
  print_env("CK_ENV_LIB_STDCPP_INCLUDE");
  print_env("CK_ENV_LIB_STDCPP_INCLUDE_EXTRA");
  print_env("CK_CXX_EXTRA");
  print_env("CK_ENV_LIB_STDCPP_STATIC");
  cout << endl;
  return 0;
}
