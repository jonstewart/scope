/*
  © 2009–2016, Jon Stewart
  Released under the terms of the Boost license (http://www.boost.org/LICENSE_1_0.txt). See License.txt for details.
*/

#pragma once

#include <cassert>
#include <csignal>
#include <iostream>
#include <memory>
#include <map>
#include <regex>
#include <thread>
#include <mutex>

#include "tclap/CmdLine.h"


#include "test.h"

namespace scope {

  void runFunction(scope::TestFunction test, const char* testname, bool shouldFail, MessageList& messages) {
    try {
      test();
      if (shouldFail) {
        std::string msg(testname);
        msg.append(": marked for failure but did not throw scope::TestFailure.");
        messages.push_back(msg);
      }
    }
    catch (const TestFailure& fail) {
      if (!shouldFail) {
        std::ostringstream buf;
        buf << fail.File << ":" << fail.Line << ": " << testname << ": " << fail.what();
        messages.push_back(buf.str());
      }
    }
    catch (const std::exception& except) {
      messages.push_back(std::string(testname) + ": " + except.what());
    }
    catch (...) {
      caughtBadExceptionType(testname, "test threw unrecognized type");
      throw;
    }
  }

  void caughtBadExceptionType(const std::string& name, const std::string& msg) {
    std::cerr << name << ": " << msg << "; please at least inherit from std::exception" << std::endl;
  }

  namespace {
    template<class X>
    bool always_true(const X&) {
      return true;
    }

    class TestRunnerImpl: public TestRunner {
    public:
      TestRunnerImpl():
        NumTests(0), NumRun(0), Debug(false)
      {
        traverse([this](AutoRegister*) {
          ++this->NumTests;
        });
      }

      virtual void runTest(const TestCase& test, MessageList& messages) {
        if (!(NameFilter || SourceFilter)
          || (NameFilter && std::regex_match(test.Name, *NameFilter))
          || (SourceFilter && std::regex_match(test.SourceFile, *SourceFilter)))
        {
          lastTest() = test.Name;
          if (Debug) {
            std::cerr << "Running " << test.Name << std::endl;
          }
          ++NumRun;
          test.Run(messages);
          if (Debug) {
            std::cerr << "Done with " << test.Name << std::endl;
          }
          lastTest().clear();
        }
      }

      virtual void run(MessageList& messages) {
        traverse([this, &messages](AutoRegister* cur) { 
          std::unique_ptr<TestCase> test(cur->Construct());
          this->runTest(*test, messages);        
        });
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

      virtual void setFilter(const std::shared_ptr<std::regex>& filter) {
        NameFilter = filter;
      }

      virtual void setSourceFilter(const std::shared_ptr<std::regex>& sourceFilter) {
        SourceFilter = sourceFilter;
      }

      template<class AutoRegFnType>
      void traverse(AutoRegFnType&& fn) {
        auto& r(root());
        for (auto cur(r.FirstChild); cur; cur = cur->Next) {
          fn(cur);
        }
      }

    private:
      std::shared_ptr<std::regex> NameFilter,
                                  SourceFilter;

      unsigned int  NumTests,
                    NumRun;
      bool          Debug;
    };
  }

  Node<AutoRegister>& TestRunner::root(void) {
    static Node<AutoRegister> root;
    return root;
  }

  std::string& TestRunner::lastTest(void) {
    static std::string last;
    return last;
  }

  void handleTerminate() {
    // the handler can be called on multiple threads
    // this is a legitimate use of a static mutex
    // weirdly
    static std::mutex theHighlander;
    std::lock_guard<std::mutex> lock(theHighlander);
    {
      std::cerr << "std::terminate called, last test was "
                << TestRunner::lastTest()
                << ". Aborting." << std::endl;
      std::abort();
    }
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
      << ". Last test was " << TestRunner::lastTest() << ". Aborting." << std::endl;
    std::abort();
  }

  template<typename HandlerT>
  void setHandlers(HandlerT handler) {
    auto sigmap = signalMap();
    for (auto sig: sigmap) {
      std::signal(sig.first, handler);
    }
  }

  bool DefaultRun(std::ostream& out, int argc, char** argv) {
    TCLAP::CmdLine parser("Scope test", ' ', "version number? what's a version number?", true);

    TCLAP::ValueArg<std::string> sourceFile("s", "source-filter", "Run tests from source files where the filenames match the provided regexp", false, "", "regexp", parser);
    TCLAP::ValueArg<std::string> filter("f", "filter", "Only run test cases whose names match provided regexp", false, "", "regexp", parser);

    TCLAP::SwitchArg verbose("v", "verbose", "Print debugging info", parser);
    TCLAP::SwitchArg list("l", "list", "List test names", parser);

    try {
      parser.parse(argc, argv);
    }
    catch (TCLAP::ArgException& e) {
      std::cerr << "Error: " << e.error() << " for argument " << e.argId() << std::endl;
      return false;
    }

    MessageList msgs;
    TestRunnerImpl runner;
    std::string f(filter.getValue());
    if (!f.empty()) {
      try {
        runner.setFilter(std::make_shared<std::regex>(f));
      }
      catch (std::regex_error& e) {
        std::cerr << "Error with filter regexp '" << f << "': " << e.what() << std::endl;
        return false;
      }
    }
    std::string s(sourceFile.getValue());
    if (!s.empty()) {
      try {
        runner.setSourceFilter(std::make_shared<std::regex>(s));
      }
      catch (std::regex_error& e) {
        std::cerr << "Error with filter regexp '" << s << "': " << e.what() << std::endl;
        return false;
      }
    }

    if (list.getValue()) {
      runner.traverse([](AutoRegister* cur) { std::cerr << cur->TestName << '\t' << cur->SourceFile << '\n'; });
      return true;
    }
    if (verbose.getValue()) {
      runner.setDebug(true);
      out << "Running in debug mode" << std::endl;
    }
    setHandlers(handleSignal);
    std::set_terminate(&handleTerminate);
    runner.run(msgs);
    std::set_terminate(0);
    setHandlers(SIG_DFL);

    for(const std::string& m : msgs) {
      out << m << '\n';
    }

    if (msgs.empty()) {
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
