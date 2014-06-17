#include <cxxtest/TestSuite.h>
#include "RAT/DB.hh"
#include "RAT/DBMessenger.hh"
#include <G4UImanager.hh>

using namespace std;
using namespace RAT;

class TestDBManager : public CxxTest::TestSuite
{
public:
  DB *db;
  DBMessenger *dbm;
  G4UImanager *ui;
  DBLinkPtr ltest1;

  void setUp() {
    db = new DB();
    dbm = new DBMessenger(db);
    ltest1 = db->GetLink("TEST", "foo");
    ui = G4UImanager::GetUIpointer();
  };

  void tearDown() {
    delete dbm;
    delete db;
  };

  void testLoad() {
    ui->ApplyCommand("/rat/db/load test2.ratdb");
    TS_ASSERT_EQUALS(ltest1->GetI("myint"), 17);
  };

  void testSet() {
    ui->ApplyCommand("/rat/db/set TEST[foo] myint 16");
    TS_ASSERT_EQUALS(ltest1->GetI("myint"), 16);
    ui->ApplyCommand("/rat/db/set TEST[foo] mydouble 16.5d-3");
    TS_ASSERT_DELTA(ltest1->GetD("mydouble"), 16.5e-3, 1e-10);
    ui->ApplyCommand("/rat/db/set TEST[foo] mystring 'ratratrat'");
    TS_ASSERT_EQUALS(ltest1->GetS("mystring"), "ratratrat");
  }

  void testSetAfterLoad() {
    testLoad();
    testSet();
  }

};
   
 
