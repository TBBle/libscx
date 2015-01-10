#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do
                           // this in one cpp file
#include "catch.hpp"

#include "scx.hpp"

#include <string>
using std::string;

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

  const uint8_t nullSceneBlob[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  const Scene& scene0 = scxfile.scene(0);
  REQUIRE(scene0.text == u8"");
  REQUIRE(scene0.chapter == 0);
  REQUIRE(scene0.scene == 0);
  REQUIRE(scene0.command == 0);
  REQUIRE(scene0.unk1 == 0);
  REQUIRE(scene0.unk2 == 0xffff);
  REQUIRE(scene0.chapterJump == 0xffff);
  REQUIRE(scene0.sceneJump1 == 0xffff);
  REQUIRE(memcmp(&scene0.sceneJumpInfo1[0], nullSceneBlob, 0x30) == 0);
  REQUIRE(scene0.sceneJump2 == 0xffff);
  REQUIRE(memcmp(&scene0.sceneJumpInfo2[0], nullSceneBlob, 0x30) == 0);
  REQUIRE(scene0.sceneJump3 == 0xffff);
  REQUIRE(memcmp(&scene0.sceneJumpInfo3[0], nullSceneBlob, 0x30) == 0);
  REQUIRE(scene0.sceneJump4 == 25);
  REQUIRE(memcmp(&scene0.sceneJumpInfo4[0], nullSceneBlob, 0x30) == 0);
  REQUIRE(scene0.unk3 == 0x00000000);

  const Scene& scene1 = scxfile.scene(1);
  REQUIRE(scene1.text == u8"[\\w,2,=,2,+,-2][\\w,589,=,-1]");
  REQUIRE(scene1.chapter == 0);
  REQUIRE(scene1.scene == 1);
  REQUIRE(scene1.command == 0);
  REQUIRE(scene1.unk1 == 0);
  REQUIRE(scene1.unk2 == 0xffff);
  REQUIRE(scene1.chapterJump == 0xffff);
  REQUIRE(scene1.sceneJump1 == 0xffff);
  REQUIRE(memcmp(&scene1.sceneJumpInfo1[0], nullSceneBlob, 0x30) == 0);
  REQUIRE(scene1.sceneJump2 == 0xffff);
  REQUIRE(memcmp(&scene1.sceneJumpInfo2[0], nullSceneBlob, 0x30) == 0);
  REQUIRE(scene1.sceneJump3 == 16);
  REQUIRE(memcmp(&scene1.sceneJumpInfo3[0], nullSceneBlob, 0x30) == 0);
  REQUIRE(scene1.sceneJump4 == 0xffff);
  REQUIRE(memcmp(&scene1.sceneJumpInfo4[0], nullSceneBlob, 0x30) == 0);
  REQUIRE(scene1.unk3 == 0x00000000);

  const Scene& scene14500 = scxfile.scene(14500);

  const string scene14500Text =
      u8"[\\m,1,4][\\b,0,19][\\e,21,-2][\\c,0,15][\\c,3,15]"
      u8"「あ〜どもども。お嬢様、おひさしブリブリ〜」[\\r]"
      "[\\c,2,829][\\c,3,829]"
      "「うむ、無沙汰をしておったな、轟よ。渡したⅰポットンは役に立っておるのか"
      "？」[\\r]"
      "[\\c,3,15]"
      "「もっちろんですよ〜これさえあれば、どんな危険な任務もらっくらくって感じ"
      "〜」[\\r]"
      "[\\c,3,829]"
      "「ふむ、ならば良い。それより今日はそなたに尋ねたいことがあるのじゃ」["
      "\\r]"
      "[\\c,3,15]"
      "「僕にですかぁ〜？　光栄だなぁ〜。なんでも聞いてくださいよ〜」["
      "\\r]"
      "[\\c,3,829]「うむ。では尋ねるが…轟はＳＭというものを知っておるか？」["
      "\\r]"
      "[\\c,0,723][\\c,3,723]"
      "「えええＳＭううう〜！？　なな、なんでそんなことを？」[\\r]"
      "[\\c,3,829]"
      "「うむ。実はちと仕事の為に資料が必要になっての。ＳＭについて調べておるの"
      "じゃ」[\\r]"
      "[\\c,3,723]「は、はぁ…仕事の為ですか…びっくりしたぁ〜…」[\\r]"
      "[\\c,3,829]「それよりどうなのじゃ？　知っておるのか、いないのか？」[\\r]"
      "[\\c,0,724][\\c,3,724]"
      "「あ、え〜とですねぇ、僕より詳しい専門家がいるんで、その人に聞くのがいい"
      "かとぉ」[\\r]"
      "[\\c,3,829]「ほほう…それはなかなか興味深い人物じゃな」[\\r]"
      "[\\c,0,15][\\c,3,15]「ええ。それじゃさっそく河川敷に行きましょう」[\\r]"
      "[\\c,3,829]「河川敷？　その人物はそんなとこに住んでおるのか？」[\\r]"
      "[\\c,3,15]「違いますよ〜。でもいっつもそこに立ってるんです」[\\r]"
      "[\\c,3,829]「ふむ…。ではすぐに訪ねてみよう」[\\r]"
      "[\\c,3,15]「は〜い」[\\r]"
      "[\\c,5]";

  REQUIRE(scene14500.text.size() == scene14500Text.size());
  REQUIRE(scene14500.text == scene14500Text);
  REQUIRE(scene14500.chapter == 345);
  REQUIRE(scene14500.scene == 1);
  REQUIRE(scene14500.command == 0);
  REQUIRE(scene14500.unk1 == 0);
  REQUIRE(scene14500.unk2 == 0xffff);
  REQUIRE(scene14500.chapterJump == 0xffff);
  REQUIRE(scene14500.sceneJump1 == 0xffff);
  REQUIRE(memcmp(&scene14500.sceneJumpInfo1[0], nullSceneBlob, 0x30) == 0);
  REQUIRE(scene14500.sceneJump2 == 0xffff);
  REQUIRE(memcmp(&scene14500.sceneJumpInfo2[0], nullSceneBlob, 0x30) == 0);
  REQUIRE(scene14500.sceneJump3 == 2);
  REQUIRE(memcmp(&scene14500.sceneJumpInfo3[0], nullSceneBlob, 0x30) == 0);
  REQUIRE(scene14500.sceneJump4 == 0xffff);
  REQUIRE(memcmp(&scene14500.sceneJumpInfo4[0], nullSceneBlob, 0x30) == 0);
  REQUIRE(scene14500.unk3 == 0x00000000);

  const Scene& scene16912 = scxfile.scene(16912);
  REQUIRE(scene16912.text == u8"[\\e,14,675,-1][\\w,771,=,-4]");
  REQUIRE(scene16912.chapter == 473);
  REQUIRE(scene16912.scene == 53);
  REQUIRE(scene16912.command == 0);
  REQUIRE(scene16912.unk1 == 0);
  REQUIRE(scene16912.unk2 == 0xffff);
  REQUIRE(scene16912.chapterJump == 0xffff);
  REQUIRE(scene16912.sceneJump1 == 0xffff);
  REQUIRE(memcmp(&scene16912.sceneJumpInfo1[0], nullSceneBlob, 0x30) == 0);
  REQUIRE(scene16912.sceneJump2 == 32);
  REQUIRE(memcmp(&scene16912.sceneJumpInfo2[0], nullSceneBlob, 0x30) == 0);
  REQUIRE(scene16912.sceneJump3 == 0xffff);
  REQUIRE(memcmp(&scene16912.sceneJumpInfo3[0], nullSceneBlob, 0x30) == 0);
  REQUIRE(scene16912.sceneJump4 == 0xffff);
  REQUIRE(memcmp(&scene16912.sceneJumpInfo4[0], nullSceneBlob, 0x30) == 0);
  REQUIRE(scene16912.unk3 == 0x00000000);

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

  // TODO: Check a string with the character U+2170 SMALL ROMAN NUMERAL ONE
  // which is in the data as FA 40
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
