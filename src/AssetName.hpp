#include <string>

#include <cstdint>

#include <gsl.h>

struct AssetName {
 public:
  // Reading API
  using fixed_string_span = gsl::span<const gsl::byte, 0x20>;
  void read_data(fixed_string_span string0, fixed_string_span string1);

  // Writing API
  // Writes 0x40 bytes to pStringData
  std::size_t write_data(std::uint8_t* pStringData) const;
  // utf-8 encoded
  std::string name;
  std::string abbreviation;
};
