/*
  © 2009, Jon Stewart
  Released under the terms of the Boost license (http://www.boost.org/LICENSE_1_0.txt). See License.txt for details.
*/

#pragma once

#include <string>
#include <stdexcept>
#include <sstream>
#include <list>
#include <functional>
#include <regex>
// #include <iostream>



namespace scope {
  typedef std::list<std::string> MessageList; // need to replace this with an output iterator
  typedef std::function<void()> TestFunction;

  void runFunction(TestFunction test, const char* testname, bool shouldFail, MessageList& messages);
  void caughtBadExceptionType(const std::string& testname, const std::string& msg);

  class TestFailure: public std::runtime_error {
  public:
    TestFailure(const char* const file, int line, const char *const message):
      std::runtime_error(message), File(file), Line(line) {}

    virtual ~TestFailure() {}

    std::string File;
    int Line;
  };

  template<typename ExceptionType>
  void evalCondition(bool good, const char* const file, int line, const char *const expression) {
    if (!good) {
      throw ExceptionType(file, line, expression);
    }
  }

  template<typename ExceptionType, typename ExpectedT, typename ActualT>
  auto evalEqualImpl(ExpectedT&& e, ActualT&& a, long, const char* const file, int line, const char* msg = "")
   -> decltype((e == a), std::declval<std::ostringstream&>() << e, std::declval<std::ostringstream&>() << a, void())
  {
    // it'd be good to have a CTAssert on (ExpectedT==ActualT)
    if (!(e == a)) {
      std::ostringstream buf;
      if (*msg) {
        buf << msg << " ";
      }
      buf << "Expected: " << e << ", Actual: " << a;
      throw ExceptionType(file, line, buf.str().c_str());
    }
  }

  // evalEqualImpl for anything having std::begin and std::end overloads
  template <
    typename ExceptionType,
    typename ExpSequenceT,
    typename ActSequenceT
  >
  auto evalEqualImpl(ExpSequenceT&& e, ActSequenceT&& a, int, const char* const file, int line, const char* msg = "")
   -> decltype(std::begin(e), std::end(e), std::begin(a), std::end(a), void())
  {
    const auto abeg = std::begin(a);
    const auto aend = std::end(a);
    const auto ebeg = std::begin(e);
    const auto eend = std::end(e);

    const auto mis = std::mismatch(ebeg, eend, abeg);

    if (mis.first != eend || mis.second != aend) {
      std::ostringstream buf;
      if (*msg) {
        buf << msg << " ";
      }

      buf << "Mismatch at index "
          << std::distance(ebeg, mis.first)
          << ". Expected: ";

      if (mis.first == eend) {
        buf << "*past end*";
      }
      else {
        buf << *mis.first;
      }

      buf << ", Actual: ";

      if (mis.second == aend) {
        buf << "*past end*";
      }
      else {
        buf << *mis.second;
      }

      buf << ", Expected size: " << std::distance(ebeg, eend)
          << ", Actual size: " << std::distance(abeg, aend);

      throw ExceptionType(file, line, buf.str().c_str());
    }
  }

  template <
    typename TupleT
  >
  const char* tupleGetOrDefault(TupleT&&, std::integral_constant<size_t, std::tuple_size<typename std::remove_reference<TupleT>::type>::value>)
  {
    return "*past end*";
  }

  template <
    typename TupleT,
    size_t I,
    typename = typename std::enable_if<I!=std::tuple_size<typename std::remove_reference<TupleT>::type>::value>::type
  >
  const char* tupleGetOrDefault(TupleT&& tup, std::integral_constant<size_t, I>)
  {
    std::ostringstream buf;
    buf << std::get<I>(tup);
    return buf.str().c_str();
  }

