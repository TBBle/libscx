#pragma once

#include <cstring>
using std::memcpy;
#include <cstdint>
using std::uint8_t;

#include <gsl.h>

namespace buffutils {

using dynbuffer = gsl::span<uint8_t>;

// Some templates to make it easier and safer to deal with
// moving raw data in and out of big buffers.
// Somewhat inspired by boost::asio::buffer

// Interesting types:
// Raw data is held in a span<uint8_t>
//  - At some point, this might become a boost::asio::buffer

// Interesting operations:
// Copy a range of bytes out of a large buffer as a POD type
//  - A POD type with a static offset member lives at a fixed
//	location in the buffer. We rely on RVO to avoid an extra copy.
template <typename T>
T get(dynbuffer buffer) {
  assert(T::offset + T::size <= buffer.size());
  T result;
  // TODO check result
  memcpy(&result, buffer.data() + T::offset, T::size);
  return result;
}

// TODO: Maybe?
// Extract a UTF-8 encoding string from the Windows-932 encoding
// zstring at the given offset
}