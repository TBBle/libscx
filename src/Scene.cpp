#include "Scene.hpp"

#include <boost/locale.hpp>
using boost::locale::conv::to_utf;

#include <array>
using std::array;
#include <string>
using std::string;
#include <utility>
using std::pair;
using std::make_pair;

#include <cstring>
using std::memcpy;
#include <cstddef>
using std::size_t;
#include <cstdint>
using std::uint8_t;

namespace {

template <typename T>
void buffcopy(T& output, const uint8_t* pBuffer, size_t offset) {
  memcpy(&output, pBuffer + offset, sizeof(T));
}
}

pair<size_t, size_t> Scene::read_data(const uint8_t* pStringData,
                                      const uint8_t* pBlobData) {
  // String is null-terminated and encoded in CP932

  // Differences compared to Python's CP932 support:
  // Python's CP932 cannot encode ⅰ 'SMALL ROMAN NUMERAL ONE' (U+2170) from
  // UTF-8, but was able to encode it _to_ UTF-8
  // boost::locale came up with 〜 'WAVE DASH' (U+301C) where Python output ～
  // 'FULLWIDTH TILDE' (U+FF5E)

  string cp932data;
  if (pStringData != 0) {
    while (*pStringData != '\0') {
      cp932data.push_back(*pStringData++);
    }
    text = to_utf<char>(cp932data, "CP932");
  }

  buffcopy(chapter, pBlobData, 0);
  buffcopy(scene, pBlobData, 2);
  buffcopy(command, pBlobData, 4);
  buffcopy(unk1, pBlobData, 6);
  buffcopy(unk2, pBlobData, 8);
  buffcopy(chapterJump, pBlobData, 10);
  buffcopy(sceneJump1, pBlobData, 12);
  buffcopy(sceneJump2, pBlobData, 14);
  buffcopy(sceneJump3, pBlobData, 16);
  buffcopy(sceneJump4, pBlobData, 18);
  memcpy(sceneJumpInfo1.data(), pBlobData + 20, 48);
  memcpy(sceneJumpInfo2.data(), pBlobData + 68, 48);
  memcpy(sceneJumpInfo3.data(), pBlobData + 116, 48);
  memcpy(sceneJumpInfo4.data(), pBlobData + 164, 48);
  buffcopy(unk3, pBlobData, 212);

  return make_pair(pStringData ? cp932data.size() + 1 : 0, 216);
}
