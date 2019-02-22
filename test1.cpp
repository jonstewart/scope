/*
	© 2009, Jon Stewart
	Released under the terms of the Boost license (http://www.boost.org/LICENSE_1_0.txt). See License.txt for details.
*/

#include "scope/test.h"

#include <vector>
#include <list>
#include <set>

SCOPE_TEST(simpleTest) {
  SCOPE_ASSERT(true);
}

SCOPE_TEST(failTest) {
  SCOPE_ASSERT(false); // ha-ha!
}

void doNothing() {
  ;
}

SCOPE_TEST(TestExpectMacro) {
  SCOPE_EXPECT(throw int(1), int);
  SCOPE_EXPECT(doNothing(), int);
}

SCOPE_TEST_FAILS(knownBadTest) {
  SCOPE_ASSERT(false);
}

SCOPE_TEST_FAILS(aGoodBadTest) {
  ;
}

SCOPE_TEST_IGNORE(thisTestNeverRuns) {
  *reinterpret_cast<int*>(1) = 25; // segfaults
}

SCOPE_TEST(simpleEquality) {
  SCOPE_ASSERT_EQUAL(1, 1);
}

SCOPE_TEST(sequenceEquality) {
  std::vector<int> e = {1, 2, 3};
  std::list<int> a = {1, 2, 3};
  SCOPE_ASSERT_EQUAL(e, a);
}

SCOPE_TEST(initListExpectedEqual) {
  std::vector<int> a = {1, 2, 3};
  std::list<int> b = {1, 2, 3};
  SCOPE_ASSERT_EQUAL({1, 2, 3}, a);
  SCOPE_ASSERT_EQUAL({1, 2, 3}, b);
}

SCOPE_TEST(initListActualEqual) {
  std::vector<int> a = {1, 2, 3};
  std::list<int> b = {1, 2, 3};
  SCOPE_ASSERT_EQUAL(a, {1, 2, 3});
  SCOPE_ASSERT_EQUAL(b, {1, 2, 3});
}

SCOPE_TEST(initListPedantic) {
  SCOPE_ASSERT_EQUAL({1, 2, 3}, {1, 2, 3});
}

SCOPE_TEST(pairEquality) {
  std::pair<short, std::string> a = {5, "hello"};
  std::pair<int, std::string> b = {5, "hello, world"};
  SCOPE_ASSERT_EQUAL(a, b);
}

SCOPE_TEST(setEquality) {
  std::set<std::string> a {"hello", "world"};
  std::set<std::string> b {"hello", "world"};

  SCOPE_ASSERT_EQUAL(a, b);
}

SCOPE_TEST(setInequality) {
  std::set<std::string> a {"hello", "world"};
  std::set<std::string> b {"hello"};

  SCOPE_ASSERT_EQUAL(a, b);
}

SCOPE_TEST(stringEquality) {
  char* a = (char*) "hello";
  const char* b = "hello";
  SCOPE_ASSERT_EQUAL(a, b);
  SCOPE_ASSERT_EQUAL("hello", a);
}

SCOPE_TEST(nullptrEquality) {
  int* a = nullptr;
  SCOPE_ASSERT_EQUAL(nullptr, a);
}
