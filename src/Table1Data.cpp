#include "Table1Data.hpp"

#include <boost/array.hpp>
#include <boost/locale.hpp>

std::size_t Table1Data::read_data(const std::uint8_t* pStringData) {
  boost::array<char, 0x21> buffer;
  buffer[0x20] = '\0';

  std::memcpy(&buffer[0], pStringData, 0x20);
  data = boost::locale::conv::to_utf<char>(&buffer[0], "SJIS");

  return 0x20;
}
