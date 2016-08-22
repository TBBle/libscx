#include <array>
#include <string>

#include <cstdint>

#include <gsl/gsl>

struct Variable {
 public:
  static const std::uint32_t blob_size = 0xc;

  // Reading API
  using fixed_string_span = gsl::multi_span<const gsl::byte, 0x20>;
  using blob_span = gsl::multi_span<const gsl::byte, blob_size>;
  void read_data(fixed_string_span string0, fixed_string_span string1,
                 blob_span data);

  // Writing API
  using fixed_string_span_out = gsl::multi_span<gsl::byte, 0x20>;
  using blob_span_out = gsl::multi_span<gsl::byte, blob_size>;
  void write_data(fixed_string_span_out string0, fixed_string_span_out string1,
                  blob_span_out data) const;

  // utf-8 encoded
  std::string name;
  std::string comment;
  std::array<gsl::byte, blob_size> info_blob;
};
