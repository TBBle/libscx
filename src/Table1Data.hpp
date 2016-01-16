#include <string>

#include <cstdint>

#include <gsl.h>

struct Table1Data {
 public:
  // Reading API
  using fixed_string_span = gsl::span<const gsl::byte, 0x20>;
  void read_data(fixed_string_span string);

  // Writing API
  // Writes 0x20 bytes to pStringData
  void write_data(std::uint8_t* pStringData) const;
  // utf-8 encoded, I have no idea what this class represents
  std::string data;
};
