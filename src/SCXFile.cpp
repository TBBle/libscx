#include "SCXFile.hpp"

#include <array>
using std::array;
#include <fstream>
using std::filebuf;
#include <ios>
using std::ios_base;
#include <memory>
using std::unique_ptr;
#include <string>
using std::string;
#include <utility>
using std::pair;
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

#include <gsl/gsl>
using gsl::as_multi_span;
using gsl::byte;
using gsl::dim;
using gsl::dynamic_range;
using gsl::narrow_cast;
using gsl::multi_span;

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

// All following data is encrypted
struct SCXFileHeader {
  const static size_t size = 0x3c;
  const static size_t offset = 0x08;

  enum fixed_strings : size_t {
    table1,
    variable,
    BG,
    CHR,
    SE,
    BGM,
    VOICE,
    COUNT,
  };

  uint32_t scene_count;
  std::array<uint32_t, COUNT> counts;
  std::array<uint32_t, COUNT> offsets;
};

static_assert(sizeof(SCXFileHeader) == SCXFileHeader::size,
              "SCXFileHeader did not pack correctly");

using scene_blobs_span =
    multi_span<const byte, dynamic_range, Scene::blob_size>;

void read_scene_data(vector<Scene>& scene_data, scene_blobs_span scene_blobs,
                     multi_span<const uint32_t> scene_string_offsets,
                     multi_span<const byte> buffer) {
  Expects(scene_blobs.extent() == scene_string_offsets.extent());
  scene_data.resize(scene_blobs.extent());
  for (size_t i = 0; i < scene_data.size(); ++i) {
    auto& scene = scene_data[i];
    const auto& blob = scene_blobs[i];
    auto offset = scene_string_offsets[i];
    // XXX: This breaks if buffer[offset] does not point to a null-terminated
    // string!
    auto pString =
        offset ? &as_multi_span<const char>(buffer).data()[offset] : nullptr;
    scene.read_data(pString, blob);
  }
}

using scene_blobs_writeable_span =
    multi_span<byte, dynamic_range, Scene::blob_size>;
using scene_storage_data =
    vector<pair<array<byte, Scene::blob_size>, unique_ptr<string>>>;

void write_scene_data(const scene_storage_data& scene_data,
                      scene_blobs_writeable_span scene_blobs,
                      multi_span<uint32_t> scene_text_offsets,
                      multi_span<byte> scene_text_storage,
                      uint32_t base_offset) {
  Expects(scene_data.size() == scene_blobs.extent());
  Expects(scene_data.size() == scene_text_offsets.extent());
  for (size_t i = 0; i < scene_data.size(); ++i) {
    auto blob = scene_blobs[i];
    auto& offset = scene_text_offsets[i];
    auto& data = scene_data[i];
    memcpy(blob.data(), data.first.data(), Scene::blob_size);
    if (!data.second) {
      offset = 0;
      continue;
    }
    auto& text = *data.second;
    uint32_t buffSize = narrow_cast<uint32_t>(text.size() + 1);
    assert(scene_text_storage.size_bytes() >= buffSize);
    memcpy(scene_text_storage.data(), text.c_str(), buffSize);
    offset = base_offset;
    base_offset += buffSize;
    scene_text_storage = scene_text_storage.subspan(buffSize);
  }
  Ensures(scene_text_storage.size_bytes() == 0);
}

static const uint32_t fixed_string_size = 0x20;
using fixed_strings_span =
    multi_span<const byte, dynamic_range, fixed_string_size>;

void read_table1_data(vector<Table1Data>& table1_data,
                      fixed_strings_span table1_strings) {
  table1_data.resize(table1_strings.extent());
  for (size_t i = 0; i < table1_data.size(); ++i) {
    auto& table1_entry = table1_data[i];
    table1_entry.read_data(table1_strings[i]);
  }
}

using fixed_strings_writeable_span =
    multi_span<byte, dynamic_range, fixed_string_size>;

