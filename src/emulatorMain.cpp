#include "../inc/emulator.hpp"

int main(int argc, char** argv){

  Emulator* e = new Emulator();

  string input = argv[1];

  e->emulate(input);

  return 0;
}