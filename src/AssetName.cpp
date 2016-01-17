#include "AssetName.hpp"

#include <boost/locale.hpp>
using boost::locale::conv::to_utf;

#include <array>
using std::array;

#include <gsl.h>
using gsl::as_span;

void AssetName::read_data(fixed_string_span string0,
                          fixed_string_span string1) {
  array<char, 0x21> buffer;
  buffer[0x20] = '\0';

  auto charstring0 = as_span<const char>(string0);
  copy(charstring0.begin(), charstring0.end(), buffer.begin());
  name = to_utf<char>(&buffer[0], "windows-932");

  auto charstring1 = as_span<const char>(string1);
  copy(charstring1.begin(), charstring1.end(), buffer.begin());
  abbreviation = to_utf<char>(&buffer[0], "windows-932");
}
