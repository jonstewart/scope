/*
	© 2009, Jon Stewart
	Released under the terms of the Boost license (http://www.boost.org/LICENSE_1_0.txt). See License.txt for details.
*/

#include <iostream>

#include "scope/testrunner.h"

int main(int argc, char *argv[]) {
  return scope::DefaultRun(std::cout, argc, argv) ? 0: 1;
}
