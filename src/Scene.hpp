#include <cstdint>
#include <string>

#include <boost/array.hpp>

struct Scene {
 public:
  // Reading API
  void read_data(const std::uint8_t* pStringData,
                 const std::uint8_t* pBlobData);
  // Writing API: Not ready yet.
  std::size_t stringSize() const;
  std::size_t write_data(std::uint8_t* pStringData,
                         std::uint8_t* pBlobData) const;

  std::string text;  // utf-8 encoded
  boost::array<std::uint8_t, 0xdb> info_blob;
};
