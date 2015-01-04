#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do
                           // this in one cpp file
#include "catch.hpp"

#include "libscx.hpp"

#include <cstring>
using std::memcmp;

TEST_CASE("Load the original avking SCX file") {
  SCXFile scxfile;

  REQUIRE(scxfile.scene_count() == 0);
  REQUIRE(scxfile.table1_count() == 0);
  REQUIRE(scxfile.variable_count() == 0);
  REQUIRE(scxfile.bg_count() == 0);
  REQUIRE(scxfile.chr_count() == 0);
  REQUIRE(scxfile.se_count() == 0);
  REQUIRE(scxfile.bgm_count() == 0);
  REQUIRE(scxfile.voice_count() == 0);

  REQUIRE(scxfile.read("../../avking.scx") == true);

  REQUIRE(scxfile.scene_count() == 16913);
  REQUIRE(scxfile.table1_count() == 474);
  REQUIRE(scxfile.variable_count() == 781);
  REQUIRE(scxfile.bg_count() == 553);
  REQUIRE(scxfile.chr_count() == 985);
  REQUIRE(scxfile.se_count() == 191);
  REQUIRE(scxfile.bgm_count() == 33);
  REQUIRE(scxfile.voice_count() == 24454);

  const Table1Data& table1_0 = scxfile.table1(0);
  REQUIRE(table1_0.data == u8"ゲームメイン・ゲームパート");

  const Table1Data& table1_45 = scxfile.table1(45);
  REQUIRE(table1_45.data == u8"メイン・撮影処理・撮影後判定");

  const Table1Data& table1_473 = scxfile.table1(473);
  REQUIRE(table1_473.data == u8"ＳＢ・縄掛けフェラ・顔射");

  const AssetName& bg0 = scxfile.bg(0);
  REQUIRE(bg0.name == u8"黎明学園・教室・昼");
  REQUIRE(bg0.abbreviation == u8"教室");

  const AssetName& bg1 = scxfile.bg(1);
  REQUIRE(bg1.name == u8"全体ＭＡＰ・移動モード・昼");
  REQUIRE(bg1.abbreviation == u8"黎明町");

  const AssetName& bg552 = scxfile.bg(552);
  REQUIRE(bg552.name == u8"ＳＢ・輪姦・騎乗位Ｂ・日本");
  REQUIRE(bg552.abbreviation == u8"");

  const Variable& var0 = scxfile.variable(0);
  const uint8_t var0Blob[] = {0x10, 0x00, 0x32, 0x00, 0x32, 0x00,
                              0x00, 0x00, 0x64, 0x00, 0xff, 0xff};

  REQUIRE(var0.name == u8"撮影モード終了時");
  REQUIRE(var0.comment == u8"");
  /*
  REQUIRE(var0.info_blob[0x0] == 0x10);
  REQUIRE(var0.info_blob[0x1] == 0x00);
  REQUIRE(var0.info_blob[0x2] == 0x32);
  REQUIRE(var0.info_blob[0x3] == 0x00);
  REQUIRE(var0.info_blob[0x4] == 0x32);
  REQUIRE(var0.info_blob[0x5] == 0x00);
  REQUIRE(var0.info_blob[0x6] == 0x00);
  REQUIRE(var0.info_blob[0x7] == 0x00);
  REQUIRE(var0.info_blob[0x8] == 0x64);
  REQUIRE(var0.info_blob[0x9] == 0x00);
  REQUIRE(var0.info_blob[0xa] == 0xff);
  REQUIRE(var0.info_blob[0xb] == 0xff);
  */
  REQUIRE(memcmp(&var0.info_blob[0], var0Blob, 0x0c) == 0);

  const Variable& var1 = scxfile.variable(1);
  const uint8_t var1Blob[] = {0x10, 0x00, 0x64, 0x00, 0x64, 0x00,
                              0x00, 0x00, 0x30, 0x75, 0xff, 0xff};

  REQUIRE(var1.name == u8"基本ステータス・所持金");
  REQUIRE(var1.comment == u8"");
  REQUIRE(memcmp(&var1.info_blob[0], var1Blob, 0x0c) == 0);

  const Variable& var283 = scxfile.variable(283);
  REQUIRE(var283.name == u8"ＴＣ・卑語・レッスン回数");
  REQUIRE(var283.comment == u8"場所・みのバーカウンター");

  const Variable& var780 = scxfile.variable(780);
  REQUIRE(var780.name == u8"システム・Ｑ＆Ａモード");
  REQUIRE(var780.comment == u8"");
}
