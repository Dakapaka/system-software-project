#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP
#include <iostream>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <string>
#include <bitset>
#include <fstream>

using namespace std;

enum visibility{
  undf, loc, glb, ext, section
};

enum instruction{
  NONE_INS, HALT_INS, INT_INS, IRET_INS, CALL_INS, RET_INS, JMP_INS, JEQ_INS, JNE_INS, JGT_INS, PUSH_INS, POP_INS, XCHG_INS, ADD_INS, SUB_INS, MUL_INS, DIV_INS, CMP_INS, NOT_INS, AND_INS, OR_INS, XOR_INS, TEST_INS, SHL_INS, SHR_INS, LDR_INS, STR_INS
};

enum dataAddress{
  none, immedDec, immedHex, immedSym, memDec, memHex, memSym, pcSym, regdir, regind, regindoffDec, regindoffHex, regindoffSym
};

enum memAddress{
  none_mem, immedDec_mem, immedHex_mem, immedSym_mem, memDec_mem, memHex_mem, memSym_mem, pcSym_mem, regdir_mem, regind_mem, regindoffDec_mem, regindoffHex_mem, regindoffSym_mem
};

enum relType{
  pc, absolute
};

struct SymbolTable{

  int id;
  string name;
  visibility vis;
  string section;
  int offset;
  int defined;

};

struct Registers{
  int first = -1;
  int second = -1;
};

struct RelocationTable{
  int id = 0;
  int offset = 0;
  string section = "";
  relType type;
  string symbol = "";
  int addend = 0;
};

class Assembler{

  public:

    unordered_map<string, SymbolTable> symTable;
    unordered_map<string, vector<string>> sectionTable;
    unordered_map<string, unordered_map<int, RelocationTable>> relTable;
    Registers regs;
    instruction cur_ins = NONE_INS;
    dataAddress cur_data_addr = none;
    memAddress cur_mem_addr = none_mem;

    visibility cur_vis = undf;
    string cur_section = "NONE";
    int loc_cnt = 0;
    int cntSymbols = 0;
    int cntRelocations = 0;
    int arg = 0;
    string cur_symbol;

    Assembler();
    void setSection(string section);
    void setVisibility(string vis);
    void addSymbol(string symbol);
    int hexToInt(string hex);
    void pushInstruction();
    void popInstruction();
    void xchgInstruction();
    void addInstruction();
    void subInstruction();
    void mulInstruction();
    void divInstruction();
    void cmpInstruction();
    void notInstruction();
    void andInstruction();
    void orInstruction();
    void xorInstruction();
    void testInstruction();
    void shlInstruction();
    void shrInstruction();
    void haltInstruction();
    void retInstruction();
    void iretInstruction();
    void intInstruction();
    void ldrInstruction();
    void strInstruction();
    void callInstruction();
    void jmpInstruction();
    void jeqInstruction();
    void jneInstruction();
    void jgtInstruction();
    void executeInstruction();
    void printSymTable();
    void printSecTables();
    void printRelocTables();
    void reg0();
    void reg1();
    void reg2();
    void reg3();
    void reg4();
    void reg5();
    void reg6();
    void reg7();
    void reg8();
    void skipDirective(int param);
    void wordDirective(int param, string s);
    void setDataAddr(dataAddress d, int a, string s);
    void setMemAddr(memAddress m, int a, string s);
    void dataAddressFunction();
    void jumpAddressFunction();
    void writeSymTable(string filename);
    void writeSecTables(string filename);
    void writeRelocTables(string filename);

};
#endif