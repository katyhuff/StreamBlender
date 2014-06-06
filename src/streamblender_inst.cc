#include "streamblender_inst.h"

using streamblender::StreamblenderInst;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
StreamblenderInst::StreamblenderInst(cyclus::Context* ctx)
    : cyclus::Institution(ctx) {};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
StreamblenderInst::~StreamblenderInst() {}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string StreamblenderInst::str() {
  return Institution::str();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructStreamblenderInst(cyclus::Context* ctx) {
  return new StreamblenderInst(ctx);
}
