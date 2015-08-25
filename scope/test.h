/*
  © 2009, Jon Stewart
  Released under the terms of the Boost license (http://www.boost.org/LICENSE_1_0.txt). See License.txt for details.
*/

#pragma once

#include <string>
#include <stdexcept>
#include <sstream>
#include <list>
#include <vector>
#include <functional>
// #include <iostream>

namespace scope {
  typedef std::list<std::string> MessageList; // need to replace this with an output iterator
  typedef std::function<void()> TestFunction;

  void RunFunction(TestFunction test, const char* testname, MessageList& messages);
  void CaughtBadExceptionType(const std::string& testname, const std::string& msg);
  
  class test_failure : public std::runtime_error {
  public:
    test_failure(const char* const file, int line, const char *const message):
      std::runtime_error(message), File(file), Line(line) {}

    virtual ~test_failure() {}
    
    std::string File;
    int Line;
  };
  
  template<typename ExceptionType>
  void eval_condition(bool good, const char* const file, int line, const char *const expression) {
    if (!good) {
      throw ExceptionType(file, line, expression);
    }
  }

  template<
    typename ExceptionType,
    typename ExpectedT,
    typename ActualT
  >
  void eval_equal_impl(ExpectedT&& e, ActualT&& a, const char* const file, int line, const char* msg = "") {
    // it'd be good to have a CTAssert on (ExpectedT==ActualT)
    if (!((e) == (a))) {
      std::ostringstream buf;
      if (*msg) {
        buf << msg << " ";
      }
      buf << "Expected: " << e << ", Actual: " << a;
      throw ExceptionType(file, line, buf.str().c_str());
    }
  }

  // policy for passing arguments to eval_equal by value
  template <typename L, typename R>
  using pass_by_value = std::integral_constant<bool,
    (!std::is_class<L>::value &&
      std::is_convertible<const L, const R>::value) ||
    (!std::is_class<R>::value &&
      std::is_convertible<const R, const L>::value)
  >;

  // eval_equal for arguments passed by value
  template<
    typename ExceptionType,
    typename ExpectedT,
    typename ActualT,
    typename = typename std::enable_if<
      pass_by_value<ExpectedT, ActualT>::value
    >::type
  >
  void eval_equal(const ExpectedT e, const ActualT a, const char* const file, int line, const char* msg = "") {
    eval_equal_impl<ExceptionType>(
      std::forward<const ExpectedT>(e),
      std::forward<const ActualT>(a),
      file, line, msg
    );
  }

  // eval_equal for arguments passed by const reference
  template <
    typename ExceptionType,
    typename ExpectedT,
    typename ActualT,
    typename = typename std::enable_if<
      !pass_by_value<ExpectedT, ActualT>::value
    >::type
  >
  void eval_equal(const ExpectedT& e, const ActualT& a, const char* const file, int line, const char* msg = "") {
    eval_equal_impl<ExceptionType>(
      std::forward<const ExpectedT>(e),
      std::forward<const ActualT>(a),
      file, line, msg
    );
  }

  // eval_equal for std::vector
  template<typename ExceptionType, typename ElementT>
  void eval_equal(const std::vector<ElementT>& e, const std::vector<ElementT>& a, const char* const file, int line, const char* msg = "") {
    if (e.size() == a.size()) {
      for (size_t i = 0; i < e.size(); ++i) {
        if (e[i] != a[i]) {
          std::ostringstream buf;
          if (*msg) {
            buf << msg << " ";
          }
          buf << "Expected[" << i << "]: " << e[i] << ", Actual[" << i << "]: " << a[i];
          throw ExceptionType(file, line, buf.str().c_str());
        }
      }
    }
    else {
      std::ostringstream buf;
      if (*msg) {
        buf << msg << " ";
      }
      buf << "Expected size: " << e.size() << ", Actual size: " << a.size();
      throw ExceptionType(file, line, buf.str().c_str());
    }
  }

  struct TestCommon {
    TestCommon(const std::string& name): Name(name) {}
    virtual ~TestCommon() {}

    std::string Name;
  };

  class Test : public TestCommon {
  public:
    Test(const std::string& name): TestCommon(name) {}
    virtual ~Test() {}

    void Run(MessageList& messages) const {
      _Run(messages);
    }

  private:
    virtual void _Run(MessageList& messages) const = 0;
  };

  class BoundTest : public Test {
  public:
    TestFunction Fn;

    BoundTest(const std::string& name, TestFunction fn):
      Test(name), Fn(fn) {}
    
  private:
    virtual void _Run(MessageList& messages) const {
      RunFunction(Fn, Name.c_str(), messages);
    }
  };

  class SetTest : public Test {
  public:
    SetTest(const std::string& name): Test(name) {}

  private:
    virtual void _Run(MessageList&) const {}
  };
  
  template<class FixtureType> FixtureType* DefaultFixtureConstruct() {
    return new FixtureType;
  }
  
