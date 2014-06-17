#include <cxxtest/TestSuite.h>
#include <cxxtest/GlobalFixture.h>
#include "RAT/Log.hh"

using namespace std;
using namespace RAT;

class Fixture1 : public CxxTest::GlobalFixture
{
public:
  bool setUpWorld() { 
    Log::Init("testrat.log", Log::DETAIL, Log::DEBUG);
    return true;
  };
};

// Ensure that Log isn't trapping output from other tests
static Fixture1 init_fixture;

class TestLog : public CxxTest::TestSuite
{
public:
  void testNormal() {
    Log::Init("test.log", Log::INFO, Log::DETAIL);

    warn << "You should see this warning" << newline;
    info << "You should see this info line" << newline;
    detail << "This detail line should only be in the log file" << newline;
    debug << "This should not be visible anywhere" << newline;
  };

  void testDebug() {
    Log::Init("testdebug.log", Log::INFO, Log::DEBUG);

    warn << "You should see this warning" << newline;
    info << "You should see this info line" << newline;
    detail << "This detail line should only be in the log file" << newline;
    debug << "This debug line should also be in the log file" << newline;
  };

  // Leave this test commented out since it terminates the program
//   void testDie() {
//     Log::Init("testdie.log", Log::INFO, Log::DETAIL);

//     Log::Die("terminating immediately");
//     warn << "You should never see this." << newline;
//   };

};
   
 
