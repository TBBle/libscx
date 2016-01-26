#include "AssetName.hpp"

#include <boost/locale.hpp>
using boost::locale::conv::to_utf;
using boost::locale::conv::from_utf;

#include <array>
using std::array;

#include <gsl.h>
using gsl::as_span;

void AssetName::read_data(fixed_string_span string0,
                          fixed_string_span string1) {
  array<char, 0x21> buffer;
  buffer[0x20] = '\0';

  auto charstring0 = as_span<const char>(string0);
  copy(charstring0.cbegin(), charstring0.cend(), buffer.begin());
  name = to_utf<char>(&buffer[0], "windows-932");

  auto charstring1 = as_span<const char>(string1);
  copy(charstring1.cbegin(), charstring1.cend(), buffer.begin());
  abbreviation = to_utf<char>(&buffer[0], "windows-932");
}

void AssetName::write_data(fixed_string_span_out string0,
                           fixed_string_span_out string1) const {
  const auto cp932string0(from_utf<char>(name, "windows-932"));
  Expects(cp932string0.length() < 0x21);
  memset(string0.data(), 0, string0.size_bytes());
  auto charstring0 = as_span<char>(string0);
  copy(cp932string0.cbegin(), cp932string0.cend(), charstring0.begin());

  const auto cp932string1(from_utf<char>(abbreviation, "windows-932"));
  Expects(cp932string1.length() < 0x21);
  memset(string1.data(), 0, string1.size_bytes());
  auto charstring1 = as_span<char>(string1);
  copy(cp932string1.cbegin(), cp932string1.cend(), charstring1.begin());
}