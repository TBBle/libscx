#include <iostream>
using std::cerr;
using std::cout;
#include <string>
using std::string;

#include "scx.hpp"

int main(void) {
  SCXFile scxfile;

  const auto sourceFile = string{"../../avking.scx"};

  if (!scxfile.read(sourceFile)) {
    cerr << "Failed to read source: " << sourceFile << "\n";
    return 1;
  }

  /*
   * PO File format:
   * https://www.gnu.org/software/gettext/manual/gettext.html#PO-Files
   */

  cout << "# AVKing Scenario Translation\n";
  cout << "msgid \"\"\n";
  cout << "msgstr \"\"\n";
  cout << "\"Language: ja\\n\"\n";
  cout << "\"Content-Type: text/plain; charset=utf-8\\n\"\n";
  cout << "\"Content-Transfer-Encoding: 8bit\\n\"\n";
  cout << "\n";

  // TODO: msgid is a unique key, and some duplicate (Command-only?) scenes
  // exist
  for (std::size_t i = 0; i < scxfile.scene_count(); ++i) {
    const auto& scene = scxfile.scene(i);
    if (scene.text.empty()) {
      continue;
    }
    // TODO: Skip scenes that contain only commands.
    // TODO: Split dialog lines at commands? Otherwise, need to escape the
    // commands since the \n and \r commands raise warnings in Poedit
    cout << "#. Chapter " << scene.chapter << " Scene " << scene.scene << "\n";
    cout << "#: Scene(" << i << ").text\n";
    cout << "msgid \"" << scene.text << "\"\n";
    cout << "msgstr \"\"\n\n";
  }
  return 0;
}
