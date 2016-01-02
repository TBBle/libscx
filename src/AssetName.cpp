#include "AssetName.hpp"

#include <boost/locale.hpp>
using boost::locale::conv::to_utf;

#include <array>
using std::array;

#include <cstring>
using std::memcpy;
#include <cstdint>
using std::uint8_t;
#include <cstddef>
using std::size_t;

size_t AssetName::read_data(const uint8_t* pStringData) {
  array<char, 0x21> buffer;
  buffer[0x20] = '\0';

  memcpy(&buffer[0], pStringData, 0x20);
  name = to_utf<char>(&buffer[0], "CP932");

  memcpy(&buffer[0], pStringData + 0x20, 0x20);
  abbreviation = to_utf<char>(&buffer[0], "CP932");

  return 0x40;
}
