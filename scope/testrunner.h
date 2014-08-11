/*
	© 2009, Jon Stewart
	Released under the terms of the Boost license (http://www.boost.org/LICENSE_1_0.txt). See License.txt for details.
*/

#pragma once

#include <cassert>
#include <csignal>
#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/smart_ptr.hpp>

#include "test.h"

namespace scope {

  void RunFunction(scope::TestFunction test, const char* testname, MessageList& messages) {
    try {
      test();
    }
    catch (const test_failure& fail) {
      std::stringstream buf;
      buf << fail.File << ":" << fail.Line << ": " << testname << ": " << fail.what();
      messages.push_back(buf.str());
    }
    catch (const std::exception& except) {
      messages.push_back(std::string(testname) + ": " + except.what());
    }
    catch (...) {
      CaughtBadExceptionType(testname, "test threw unrecognized type");
      throw;
    }
  }

  void CaughtBadExceptionType(const std::string& name, const std::string& msg) {
    std::cerr << name << ": " << msg << "; please at least inherit from std::exception" << std::endl;
  }

  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, boost::property<boost::vertex_color_t, boost::default_color_type>> TestGraph;
  typedef boost::graph_traits<TestGraph>::vertex_descriptor Vertex;
  typedef boost::graph_traits<TestGraph>::vertex_iterator VertexIter;
  typedef boost::property_map<TestGraph, boost::vertex_color_t>::type Color;
  typedef boost::property_map<TestGraph, boost::vertex_index_t>::type IndexMap;
  typedef boost::shared_ptr<Test> TestPtr;
  typedef std::vector<TestPtr> TestMap;

  class TestVisitor : public boost::default_dfs_visitor {
  public:
  	TestVisitor(TestRunner& runner, TestMap& tests, MessageList& messages): Runner(runner), Tests(tests), Messages(messages) {}

  	template <typename Vertex, typename Graph> void discover_vertex(Vertex v, const Graph& g) const {
  	  using namespace boost;
  	  //std::cerr << "Running " << Tests[get(vertex_index, g)[v]]->Name << "\n";
      TestPtr t(Tests[get(vertex_index, g)[v]]);
      Runner.runTest(t.get(), Messages);
  	}

  private:
    TestRunner& Runner;
  	TestMap& Tests;
  	MessageList& Messages;
  };

  namespace {
    class TestRunnerImpl : public TestRunner {
    public:
      TestRunnerImpl():
        FirstTest(0), FirstEdge(0), NumTests(0), NumRun(0), Debug(false) {}

      virtual void runTest(const Test* const test, MessageList& messages) {
        if (dynamic_cast<const SetTest*>(test) == 0 && (NameFilter.empty() || NameFilter == test->Name)) {
          LastTest = test->Name;
          if (Debug) {
            std::cerr << "Running " << test->Name << std::endl;
          }
          ++NumRun;
    	    test->Run(messages);
    	    if (Debug) {
            std::cerr << "Done with " << test->Name << std::endl;
    	    }
    	  }
      }

      virtual void run(MessageList& messages, const std::string& nameFilter) {
      	using namespace boost;
        NameFilter = nameFilter;
        for (AutoRegister* cur = FirstTest; cur != 0; cur = cur->Next) {
          Add(TestPtr(cur->Construct()));
        }
        for (CreateEdge* cur = FirstEdge; cur; cur = cur->Next) {
          CreateLink(cur->From, cur->To);
        }
      	TestVisitor vis(*this, Tests, messages);
      	depth_first_search(Graph, vis, get(vertex_color, Graph));
        /*	for (std::pair<VertexIter, VertexIter> vipair(vertices(Graph)); vipair.first != vipair.second; ++vipair.first) {
    	  Tests[get(vertex_index, Graph)[*vipair.first]]->Run(messages);
    	  }*/
      }

      virtual size_t Add(TestPtr test) {
      	using namespace boost;
      	Vertex v(add_vertex(Graph));
      	size_t ret = get(vertex_index, Graph)[v];
      	assert(ret == Tests.size());
      	Tests.push_back(test);
      	if (!dynamic_pointer_cast<SetTest>(test)) {
          ++NumTests;
      	}
      	return ret;
      }

      virtual void CreateLink(const AutoRegister& from, const AutoRegister& to) {
      	FastCreateLink(from, to); // we should check for cycles. maybe?
      }

      virtual void FastCreateLink(const AutoRegister& from, const AutoRegister& to) {
      	boost::add_edge(from.Index, to.Index, Graph);
      }

      virtual void addTest(AutoRegister& test) {
        if (0 == FirstTest) {
          FirstTest = &test;
        }
        else {
          FirstTest->insert(test);
        }
      }

      virtual void addLink(CreateEdge&) {
        
      }

      virtual unsigned int numTests() const {
        return NumTests;
      }

      virtual unsigned int numRun() const {
        return NumRun;
      }

      virtual void setDebug(bool val) {
        Debug = val;
      }

      virtual std::string lastTest() const {
        return LastTest;
      }

    private:
      TestGraph Graph;
      TestMap   Tests;
      AutoRegister* FirstTest;
      CreateEdge*   FirstEdge;
      std::string   NameFilter,
                    LastTest;
      unsigned int  NumTests,
                    NumRun;
      bool          Debug;
    };
  }

  TestRunner& TestRunner::Get(void) {
    static TestRunnerImpl singleton;
    return singleton;
  }

  std::map<int, std::string> signalMap() {
    return std::map<int, std::string> {
      {SIGFPE, "floating point exception (SIGFPE)"},
      {SIGSEGV, "segmentation fault (SIGSEGV)"},
      {SIGTERM, "termination request (SIGTERM)"},
      {SIGINT, "interrupt request (SIGINT)"}
    };
  }

  void handleSignal(int signum) {
    auto friendlySig = signalMap()[signum];
    std::cerr << "Received signal " << signum << ", " << friendlySig
      << ". Last test was " << TestRunner::Get().lastTest() << ". Terminating." << std::endl;
    std::terminate();
  }

  template<typename HandlerT>
  void setHandlers(HandlerT handler) {
    auto sigmap = signalMap();
    for (auto sig: sigmap) {
      std::signal(sig.first, handler);
    }
  }

  bool DefaultRun(std::ostream& out, int argc, char** argv) {
    MessageList msgs;
    TestRunner &runner(TestRunner::Get());
    std::string debug("--debug");
    if ((argc == 3 && debug == argv[2]) || (argc == 4 && debug == argv[3])) {
      runner.setDebug(true);
      out << "Running in debug mode" << std::endl;
    }
    std::string nameFilter(argc > 2 && debug != argv[2] ? argv[2]: "");

    setHandlers(handleSignal);
    runner.run(msgs, nameFilter);
    setHandlers(SIG_DFL);

    for(MessageList::const_iterator it(msgs.begin()); it != msgs.end(); ++it) {
      out << *it << '\n';
    }
    if (msgs.begin() == msgs.end()) {
      out << "OK (" << runner.numRun() << " tests)" << std::endl;
      return true;
    }
    else {
      out << "Failures!" << std::endl;
      out << "Tests run: " << runner.numRun() << ", Failures: " << msgs.size() << std::endl;
      return false;
    }
  }
}
