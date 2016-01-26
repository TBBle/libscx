#include <string>

#include <cstdint>

#include <gsl.h>

struct AssetName {
 public:
  // Reading API
  using fixed_string_span = gsl::span<const gsl::byte, 0x20>;
  void read_data(fixed_string_span string0, fixed_string_span string1);

  // Writing API
  using fixed_string_span_out = gsl::span<gsl::byte, 0x20>;
  void write_data(fixed_string_span_out string0,
                  fixed_string_span_out string1) const;

  // utf-8 encoded
  std::string name;
  std::string abbreviation;
};
