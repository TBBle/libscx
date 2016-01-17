#include "SCXFile.hpp"

#include <array>
using std::array;
#include <string>
using std::string;
#include <vector>
using std::vector;

#include <cstddef>
using std::size_t;
#include <cstring>
using std::memcmp;
#include <cstdint>
using std::uint8_t;
using std::int8_t;
using std::uint32_t;

#include <gsl.h>
using gsl::as_span;
using gsl::byte;
using gsl::dim;
using gsl::dynamic_range;
using gsl::span;

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

using scene_blobs_span = span<const byte, dynamic_range, Scene::blob_size>;

void read_scene_data(vector<Scene>& scene_data, scene_blobs_span scene_blobs,
                     span<const uint32_t> scene_string_offsets,
                     span<const byte> buffer) {
  assert(scene_blobs.extent() == scene_string_offsets.extent());
  scene_data.resize(scene_blobs.extent());
  for (size_t i = 0; i < scene_data.size(); ++i) {
    auto& scene = scene_data[i];
    const auto& blob = scene_blobs[i];
    auto offset = scene_string_offsets[i];
    // XXX: This breaks if buffer[offset] does not point to a null-terminated
    // string!
    auto pString =
        offset ? &as_span<const char>(buffer).data()[offset] : nullptr;
    scene.read_data(pString, blob);
  }
}

static const uint32_t fixed_string_size = 0x20;
using fixed_strings_span = span<const byte, dynamic_range, fixed_string_size>;

void read_table1_data(vector<Table1Data>& table1_data,
                      fixed_strings_span table1_strings) {
  table1_data.resize(table1_strings.extent());
  for (size_t i = 0; i < table1_data.size(); ++i) {
    auto& table1_entry = table1_data[i];
    table1_entry.read_data(table1_strings[i]);
  }
}

using variable_blobs_span =
    span<const byte, dynamic_range, Variable::blob_size>;
using fixed_string_pairs_span =
    span<const byte, dynamic_range, 2, fixed_string_size>;

void read_variable_data(vector<Variable>& variable_data,
                        variable_blobs_span variable_blobs,
                        fixed_string_pairs_span variable_strings_buffers) {
  assert(variable_blobs.extent() == variable_strings_buffers.extent());
  variable_data.resize(variable_blobs.extent());
  for (size_t i = 0; i < variable_data.size(); ++i) {
    auto& variable = variable_data[i];
    auto strings = variable_strings_buffers[i];
    variable.read_data(strings[0], strings[1], variable_blobs[i]);
  }
}

void read_asset_strings(vector<AssetName>& asset_data,
                        fixed_string_pairs_span asset_strings_buffers) {
  asset_data.resize(asset_strings_buffers.extent());
  for (size_t i = 0; i < asset_data.size(); ++i) {
    auto& asset = asset_data[i];
    auto strings = asset_strings_buffers[i];
    asset.read_data(strings[0], strings[1]);
  }
}

// 0x535f5c in the avking.exe image
static const array<uint8_t, 11> ENCRYPTION_KEY{
    0xa9, 0xb3, 0xf2, 0x87, 0xdc, 0xaf, 0x13, 0x67, 0xd5, 0x91, 0xec};

// The offset is within the encrypted region
uint8_t xor_key(ptrdiff_t offset) {
  uint8_t key = ENCRYPTION_KEY[offset % ENCRYPTION_KEY.size()];
  // Wrapping is expected here.
  return key + static_cast<uint8_t>(offset);
}
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

