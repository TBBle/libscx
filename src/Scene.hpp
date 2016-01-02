#include <array>
#include <string>
#include <utility>

#include <cstdint>

#include <gsl.h>

struct Scene {
 public:
  // Reading API
  // Returns the number of bytes read from the string, including the terminating
  // '\0'
  size_t read_data(gsl::czstring<> cp932text, gsl::span<uint8_t, 0xd8> data);
  // Writing API: Not ready yet.
  std::size_t stringSize() const;
  std::size_t write_data(std::uint8_t* pStringData,
                         std::uint8_t* pBlobData) const;

  std::string text;  // utf-8 encoded
  std::uint16_t chapter;
  std::uint16_t scene;
  std::uint16_t command;
  std::uint16_t unk1;
  std::uint16_t unk2;
  std::uint16_t chapterJump;
  std::uint16_t sceneJump1;
  std::array<std::uint8_t, 0x30> sceneJumpInfo1;
  std::uint16_t sceneJump2;
  std::array<std::uint8_t, 0x30> sceneJumpInfo2;
  std::uint16_t sceneJump3;
  std::array<std::uint8_t, 0x30> sceneJumpInfo3;
  std::uint16_t sceneJump4;
  std::array<std::uint8_t, 0x30> sceneJumpInfo4;
  std::uint32_t unk3;
};