  template<class FixtureT> class FixtureTest: public Test {
  public:
    typedef void (*FixtureTestFunction)(FixtureT&);
    typedef FixtureT* (*FixtureCtorFunction)(void);

    FixtureTestFunction Fn;
    FixtureCtorFunction Ctor;

    FixtureTest(const std::string& name, FixtureTestFunction fn, FixtureCtorFunction ctor): Test(name), Fn(fn), Ctor(ctor) {}

  private:
    virtual void _Run(MessageList& messages) const {
      FixtureT* fixture;
      bool setup = true;
      try {
        // std::cerr << "constructing fixture " << Name << std::endl;
        fixture = (*Ctor)();
        // std::cerr << "constructed fixture " << std::endl;
      }
      catch (const test_failure& fail) {
        messages.push_back(Name + ": " + fail.what());
        setup = false;
      }
      catch (const std::exception& except) {
        messages.push_back(Name + ": " + except.what());
        setup = false;
      }
      catch (...) {
        CaughtBadExceptionType(Name, "setup threw unknown exception type");
        setup = false;
        throw;
      }
      if (!setup) {
        return;
      }
      try {
        // std::cerr << "running test" << std::endl;
        (*Fn)(*fixture);
        // std::cerr << "ran test" << std::endl;
      }
      catch (const test_failure& fail) {
        std::ostringstream buf;
        buf << fail.File << ":" << fail.Line << ": " << Name << ": " << fail.what();
        messages.push_back(buf.str());
      }
      catch (const std::exception& except) {
        messages.push_back(Name + ": " + except.what());
      }
      catch (...) {
        CaughtBadExceptionType(Name, "test threw unknown exception type, fixture will leak");
        throw;
      }
      try {
        // std::cerr << "deleting fixture" << std::endl;
        delete fixture;
        // std::cerr << "deleted fixture" << std::endl;
      }
      catch (const test_failure& fail) {
        // std::cerr << "fixture destructor threw test_failure" << std::endl;
        messages.push_back(Name + ": " + fail.what());
      }
      catch (const std::exception& except) {
        // std::cerr << "fixture destructor threw std::exception" << std::endl;
        messages.push_back(Name + ": " + except.what());
      }
      catch (...) {
        // std::cerr << "fixture destructor threw something" << std::endl;
        CaughtBadExceptionType(Name, "teardown threw unknown exception type");
        throw;
      }
    }
  };

  class AutoRegister;
  class CreateEdge;

  class TestRunner {
    friend class CreateEdge;
  public:
    static TestRunner& Get(void);

    virtual void runTest(const Test* const test, MessageList& messages) = 0;
    virtual void run(MessageList& messages, const std::string& nameFilter = "") = 0;

//    virtual size_t Add(TestPtr test) = 0;

    virtual void addTest(AutoRegister& test) = 0;
    virtual void addLink(CreateEdge& link) = 0;
    virtual void CreateLink(const AutoRegister& from, const AutoRegister& to) = 0;
    virtual unsigned int numTests() const = 0;
    virtual unsigned int numRun() const = 0;
    virtual void setDebug(bool) = 0;
    virtual std::string lastTest() const = 0;

  protected:
    TestRunner() {}
    virtual ~TestRunner() {}

    virtual void FastCreateLink(const AutoRegister& from, const AutoRegister& to) = 0;

  private:
    TestRunner(const TestRunner&);
  };

  template<class T> class Node {
  public:
    void insert(T& node) {
      node.Next = Next;
      Next = &node;
    }

    T*  Next;
  };

  class AutoRegister: public Node<AutoRegister> {
  public:
    size_t          Index;
    const char*     TestName;

    AutoRegister(const char* name):
      Index(0), TestName(name)
    {
      TestRunner::Get().addTest(*this);
    }

    virtual ~AutoRegister() {}

    virtual Test* Construct() = 0;
  };

  class AutoRegisterSet: public AutoRegister {
  public:

    AutoRegisterSet(const char* name): AutoRegister(name) {}
    virtual ~AutoRegisterSet() {}

    virtual Test* Construct() {
      return new SetTest(TestName);
    }
  };

  class AutoRegisterSimple: public AutoRegister {
  public:
    TestFunction  Fn;

    AutoRegisterSimple(const char* name, TestFunction fn): AutoRegister(name), Fn(fn) {}
    virtual ~AutoRegisterSimple() {}

    virtual Test* Construct() {
      return new BoundTest(TestName, Fn);
    }
  };

  template<class FixtureT> class AutoRegisterFixture: public AutoRegister {
  public:
    typedef void (*FixtureTestFunction)(FixtureT&);
    typedef FixtureT* (*FixtureCtorFunction)(void);

    FixtureTestFunction Fn;
    FixtureCtorFunction Ctor;

    AutoRegisterFixture(const char* name, FixtureTestFunction fn, FixtureCtorFunction ctor): AutoRegister(name), Fn(fn), Ctor(ctor) {}
    virtual ~AutoRegisterFixture() {}

    virtual Test* Construct() {
      return new FixtureTest<FixtureT>(TestName, Fn, Ctor);
    }
  };

