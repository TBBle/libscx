#pragma once

#include <array>

#include <cstddef>
using std::ptrdiff_t;
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
// Copy a range of bytes out of a large buffer as a POD type.
//  We rely on RVO to avoid an extra copy.

//  - A POD type with a static offset member lives at a fixed
//	location in the buffer.
template <typename T>
T get(dynbuffer buffer) {
  assert(T::offset + sizeof(T) <= buffer.size());
  T result;
  // TODO check result
  memcpy(&result, buffer.data() + T::offset, sizeof(T));
  return result;
}

//  - A POD type that lives at a dynamic location in the buffer,
//    or a built-in type we can't add the static offset member to.
template <typename T>
T get(dynbuffer buffer, ptrdiff_t offset) {
  assert(offset + sizeof(T) <= buffer.size());
  T result;
  // TODO check result
  memcpy(&result, buffer.data() + offset, sizeof(T));
  return result;
}

// Copy a range of bytes out of a large buffer into a container
//  - A std::array of PODs that lives at a dynamic location in the buffer,
//    or a fixed length buffer of unrecognised data.
template <typename T, size_t N>
void copy(dynbuffer buffer, ptrdiff_t offset, std::array<T, N>& output) {
  assert(offset + N * sizeof(T) <= buffer.size());
  // TODO check result
  memcpy(output.data(), buffer.data() + offset, N);
}

// TODO: Maybe?
// Extract a UTF-8 encoding string from the Windows-932 encoding
// zstring at the given offset
}