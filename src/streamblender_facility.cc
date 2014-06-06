#include "streamblender_facility.h"

namespace streamblender {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
StreamblenderFacility::StreamblenderFacility(cyclus::Context* ctx)
    : cyclus::Facility(ctx) {};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string StreamblenderFacility::str() {
  return Facility::str();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StreamblenderFacility::Tick() {}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StreamblenderFacility::Tock() {}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructStreamblenderFacility(cyclus::Context* ctx) {
  return new StreamblenderFacility(ctx);
}

}
