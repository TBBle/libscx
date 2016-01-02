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

#include <cstddef>
using std::size_t;
#include <cstdint>
using std::uint8_t;

#include "buffutils.hpp"

size_t Scene::read_data(gsl::czstring<> cp932text,
                        gsl::span<uint8_t, 0xd8> data) {
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

  chapter = buffutils::get<uint16_t>(data, 0);
  scene = buffutils::get<uint16_t>(data, 2);
  command = buffutils::get<uint16_t>(data, 4);
  unk1 = buffutils::get<uint16_t>(data, 6);
  unk2 = buffutils::get<uint16_t>(data, 8);
  chapterJump = buffutils::get<uint16_t>(data, 10);
  sceneJump1 = buffutils::get<uint16_t>(data, 12);
  sceneJump2 = buffutils::get<uint16_t>(data, 14);
  sceneJump3 = buffutils::get<uint16_t>(data, 16);
  sceneJump4 = buffutils::get<uint16_t>(data, 18);
  buffutils::copy(data, 20, sceneJumpInfo1);
  buffutils::copy(data, 68, sceneJumpInfo2);
  buffutils::copy(data, 116, sceneJumpInfo3);
  buffutils::copy(data, 164, sceneJumpInfo3);
  unk3 = buffutils::get<uint16_t>(data, 212);

  return cp932text ? cp932data.size() + 1 : 0;
}
