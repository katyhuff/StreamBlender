#include <gtest/gtest.h>

#include "streamblender_region.h"

#include "region_tests.h"
#include "agent_tests.h"

using streamblender::StreamblenderRegion;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class StreamblenderRegionTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  StreamblenderRegion* src_region_;

  virtual void SetUp() {
    src_region_ = new StreamblenderRegion(tc_.get());
  }

  virtual void TearDown() {}
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamblenderRegionTest, clone) {
  StreamblenderRegion* cloned_fac =
      dynamic_cast<StreamblenderRegion*> (src_region_->Clone());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamblenderRegionTest, InitialState) {
  // Test things about the initial state of the region here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamblenderRegionTest, Print) {
  EXPECT_NO_THROW(std::string s = src_region_->str());
  // Test StreamblenderRegion specific aspects of the print method here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamblenderRegionTest, Tick) {
  EXPECT_NO_THROW(src_region_->Tick());
  // Test StreamblenderRegion specific behaviors of the handleTick function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamblenderRegionTest, Tock) {
  EXPECT_NO_THROW(src_region_->Tock());
  // Test StreamblenderRegion specific behaviors of the handleTock function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* StreamblenderRegionConstructor(cyclus::Context* ctx) {
  return new StreamblenderRegion(ctx);
}

// required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif // CYCLUS_AGENT_TESTS_CONNECTED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(StreamblenderRegion, RegionTests,
                        ::testing::Values(&StreamblenderRegionConstructor));
INSTANTIATE_TEST_CASE_P(StreamblenderRegion, AgentTests,
                        ::testing::Values(&StreamblenderRegionConstructor));
