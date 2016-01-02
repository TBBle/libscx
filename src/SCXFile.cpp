#include "SCXFile.hpp"

#include <string>
using std::string;
#include <utility>
using std::make_pair;
#include <vector>
using std::vector;

#include <cstddef>
using std::size_t;
#include <cstring>
using std::memcpy;
using std::memcmp;
#include <cstdint>
using std::uint8_t;
using std::int8_t;
using std::uint32_t;

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
using namespace boost::interprocess;

namespace {

template <typename T>
void buffcopy(T& output, vector<uint8_t>& buffer, size_t offset) {
  // assert offset + sizeof(T) <= buffer.size();
  memcpy(&output, &buffer[offset], sizeof(T));
}

template <size_t dataSize, class TwoParamDataT>
bool read_variable_data(vector<TwoParamDataT>& dataToFill,
                        vector<uint8_t>& buffer, uint32_t offset1,
                        uint32_t offset2) {
  vector<uint32_t> stringOffsets;

  stringOffsets.resize(dataToFill.size());
  memcpy(stringOffsets.data(), &buffer[offset1],
         dataToFill.size() * sizeof(uint32_t));

  for (size_t i = 0; i < dataToFill.size(); ++i) {
    // TODO: Check string size result
    const uint32_t& stringOffset = stringOffsets[i];
    if (dataToFill[i]
            .read_data((stringOffset == 0 ? NULL : &buffer[stringOffset]),
                       &buffer[offset2 + dataSize * i])
            .second != dataSize)
      return false;
  }
  return true;
}

template <size_t dataSize, class OneParamDataT>
bool read_fixed_data(vector<OneParamDataT>& dataToFill, vector<uint8_t>& buffer,
                     uint32_t offset) {
  for (size_t i = 0; i < dataToFill.size(); ++i) {
    if (dataToFill[i].read_data(&buffer[offset + dataSize * i]) != dataSize)
      return false;
  }
  return true;
}

template <size_t data1Size, size_t data2Size, class TwoParamDataT>
bool read_fixed_data(vector<TwoParamDataT>& dataToFill, vector<uint8_t>& buffer,
                     uint32_t offset1, uint32_t offset2) {
  for (size_t i = 0; i < dataToFill.size(); ++i) {
    if (dataToFill[i].read_data(&buffer[offset1 + data1Size * i],
                                &buffer[offset2 + data2Size * i]) !=
        make_pair(data1Size, data2Size))
      return false;
  }
  return true;
}
}

// 0x535f5c in the avking.exe image
static const uint8_t ENCRYPTION_KEY[] = {0xa9, 0xb3, 0xf2, 0x87, 0xdc, 0xaf,
                                         0x13, 0x67, 0xd5, 0x91, 0xec};

SCXFile::SCXFile()
    : scenes_(),
      variables_(),
      table1_(),
      bg_names_(),
      chr_names_(),
      se_names_(),
      bgm_names_(),
      voice_names_() {}

/* Structure:
4 bytes scx\0  - Not encrypted
4 byte checksum  - Not encrypted, checksum of encrypted data
// Everything below here is XOR'd against:
//  0xa9 0xb3 0xf2 0x87 0xdc 0xaf 0x13 0x67 0xd5 0x91 0xec
0x08: uint32 scene_count (16913)
0x0c: uint32 table1_count (474)
0x10: uint32 variable_count (781)
0x14: uint32 BG_count
0x18: uint32 CHR_count
0x1c: uint32 SE_count
0x20: uint32 BGM_count
0x24: uint32 VOICE_count
0x28: uint32 table1_strings_offset
0x2c: uint32 variable_names_offset
Asset names:
0x30: uint32 BG_names_offset
0x34: uint32 CHR_names_offset
0x38: uint32 SE_names_offset
0x3c: uint32 BGM_names_offset
0x40: uint32 VOICE_names_offset
STRING OFFSETS:
0x44: scene_count * uint32 string offsets (may be 0 - no string)
BLOBS: following the scene offsets
....: scene_count * 0xd8 scene info blobs
....: variable_count * 0xc variable info blobs
STRINGS: Offsets read from header
....: table1 strings (0x20 byte fixed string buffer, stops at first null)
....: variable string-pairs (0x20 byte fixed string buffer, stops at first null,
x2)
  Looks like comment then description.
....: Asset name string-pairs (0x20 byte fixed string buffer, stops at first
null, x2)
  Looks like name then display-name?
*/