  template <
    typename ExceptionType,
    typename ExpTupleT,
    typename ActTupleT
  >
  void  evalEqualImplTuple(ExpTupleT&& e,
                          ActTupleT && a,
                          std::integral_constant<size_t,
                                                std::min(std::tuple_size<typename std::remove_reference<ExpTupleT>::type>::value,
                                                         std::tuple_size<typename std::remove_reference<ActTupleT>::type>::value)> I,
                          const char* const file, int line, const char* msg )
  {
    if (I != std::max(std::tuple_size<typename std::remove_reference<ExpTupleT>::type>::value,
                      std::tuple_size<typename std::remove_reference<ActTupleT>::type>::value)) {
      size_t eSize = std::tuple_size<typename std::remove_reference<ExpTupleT>::type>::value;
      size_t aSize = std::tuple_size<typename std::remove_reference<ActTupleT>::type>::value;
      std::ostringstream buf;
      if (*msg) {
        buf << msg << " ";
      }

      buf << "Mismatch at index "
          << I
          << ". Expected: "
          << tupleGetOrDefault(e, std::integral_constant<size_t, I>())
          << ", Actual: "
          << tupleGetOrDefault(a, std::integral_constant<size_t, I>())
          << ", Expected size: " << eSize
          << ", Actual size: " << aSize;

      throw ExceptionType(file, line, buf.str().c_str());
    }

  }

  template <
    typename ExceptionType,
    typename ExpTupleT,
    typename ActTupleT,
    size_t I,
    typename = typename std::enable_if<I!=std::min(std::tuple_size<typename std::remove_reference<ExpTupleT>::type>::value,
                                                   std::tuple_size<typename std::remove_reference<ActTupleT>::type>::value)>::type
  >
  void evalEqualImplTuple(ExpTupleT&& e, ActTupleT && a, std::integral_constant<size_t, I>, const char* const file, int line, const char* msg)
  {
    if (std::get<I>(e) != std::get<I>(a)) {
      std::ostringstream buf;
      size_t eSize = std::tuple_size<typename std::remove_reference<ExpTupleT>::type>::value;
      size_t aSize = std::tuple_size<typename std::remove_reference<ActTupleT>::type>::value;
      if (*msg) {
        buf << msg << " ";
      }

      buf << "Mismatch at index "
          << I
          << ". Expected: "
          << std::get<I>(e)
          << ", Actual: "
          << std::get<I>(a)
          << ", Expected size: " << eSize
          << ", Actual size: " << aSize;

      throw ExceptionType(file, line, buf.str().c_str());
    }
    evalEqualImplTuple<ExceptionType, ExpTupleT, ActTupleT>(e, a, std::integral_constant<size_t, I+1>(), file, line, msg);
  }

  // evalEqualImpl for std::tuple -- still needs work for clang & gcc
  template <
    typename ExceptionType,
    typename ExpTupleT,
    typename ActTupleT
  >
  auto evalEqualImpl(ExpTupleT&& e, ActTupleT&& a, int, const char* const file, int line, const char* msg = "")
   -> decltype(std::tuple_size<typename std::remove_reference<ExpTupleT>::type>::value,
               std::tuple_size<typename std::remove_reference<ActTupleT>::type>::value,
               void())
   {
     evalEqualImplTuple<ExceptionType, ExpTupleT, ActTupleT>(e, a, std::integral_constant<size_t, 0>(), file, line, msg);
   }

  // policy for passing arguments to evalEqual by value
  template <typename L, typename R>
  using pass_by_value = std::integral_constant<bool,
    (!std::is_class<L>::value &&
      std::is_convertible<const L, const R>::value) ||
    (!std::is_class<R>::value &&
      std::is_convertible<const R, const L>::value)
  >;

  // evalEqual for arguments passed by value
  template<
    typename ExceptionType,
    typename ExpectedT,
    typename ActualT,
    typename = typename std::enable_if<
      pass_by_value<ExpectedT, ActualT>::value
    >::type
  >
  void evalEqual(const char* const file, int line, const ExpectedT e, const ActualT a, const char* msg = "") {
    evalEqualImpl<ExceptionType>(
      std::forward<const ExpectedT>(e),
      std::forward<const ActualT>(a),
      0, // prefer sequence overload, because 0 is an int
      file, line, msg
    );
  }