void write_table1_data(const vector<Table1Data>& table1_data,
                       fixed_strings_writeable_span table1_strings) {
  Expects(table1_data.size() == table1_strings.extent());
  for (size_t i = 0; i < table1_data.size(); ++i) {
    const auto& table1_entry = table1_data[i];
    table1_entry.write_data(table1_strings[i]);
  }
}

using variable_blobs_span =
    multi_span<const byte, dynamic_range, Variable::blob_size>;
using fixed_string_pairs_span =
    multi_span<const byte, dynamic_range, 2, fixed_string_size>;

void read_variable_data(vector<Variable>& variable_data,
                        variable_blobs_span variable_blobs,
                        fixed_string_pairs_span variable_strings_buffers) {
  Expects(variable_blobs.extent() == variable_strings_buffers.extent());
  variable_data.resize(variable_blobs.extent());
  for (size_t i = 0; i < variable_data.size(); ++i) {
    auto& variable = variable_data[i];
    auto strings = variable_strings_buffers[i];
    variable.read_data(strings[0], strings[1], variable_blobs[i]);
  }
}

using variable_blobs_writeable_span =
    multi_span<byte, dynamic_range, Variable::blob_size>;
using fixed_string_pairs_writeable_span =
    multi_span<byte, dynamic_range, 2, fixed_string_size>;

