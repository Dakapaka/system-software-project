#include <iostream>
#include <vector>
#include "../inc/linker.hpp"

using namespace std;

int main(int argc, char** argv){

  Linker *s = new Linker();

  vector<string> outputs;
  string hex;

  for(int i = 4; i < argc; i++){
    outputs.push_back(argv[i]);
  }

  hex = argv[3];

  s->readAssemblerOutput(outputs);

  /*cout << "All Symbol tables" << endl;
  s->printAllSymTables();
  cout << "All Section tables" << endl;
  s->printAllSecTables();
  cout << "All Relocation tables" << endl;
  s->printAllRelocTables();
  cout << "Mapped sections" << endl;
  s->printSections();
  cout << "Mapped sections by file" << endl;
  s->printSectionsByFile();*/
  s->createGlobalSymbolTable();
  /*cout << "Global Symbol Table" << endl;
  s->printSymbolTable();*/
  s->relocate();
  /*cout << "Output" << endl;*/
  s->writeOutput(hex);
  
  return 0;
}