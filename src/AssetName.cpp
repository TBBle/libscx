#include "AssetName.hpp"

#include <boost/locale.hpp>
using boost::locale::conv::to_utf;

#include <array>
using std::array;

#include <cstring>
using std::memcpy;

void AssetName::read_data(fixed_string_span string0,
                          fixed_string_span string1) {
  array<char, 0x21> buffer;
  buffer[0x20] = '\0';

  memcpy(&buffer[0], string0.data(), string0.size());
  name = to_utf<char>(&buffer[0], "windows-932");

  memcpy(&buffer[0], string1.data(), string1.size());
  abbreviation = to_utf<char>(&buffer[0], "windows-932");
}