  class CreateEdge: public Node<CreateEdge> {
  public:
    CreateEdge(const AutoRegister& from, const AutoRegister& to):
      From(from), To(to)
    {
      TestRunner::Get().addLink(*this);
    }

    const AutoRegister  &From,
                        &To;
  };

  class FreeTest {
  public:
    FreeTest(const std::function<void(void)>&) {

    }
  };

  namespace {
    AutoRegister& GetTranslationUnitSet() {
      static AutoRegisterSet singleton(__FILE__);
      return singleton;
    }
  }

  namespace user_defined {}

}

#define SCOPE_CAT(s1, s2) s1##s2
#define SCOPE_UNIQUENAME(base) SCOPE_CAT(base, __LINE__)

// puts the auto-register variable into a unique -- but reference-able -- namespace.
// the auto-register variable can then contain some useful information which we can reference inside of the test.
// I wish that the testname could be the name of the innermost namespace, but C++ seems to be confused by the re-use of the identifier.
// thus, "ns" is trivially appended to ${testname} so as not to confuse C++.
// the requirements put on the namespace are isomorphic to the reqs we use for the scope of the test function, i.e.
// as long as "void testname(void) {}" works in the global scope, the namespacing will work, too.
// if "void testname(void) {}" results in a multiple-symbol linker error, then so will the namespacing.


#define SCOPE_TEST_AUTO_REGISTRATION(testname) \
  namespace scope { namespace user_defined { namespace { namespace SCOPE_CAT(testname, ns) { \
    AutoRegisterSimple reg(#testname, testname); \
    CreateEdge SCOPE_CAT(testname, translation_unit)(GetTranslationUnitSet(), reg); \
  } } } }

#define SCOPE_TEST(testname) \
  void testname(void);      \
  SCOPE_TEST_AUTO_REGISTRATION(testname) \
  void testname(void)

#define SCOPE_SET_WITH_NAME(setidentifier, setname) \
  namespace scope { namespace user_defined { namespace did_you_forget_to_define_your_set { \
    AutoRegisterSet setidentifier(setname); \
    CreateEdge SCOPE_CAT(setidentifier, translation_unit)(GetTranslationUnitSet(), setidentifier); \
  } } }

#define SCOPE_SET(setname) \
  SCOPE_SET_WITH_NAME(setname, #setname)

#define SCOPE_TEST_BELONGS_TO(testname, setname) \
  namespace scope { namespace user_defined { namespace did_you_forget_to_define_your_set { \
    extern AutoRegisterSet setname; } \
  namespace { namespace SCOPE_CAT(testname, ns) { \
    using namespace scope::user_defined::did_you_forget_to_define_your_set; \
    CreateEdge SCOPE_CAT(setname, edgecreator)(setname, reg); \
  } } } }

#define SCOPE_FIXTURE_AUTO_REGISTRATION(fixtureType, testfunction, ctorfunction) \
  namespace scope { namespace user_defined { namespace { namespace SCOPE_CAT(testfunction, ns) { \
    AutoRegisterFixture<fixtureType> reg(#testfunction, testfunction, ctorfunction); \
    CreateEdge SCOPE_CAT(testfunction, translation_unit)(GetTranslationUnitSet(), reg); \
  } } } }

#define SCOPE_FIXTURE(testname, fixtureType) \
  void testname(fixtureType& fixture); \
  SCOPE_FIXTURE_AUTO_REGISTRATION(fixtureType, testname, &DefaultFixtureConstruct<fixtureType>) \
  void testname(fixtureType& fixture)

#define SCOPE_FIXTURE_CTOR(testname, fixtureType, ctorExpr) \
  void testname(fixtureType& fixture); \
  namespace scope { namespace user_defined { namespace { namespace SCOPE_CAT(testname, ns) { \
    fixtureType* fixConstruct() { return new ctorExpr; } \
  } } } } \
  SCOPE_FIXTURE_AUTO_REGISTRATION(fixtureType, testname, scope::user_defined::SCOPE_CAT(testname, ns)::fixConstruct) \
  void testname(fixtureType& fixture)

#define SCOPE_ASSERT_THROW(condition, exceptiontype) \
  scope::eval_condition<exceptiontype>((condition) ? true: false, __FILE__, __LINE__, #condition)

#define SCOPE_ASSERT(condition) \
  SCOPE_ASSERT_THROW(condition, scope::test_failure)
  
#define SCOPE_ASSERT_EQUAL(expected, actual) \
  scope::eval_equal<scope::test_failure>((expected), (actual), __FILE__, __LINE__)

#define SCOPE_ASSERT_EQUAL_MSG(expected, actual, msg) \
  scope::eval_equal<scope::test_failure>((expected), (actual), __FILE__, __LINE__, msg)

#define SCOPE_EXPECT(statement, exception) \
  try { \
    statement; \
    throw scope::test_failure(__FILE__, __LINE__, "Expected exception not caught"); \
  } \
  catch (const exception&) { \
    ; \
  }
