#include "Table1Data.hpp"

#include <boost/locale.hpp>
using boost::locale::conv::to_utf;
using boost::locale::conv::from_utf;

#include <array>
using std::array;

#include <gsl/gsl>
using gsl::as_multi_span;

void Table1Data::read_data(fixed_string_span string) {
  array<char, 0x21> buffer;
  buffer[0x20] = '\0';

  auto charstring = as_multi_span<const char>(string);
  copy(charstring.cbegin(), charstring.cend(), buffer.begin());
  data = to_utf<char>(&buffer[0], "windows-932");
}

void Table1Data::write_data(fixed_string_span_out string) const {
  const auto cp932string(from_utf<char>(data, "windows-932"));
  Expects(cp932string.length() < 0x21);
  memset(string.data(), 0, string.size_bytes());
  auto charstring = as_multi_span<char>(string);
  copy(cp932string.cbegin(), cp932string.cend(), charstring.begin());
}