  // evalEqual for arguments passed by const reference
  template <
    typename ExceptionType,
    typename ExpectedT,
    typename ActualT,
    typename = typename std::enable_if<
      !pass_by_value<ExpectedT, ActualT>::value
    >::type
  >
  void evalEqual(const char* const file, int line, const ExpectedT& e, const ActualT& a, const char* msg = "") {
    evalEqualImpl<ExceptionType>(
      std::forward<const ExpectedT&>(e),
      std::forward<const ActualT&>(a),
      0, // prefer sequence overload, because 0 is an int
      file, line, msg
    );
  }

  // evalEqual for std::pairs (const-ref)
  template <
    typename ExceptionType,
    typename ExpectedFirstT,
    typename ExpectedSecondT,
    typename ActualFirstT,
    typename ActualSecondT
  >
  void evalEqual(const char* const file,
                 int line,
                 const std::pair<ExpectedFirstT, ExpectedSecondT>& e,
                 const std::pair<ActualFirstT, ActualSecondT>& a,
                 const char* msg = "")
  {
    if (e.first != a.first || e.second != a.second) {
      std::ostringstream buf;
      if (*msg) {
        buf << msg << ". ";
      }
      if (e.first != a.first) {
        buf << "Expected first: " << e.first
          << ", Actual first: " << a.first << ". ";
      }
      if (e.second != a.second) {
        buf << "Expected second: " << e.second
          << ", Actual second: " << a.second << ".";
      }
      throw ExceptionType(file, line, buf.str().c_str());
    }
  }

  // evalEqual for std::initializer_list as expected arg, actual by const-ref
  template <
    typename ExceptionType,
    typename ExpectedT,
    typename ActualT,
    typename = typename std::enable_if<
      !pass_by_value<ExpectedT, ActualT>::value
    >::type
  >
  void evalEqual(const char* const file, int line, const std::initializer_list<ExpectedT>& e, const ActualT& a, const char* msg = "") {
    evalEqualImpl<ExceptionType>(
      std::forward<const std::initializer_list<ExpectedT>&>(e),
      std::forward<const ActualT&>(a),
      0, // prefer sequence overload, because 0 is an int
      file, line, msg
    );
  }

  // evalEqual for std::initializer_list as actual arg, expected by const-ref
  template <
    typename ExceptionType,
    typename ExpectedT,
    typename ActualT,
    typename = typename std::enable_if<
      !pass_by_value<ExpectedT, ActualT>::value
    >::type
  >
  void evalEqual(const char* const file, int line, const ExpectedT& e, const std::initializer_list<ActualT>& a, const char* msg = "") {
    evalEqualImpl<ExceptionType>(
      std::forward<const ExpectedT&>(e),
      std::forward<const std::initializer_list<ActualT>&>(a),
      0, // prefer sequence overload, because 0 is an int
      file, line, msg
    );
  }

  // evalEqual for std::initializer_list as actual arg, expected by const-ref
  template <
    typename ExceptionType,
    typename ExpectedT,
    typename ActualT
  >
  void evalEqual(const char* const file,
                 int line,
                 const std::initializer_list<ExpectedT>& e,
                 const std::initializer_list<ActualT>& a,
                 const char* msg = "")
  {
    evalEqualImpl<ExceptionType>(
      std::forward<const std::initializer_list<ExpectedT>&>(e),
      std::forward<const std::initializer_list<ActualT>&>(a),
      0, // prefer sequence overload, because 0 is an int
      file, line, msg
    );
  }

  struct TestCommon {
    TestCommon(const std::string& name, const std::string& source):
      Name(name), SourceFile(source) {}

    virtual ~TestCommon() {}

