/**
 * @file boostbind.cc
 * @author koritafei (koritafei@gmail.com)
 * @brief
 * @version 0.1
 * @date 2021-10-08
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <iostream>

class Foo {
public:
  void memberFunc(double d, int i, int j) {
    std::cout << "double d = " << d << std::endl;
    std::cout << "int i = " << i << std::endl;
    std::cout << "int j = " << j << std::endl;
  }

private:
};

int main(int argc, const char** argv) {
  Foo foo;

  boost::function<void(int, int)> fp =
      boost::bind(&Foo::memberFunc, &foo, 0.5, -1, 1000);
  fp(10, 2000);

  return 0;
}
