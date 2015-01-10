#include <array>
#include <string>

#include <cstdint>

struct Variable {
 public:
  // Reading API: Returns how much was read from the two pointers supplied
  std::pair<std::size_t, std::size_t> read_data(const std::uint8_t* pStringData,
                                                const std::uint8_t* pBlobData);
  // Writing API
  // Writes 0x40 bytes to pStringData, and 0xc bytes at pBlobData
  std::size_t write_data(std::uint8_t* pStringData,
                         std::uint8_t* pBlobData) const;
  // utf-8 encoded
  std::string name;
  std::string comment;
  std::array<std::uint8_t, 0xc> info_blob;
};
