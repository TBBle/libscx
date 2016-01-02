#include "Table1Data.hpp"

#include <boost/locale.hpp>
using boost::locale::conv::to_utf;

#include <array>
using std::array;

#include <cstring>
using std::memcpy;
#include <cstddef>
using std::size_t;
#include <cstdint>
using std::uint8_t;

size_t Table1Data::read_data(const uint8_t* pStringData) {
  array<char, 0x21> buffer;
  buffer[0x20] = '\0';

  memcpy(&buffer[0], pStringData, 0x20);
  data = to_utf<char>(&buffer[0], "windows-932");

  return 0x20;
}
