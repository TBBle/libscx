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

#include "buffutils.hpp"

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
using namespace boost::interprocess;

namespace {

// All data is little-endian
// Not encrypted
struct SCXFileIdentifier {
  const static size_t size = 0x08;
  const static size_t offset = 0x0;

  char fileprefix[4];  // "scx\0"
  uint32_t checksum;
};

static_assert(sizeof(SCXFileIdentifier) == SCXFileIdentifier::size,
              "SCXFileIdentifier did not pack correctly");

// All following data is encrupted
struct SCXFileHeader {
  const static size_t size = 0x3c;
  const static size_t offset = 0x08;

  uint32_t scene_count;
  uint32_t table1_count;
  uint32_t variable_count;
  uint32_t BG_count;
  uint32_t CHR_count;
  uint32_t SE_count;
  uint32_t BGM_count;
  uint32_t VOICE_count;
  uint32_t table1_strings_offset;
  uint32_t variable_names_offset;
  uint32_t BG_names_offset;
  uint32_t CHR_names_offset;
  uint32_t SE_names_offset;
  uint32_t BGM_names_offset;
  uint32_t VOICE_names_offset;
};

static_assert(sizeof(SCXFileHeader) == SCXFileHeader::size,
              "SCXFileHeader did not pack correctly");

template <size_t dataSize, class TwoParamDataT>
bool read_variable_data(vector<TwoParamDataT>& dataToFill,
                        buffutils::dynbuffer buffer, uint32_t offset1,
                        uint32_t offset2) {
  vector<uint32_t> stringOffsets;

  stringOffsets.resize(dataToFill.size());
  memcpy(stringOffsets.data(), &buffer[offset1],
         dataToFill.size() * sizeof(uint32_t));

  for (size_t i = 0; i < dataToFill.size(); ++i) {
    // TODO: Check string size result
    const uint32_t& stringOffset = stringOffsets[i];
    dataToFill[i].read_data(
        (stringOffset == 0 ? nullptr : reinterpret_cast<const char*>(
                                           &buffer[stringOffset])),
        gsl::span<uint8_t, dataSize>(&buffer[offset2 + dataSize * i],
                                     dataSize));
  }
  return true;
}

template <size_t dataSize, class OneParamDataT>
bool read_fixed_data(vector<OneParamDataT>& dataToFill,
                     buffutils::dynbuffer buffer, uint32_t offset) {
  for (size_t i = 0; i < dataToFill.size(); ++i) {
    if (dataToFill[i].read_data(&buffer[offset + dataSize * i]) != dataSize)
      return false;
  }
  return true;
}

template <size_t data1Size, size_t data2Size, class TwoParamDataT>
bool read_fixed_data(vector<TwoParamDataT>& dataToFill,
                     buffutils::dynbuffer buffer, uint32_t offset1,
                     uint32_t offset2) {
  for (size_t i = 0; i < dataToFill.size(); ++i) {
    if (dataToFill[i].read_data(&buffer[offset1 + data1Size * i],
                                &buffer[offset2 + data2Size * i]) !=
        make_pair(data1Size, data2Size))
      return false;
  }
  return true;
}

// 0x535f5c in the avking.exe image
static const uint8_t ENCRYPTION_KEY[] = {0xa9, 0xb3, 0xf2, 0x87, 0xdc, 0xaf,
                                         0x13, 0x67, 0xd5, 0x91, 0xec};
}

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

  // TODO: Consider boost::asio::buffer for this instead
  vector<uint8_t> storage(reinterpret_cast<uint8_t*>(addr),
                          reinterpret_cast<uint8_t*>(addr) + size);

  buffutils::dynbuffer buffer(storage);

  SCXFileIdentifier ident = buffutils::get<SCXFileIdentifier>(buffer);

  if (memcmp(&ident.fileprefix, "scx\0", 4)) {
    return false;
  }

  // Routine at 0x4352a0 in the binary does the checksum and decrypting
  uint32_t calc = 0;
  for (ptrdiff_t i = 8; i < buffer.size(); ++i) {
    // Checksum
    calc += int8_t(buffer[i]);
    // Decrypt
    size_t dataOffset = i - 8;
    uint8_t key = ENCRYPTION_KEY[dataOffset % sizeof(ENCRYPTION_KEY)];
    key += static_cast<uint8_t>(dataOffset);
    buffer[i] ^= key;
  }

  if (calc != ident.checksum) {
    return false;
  }

  // For now, we're copying data, to avoid alignment issues
  SCXFileHeader header = buffutils::get<SCXFileHeader>(buffer);

  scenes_.resize(header.scene_count);
  table1_.resize(header.table1_count);
  variables_.resize(header.variable_count);
  bg_names_.resize(header.BG_count);
  chr_names_.resize(header.CHR_count);
  se_names_.resize(header.SE_count);
  bgm_names_.resize(header.BGM_count);
  voice_names_.resize(header.VOICE_count);

  // These are offsets to tables of fixed-size string data
  uint32_t table1_strings_offset = header.table1_strings_offset;
  uint32_t variable_strings_offset = header.variable_names_offset;
  uint32_t bg_strings_offset = header.BG_names_offset;
  uint32_t chr_strings_offset = header.CHR_names_offset;
  uint32_t se_strings_offset = header.SE_names_offset;
  uint32_t bgm_strings_offset = header.BGM_names_offset;
  uint32_t voice_strings_offset = header.VOICE_names_offset;

  // This is a table of uint32 offsets to variable-sized string data
  uint32_t scene_string_offsets = SCXFileHeader::offset + SCXFileHeader::size;

  // This is a table of 0xd8-byte scene blobs
  uint32_t scene_blobs_offset =
      scene_string_offsets + sizeof(uint32_t) * header.scene_count;
  static const uint32_t scene_blob_size = 0xd8;

  // This is a table of 0xc-byte variable info blobs
  uint32_t variable_blobs_offset =
      scene_blobs_offset + scene_blob_size * header.scene_count;

  if (!read_variable_data<scene_blob_size>(
          scenes_, buffer, scene_string_offsets, scene_blobs_offset))
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
