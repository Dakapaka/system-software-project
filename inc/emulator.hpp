#ifndef EMULATOR_HPP
#define EMULATOR_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <bitset>

using namespace std;

class Emulator{
  public:

    const int SIZE = 65536;
    bool running;
    vector<unsigned char> memory;
    short r[8];
    short psw;
  
    Emulator();
    void emulate(string input);
    void dataAddress(unsigned short ins, int i, int j);
    void jumpAddress(unsigned short ins, int j);
    void printOutput(string s);
};

#endif