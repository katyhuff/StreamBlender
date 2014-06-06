#include <gtest/gtest.h>

#include <string>

#include "streamblender_inst.h"

#include "institution_tests.h"
#include "agent_tests.h"

using streamblender::StreamblenderInst;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class StreamblenderInstTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  StreamblenderInst* src_inst_;

  virtual void SetUp() {
    src_inst_ = new StreamblenderInst(tc_.get());
  }

  virtual void TearDown() {}
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamblenderInstTest, clone) {
  StreamblenderInst* cloned_fac =
      dynamic_cast<StreamblenderInst*> (src_inst_->Clone());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamblenderInstTest, InitialState) {
  // Test things about the initial state of the inst here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamblenderInstTest, Print) {
  EXPECT_NO_THROW(std::string s = src_inst_->str());
  // Test StreamblenderInst specific aspects of the print method here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamblenderInstTest, Tick) {
  int time = 1;
  EXPECT_NO_THROW(src_inst_->Tick());
  // Test StreamblenderInst specific behaviors of the handleTick function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamblenderInstTest, Tock) {
  int time = 1;
  EXPECT_NO_THROW(src_inst_->Tick());
  // Test StreamblenderInst specific behaviors of the handleTock function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* StreamblenderInstitutionConstructor(cyclus::Context* ctx) {
  return new StreamblenderInst(ctx);
}

// required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif // CYCLUS_AGENT_TESTS_CONNECTED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(StreamblenderInst, InstitutionTests,
                        ::testing::Values(&StreamblenderInstitutionConstructor));
INSTANTIATE_TEST_CASE_P(StreamblenderInst, AgentTests,
                        ::testing::Values(&StreamblenderInstitutionConstructor));
