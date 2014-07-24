#include <gtest/gtest.h>

#include "streamblender_facility.h"

#include "context.h"
#include "facility_tests.h"
#include "agent_tests.h"

using streamblender::StreamblenderFacility;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class StreamblenderFacilityTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  StreamblenderFacility* src_facility_;

  virtual void SetUp() {
    src_facility_ = new StreamblenderFacility(tc_.get());
  }

  virtual void TearDown() {}
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamblenderFacilityTest, clone) {
  StreamblenderFacility* cloned_fac =
      dynamic_cast<StreamblenderFacility*> (src_facility_->Clone());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamblenderFacilityTest, InitialState) {
  // Test things about the initial state of the facility here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamblenderFacilityTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility_->str());
  // Test StreamblenderFacility specific aspects of the print method here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamblenderFacilityTest, Tick) {
  ASSERT_NO_THROW(src_facility_->Tick());
  // Test StreamblenderFacility specific behaviors of the Tick function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamblenderFacilityTest, Tock) {
  EXPECT_NO_THROW(src_facility_->Tock());
  // Test StreamblenderFacility specific behaviors of the Tock function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* StreamblenderFacilityConstructor(cyclus::Context* ctx) {
  return new StreamblenderFacility(ctx);
}

// required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif // CYCLUS_AGENT_TESTS_CONNECTED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(StreamblenderFac, FacilityTests,
                        ::testing::Values(&StreamblenderFacilityConstructor));

INSTANTIATE_TEST_CASE_P(StreamblenderFac, AgentTests,
                        ::testing::Values(&StreamblenderFacilityConstructor));
