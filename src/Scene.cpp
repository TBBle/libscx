#include "Scene.hpp"

#include <boost/locale.hpp>
using boost::locale::conv::to_utf;

#include <algorithm>
using std::copy;

#include <string>
using std::string;

#include <cstdint>
using std::uint8_t;

void Scene::read_data(gsl::czstring<> cp932text, blob_span data) {
  // String is null-terminated and encoded in Windows code-page 932, which
  // is known as "Shift_JIS" only within the MS API, and as "CP932" everywhere
  // except non-Windows ICU. msys2's mingw64 build of boost appears to be
  // using non-Windows ICU as its backend, so we can't use that name.
  // After much experimentation, it appears both the WindowsAPI-backed
  // boost::locale::conv in Visual Studio, and the ICU-backed version in
  // msys2's mingw64, agree on "windows-932".
  // http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/CP932.TXT
  // https://msdn.microsoft.com/en-us/goglobal/cc305152.aspx
  // http://demo.icu-project.org/icu-bin/convexp?conv=ibm-943_P15A-2003&s=ALL
  // There's only four characters which give different results between the
  // two code-pages per
  // http://hp.vector.co.jp/authors/VA003720/lpproj/test/cp932sj.htm
  // Shift-JIS => CP932 vs SHIFT-JIS
  // 0x815c => U+2015 HORIZONTAL BAR (―) vs U+2015 EM DASH (—)
  // 0x8160 => U+FF5E FULLWIDTH TILDE (～) vs U+301C WAVE DASH (〜)
  // 0x8161 => U+2225 PARALLEL TO (∥) vs U+2016 DOUBLE VERTICAL LINE (‖)
  // 0x817c => U+FF0D FULLWIDTH HYPHEN-MINUS (－) vs U+2212 MINUS SIGN (−)
  // 0x8160 shows up in the unit tests, which will catch platform issues.
  // Note that some characters (including) U+2170 SMALL ROMAN NUMERAL ONE (ⅰ)
  // have multiple representations in CP932, so the following non-round trips
  // may occur: https://support.microsoft.com/en-us/kb/170559 according to
  // http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WindowsBestFit/bestfit932.txt

  string cp932data;
  if (cp932text != nullptr) {
    while (*cp932text != '\0') {
      cp932data.push_back(*cp932text++);
    }
    text = to_utf<char>(cp932data, "windows-932");
  }

  // 10 x uint16_t, 8 known and two mystery
  auto known =
      gsl::as_span<const uint16_t>(data.first<sizeof(uint16_t) * 10>());
  auto buffer = data.subspan(known.size_bytes());

  chapter = known[0];
  scene = known[1];
  command = known[2];
  unk1 = known[3];
  unk2 = known[4];
  chapterJump = known[5];
  sceneJump1 = known[6];
  sceneJump2 = known[7];
  sceneJump3 = known[8];
  sceneJump4 = known[9];

  // 4 x 0x30 blobs of mystery
  using scene_jumps_span = gsl::span<const gsl::byte, 4, scene_jump_blob_size>;
  scene_jumps_span scene_jumps =
      gsl::as_span(buffer.first<4 * scene_jump_blob_size>(), gsl::dim<4>(),
                   gsl::dim<scene_jump_blob_size>());
  buffer = buffer.subspan(scene_jumps.size_bytes());

  auto scene_jumps1 = scene_jumps[0];
  copy(scene_jumps1.begin(), scene_jumps1.end(), sceneJumpInfo1.begin());

  auto scene_jumps2 = scene_jumps[1];
  copy(scene_jumps2.begin(), scene_jumps2.end(), sceneJumpInfo2.begin());

  auto scene_jumps3 = scene_jumps[2];
  copy(scene_jumps3.begin(), scene_jumps3.end(), sceneJumpInfo3.begin());

  auto scene_jumps4 = scene_jumps[3];
  copy(scene_jumps4.begin(), scene_jumps4.end(), sceneJumpInfo4.begin());

  // A trailing uint16_t, also of mystery
  auto unknown = gsl::as_span<const uint16_t>(buffer);
  unk3 = unknown[0];
}
