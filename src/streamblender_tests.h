#ifndef STREAMBLENDER_TESTS_H_
#define STREAMBLENDER_TESTS_H_

#include <gtest/gtest.h>

#include "streamblender.h"

#include "context.h"
#include "facility_tests.h"
#include "agent_tests.h"

namespace streamblender {
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class StreamBlenderTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  StreamBlender* src_facility_;

  virtual void SetUp();
  virtual void TearDown();
  void InitParameters();
  void SetUpStreamBlender();
  void TestInitState(streamblender::StreamBlender* fac);
  void TestRequest(streamblender::StreamBlender* fac, double cap);
  void TestAddMat(streamblender::StreamBlender* fac, 
      cyclus::Material::Ptr mat);
  void TestBuffers(streamblender::StreamBlender* fac, double inv, double 
      proc, double rawbuff);

  std::string out_c1, out_r1, in_c1, in_c2, in_c3;
  int iso_1, iso_2, iso_3;
  std::string src_11, src_12, src_21, src_22, src_31;
  std::vector< std::string > ins, srcs;

  int process_time;
  double capacity, max_inv_size, cost;
};
} // namespace streamblender
#endif // STREAMBLENDER_TESTS_H_