bool SCXFile::read(string fileName) {
  file_mapping file(fileName.c_str(), read_only);
  mapped_region region(file, read_only);
  void* addr = region.get_address();
  size_t size = region.get_size();

  vector<uint8_t> buffer(size);
  memcpy(&buffer[0], addr, size);

  if (memcmp(&buffer[0], "scx\0", 4)) {
    return false;
  }

  // Routine at 0x4352a0 in the binary does the checksum and decrypting
  uint32_t* pChecksum = reinterpret_cast<uint32_t*>(&buffer[4]);
  uint32_t calc = 0;
  for (size_t i = 8; i < buffer.size(); ++i) {
    // Checksum
    calc += int8_t(buffer[i]);
    // Decrypt
    size_t dataOffset = i - 8;
    uint8_t key = ENCRYPTION_KEY[dataOffset % sizeof(ENCRYPTION_KEY)];
    key += static_cast<uint8_t>(dataOffset);
    buffer[i] ^= key;
  }

  if (calc != *pChecksum) {
    return false;
  }

  /*
  http://stackoverflow.com/questions/2079912/simpler-way-to-create-a-c-memorystream-from-char-size-t-without-copying-t
  */

  // Is there a better way to do this?
  uint32_t data;

  buffcopy(data, buffer, 0x08);
  scenes_.resize(data);
  buffcopy(data, buffer, 0x0c);
  table1_.resize(data);
  buffcopy(data, buffer, 0x10);
  memcpy(&data, &buffer[0x10], sizeof(data));
  variables_.resize(data);
  buffcopy(data, buffer, 0x14);
  bg_names_.resize(data);
  buffcopy(data, buffer, 0x18);
  chr_names_.resize(data);
  buffcopy(data, buffer, 0x1c);
  se_names_.resize(data);
  buffcopy(data, buffer, 0x20);
  bgm_names_.resize(data);
  buffcopy(data, buffer, 0x24);
  voice_names_.resize(data);

  // These are offsets to tables of fixed-size string data
  buffcopy(data, buffer, 0x28);
  uint32_t table1_strings_offset = data;
  buffcopy(data, buffer, 0x2c);
  uint32_t variable_strings_offset = data;
  buffcopy(data, buffer, 0x30);
  uint32_t bg_strings_offset = data;
  buffcopy(data, buffer, 0x34);
  uint32_t chr_strings_offset = data;
  buffcopy(data, buffer, 0x38);
  uint32_t se_strings_offset = data;
  buffcopy(data, buffer, 0x3c);
  uint32_t bgm_strings_offset = data;
  buffcopy(data, buffer, 0x40);
  uint32_t voice_strings_offset = data;

  // This is a table of uint32 offsets to variable-sized string data
  uint32_t scene_string_offsets = 0x44;

  // This is a table of 0xd8-byte scene blobs
  uint32_t scene_blobs_offset =
      0x44 + 0x04 * static_cast<uint32_t>(scenes_.size());
  // This is a table of 0xc-byte variable info blobs
  uint32_t variable_blobs_offset =
      scene_blobs_offset + 0xd8 * static_cast<uint32_t>(scenes_.size());

  if (!read_variable_data<0xd8>(scenes_, buffer, scene_string_offsets,
                                scene_blobs_offset))
    return false;

  if (!read_fixed_data<0x20>(table1_, buffer, table1_strings_offset))
    return false;

  if (!read_fixed_data<0x40, 0x0c>(variables_, buffer, variable_strings_offset,
                                   variable_blobs_offset))
    return false;

  if (!read_fixed_data<0x40>(bg_names_, buffer, bg_strings_offset))
    return false;

  if (!read_fixed_data<0x40>(chr_names_, buffer, chr_strings_offset))
    return false;

  if (!read_fixed_data<0x40>(se_names_, buffer, se_strings_offset))
    return false;

  if (!read_fixed_data<0x40>(bgm_names_, buffer, bgm_strings_offset))
    return false;

  if (!read_fixed_data<0x40>(voice_names_, buffer, voice_strings_offset))
    return false;

  return true;
}
