#include "Variable.hpp"

#include <boost/locale.hpp>
using boost::locale::conv::to_utf;

#include <array>
using std::array;
#include <utility>
using std::pair;
using std::make_pair;

#include <cstddef>
using std::size_t;
#include <cstring>
using std::memcpy;
#include <cstdint>
using std::uint8_t;

pair<size_t, size_t> Variable::read_data(const uint8_t* pStringData,
                                         const uint8_t* pBlobData) {
  array<char, 0x21> buffer;
  buffer[0x20] = '\0';

  memcpy(&buffer[0], pStringData, 0x20);
  comment = to_utf<char>(&buffer[0], "windows-932");

  memcpy(&buffer[0], pStringData + 0x20, 0x20);
  name = to_utf<char>(&buffer[0], "windows-932");

  memcpy(&info_blob[0], pBlobData, 0x0c);

  return make_pair(0x40, 0xc);
}
