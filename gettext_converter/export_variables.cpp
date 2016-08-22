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

  // TODO: msgid is a unique key, and some duplicate comments exist.
  for (std::size_t i = 0; i < scxfile.variable_count(); ++i) {
    const auto& variable = scxfile.variable(i);
    cout << "#: Variable(" << i << ").name\n";
    cout << "msgid \"" << variable.name << "\"\n";
    cout << "msgstr \"\"\n\n";
    if (!variable.comment.empty()) {
      cout << "#: Variable(" << i << ").comment\n";
      cout << "msgid \"" << variable.comment << "\"\n";
      cout << "msgstr \"\"\n\n";
    }
  }
  return 0;
}
