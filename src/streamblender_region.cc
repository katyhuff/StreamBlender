#include "streamblender_region.h"

using streamblender::StreamblenderRegion;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
StreamblenderRegion::StreamblenderRegion(cyclus::Context* ctx)
    : cyclus::Region(ctx) {};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
StreamblenderRegion::~StreamblenderRegion() {}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string StreamblenderRegion::str() {
  return Region::str();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructStreamblenderRegion(cyclus::Context* ctx) {
  return new StreamblenderRegion(ctx);
}
