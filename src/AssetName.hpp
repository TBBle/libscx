#include <string>

#include <cstdint>

struct AssetName {
 public:
  // Reading API
  std::size_t read_data(const std::uint8_t* pStringData);
  // Writing API
  // Writes 0x40 bytes to pStringData
  std::size_t write_data(std::uint8_t* pStringData) const;
  // utf-8 encoded
  std::string name;
  std::string abbreviation;
};
