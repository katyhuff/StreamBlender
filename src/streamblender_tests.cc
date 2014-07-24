#include <gtest/gtest.h>

#include "streamblender.h"

#include "context.h"
#include "facility_tests.h"
#include "agent_tests.h"

using streamblender::StreamBlender;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class StreamBlenderTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  StreamBlender* src_facility_;

  virtual void SetUp() {
    src_facility_ = new StreamBlender(tc_.get());
  }

  virtual void TearDown() {}
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamBlenderTest, clone) {
  StreamBlender* cloned_fac =
      dynamic_cast<StreamBlender*> (src_facility_->Clone());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamBlenderTest, InitialState) {
  // Test things about the initial state of the facility here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamBlenderTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility_->str());
  // Test StreamBlender specific aspects of the print method here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamBlenderTest, Tick) {
  ASSERT_NO_THROW(src_facility_->Tick());
  // Test StreamBlender specific behaviors of the Tick function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamBlenderTest, Tock) {
  EXPECT_NO_THROW(src_facility_->Tock());
  // Test StreamBlender specific behaviors of the Tock function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* StreamBlenderConstructor(cyclus::Context* ctx) {
  return new StreamBlender(ctx);
}

// required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif // CYCLUS_AGENT_TESTS_CONNECTED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(StreamBlenderFac, FacilityTests,
                        ::testing::Values(&StreamBlenderConstructor));

INSTANTIATE_TEST_CASE_P(StreamBlenderFac, AgentTests,
                        ::testing::Values(&StreamBlenderConstructor));
