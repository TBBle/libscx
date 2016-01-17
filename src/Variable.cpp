#include "Variable.hpp"

#include <boost/locale.hpp>
using boost::locale::conv::to_utf;

#include <algorithm>
using std::copy;
#include <array>
using std::array;

#include <cassert>

#include <gsl.h>
using gsl::as_span;

void Variable::read_data(fixed_string_span string0, fixed_string_span string1,
                         blob_span data) {
  array<char, 0x21> buffer;
  buffer[0x20] = '\0';

  auto charstring0 = as_span<const char>(string0);
  copy(charstring0.begin(), charstring0.end(), buffer.begin());
  comment = to_utf<char>(&buffer[0], "windows-932");

  auto charstring1 = as_span<const char>(string1);
  copy(charstring1.begin(), charstring1.end(), buffer.begin());
  name = to_utf<char>(&buffer[0], "windows-932");

  assert(data.size() == info_blob.size());
  copy(data.begin(), data.end(), info_blob.begin());
}
