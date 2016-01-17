#include "Table1Data.hpp"

#include <boost/locale.hpp>
using boost::locale::conv::to_utf;

#include <array>
using std::array;

#include <gsl.h>
using gsl::as_span;

void Table1Data::read_data(fixed_string_span string) {
  array<char, 0x21> buffer;
  buffer[0x20] = '\0';

  auto charstring = as_span<const char>(string);
  copy(charstring.begin(), charstring.end(), buffer.begin());
  data = to_utf<char>(&buffer[0], "windows-932");
}
