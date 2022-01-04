#include "application.hpp"

int main(int argc, char* argv[]) {
  application::init();
  application::run();
  application::free();
}
