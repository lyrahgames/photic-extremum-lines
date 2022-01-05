#include <fstream>
//
#include "application.hpp"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    cout << "usage:\n" << argv[0] << " <STL object file path>\n";
    return 0;
  }
  application::init();
  application::load_model(argv[1]);
  application::run();
}
