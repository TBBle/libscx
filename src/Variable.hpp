#include <array>
#include <string>

#include <cstdint>

#include <gsl.h>

struct Variable {
 public:
  static const std::uint32_t blob_size = 0xc;

  // Reading API
  using fixed_string_span = gsl::span<const gsl::byte, 0x20>;
  using blob_span = gsl::span<const gsl::byte, blob_size>;
  void read_data(fixed_string_span string0, fixed_string_span string1,
                 blob_span data);

  // Writing API
  // Writes 0x40 bytes to pStringData, and 0xc bytes at pBlobData
  std::size_t write_data(std::uint8_t* pStringData,
                         std::uint8_t* pBlobData) const;
  // utf-8 encoded
  std::string name;
  std::string comment;
  std::array<gsl::byte, blob_size> info_blob;
};