bool SCXFile::read(const string& fileName) {
  file_mapping file(fileName.c_str(), read_only);
  mapped_region region(file, read_only);
  void* addr = region.get_address();
  size_t size = region.get_size();

  vector<byte> storage(reinterpret_cast<byte*>(addr),
                       reinterpret_cast<byte*>(addr) + size);

  span<const byte> buffer(storage);

  // Extract and advance past an SCXFileIdentifier
  const auto& ident =
      as_span<SCXFileIdentifier>(buffer.first(sizeof(SCXFileIdentifier)))[0];
  buffer = buffer.subspan(sizeof(SCXFileIdentifier));

  if (memcmp(&ident.fileprefix, "scx\0", 4)) {
    return false;
  }

  // Routine at 0x4352a0 in the binary does the checksum and decrypting
  auto encrypted = span<byte>(storage).subspan(sizeof(SCXFileIdentifier));
  uint32_t calc = 0;
  for (ptrdiff_t i = 0; i < encrypted.extent(); ++i) {
    // Checksum: Signed math
    calc += static_cast<int8_t>(encrypted[i]);
    // Decrypt: Unsigned math
    reinterpret_cast<uint8_t&>(encrypted[i]) ^= xor_key(i);
  }

  if (calc != ident.checksum) {
    return false;
  }

  // Extract and advance past an SCXFileHeader
  const auto& header =
      as_span<SCXFileHeader>(buffer.first(sizeof(SCXFileHeader)))[0];
  buffer = buffer.subspan(sizeof(SCXFileHeader));

  // Extract and advance past a table of uint32 offsets to variable-sized string
  // data
  auto scene_string_offsets = as_span<const uint32_t>(
      buffer.first(sizeof(uint32_t) * header.scene_count));
  buffer = buffer.subspan(scene_string_offsets.size_bytes());

  // Extract and advance past an array of 0xd8-byte data structures
  scene_blobs_span scene_blobs =
      as_span(buffer.first(Scene::blob_size * header.scene_count),
              dim<>(header.scene_count), dim<Scene::blob_size>());
  buffer = buffer.subspan(scene_blobs.size_bytes());

  // Extract and advance past an array of 0xc-byte data structures
  variable_blobs_span variable_blobs =
      as_span(buffer.first(Variable::blob_size * header.variable_count),
              dim<>(header.variable_count), dim<Variable::blob_size>());
  buffer = buffer.subspan(variable_blobs.size_bytes());

  // All offsets are relative to the whole file
  buffer = storage;

  // A blob and a variable string per scene
  read_scene_data(scenes_, scene_blobs, scene_string_offsets, buffer);

  // A fixed string per table1 entry
  fixed_strings_span table1_string_buffers =
      as_span(buffer.subspan(header.table1_strings_offset,
                             fixed_string_size * header.table1_count),
              dim<>(header.table1_count), dim<fixed_string_size>());
  read_table1_data(table1_, table1_string_buffers);

  // A blob and a pair of fixed strings per variable
  fixed_string_pairs_span variable_strings_buffers =
      as_span(buffer.subspan(header.variable_names_offset,
                             fixed_string_size * 2 * header.variable_count),
              dim<>(header.variable_count), dim<2>(), dim<fixed_string_size>());
  read_variable_data(variables_, variable_blobs, variable_strings_buffers);

// Here on are all pairs of fixed strings
#define READ_ASSET_STRINGS(ASSETTYPE, STORAGE)                              \
  \
fixed_string_pairs_span ASSETTYPE##_strings_buffers = as_span(              \
      buffer.subspan(header.ASSETTYPE##_names_offset,                       \
                     header.ASSETTYPE##_count * fixed_string_size * 2),     \
      dim<>(header.ASSETTYPE##_count), dim<2>(), dim<fixed_string_size>()); \
  \
read_asset_strings(STORAGE, ASSETTYPE##_strings_buffers);

  READ_ASSET_STRINGS(BG, bg_names_);
  READ_ASSET_STRINGS(CHR, chr_names_);
  READ_ASSET_STRINGS(SE, se_names_);
  READ_ASSET_STRINGS(BGM, bgm_names_);
  READ_ASSET_STRINGS(VOICE, voice_names_);

#undef READ_ASSET_STRINGS

  return true;
}