    std::string Name,
                SourceFile;
  };

  class TestCase: public TestCommon {
  public:
    TestCase(const std::string& name, const std::string& source): TestCommon(name, source) {}
    virtual ~TestCase() {}

    void Run(MessageList& messages) const {
      _Run(messages);
    }

  private:
    virtual void _Run(MessageList& messages) const = 0;
  };

  class BoundTest: public TestCase {
  public:
    TestFunction Fn;
    bool         ShouldFail;

    BoundTest(const std::string& name, const std::string& source, TestFunction fn, bool shouldFail):
      TestCase(name, source), Fn(fn), ShouldFail(shouldFail) {}

  private:
    virtual void _Run(MessageList& messages) const {
      runFunction(Fn, Name.c_str(), ShouldFail, messages);
    }
  };

  template<class FixtureType> FixtureType* DefaultFixtureConstruct() {
    return new FixtureType;
  }

  template<class FixtureT> class FixtureTest: public TestCase {
  public:
    typedef void (*FixtureTestFunction)(FixtureT&);
    typedef FixtureT* (*FixtureCtorFunction)(void);

    FixtureTestFunction Fn;
    FixtureCtorFunction Ctor;

    FixtureTest(const std::string& name, const std::string& source, FixtureTestFunction fn, FixtureCtorFunction ctor):
      TestCase(name, source), Fn(fn), Ctor(ctor) {}

  private:
    virtual void _Run(MessageList& messages) const {
      FixtureT* fixture;
      bool setup = true;
      try {
        // std::cerr << "constructing fixture " << Name << std::endl;
        fixture = (*Ctor)();
        // std::cerr << "constructed fixture " << std::endl;
      }
      catch (const TestFailure& fail) {
        messages.push_back(Name + ": " + fail.what());
        setup = false;
      }
      catch (const std::exception& except) {
        messages.push_back(Name + ": " + except.what());
        setup = false;
      }
      catch (...) {
        caughtBadExceptionType(Name, "setup threw unknown exception type");
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
      catch (const TestFailure& fail) {
        std::ostringstream buf;
        buf << fail.File << ":" << fail.Line << ": " << Name << ": " << fail.what();
        messages.push_back(buf.str());
      }
      catch (const std::exception& except) {
        messages.push_back(Name + ": " + except.what());
      }
      catch (...) {
        caughtBadExceptionType(Name, "test threw unknown exception type, fixture will leak");
        throw;
      }
      try {
        // std::cerr << "deleting fixture" << std::endl;
        delete fixture;
        // std::cerr << "deleted fixture" << std::endl;
      }
      catch (const TestFailure& fail) {
        // std::cerr << "fixture destructor threw TestFailure" << std::endl;
        messages.push_back(Name + ": " + fail.what());
      }
      catch (const std::exception& except) {
        // std::cerr << "fixture destructor threw std::exception" << std::endl;
        messages.push_back(Name + ": " + except.what());
      }
      catch (...) {
        // std::cerr << "fixture destructor threw something" << std::endl;
        caughtBadExceptionType(Name, "teardown threw unknown exception type");
        throw;
      }
    }
  };

  template<class T> class Node;
  class AutoRegister;

  class TestRunner {
  public:
    static Node<AutoRegister>& root();
    static std::string& lastTest();

    virtual ~TestRunner() {}

    virtual void runTest(const TestCase& test, MessageList& messages) = 0;
    virtual void run(MessageList& messages) = 0;

    virtual unsigned int numTests() const = 0;
    virtual unsigned int numRun() const = 0;
    virtual void setDebug(bool) = 0;
    virtual void setFilter(const std::shared_ptr<std::regex>& filter) = 0;
  };

  template<class T> class Node {
  public:
    Node(): Next(nullptr), FirstChild(nullptr) {}

    void insert(T& node) {
      node.Next = FirstChild;
      FirstChild = &node;
    }

    T*  Next;
    T*  FirstChild;
  };

  class AutoRegister: public Node<AutoRegister> {
  public:
    const char*     TestName;
    const char*     SourceFile;

    AutoRegister(const char* name, const char* source): TestName(name), SourceFile(source) {}

    virtual ~AutoRegister() {}

    virtual TestCase* Construct() { return nullptr; };
  };

  class AutoRegisterTest: public AutoRegister {
  public:
    AutoRegisterTest(const char* name, const char* source):
      AutoRegister(name, source)
    {
      TestRunner::root().insert(*this);
    }
  };

  class AutoRegisterSet: public AutoRegister {
  public:
    AutoRegisterSet(const char* name):
      AutoRegister(name, "")
    {
      TestRunner::root().insert(*this);
    }
  };

  class Test: public AutoRegisterTest {
  public:
    TestFunction  Fn;
    bool          ShouldFail;

    Test(const char* name, const char* source, TestFunction fn, bool shouldFail = false):
      AutoRegisterTest(name, source), Fn(fn), ShouldFail(shouldFail) {}

    virtual ~Test() {}

    virtual TestCase* Construct() {
      return new BoundTest(TestName, SourceFile, Fn, ShouldFail);
    }
  };

  template<class FixtureT> class AutoRegisterFixture: public AutoRegisterTest {
  public:
    typedef void (*FixtureTestFunction)(FixtureT&);
    typedef FixtureT* (*FixtureCtorFunction)(void);

    FixtureTestFunction Fn;
    FixtureCtorFunction Ctor;

    AutoRegisterFixture(const char* name, const char* source, FixtureTestFunction fn, FixtureCtorFunction ctor):
      AutoRegisterTest(name, source), Fn(fn), Ctor(ctor) {}

    virtual ~AutoRegisterFixture() {}

    virtual TestCase* Construct() {
      return new FixtureTest<FixtureT>(TestName, SourceFile, Fn, Ctor);
    }
  };

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


#define SCOPE_TEST_AUTO_REGISTRATION(testname, shouldFail) \
  namespace scope { namespace user_defined { namespace { namespace SCOPE_CAT(testname, ns) { \
    Test reg(#testname, __FILE__, testname, shouldFail); \
  } } } }

#define SCOPE_TEST(testname) \
  void testname(void);      \
  SCOPE_TEST_AUTO_REGISTRATION(testname, false) \
  void testname(void)

#define SCOPE_TEST_FAILS(testname) \
  void testname(void);            \
  SCOPE_TEST_AUTO_REGISTRATION(testname, true) \
  void testname(void)

// no need for auto-register if the test is i
#define SCOPE_TEST_IGNORE(testname) \
  void testname(void)

#define SCOPE_FIXTURE_AUTO_REGISTRATION(fixtureType, testfunction, ctorfunction) \
  namespace scope { namespace user_defined { namespace { namespace SCOPE_CAT(testfunction, ns) { \
    AutoRegisterFixture<fixtureType> reg(#testfunction, __FILE__, testfunction, ctorfunction); \
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
  scope::evalCondition<exceptiontype>((condition) ? true: false, __FILE__, __LINE__, #condition)

#define SCOPE_ASSERT(condition) \
  SCOPE_ASSERT_THROW(condition, scope::TestFailure)

#define SCOPE_ASSERT_EQUAL(...) \
  scope::evalEqual<scope::TestFailure>(__FILE__, __LINE__, __VA_ARGS__)

#define SCOPE_ASSERT_EQUAL_MSG(...) \
  scope::evalEqual<scope::TestFailure>(__FILE__, __LINE__, __VA_ARGS__)

#define SCOPE_EXPECT(statement, exception) \
  try { \
    statement; \
    throw scope::TestFailure(__FILE__, __LINE__, "Expected exception not caught"); \
  } \
  catch (const exception&) { \
    ; \
  }
