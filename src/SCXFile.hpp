#include "AssetName.hpp"
#include "Scene.hpp"
#include "Table1Data.hpp"
#include "Variable.hpp"

#include <string>
#include <vector>

class SCXFile {
 public:
  SCXFile();

  bool read(const std::string& fileName);
  bool write(const std::string& fileName);

  std::size_t scene_count() const { return scenes_.size(); }
  std::size_t table1_count() const { return table1_.size(); }
  std::size_t variable_count() const { return variables_.size(); }
  std::size_t bg_count() const { return bg_names_.size(); }
  std::size_t chr_count() const { return chr_names_.size(); }
  std::size_t se_count() const { return se_names_.size(); }
  std::size_t bgm_count() const { return bgm_names_.size(); }
  std::size_t voice_count() const { return voice_names_.size(); }

  const Scene& scene(std::size_t index) const { return scenes_[index]; }
  const Table1Data& table1(std::size_t index) const { return table1_[index]; }
  const Variable& variable(std::size_t index) const {
    return variables_[index];
  }
  const AssetName& bg(std::size_t index) const { return bg_names_[index]; }
  const AssetName& chr(std::size_t index) const { return chr_names_[index]; }
  const AssetName& se(std::size_t index) const { return se_names_[index]; }
  const AssetName& bgm(std::size_t index) const { return bgm_names_[index]; }
  const AssetName& voice(std::size_t index) const {
    return voice_names_[index];
  }

 private:
  std::vector<Scene> scenes_;
  std::vector<Table1Data> table1_;
  std::vector<Variable> variables_;
  std::vector<AssetName> bg_names_;
  std::vector<AssetName> chr_names_;
  std::vector<AssetName> se_names_;
  std::vector<AssetName> bgm_names_;
  std::vector<AssetName> voice_names_;
};
