#include <cstdint>
#include <string>

struct Table1Data {
 public:
  // Reading API
  std::size_t read_data(const std::uint8_t* pStringData);
  // Writing API
  // Writes 0x20 bytes to pStringData
  void write_data(std::uint8_t* pStringData) const;
  // utf-8 encoded, I have no idea what this class represents
  std::string data;
};
