#include "scope/test.h"

namespace {
  // Execute the callable and 
  // if it throws the test fails
  // else the test passes
  scope::FreeTest traditionalFreeTest([]{
    SCOPE_ASSERT_EQUAL(2, 1 + 1);
  });

  // Execute the callable and use its return
  // value to denote success/failure.
  // You won't get as nice of an error message
  // without the macro, but you also don't have
  // the macro, i.e., things are a bit more
  // readable. So you could choose this style 
  // of test if the situation demanded it.
  // (i.e., both styles would be supported)
  // scope::Test boolReturnValueTest([]{
  //   return 2 == 1 + 1;
  // });

  // If the callable returns something else that's
  // callable with either a void or a bool return type,
  // then execute the intermediate callable and 
  // treat it as a test like above. This is a nice way
  // to provide the traditional setup()/teardown()
  // test framework feature, since the first callable 
  // can do the setup (and automatic destructors
  // can handle the teardown).
  //
  // The closure should generally copy values instead of reference
  // them, because any temporaries will have been destroyed by the 
  // time the returned lambda is executed.
  // scope::Test fixtureTest([]{
  //   shared_ptr<Foo> foo(make_shared<Foo>("some test text"));
  //   return [foo]{
  //     SCOPE_ASSERT(foo.bar()); // or "return foo.bar();"
  //   };
  // });

  // If the callable returns a sequence and a callable,
  // then apply the callable to every item in the sequence
  // and treat that as a test.
  //
  // It's very similar to std::all_of(), with the exception
  // that there will be no early-exit (every element will
  // be evaluated) and every element that fails will be 
  // reported, so you know which things failed the test.
  // 
  // I'm less certain about the use of std::pair here, but 
  // it doesn't seem like a horrible way to go. The type of
  // the first element in the pair would either have to be
  // an initializer list or something that had begin() and 
  // end() methods that returned Forward iterators.
  //
  // From a counting perspective, I think that Scope would
  // have to count every item in the sequence as an individual
  // test, but not count the initial closing lambda.
  // scope::Test sequenceTests([]{
  //   return make_pair({1, 2, 3, 4, 5},
  //                   [](int x){ return x == 2; });
  // });
}
