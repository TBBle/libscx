#include <string>

#include <cstdint>

#include <gsl.h>

struct Table1Data {
 public:
  // Reading API
  using fixed_string_span = gsl::span<const gsl::byte, 0x20>;
  void read_data(fixed_string_span string);

  // Writing API
  using fixed_string_span_out = gsl::span<gsl::byte, 0x20>;
  void write_data(fixed_string_span_out string) const;

  // utf-8 encoded, I have no idea what this class represents
  std::string data;
};
