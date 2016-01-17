#include "Table1Data.hpp"

#include <boost/locale.hpp>
using boost::locale::conv::to_utf;

#include <array>
using std::array;

#include <cstring>
using std::memcpy;

void Table1Data::read_data(fixed_string_span string) {
  array<char, 0x21> buffer;
  buffer[0x20] = '\0';

  // narrow_cast isn't constexpr?
  //  auto charstring = gsl::as_span<const char>(string);
  //  copy(string.begin(), string.end(), buffer.begin());
  memcpy(&buffer[0], string.data(), string.size());
  data = to_utf<char>(&buffer[0], "windows-932");
}
