© 2009, Jon Stewart

Scope is a simple, lightweight unit testing framework for C++.

It should be easy to create tests, so it's important for Scope to scale down
when required. C++ is also a notoriously arcane language, with lots of gotchas.
Scope should avoid most of them, and give you a heads up about the ones you
could run into when using it. It should also adhere to as many of the C++ idioms
as possible, rather than copying idioms from other languages which aren't a good
fit in C++.

In Scope, tests are free functions. Macros are used to auto-register them, and
there's no need to register sets of tests in some other source file. When setup
and teardown functionality is needed, macros allow the test writer to specify
the type of an object, which is then created immediately before the test is run,
passed to it as a parameter, and destroyed immediately after the test runs, i.e.
Scope uses constructors and destructors instead of separate setup() and 
teardown() functions.

Scope's autoregistration system does use a bunch of static variables, as well as
a static singleton to collect them, but it avoids many of the pitfalls inherit
with singletons and static allocation in C++. In particular, it does not depend
on the order of construction (or guarantees it using a Meyers singleton) and it
does not use heap-allocated memory (i.e. no calls to new or malloc() before
main()) in accordance with the standard. Among other things, this makes leak-
detectors much easier to use, since Scope shouldn't generate any noise.

Tests are organized into sets, but sets can be relational, not just 
hierarchical--Scope uses a graph to represent the connections between sets and
tests. One set per source file is automatically created, and each test is put
into its corresponding source set automatically, giving you a hierarchical 
structure out of the box. However, it's possible to specify that a test should
belong to other sets.

Scope needs more work with respect to command-line features, friendly output,
and performance profiling.

Scope is released under the Boost license. See License.txt for details.