void write_variable_data(
    const vector<Variable>& variable_data,
    variable_blobs_writeable_span variable_blobs,
    fixed_string_pairs_writeable_span variable_strings_buffers) {
  Expects(variable_data.size() == variable_blobs.extent());
  Expects(variable_data.size() == variable_strings_buffers.extent());
  size_t index = 0;
  for (const auto& variable : variable_data) {
    auto blob = variable_blobs[index];
    // sunspan always returns a single-dimensional span...
    // variable_blobs = variable_blobs.subspan(1);
    auto strings = variable_strings_buffers[index];
    // variable_strings_buffers = variable_strings_buffers.subspan(1);
    variable.write_data(strings[0], strings[1], blob);
    ++index;
  }
  Ensures(variable_blobs.extent() == index);
  Ensures(variable_strings_buffers.extent() == index);
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

void write_asset_strings(
    const vector<AssetName>& asset_data,
    fixed_string_pairs_writeable_span asset_strings_buffers) {
  Expects(asset_data.size() == asset_strings_buffers.extent());
  size_t index = 0;
  for (const auto& asset : asset_data) {
    auto strings = asset_strings_buffers[index];
    asset.write_data(strings[0], strings[1]);
    ++index;
  }
  Ensures(asset_strings_buffers.extent() == index);
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

  multi_span<const byte> buffer(storage);

  // Extract and advance past an SCXFileIdentifier
  const auto& ident = as_multi_span<SCXFileIdentifier>(
      buffer.first<sizeof(SCXFileIdentifier)>())[0];
  buffer = buffer.subspan(sizeof(SCXFileIdentifier));

  if (memcmp(&ident.fileprefix, "scx\0", 4)) {
    return false;
  }

  // Routine at 0x4352a0 in the binary does the checksum and decrypting
  auto encrypted = multi_span<byte>(storage).subspan(sizeof(SCXFileIdentifier));
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
      as_multi_span<SCXFileHeader>(buffer.first<sizeof(SCXFileHeader)>())[0];
  buffer = buffer.subspan(sizeof(SCXFileHeader));

  // Extract and advance past a table of uint32 offsets to variable-sized string
  // data
  auto scene_string_offsets = as_multi_span<const uint32_t>(
      buffer.first(sizeof(uint32_t) * header.scene_count));
  buffer = buffer.subspan(scene_string_offsets.size_bytes());

  // Extract and advance past an array of 0xd8-byte data structures
  scene_blobs_span scene_blobs =
      as_multi_span(buffer.first(Scene::blob_size * header.scene_count),
                    dim<>(header.scene_count), dim<Scene::blob_size>());
  buffer = buffer.subspan(scene_blobs.size_bytes());

  // Extract and advance past an array of 0xc-byte data structures
  variable_blobs_span variable_blobs =
      as_multi_span(buffer.first(Variable::blob_size *
                                 header.counts[SCXFileHeader::variable]),
                    dim<>(header.counts[SCXFileHeader::variable]),
                    dim<Variable::blob_size>());
  buffer = buffer.subspan(variable_blobs.size_bytes());

  // All offsets are relative to the whole file
  buffer = storage;

  // A blob and a variable string per scene
  read_scene_data(scenes_, scene_blobs, scene_string_offsets, buffer);

  // A fixed string per table1 entry
  fixed_strings_span table1_string_buffers = as_multi_span(
      buffer.subspan(header.offsets[SCXFileHeader::table1],
                     fixed_string_size * header.counts[SCXFileHeader::table1]),
      dim<>(header.counts[SCXFileHeader::table1]), dim<fixed_string_size>());
  read_table1_data(table1_, table1_string_buffers);

  // A blob and a pair of fixed strings per variable
  fixed_string_pairs_span variable_strings_buffers =
      as_multi_span(buffer.subspan(header.offsets[SCXFileHeader::variable],
                                   fixed_string_size * 2 *
                                       header.counts[SCXFileHeader::variable]),
                    dim<>(header.counts[SCXFileHeader::variable]), dim<2>(),
                    dim<fixed_string_size>());
  read_variable_data(variables_, variable_blobs, variable_strings_buffers);

// Here on are all pairs of fixed strings
#define READ_ASSET_STRINGS(ASSETTYPE, STORAGE)                                \
  fixed_string_pairs_span ASSETTYPE##_strings_buffers =                       \
      as_multi_span(buffer.subspan(header.offsets[SCXFileHeader::ASSETTYPE],  \
                                   header.counts[SCXFileHeader::ASSETTYPE] *  \
                                       fixed_string_size * 2),                \
                    dim<>(header.counts[SCXFileHeader::ASSETTYPE]), dim<2>(), \
                    dim<fixed_string_size>());                                \
  read_asset_strings(STORAGE, ASSETTYPE##_strings_buffers);

  READ_ASSET_STRINGS(BG, bg_names_);
  READ_ASSET_STRINGS(CHR, chr_names_);
  READ_ASSET_STRINGS(SE, se_names_);
  READ_ASSET_STRINGS(BGM, bgm_names_);
  // Voice file names are not stored in this file.
  assert(header.offsets[SCXFileHeader::VOICE] == 0);
  voice_names_.resize(header.counts[SCXFileHeader::VOICE]);
// READ_ASSET_STRINGS(VOICE, voice_names_);

#undef READ_ASSET_STRINGS

  return true;
}

bool SCXFile::write(const string& fileName) {
  // How this will work:
  /* Roughly, we have the fixed-size data structures, then the var-len strings,
   * then the fixed-size strings.
   * The first part is known-size:
   *  SCXFileIdentifier
   *  SCXFileHeader
   *  uint32 * scene count
   *  0xd8 blob * scene_count
   *  0xc blob * variable_count
   *
   * Then the scene text buffer
   *
   * Then another known-size part:
   *  0x20 string * table1_count
   *  0x40 stringpair * variable_count
   *  0x40 stringpair * BG_count
   *  0x40 stringpair * CHR_count
   *  0x40 stringpair * SE_count
   *  0x40 stringpair * BGM_count
   *  0x40 stringpair * VOICE_count
   *
   * By the file format, the scene text could appear after the other text, it's
   * all offsets here, but this ordering matches the distributed version.
   */

  // Calculate the size of the buffer needed for all the data
  const size_t pre_text_size =
      sizeof(SCXFileIdentifier) + sizeof(SCXFileHeader) +
      scenes_.size() * sizeof(uint32_t) + scenes_.size() * Scene::blob_size +
      variables_.size() * Variable::blob_size;

  const size_t post_text_size =
      fixed_string_size *
      (table1_.size() + variables_.size() * 2 + bg_names_.size() * 2 +
       chr_names_.size() * 2 + se_names_.size() * 2 + bgm_names_.size() * 2);

  // Start with some storage to collect all the scene data, as that is the only
  // part that varies in size.
  scene_storage_data scene_data(scenes_.size());

  size_t scene_text_size_total = 0;
  for (size_t i = 0; i < scenes_.size(); ++i) {
    const auto& scene = scenes_[i];
    auto& output = scene_data[i];
    output.second = scene.write_data(output.first);
    if (output.second) {
      // Seems to be a bug in the game client if this does not hold
      // TODO: Test this and see if simple '\0'-padding fixes it.
      // Alternatively, could be a bug with narrow-width ASCII rendering?
      // TODO: Actually, this is not accurate. The _text_ (i.e. not things in
      // []) probably needs to be a multiple of 2 bytes. It's probably a bug in
      // the command-parser.
      //      assert(output.second->size() % 2 == 0);

      // Need to allow for the null
      scene_text_size_total += output.second->size() + 1;
    }
  }

  vector<byte> storage(pre_text_size + scene_text_size_total + post_text_size);

  // Now we can just write all the data out.
  multi_span<byte> buffer(storage);

  // Take a reference to and advance past the SCXFileIdentifier
  auto& ident = as_multi_span<SCXFileIdentifier>(
      buffer.first<sizeof(SCXFileIdentifier)>())[0];
  buffer = buffer.subspan(sizeof(SCXFileIdentifier));

  ident.fileprefix[0] = 's';
  ident.fileprefix[1] = 'c';
  ident.fileprefix[2] = 'x';
  ident.fileprefix[3] = '\0';

  // Take a reference to and advance past the SCXFileHeader
  auto& header =
      as_multi_span<SCXFileHeader>(buffer.first<sizeof(SCXFileHeader)>())[0];
  buffer = buffer.subspan(sizeof(SCXFileHeader));

  // Fill in the counts, since we have those now
  header.scene_count = narrow_cast<uint32_t>(scenes_.size());
  header.counts[SCXFileHeader::table1] = narrow_cast<uint32_t>(table1_.size());
  header.counts[SCXFileHeader::variable] =
      narrow_cast<uint32_t>(variables_.size());
  header.counts[SCXFileHeader::BG] = narrow_cast<uint32_t>(bg_names_.size());
  header.counts[SCXFileHeader::CHR] = narrow_cast<uint32_t>(chr_names_.size());
  header.counts[SCXFileHeader::SE] = narrow_cast<uint32_t>(se_names_.size());
  header.counts[SCXFileHeader::BGM] = narrow_cast<uint32_t>(bgm_names_.size());
  header.counts[SCXFileHeader::VOICE] =
      narrow_cast<uint32_t>(voice_names_.size());

  // Take a reference to and advance past a table of uint32 offsets to
  // variable-sized string data
  auto scene_string_offsets = as_multi_span<uint32_t>(
      buffer.first(sizeof(uint32_t) * header.scene_count));
  buffer = buffer.subspan(scene_string_offsets.size_bytes());

  // Take a reference to and advance past an array of 0xd8-byte data structures
  scene_blobs_writeable_span scene_blobs =
      as_multi_span(buffer.first(Scene::blob_size * header.scene_count),
                    dim<>(header.scene_count), dim<Scene::blob_size>());
  buffer = buffer.subspan(scene_blobs.size_bytes());

  // Take a reference to and advance past an array of 0xc-byte data structures
  variable_blobs_writeable_span variable_blobs =
      as_multi_span(buffer.first(Variable::blob_size *
                                 header.counts[SCXFileHeader::variable]),
                    dim<>(header.counts[SCXFileHeader::variable]),
                    dim<Variable::blob_size>());
  buffer = buffer.subspan(variable_blobs.size_bytes());

  assert(buffer.size_bytes() == scene_text_size_total + post_text_size);

  // Take a reference to and advance past the variably-sized string buffer
  auto scene_text_blob = as_multi_span(buffer.first(scene_text_size_total),
                                       dim<>(scene_text_size_total));
  buffer = buffer.subspan(scene_text_blob.size_bytes());

  assert(buffer.size_bytes() == post_text_size);

  write_scene_data(scene_data, scene_blobs, scene_string_offsets,
                   scene_text_blob, narrow_cast<uint32_t>(pre_text_size));

  // Take a reference to and advance past each of the fixed-size-string buffers
  header.offsets[SCXFileHeader::table1] =
      narrow_cast<uint32_t>(storage.size() - buffer.size_bytes());
  fixed_strings_writeable_span table1_string_buffers = as_multi_span(
      buffer.first(fixed_string_size * header.counts[SCXFileHeader::table1]),
      dim<>(header.counts[SCXFileHeader::table1]), dim<fixed_string_size>());
  buffer = buffer.subspan(table1_string_buffers.size_bytes());

  write_table1_data(table1_, table1_string_buffers);

  header.offsets[SCXFileHeader::variable] =
      narrow_cast<uint32_t>(storage.size() - buffer.size_bytes());
  fixed_string_pairs_writeable_span variable_strings_buffers =
      as_multi_span(buffer.first(fixed_string_size * 2 *
                                 header.counts[SCXFileHeader::variable]),
                    dim<>(header.counts[SCXFileHeader::variable]), dim<2>(),
                    dim<fixed_string_size>());
  buffer = buffer.subspan(variable_strings_buffers.size_bytes());

  write_variable_data(variables_, variable_blobs, variable_strings_buffers);

#define WRITE_ASSET_STRINGS(ASSETTYPE, STORAGE)                               \
  header.offsets[SCXFileHeader::ASSETTYPE] =                                  \
      narrow_cast<uint32_t>(storage.size() - buffer.size_bytes());            \
  fixed_string_pairs_writeable_span ASSETTYPE##_strings_buffers =             \
      as_multi_span(buffer.first(fixed_string_size * 2 *                      \
                                 header.counts[SCXFileHeader::ASSETTYPE]),    \
                    dim<>(header.counts[SCXFileHeader::ASSETTYPE]), dim<2>(), \
                    dim<fixed_string_size>());                                \
  buffer = buffer.subspan(ASSETTYPE##_strings_buffers.size_bytes());          \
  write_asset_strings(STORAGE, ASSETTYPE##_strings_buffers);

  WRITE_ASSET_STRINGS(BG, bg_names_);
  WRITE_ASSET_STRINGS(CHR, chr_names_);
  WRITE_ASSET_STRINGS(SE, se_names_);
  WRITE_ASSET_STRINGS(BGM, bgm_names_);
// Voice file names are not stored in this file.
// WRITE_ASSET_STRINGS(VOICE, voice_names_);

#undef WRITE_ASSET_STRINGS

  assert(buffer.size_bytes() == 0);

  auto encrypted = multi_span<byte>(storage).subspan(sizeof(SCXFileIdentifier));
  uint32_t calc = 0;
  for (ptrdiff_t i = 0; i < encrypted.extent(); ++i) {
    // Encrypt: Unsigned math
    reinterpret_cast<uint8_t&>(encrypted[i]) ^= xor_key(i);
    // Checksum: Signed math
    calc += static_cast<int8_t>(encrypted[i]);
  }

  ident.checksum = calc;

  // Create a new file of the desired size
  {
    filebuf fbuf;
    fbuf.open(fileName.c_str(), ios_base::in | ios_base::out | ios_base::trunc |
                                    ios_base::binary);
    fbuf.pubseekoff(storage.size() - 1, ios_base::beg);
    fbuf.sputc('\0');
  }

  file_mapping file(fileName.c_str(), read_write);
  mapped_region region(file, read_write);
  void* addr = region.get_address();
  size_t size = region.get_size();

  assert(size == storage.size());

  memcpy(addr, storage.data(), size);

  return true;
}
