#include <gtest/gtest.h>

#include "streamblender_tests.h"

namespace streamblender {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StreamBlenderTest::SetUp() {
  src_facility_ = new StreamBlender(tc_.get());
  InitParameters();
  SetUpStreamBlender();
}

void StreamBlenderTest::TearDown() {
  delete src_facility_;
}

void StreamBlenderTest::InitParameters(){
  in_c1 = "in_c1";
  in_c2 = "in_c2";
  in_c3 = "in_c3";
  out_c1 = "out_c1";
  ins.push_back(in_c1);
  ins.push_back(in_c2);
  ins.push_back(in_c3);
  iso_1 = 92235;
  iso_2 = 94240;
  iso_3 = 95241;
  src_11 = in_c1;
  src_12 = in_c2;
  src_21 = in_c1;
  src_22 = in_c2;
  src_31 = in_c3;
  srcs.push_back(src_11);
  srcs.push_back(src_12);
  srcs.push_back(src_21);
  srcs.push_back(src_22);
  srcs.push_back(src_31);
  process_time = 10;
  max_inv_size = 200;
  capacity = 20;
  cost = 1;

  cyclus::CompMap v;
  v[922350000] = 1;
  v[922380000] = 2;
  cyclus::Composition::Ptr recipe = cyclus::Composition::CreateFromAtom(v);
  tc_.get()->AddRecipe(src_11, recipe);

  cyclus::CompMap w;
  w[922350000] = 1;
  recipe = cyclus::Composition::CreateFromAtom(w);
  tc_.get()->AddRecipe(src_12, recipe);

  w[942400000] = 1;
  recipe = cyclus::Composition::CreateFromAtom(w);
  tc_.get()->AddRecipe(src_21, recipe);

  v[942400000] = 1;
  recipe = cyclus::Composition::CreateFromAtom(v);
  tc_.get()->AddRecipe(src_22, recipe);

  cyclus::CompMap x;
  x[952410000] = 0.25;
  recipe = cyclus::Composition::CreateFromAtom(x);
  tc_.get()->AddRecipe(src_31, recipe);

  cyclus::CompMap y;
  y[922350000] = 1;
  y[942400000] = 2;
  y[952410000] = 1;
  recipe = cyclus::Composition::CreateFromAtom(y);
  tc_.get()->AddRecipe(out_r1, recipe);
}



void StreamBlenderTest::SetUpStreamBlender(){
  src_facility_->in_commods_(ins);
  src_facility_->out_commod_(out_c1);
  src_facility_->in_recipes_(srcs);
  src_facility_->out_recipe_(out_r1);
  src_facility_->process_time_(process_time);
  src_facility_->max_inv_size_(max_inv_size);
  src_facility_->capacity_(capacity);
  src_facility_->cost_(cost);
  src_facility_->isos_(isos);
  src_facility_->sources_(srcs);
}

void StreamBlenderTest::TestInitState(StreamBlender* fac){
  EXPECT_EQ(ins, fac->in_commods_());
  EXPECT_EQ(out_c1, fac->out_commod_());
  EXPECT_EQ(in_recipes, fac->in_recipes_());
  EXPECT_EQ(out_c1, fac->out_recipe_());
  EXPECT_EQ(process_time, fac->process_time_());
  EXPECT_EQ(max_inv_size, fac->max_inv_size_());
  EXPECT_EQ(capacity, fac->capacity_());
  EXPECT_EQ(cost, fac->cost_());
  EXPECT_EQ(isos, fac->isos_());
  EXPECT_EQ(srcs, fac->sources_());

}

void StreamBlenderTest::TestRequest(StreamBlender* fac, double cap){
  //cyclus::Material::Ptr req = fac->Request_();
  //EXPECT_EQ(cap, req->quantity());
}

void StreamBlenderTest::TestAddMat(StreamBlender* fac, 
    cyclus::Material::Ptr mat){
  double amt = mat->quantity();
  double before = fac->blendbuff.quantity();
  //fac->AddMat_(mat);
  double after = fac->blendbuff.quantity();
  EXPECT_EQ(amt, after - before);
}

void StreamBlenderTest::TestBuffers(StreamBlender* fac, double inv, 
    double proc, double rawbuffs){
  double t = tc_.get()->time();

  EXPECT_EQ(inv, fac->blendbuff.quantity());
  EXPECT_EQ(rawbuffs, fac->rawbuffs_quantity());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamBlenderTest, clone) {
  StreamBlender* cloned_fac =
      dynamic_cast<StreamBlender*> (src_facility_->Clone());
  TestInitState(cloned_fac);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamBlenderTest, InitialState) {
  // Test things about the initial state of the facility here
  TestInitState(src_facility_);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamBlenderTest, CurrentCapacity) {
  EXPECT_EQ(capacity, src_facility_->current_capacity());
  src_facility_->max_inv_size_(1e299);
  EXPECT_EQ(1e299, src_facility_->max_inv_size_());
  EXPECT_EQ(capacity, src_facility_->capacity_());
  EXPECT_EQ(capacity, src_facility_->current_capacity());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamBlenderTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility_->str());
  // Test StreamBlender specific aspects of the print method here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamBlenderTest, Request) { 
  TestRequest(src_facility_, src_facility_->current_capacity());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamBlenderTest, AddMats) { 
  double cap = src_facility_->current_capacity();
  cyclus::Material::Ptr mat = cyclus::NewBlankMaterial(0.5*cap);
  TestAddMat(src_facility_, mat);

  // cyclus::Composition::Ptr rec = tc_.get()->GetRecipe(in_r1);
  // cyclus::Material::Ptr recmat = cyclus::Material::CreateUntracked(0.5*cap, rec);
  //TestAddMat(src_facility_, recmat);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamBlenderTest, Tick) {
  ASSERT_NO_THROW(src_facility_->Tick());
  // Test StreamBlender specific behaviors of the Tick function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamBlenderTest, Tock) {

  // initially, nothing in the buffers
  TestBuffers(src_facility_,0,0,0);

  double cap = src_facility_->current_capacity();
  //cyclus::Composition::Ptr rec = tc_.get()->GetRecipe(in_r1);
  //cyclus::Material::Ptr mat = cyclus::Material::CreateUntracked(cap, rec);
  //TestAddMat(src_facility_, mat);

  // affter add, the blendbuff has the material
  TestBuffers(src_facility_,cap,0,0);

  EXPECT_NO_THROW(src_facility_->Tock());

  // after tock, the processing buffer has the material
  TestBuffers(src_facility_,0,cap,0);

  EXPECT_EQ(0, tc_.get()->time());
  for( int i = 1; i < process_time-1; ++i){
    tc_.get()->time(i);
    EXPECT_NO_THROW(src_facility_->Tock());
    TestBuffers(src_facility_,0,0,0);
  }
  
  tc_.get()->time(process_time);
  EXPECT_EQ(process_time, tc_.get()->time());
  EXPECT_EQ(0, src_facility_->ready());
  src_facility_->Tock();
  TestBuffers(src_facility_,0,0,cap);

  tc_.get()->time(process_time+1);
  EXPECT_EQ(1, src_facility_->ready());
  src_facility_->Tock();
  TestBuffers(src_facility_,0,0,cap);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StreamBlenderTest, NoProcessTime) {
  // tests what happens when the process time is zero
  src_facility_->process_time_(0);
  EXPECT_EQ(0, src_facility_->process_time_());

  double cap = src_facility_->current_capacity();
  //cyclus::Composition::Ptr rec = tc_.get()->GetRecipe(in_r1);
  //cyclus::Material::Ptr mat = cyclus::Material::CreateUntracked(cap, rec);
  //TestAddMat(src_facility_, mat);

  // affter add, the blendbuff has the material
  TestBuffers(src_facility_,cap,0,0);

  EXPECT_NO_THROW(src_facility_->Tock());

  // affter tock, the rawbuffs have the material
  TestBuffers(src_facility_,0,0,cap);
}


} // namespace streamblender

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* StreamBlenderConstructor(cyclus::Context* ctx) {
  return new streamblender::StreamBlender(ctx);
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
