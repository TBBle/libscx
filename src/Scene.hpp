#include <array>
#include <string>

#include <cstdint>

#include <gsl/gsl>

struct Scene {
 public:
  static const uint32_t blob_size = 0xd8;

  // Reading API
  using blob_span = gsl::multi_span<const gsl::byte, blob_size>;
  void read_data(gsl::czstring<> cp932text, blob_span data);

  // Writing API
  using blob_span_out = gsl::multi_span<gsl::byte, blob_size>;
  std::unique_ptr<std::string> write_data(blob_span_out data) const;

  static const size_t scene_jump_blob_size = 0x30;
  using scene_jump_blob = std::array<gsl::byte, scene_jump_blob_size>;

  std::string text;  // utf-8 encoded
  std::uint16_t chapter;
  std::uint16_t scene;
  std::uint16_t command;
  std::uint16_t unk1;
  std::uint16_t unk2;
  std::uint16_t chapterJump;
  std::uint16_t sceneJump1;
  scene_jump_blob sceneJumpInfo1;
  std::uint16_t sceneJump2;
  scene_jump_blob sceneJumpInfo2;
  std::uint16_t sceneJump3;
  scene_jump_blob sceneJumpInfo3;
  std::uint16_t sceneJump4;
  scene_jump_blob sceneJumpInfo4;
  std::uint32_t unk3;
};
