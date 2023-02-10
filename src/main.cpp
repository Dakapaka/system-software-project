#include "../inc/assembler.hpp"

extern int pmain(Assembler* a, string input);

int main(int argc, char** argv){

  string output, input;

  output = argv[2];
  input = argv[3];

  Assembler *a = new Assembler();

  pmain(a, input);

  /*cout << "Symbol table" << endl;
  a->printSymTable();
  cout << "Section tables" << endl;
  a->printSecTables();
  cout << "Relocation tables" << endl;
  a->printRelocTables();*/

  a->writeSymTable(output);
  a->writeSecTables(output);
  a->writeRelocTables(output);
  
  return 0;
}