/*
	© 2009, Jon Stewart
	Released under the terms of the Boost license (http://www.boost.org/LICENSE_1_0.txt). See License.txt for details.
*/

#include "scope/test.h"

struct Fixture1 {
  Fixture1():
    String("cool"), Int(42) {}
  virtual ~Fixture1() {}

  std::string String;
  int         Int;
};

SCOPE_FIXTURE(fix1, Fixture1) {
  SCOPE_ASSERT(std::string("cool") == fixture.String);
  SCOPE_ASSERT_EQUAL(42, fixture.Int);
  SCOPE_ASSERT_EQUAL_MSG(41, fixture.Int, "silly");
}

struct Fixture2: public Fixture1 {
  Fixture2() {
    SCOPE_ASSERT(!"Fixture2's constructor threw");
  }
};

SCOPE_FIXTURE(badSetup, Fixture2) {
  SCOPE_ASSERT_EQUAL(41, fixture.Int); // should not be called
}

struct Fixture3: public Fixture1 {
  ~Fixture3() {
    SCOPE_ASSERT(!"Fixture3's destructor threw");
  }
};

SCOPE_FIXTURE(badTeardown, Fixture3) {
  SCOPE_ASSERT_EQUAL(42, fixture.Int);
}

struct Fixture4: public Fixture1 {
  Fixture4(int i):
    Fixture1()
  {
    Int = i;
  }
};

SCOPE_FIXTURE_CTOR(customFixture, Fixture1, Fixture4(7)) {
  SCOPE_ASSERT_EQUAL(7, fixture.Int);
}
