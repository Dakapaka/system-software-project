#ifndef LINKER_HPP
#define LINKER_HPP
#include <iostream>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <string>
#include <bitset>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cctype>

using namespace std;

enum visibilityLinker{
  undfL, locL, glbL, extL, sectionL
};

enum relTypeLinker{
  pcL, absoluteL
};

struct RelocationTableLinker{
  int id;
  int offset;
  string section;
  relTypeLinker type;
  string symbol;
  int addend;
  string file;
};

struct SymbolTableLinker{

  int id;
  string name;
  visibilityLinker vis;
  string section;
  int offset;
  int defined;
  string file;

};

struct Section{
  int end;
  int oldId;
  int id;
  string name;
  int size;
  vector<string> data;
  string file;
};

struct SymTableDescriptor{
  int id;
  string file;
};

class Linker{

  public:

    Linker();
    vector<unordered_map<string, SymbolTableLinker>> allSymTables;
    vector<unordered_map<string, vector<string>>> allSecTables;
    vector<unordered_map<string, unordered_map<int, RelocationTableLinker>>> allRelTables;
    vector<Section> sections;
    vector<Section> sectionsLost;
    unordered_map<string, SymbolTableLinker> symTable;
    vector<SymTableDescriptor> descriptors;
    vector<string> output;

    int cntSections = 0;
    int cntSymbols = 0;

    void readAssemblerOutput(vector<string> outputs);
    void printAllRelocTables();
    void printAllSymTables();
    void printAllSecTables();
    void printSections();
    void printSectionsByFile();
    void printSymbolTable();
    void createGlobalSymbolTable();
    void relocate();
    void writeOutput(string output);

};
#endif