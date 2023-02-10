#include "../inc/assembler.hpp"

Assembler::Assembler() : arg(0), loc_cnt(0), cntSymbols(0), cur_section("NONE"), cur_ins(NONE_INS) {}

void Assembler::setSection(string section)
{
  cur_section = section;
  loc_cnt = 0;

  unordered_map<int, RelocationTable> newRelMap;
  relTable[cur_section] = newRelMap;
}

void Assembler::setVisibility(string vis)
{

  if (vis == "GLOBAL")
  {
    cur_vis = glb;
  }

  else if(vis == "EXTERN")
  {
    cur_vis = ext;
  }

  else if(vis == "LOCAL"){
    cur_vis = loc;
  }

  else{
    cur_vis = undf;
  }

}

void Assembler::addSymbol(string symbol)
{
  //micanje : iz labele
  if(cur_vis == loc){
    symbol.pop_back();
  }

  //nema simbola u tabeli
  if(symTable.find(symbol) == symTable.end()){
    SymbolTable newSymbol;
    newSymbol.id = ++cntSymbols;
    newSymbol.name = symbol;
    newSymbol.vis = cur_vis;
    if(cur_vis == undf || cur_vis == ext || cur_vis == glb){
      newSymbol.section = "NONE";
      newSymbol.offset = -1;
      newSymbol.defined = 0;
    }
    else{
      newSymbol.section = cur_section;
      newSymbol.offset = loc_cnt;
      newSymbol.defined = 1;
    }
    if(newSymbol.name == cur_section){
      newSymbol.section = "NONE";
      newSymbol.offset = -1;
      newSymbol.defined = 0;
      newSymbol.vis = section;
    }
    symTable[symbol] = newSymbol;
  }

  //simbol je u tabeli simbola
  else{
    SymbolTable curSymbol = symTable[symbol];
    if(!curSymbol.defined){
      if(curSymbol.vis == glb || curSymbol.vis == ext){
        if(cur_vis == loc){
          curSymbol.defined = 1;
          curSymbol.offset = loc_cnt;
          curSymbol.section = cur_section;
        }
      }
      else if(curSymbol.vis == undf){
        if(cur_vis == loc){
          curSymbol.defined = 1;
          curSymbol.offset = loc_cnt;
          curSymbol.section = cur_section;
          for (auto &outerMap : relTable) {
            for (auto &innerMap : outerMap.second) {
              if(innerMap.second.symbol == curSymbol.name){
                innerMap.second.addend += loc_cnt;
                innerMap.second.symbol = curSymbol.section;
              }
            }
          }
        }
      }
    }
    else{
      if(cur_vis == loc){
        cout << "Error, symbol " << curSymbol.name << " already defined!" << endl;
      }
    }
    symTable[symbol] = curSymbol;
  }
}

int Assembler::hexToInt(string hex)
{
  int res = 0;
    
	for(int i = hex.size() - 1; i > 1; i--){
		int cur = hex[i] - '0';
		if(hex[i] == 'A') cur = 10;
		if(hex[i] == 'B') cur = 11;
		if(hex[i] == 'C') cur = 12;
		if(hex[i] == 'D') cur = 13;
		if(hex[i] == 'E') cur = 14;
		if(hex[i] == 'F') cur = 15;
		int mul = 1;
		if(i == hex.size() - 1){
			mul = 1;
		}
		else{
			for(int j = 0; j < (hex.size() - i - 1); j++){
				mul *= 16;
			}
		}
		res += mul * cur;
	}

	return res;
}

void Assembler::printSymTable()
{
  for (const auto& item : symTable) {
    string strKey = item.first;
    SymbolTable myVal = item.second;
    cout << myVal.id << " " << myVal.name << " " << myVal.vis << " " << myVal.section << " " << myVal.offset << " " << myVal.defined << endl;
  }
}

void Assembler::printSecTables(){
  for (const auto& item : sectionTable) {
    string strKey = item.first;
    vector<string> myVal = item.second;

    cout << strKey << ":" << endl;

    for(string s : myVal){
      cout << s << " ";
    }
    cout << endl;
  }
}

void Assembler::printRelocTables()
{
  unordered_map<string, unordered_map<int, RelocationTable>>::iterator outerIter;
  for (outerIter = relTable.begin(); outerIter != relTable.end(); ++outerIter) {
    string outerKey = outerIter->first;
    unordered_map<int, RelocationTable> innerMap = outerIter->second;
    unordered_map<int, RelocationTable>::iterator innerIter;
    cout << outerKey << ":" << endl;
    for (innerIter = innerMap.begin(); innerIter != innerMap.end(); ++innerIter) {
      int innerKey = innerIter->first;
      RelocationTable relocationTable = innerIter->second;
      cout << relocationTable.offset << " " << relocationTable.type << " " << relocationTable.symbol << " " << relocationTable.section << " " << relocationTable.addend << endl;
    }
  }
}

string vectorToBinaryString(const vector<string>& v) {
    stringstream ss;
    for (const auto& s : v) {
        ss << bitset<8>(s.length()).to_string();
        for (const auto c : s) {
            ss << bitset<8>(c).to_string();
        }
        ss << ' ';
    }
    return ss.str();
}

void Assembler::writeSymTable(string filename){
  std::ofstream outputFile;
  outputFile.open(filename, std::ios::app);
  for (auto it = symTable.begin(); it != symTable.end(); it++) {
    SymbolTable sym = it->second;
    std::string idBinary = std::bitset<32>(sym.id).to_string();
    std::string offsetBinary = std::bitset<32>(sym.offset).to_string();
    std::string definedBinary = std::bitset<32>(sym.defined).to_string();
    std::string visBinary = std::bitset<32>(sym.vis).to_string();
    std::string nameBinary, sectionBinary;
    for (char c : sym.name) {
      nameBinary += std::bitset<8>(c).to_string();
    }
    for (char c : sym.section) {
      sectionBinary += std::bitset<8>(c).to_string();
    }
    std::string binary = idBinary + " " + offsetBinary + " " + definedBinary + " " + visBinary + " " + nameBinary + " " + sectionBinary;
    outputFile << binary << std::endl;
  }
  outputFile << endl;
  outputFile.close();
}

void Assembler::writeSecTables(string filename){
  std::ofstream outputFile;
  outputFile.open(filename, std::ios::app);
  for (const auto& sec : sectionTable) {
      string binaryString = vectorToBinaryString(sec.second);
      string keyString;
      for (char c : sec.first) {
        keyString += std::bitset<8>(c).to_string();
      }
      outputFile << keyString << " " << binaryString << endl;
  }
  outputFile << endl;
  outputFile.close();
}

void Assembler::writeRelocTables(string filename){
  std::ofstream outputFile;
  outputFile.open(filename, std::ios::app);
  for (auto it = relTable.begin(); it != relTable.end(); it++) {
    for(auto it2 = it->second.begin(); it2 != it->second.end(); it2++){
      RelocationTable rel = it2->second;
      std::string idBinary = std::bitset<32>(rel.id).to_string();
      std::string offsetBinary = std::bitset<32>(rel.offset).to_string();
      std::string addendBinary = std::bitset<32>(rel.addend).to_string();
      std::string typeBinary = std::bitset<32>(rel.type).to_string();
      std::string symbolBinary, sectionBinary;
      for (char c : rel.symbol) {
        symbolBinary += std::bitset<8>(c).to_string();
      }
      for (char c : rel.section) {
        sectionBinary += std::bitset<8>(c).to_string();
      }
      std::string binary = idBinary + " " + offsetBinary + " " + addendBinary + " " + typeBinary + " " + symbolBinary + " " + sectionBinary;
      outputFile << binary << std::endl;
    }
  }
  outputFile << endl;
  outputFile.close();
}

void Assembler::intInstruction()
{
  cur_ins = INT_INS;
}

void Assembler::pushInstruction()
{
  cur_ins = PUSH_INS;
}

void Assembler::popInstruction()
{
  cur_ins = POP_INS;
}

void Assembler::xchgInstruction()
{
  cur_ins = XCHG_INS;
}

void Assembler::addInstruction()
{
  cur_ins = ADD_INS;
}

void Assembler::subInstruction()
{
  cur_ins = SUB_INS;
}

void Assembler::mulInstruction()
{
  cur_ins = MUL_INS;
}

void Assembler::divInstruction()
{
  cur_ins = DIV_INS;
}

void Assembler::cmpInstruction()
{
  cur_ins = CMP_INS;
}

void Assembler::notInstruction()
{
  cur_ins = NOT_INS;
}

void Assembler::andInstruction()
{
  cur_ins = AND_INS;
}

void Assembler::orInstruction()
{
  cur_ins = OR_INS;
}

void Assembler::xorInstruction()
{
  cur_ins = XOR_INS;
}

void Assembler::testInstruction()
{
  cur_ins = TEST_INS;
}

void Assembler::shlInstruction()
{
  cur_ins = SHL_INS;
}

void Assembler::shrInstruction()
{
  cur_ins = SHR_INS;
}

void Assembler::haltInstruction()
{
  sectionTable[cur_section].push_back("00");
  loc_cnt++;
}

void Assembler::retInstruction()
{
  sectionTable[cur_section].push_back("40");
  loc_cnt++;
}

void Assembler::iretInstruction()
{
  sectionTable[cur_section].push_back("20");
  loc_cnt++;
}

void Assembler::ldrInstruction(){
  cur_ins = LDR_INS;
}

void Assembler::strInstruction(){
  cur_ins = STR_INS;
}

void Assembler::callInstruction(){
  cur_ins = CALL_INS;
}

void Assembler::jmpInstruction(){
  cur_ins = JMP_INS;
}

void Assembler::jeqInstruction(){
  cur_ins = JEQ_INS;
}

void Assembler::jneInstruction(){
  cur_ins = JNE_INS;
}

void Assembler::jgtInstruction(){
  cur_ins = JGT_INS;
}

void Assembler::executeInstruction(){
  string s, s1, s2;
  string result;
  string zero;
  stringstream stream;
  std::vector<std::string> hex_digits;
  int cnt = 0;
  switch(cur_ins){
    case INT_INS:
      sectionTable[cur_section].push_back("10");
      loc_cnt++;
      s = to_string(regs.first);
      s.push_back('f');
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      regs.first = -1;
      break;
    case PUSH_INS:
      sectionTable[cur_section].push_back("b0");
      loc_cnt++;
      s = to_string(regs.first);
      s.append("6");
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      regs.first = -1;
      sectionTable[cur_section].push_back("12");
      loc_cnt++;
      break;
    case POP_INS:
      sectionTable[cur_section].push_back("a0");
      loc_cnt++;
      s = to_string(regs.first);
      s.push_back('6');
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      regs.first = -1;
      sectionTable[cur_section].push_back("42");
      loc_cnt++;
      break;
    case NOT_INS:
      sectionTable[cur_section].push_back("80");
      loc_cnt++;
      s = to_string(regs.first);
      s.push_back('0');
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      regs.first = -1;
      break;
    case XCHG_INS:
      sectionTable[cur_section].push_back("60");
      loc_cnt++;
      s1 = to_string(regs.first);
      s2 = to_string(regs.second);
      s1.append(s2);
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      regs.first = regs.second = -1;
      break;
    case ADD_INS:
      sectionTable[cur_section].push_back("70");
      loc_cnt++;
      s1 = to_string(regs.first);
      s2 = to_string(regs.second);
      s1.append(s2);
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      regs.first = regs.second = -1;
      break;
    case SUB_INS:
      sectionTable[cur_section].push_back("71");
      loc_cnt++;
      s1 = to_string(regs.first);
      s2 = to_string(regs.second);
      s1.append(s2);
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      regs.first = regs.second = -1;
      break;
    case MUL_INS:
      sectionTable[cur_section].push_back("72");
      loc_cnt++;
      s1 = to_string(regs.first);
      s2 = to_string(regs.second);
      s1.append(s2);
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      regs.first = regs.second = -1;
      break;
    case DIV_INS:
      sectionTable[cur_section].push_back("73");
      loc_cnt++;
      s1 = to_string(regs.first);
      s2 = to_string(regs.second);
      s1.append(s2);
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      regs.first = regs.second = -1;
      break;
    case CMP_INS:
      sectionTable[cur_section].push_back("74");
      loc_cnt++;
      s1 = to_string(regs.first);
      s2 = to_string(regs.second);
      s1.append(s2);
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      regs.first = regs.second = -1;
      break;
    case AND_INS:
      sectionTable[cur_section].push_back("81");
      loc_cnt++;
      s1 = to_string(regs.first);
      s2 = to_string(regs.second);
      s1.append(s2);
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      regs.first = regs.second = -1;
      break;
    case OR_INS:
      sectionTable[cur_section].push_back("82");
      loc_cnt++;
      s1 = to_string(regs.first);
      s2 = to_string(regs.second);
      s1.append(s2);
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      regs.first = regs.second = -1;
      break;  
    case XOR_INS:
      sectionTable[cur_section].push_back("83");
      loc_cnt++;
      s1 = to_string(regs.first);
      s2 = to_string(regs.second);
      s1.append(s2);
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      regs.first = regs.second = -1;
      break;
    case TEST_INS:
      sectionTable[cur_section].push_back("84");
      loc_cnt++;
      s1 = to_string(regs.first);
      s2 = to_string(regs.second);
      s1.append(s2);
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      regs.first = regs.second = -1;
      break;
    case SHL_INS:
      sectionTable[cur_section].push_back("90");
      loc_cnt++;
      s1 = to_string(regs.first);
      s2 = to_string(regs.second);
      s1.append(s2);
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      regs.first = regs.second = -1;
      break;
    case SHR_INS:
      sectionTable[cur_section].push_back("91");
      loc_cnt++;
      s1 = to_string(regs.first);
      s2 = to_string(regs.second);
      s1.append(s2);
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      regs.first = regs.second = -1;
      break;
    case LDR_INS:
      sectionTable[cur_section].push_back("a0");
      loc_cnt++;

      dataAddressFunction();

      regs.first = -1;
      regs.second = -1;
      break;
    case STR_INS:
      sectionTable[cur_section].push_back("b0");
      loc_cnt++;

      dataAddressFunction();

      regs.first = -1;
      regs.second = -1;
      break;
    case CALL_INS:
      sectionTable[cur_section].push_back("30");
      loc_cnt++;

      jumpAddressFunction();

      regs.first = -1;
      regs.second = -1;
      break;
    case JMP_INS:
      sectionTable[cur_section].push_back("50");
      loc_cnt++;

      jumpAddressFunction();

      regs.first = -1;
      regs.second = -1;
      break;
    case JEQ_INS:
      sectionTable[cur_section].push_back("51");
      loc_cnt++;

      jumpAddressFunction();

      regs.first = -1;
      regs.second = -1;
      break;
    case JNE_INS:
      sectionTable[cur_section].push_back("52");
      loc_cnt++;

      jumpAddressFunction();

      regs.first = -1;
      regs.second = -1;
      break;
    case JGT_INS:
      sectionTable[cur_section].push_back("53");
      loc_cnt++;

      jumpAddressFunction();

      regs.first = -1;
      regs.second = -1;
      break;
  }
}

void Assembler::reg0(){
  if(regs.first == -1 && regs.second == -1){
    regs.first = 0;
  }
  else if(regs.first != -1 && regs.second == -1){
    regs.second = 0;
  }
}

void Assembler::reg1(){
  if(regs.first== -1 && regs.second == -1){
    regs.first= 1;
  }
  else if(regs.first!= -1 && regs.second == -1){
    regs.second = 1;
  }
}

void Assembler::reg2(){
  if(regs.first== -1 && regs.second == -1){
    regs.first= 2;
  }
  else if(regs.first!= -1 && regs.second == -1){
    regs.second = 2;
  }
}

void Assembler::reg3(){
  if(regs.first== -1 && regs.second == -1){
    regs.first= 3;
  }
  else if(regs.first!= -1 && regs.second == -1){
    regs.second = 3;
  }
}

void Assembler::reg4(){
  if(regs.first== -1 && regs.second == -1){
    regs.first= 4;
  }
  else if(regs.first!= -1 && regs.second == -1){
    regs.second = 4;
  }
}

void Assembler::reg5(){
  if(regs.first== -1 && regs.second == -1){
    regs.first= 5;
  }
  else if(regs.first!= -1 && regs.second == -1){
    regs.second = 5;
  }
}

void Assembler::reg6(){
  if(regs.first== -1 && regs.second == -1){
    regs.first= 6;
  }
  else if(regs.first!= -1 && regs.second == -1){
    regs.second = 6;
  }
}

void Assembler::reg7(){
  if(regs.first== -1 && regs.second == -1){
    regs.first= 7;
  }
  else if(regs.first!= -1 && regs.second == -1){
    regs.second = 7;
  }
}

void Assembler::reg8(){
  if(regs.first== -1 && regs.second == -1){
    regs.first= 8;
  }
  else if(regs.first!= -1 && regs.second == -1){
    regs.second = 8;
  }
}

void Assembler::skipDirective(int param){
  for(int i = 0; i < param; i++){
    sectionTable[cur_section].push_back("00");
    loc_cnt++;
  }
}

void Assembler::wordDirective(int param, string s){
  cur_symbol = s;
  if(s != ""){
    RelocationTable newRelTable;
    unordered_map<int, RelocationTable> map;
    SymbolTable sym;
    
    sym = symTable[cur_symbol];

    if(sym.vis == glb || sym.vis == ext){
      map = relTable[cur_section];
      newRelTable.id = ++cntRelocations;
      newRelTable.addend = 0;
      newRelTable.offset = loc_cnt;
      newRelTable.symbol = sym.name;
      newRelTable.type = absolute;
      newRelTable.section = cur_section;
      map[newRelTable.id] = newRelTable;
      relTable[cur_section] = map;
    }

    else if(sym.vis == loc){
      map = relTable[cur_section];
      newRelTable.id = ++cntRelocations;
      newRelTable.addend = sym.offset;
      newRelTable.offset = loc_cnt;
      newRelTable.symbol = sym.section;
      newRelTable.type = absolute;
      newRelTable.section = cur_section;
      map[newRelTable.id] = newRelTable;
      relTable[cur_section] = map;
    }

    else{
      map = relTable[cur_section];
      newRelTable.id = ++cntRelocations;
      newRelTable.addend = 0;
      newRelTable.offset = loc_cnt;
      newRelTable.symbol = sym.name;
      newRelTable.type = absolute;
      newRelTable.section = cur_section;
      map[newRelTable.id] = newRelTable;
      relTable[cur_section] = map;
    }
  }

  for(int i = 0; i < 2; i++){
    sectionTable[cur_section].push_back("00");
    loc_cnt++;
  }
}

void Assembler::setDataAddr(dataAddress d, int a, string s)
{
  cur_data_addr = d;
  arg = a;
  cur_symbol = s;
}

void Assembler::dataAddressFunction(){
  string s, s1, s2;
  string result;
  string zero;
  stringstream stream;
  std::vector<std::string> hex_digits;
  int cnt = 0;
  RelocationTable newRelTable;
  unordered_map<int, RelocationTable> map;
  SymbolTable sym;
  switch(cur_data_addr){
    case immedDec:
      s = to_string(regs.first);
      s.append("0");
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      sectionTable[cur_section].push_back("00");
      loc_cnt++;
      stream << std::hex << arg;
      result = stream.str();
      
      for (char c : result) {
        hex_digits.push_back({c});
      }

      cnt = 4 - hex_digits.size();

      if(cnt == 0){
        s1 = hex_digits[0].append(hex_digits[1]);
        s2 = hex_digits[2].append(hex_digits[3]);
      }
      else if(cnt == 1){
        zero = "0";
        s1 = zero.append(hex_digits[0]);
        s2 = hex_digits[1].append(hex_digits[2]);
      }
      else if(cnt == 2){
        s1 = "00";
        s2 = hex_digits[0].append(hex_digits[1]);
      }
      else{
        s1 = "00";
        zero = "0";
        s2 = zero.append(hex_digits[0]);
      }

      sectionTable[cur_section].push_back(s2);
      loc_cnt++;
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;

      break;
    case immedHex:
      s = to_string(regs.first);
      s.append("0");
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      sectionTable[cur_section].push_back("00");
      loc_cnt++;
      stream << std::hex << arg;
      result = stream.str();
      
      for (char c : result) {
        hex_digits.push_back({c});
      }

      cnt = 4 - hex_digits.size();

      if(cnt == 0){
        s1 = hex_digits[0].append(hex_digits[1]);
        s2 = hex_digits[2].append(hex_digits[3]);
      }
      else if(cnt == 1){
        zero = "0";
        s1 = zero.append(hex_digits[0]);
        s2 = hex_digits[1].append(hex_digits[2]);
      }
      else if(cnt == 2){
        s1 = "00";
        s2 = hex_digits[0].append(hex_digits[1]);
      }
      else{
        s1 = "00";
        zero = "0";
        s2 = zero.append(hex_digits[0]);
      }

      sectionTable[cur_section].push_back(s2);
      loc_cnt++;
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;

      break;
    case immedSym:
      s = to_string(regs.first);
      s.append("0");
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      sectionTable[cur_section].push_back("00");
      loc_cnt++;

      sym = symTable[cur_symbol];

      if(sym.vis == glb || sym.vis == ext){
        map = relTable[cur_section];
        newRelTable.id = ++cntRelocations;
        newRelTable.addend = 0;
        newRelTable.offset = loc_cnt;
        newRelTable.symbol = sym.name;
        newRelTable.type = absolute;
        newRelTable.section = cur_section;
        map[newRelTable.id] = newRelTable;
        relTable[cur_section] = map;
      }

      else if(sym.vis == loc){
        map = relTable[cur_section];
        newRelTable.id = ++cntRelocations;
        newRelTable.addend = sym.offset;
        newRelTable.offset = loc_cnt;
        newRelTable.symbol = sym.section;
        newRelTable.type = absolute;
        newRelTable.section = cur_section;
        map[newRelTable.id] = newRelTable;
        relTable[cur_section] = map;
      }

      else{
        map = relTable[cur_section];
        newRelTable.id = ++cntRelocations;
        newRelTable.addend = 0;
        newRelTable.offset = loc_cnt;
        newRelTable.symbol = sym.name;
        newRelTable.type = absolute;
        newRelTable.section = cur_section;
        map[newRelTable.id] = newRelTable;
        relTable[cur_section] = map;
      }
      
      s1 = "00";
      s2 = "00";

      sectionTable[cur_section].push_back(s2);
      loc_cnt++;
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      break;
    case memDec:
      s = to_string(regs.first);
      s.append("0");
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      sectionTable[cur_section].push_back("04");
      loc_cnt++;
      stream << std::hex << arg;
      result = stream.str();
      
      for (char c : result) {
        hex_digits.push_back({c});
      }

      cnt = 4 - hex_digits.size();

      if(cnt == 0){
        s1 = hex_digits[0].append(hex_digits[1]);
        s2 = hex_digits[2].append(hex_digits[3]);
      }
      else if(cnt == 1){
        zero = "0";
        s1 = zero.append(hex_digits[0]);
        s2 = hex_digits[1].append(hex_digits[2]);
      }
      else if(cnt == 2){
        s1 = "00";
        s2 = hex_digits[0].append(hex_digits[1]);
      }
      else{
        s1 = "00";
        zero = "0";
        s2 = zero.append(hex_digits[0]);
      }

      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      sectionTable[cur_section].push_back(s2);
      loc_cnt++;
      break; 
    case memHex:
      s = to_string(regs.first);
      s.append("0");
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      sectionTable[cur_section].push_back("04");
      loc_cnt++;
      stream << std::hex << arg;
      result = stream.str();
      
      for (char c : result) {
        hex_digits.push_back({c});
      }

      cnt = 4 - hex_digits.size();

      if(cnt == 0){
        s1 = hex_digits[0].append(hex_digits[1]);
        s2 = hex_digits[2].append(hex_digits[3]);
      }
      else if(cnt == 1){
        zero = "0";
        s1 = zero.append(hex_digits[0]);
        s2 = hex_digits[1].append(hex_digits[2]);
      }
      else if(cnt == 2){
        s1 = "00";
        s2 = hex_digits[0].append(hex_digits[1]);
      }
      else{
        s1 = "00";
        zero = "0";
        s2 = zero.append(hex_digits[0]);
      }

      sectionTable[cur_section].push_back(s2);
      loc_cnt++;
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      break;  
    case memSym:
      s = to_string(regs.first);
      s.append("0");
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      sectionTable[cur_section].push_back("04");
      loc_cnt++;

      sym = symTable[cur_symbol];

      if(sym.vis == glb || sym.vis == ext){
        map = relTable[cur_section];
        newRelTable.id = ++cntRelocations;
        newRelTable.addend = 0;
        newRelTable.offset = loc_cnt;
        newRelTable.symbol = sym.name;
        newRelTable.type = absolute;
        newRelTable.section = cur_section;
        map[newRelTable.id] = newRelTable;
        relTable[cur_section] = map;
      }

      else if(sym.vis == loc){
        map = relTable[cur_section];
        newRelTable.id = ++cntRelocations;
        newRelTable.addend = sym.offset;
        newRelTable.offset = loc_cnt;
        newRelTable.symbol = sym.section;
        newRelTable.type = absolute;
        newRelTable.section = cur_section;
        map[newRelTable.id] = newRelTable;
        relTable[cur_section] = map;
      }

      else{
        map = relTable[cur_section];
        newRelTable.id = ++cntRelocations;
        newRelTable.addend = 0;
        newRelTable.offset = loc_cnt;
        newRelTable.symbol = sym.name;
        newRelTable.type = absolute;
        newRelTable.section = cur_section;
        map[newRelTable.id] = newRelTable;
        relTable[cur_section] = map;
      }
      
      s1 = "00";
      s2 = "00";

      sectionTable[cur_section].push_back(s2);
      loc_cnt++;
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      break;
    case pcSym:
      s = to_string(regs.first);
      s.append("7");
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      sectionTable[cur_section].push_back("05");
      loc_cnt++;

      sym = symTable[cur_symbol];

      if(sym.vis == glb || sym.vis == ext){
        map = relTable[cur_section];
        newRelTable.id = ++cntRelocations;
        newRelTable.addend = -2;
        newRelTable.offset = loc_cnt;
        newRelTable.symbol = sym.name;
        newRelTable.type = pc;
        newRelTable.section = cur_section;
        map[newRelTable.id] = newRelTable;
        relTable[cur_section] = map;
      }

      else if(sym.vis == loc){
        map = relTable[cur_section];
        newRelTable.id = ++cntRelocations;
        newRelTable.addend = sym.offset - 2;
        newRelTable.offset = loc_cnt;
        newRelTable.symbol = sym.section;
        newRelTable.type = pc;
        newRelTable.section = cur_section;
        map[newRelTable.id] = newRelTable;
        relTable[cur_section] = map;
      }

      else{
        map = relTable[cur_section];
        newRelTable.id = ++cntRelocations;
        newRelTable.addend = -2;
        newRelTable.offset = loc_cnt;
        newRelTable.symbol = sym.name;
        newRelTable.type = pc;
        newRelTable.section = cur_section;
        map[newRelTable.id] = newRelTable;
        relTable[cur_section] = map;
      }
      
      s1 = "00";
      s2 = "00";

      sectionTable[cur_section].push_back(s2);
      loc_cnt++;
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      break;
    case regdir:
      s = to_string(regs.first);
      s.append(to_string(regs.second));
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      sectionTable[cur_section].push_back("01");
      loc_cnt++;
      break;
    case regind:
      s = to_string(regs.first);
      s.append(to_string(regs.second));
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      sectionTable[cur_section].push_back("02");
      loc_cnt++;
      break;
    case regindoffDec:
      s = to_string(regs.first);
      s.append(to_string(regs.second));
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      sectionTable[cur_section].push_back("03");
      loc_cnt++;
      stream << std::hex << arg;
      result = stream.str();
      
      for (char c : result) {
        hex_digits.push_back({c});
      }

      cnt = 4 - hex_digits.size();

      if(cnt == 0){
        s1 = hex_digits[0].append(hex_digits[1]);
        s2 = hex_digits[2].append(hex_digits[3]);
      }
      else if(cnt == 1){
        zero = "0";
        s1 = zero.append(hex_digits[0]);
        s2 = hex_digits[1].append(hex_digits[2]);
      }
      else if(cnt == 2){
        s1 = "00";
        s2 = hex_digits[0].append(hex_digits[1]);
      }
      else{
        s1 = "00";
        zero = "0";
        s2 = zero.append(hex_digits[0]);
      }

      sectionTable[cur_section].push_back(s2);
      loc_cnt++;
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      break;
    case regindoffHex:
      s = to_string(regs.first);
      s.append(to_string(regs.second));
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      sectionTable[cur_section].push_back("03");
      loc_cnt++;
      stream << std::hex << arg;
      result = stream.str();
      
      for (char c : result) {
        hex_digits.push_back({c});
      }

      cnt = 4 - hex_digits.size();

      if(cnt == 0){
        s1 = hex_digits[0].append(hex_digits[1]);
        s2 = hex_digits[2].append(hex_digits[3]);
      }
      else if(cnt == 1){
        zero = "0";
        s1 = zero.append(hex_digits[0]);
        s2 = hex_digits[1].append(hex_digits[2]);
      }
      else if(cnt == 2){
        s1 = "00";
        s2 = hex_digits[0].append(hex_digits[1]);
      }
      else{
        s1 = "00";
        zero = "0";
        s2 = zero.append(hex_digits[0]);
      }

      sectionTable[cur_section].push_back(s2);
      loc_cnt++;
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      break;
    case regindoffSym:
      s = to_string(regs.first);
      s.append(to_string(regs.second));
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      sectionTable[cur_section].push_back("03");
      loc_cnt++;

      sym = symTable[cur_symbol];

      if(sym.vis == glb || sym.vis == ext){
        map = relTable[cur_section];
        newRelTable.id = ++cntRelocations;
        newRelTable.addend = 0;
        newRelTable.offset = loc_cnt;
        newRelTable.symbol = sym.name;
        newRelTable.type = absolute;
        newRelTable.section = cur_section;
        map[newRelTable.id] = newRelTable;
        relTable[cur_section] = map;
      }

      else if(sym.vis == loc){
        map = relTable[cur_section];
        newRelTable.id = ++cntRelocations;
        newRelTable.addend = sym.offset;
        newRelTable.offset = loc_cnt;
        newRelTable.symbol = sym.section;
        newRelTable.type = absolute;
        newRelTable.section = cur_section;
        map[newRelTable.id] = newRelTable;
        relTable[cur_section] = map;
      }

      else{
        map = relTable[cur_section];
        newRelTable.id = ++cntRelocations;
        newRelTable.addend = 0;
        newRelTable.offset = loc_cnt;
        newRelTable.symbol = sym.name;
        newRelTable.type = absolute;
        newRelTable.section = cur_section;
        map[newRelTable.id] = newRelTable;
        relTable[cur_section] = map;
      }

      s1 = "00";
      s2 = "00";

      sectionTable[cur_section].push_back(s2);
      loc_cnt++;
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      break;
  }
}

void Assembler::setMemAddr(memAddress m, int a, string s){
  cur_mem_addr = m;
  arg = a;
  cur_symbol = s;
}

void Assembler::jumpAddressFunction(){
  string s, s1, s2;
  string result;
  string zero;
  stringstream stream;
  std::vector<std::string> hex_digits;
  RelocationTable newRelTable;
  unordered_map<int, RelocationTable> map;
  SymbolTable sym;
  int cnt = 0;
  switch(cur_mem_addr){
    case immedDec_mem:
      s = "f";
      s.append("0");
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      sectionTable[cur_section].push_back("00");
      loc_cnt++;
      stream << std::hex << arg;
      result = stream.str();
      
      for (char c : result) {
        hex_digits.push_back({c});
      }

      cnt = 4 - hex_digits.size();

      if(cnt == 0){
        s1 = hex_digits[0].append(hex_digits[1]);
        s2 = hex_digits[2].append(hex_digits[3]);
      }
      else if(cnt == 1){
        zero = "0";
        s1 = zero.append(hex_digits[0]);
        s2 = hex_digits[1].append(hex_digits[2]);
      }
      else if(cnt == 2){
        s1 = "00";
        s2 = hex_digits[0].append(hex_digits[1]);
      }
      else{
        s1 = "00";
        zero = "0";
        s2 = zero.append(hex_digits[0]);
      }

      sectionTable[cur_section].push_back(s2);
      loc_cnt++;
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;

      break;
    case immedHex_mem:
      s = "f";
      s.append("0");
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      sectionTable[cur_section].push_back("00");
      loc_cnt++;
      stream << std::hex << arg;
      result = stream.str();
      
      for (char c : result) {
        hex_digits.push_back({c});
      }

      cnt = 4 - hex_digits.size();

      if(cnt == 0){
        s1 = hex_digits[0].append(hex_digits[1]);
        s2 = hex_digits[2].append(hex_digits[3]);
      }
      else if(cnt == 1){
        zero = "0";
        s1 = zero.append(hex_digits[0]);
        s2 = hex_digits[1].append(hex_digits[2]);
      }
      else if(cnt == 2){
        s1 = "00";
        s2 = hex_digits[0].append(hex_digits[1]);
      }
      else{
        s1 = "00";
        zero = "0";
        s2 = zero.append(hex_digits[0]);
      }

      sectionTable[cur_section].push_back(s2);
      loc_cnt++;
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;

      break;
    case immedSym_mem:
      s = "f";
      s.append("0");
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      sectionTable[cur_section].push_back("00");
      loc_cnt++;

      sym = symTable[cur_symbol];

      if(sym.vis == glb || sym.vis == ext){
        map = relTable[cur_section];
        newRelTable.id = ++cntRelocations;
        newRelTable.addend = 0;
        newRelTable.offset = loc_cnt;
        newRelTable.symbol = sym.name;
        newRelTable.type = absolute;
        newRelTable.section = cur_section;
        map[newRelTable.id] = newRelTable;
        relTable[cur_section] = map;
      }

      else if(sym.vis == loc){
        map = relTable[cur_section];
        newRelTable.id = ++cntRelocations;
        newRelTable.addend = sym.offset;
        newRelTable.offset = loc_cnt;
        newRelTable.symbol = sym.section;
        newRelTable.type = absolute;
        newRelTable.section = cur_section;
        map[newRelTable.id] = newRelTable;
        relTable[cur_section] = map;
      }

      else{
        map = relTable[cur_section];
        newRelTable.id = ++cntRelocations;
        newRelTable.addend = 0;
        newRelTable.offset = loc_cnt;
        newRelTable.symbol = sym.name;
        newRelTable.type = absolute;
        newRelTable.section = cur_section;
        map[newRelTable.id] = newRelTable;
        relTable[cur_section] = map;
      }
      
      s1 = "00";
      s2 = "00";

      sectionTable[cur_section].push_back(s2);
      loc_cnt++;
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      break;
    case memDec_mem:
      s = "f";
      s.append("0");
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      sectionTable[cur_section].push_back("04");
      loc_cnt++;
      stream << std::hex << arg;
      result = stream.str();
      
      for (char c : result) {
        hex_digits.push_back({c});
      }

      cnt = 4 - hex_digits.size();

      if(cnt == 0){
        s1 = hex_digits[0].append(hex_digits[1]);
        s2 = hex_digits[2].append(hex_digits[3]);
      }
      else if(cnt == 1){
        zero = "0";
        s1 = zero.append(hex_digits[0]);
        s2 = hex_digits[1].append(hex_digits[2]);
      }
      else if(cnt == 2){
        s1 = "00";
        s2 = hex_digits[0].append(hex_digits[1]);
      }
      else{
        s1 = "00";
        zero = "0";
        s2 = zero.append(hex_digits[0]);
      }

      sectionTable[cur_section].push_back(s2);
      loc_cnt++;
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      break; 
    case memHex_mem:
      s = "f";
      s.append("0");
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      sectionTable[cur_section].push_back("04");
      loc_cnt++;
      stream << std::hex << arg;
      result = stream.str();
      
      for (char c : result) {
        hex_digits.push_back({c});
      }

      cnt = 4 - hex_digits.size();

      if(cnt == 0){
        s1 = hex_digits[0].append(hex_digits[1]);
        s2 = hex_digits[2].append(hex_digits[3]);
      }
      else if(cnt == 1){
        zero = "0";
        s1 = zero.append(hex_digits[0]);
        s2 = hex_digits[1].append(hex_digits[2]);
      }
      else if(cnt == 2){
        s1 = "00";
        s2 = hex_digits[0].append(hex_digits[1]);
      }
      else{
        s1 = "00";
        zero = "0";
        s2 = zero.append(hex_digits[0]);
      }

      sectionTable[cur_section].push_back(s2);
      loc_cnt++;
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      break;  
    case memSym_mem:
      s = "f";
      s.append("0");
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      sectionTable[cur_section].push_back("04");
      loc_cnt++;

      sym = symTable[cur_symbol];

      if(sym.vis == glb || sym.vis == ext){
        map = relTable[cur_section];
        newRelTable.id = ++cntRelocations;
        newRelTable.addend = 0;
        newRelTable.offset = loc_cnt;
        newRelTable.symbol = sym.name;
        newRelTable.type = absolute;
        newRelTable.section = cur_section;
        map[newRelTable.id] = newRelTable;
        relTable[cur_section] = map;
      }

      else if(sym.vis == loc){
        map = relTable[cur_section];
        newRelTable.id = ++cntRelocations;
        newRelTable.addend = sym.offset;
        newRelTable.offset = loc_cnt;
        newRelTable.symbol = sym.section;
        newRelTable.type = absolute;
        newRelTable.section = cur_section;
        map[newRelTable.id] = newRelTable;
        relTable[cur_section] = map;
      }

      else{
        map = relTable[cur_section];
        newRelTable.id = ++cntRelocations;
        newRelTable.addend = 0;
        newRelTable.offset = loc_cnt;
        newRelTable.symbol = sym.name;
        newRelTable.type = absolute;
        newRelTable.section = cur_section;
        map[newRelTable.id] = newRelTable;
        relTable[cur_section] = map;
      }
      
      s1 = "00";
      s2 = "00";

      sectionTable[cur_section].push_back(s2);
      loc_cnt++;
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      break;
    case pcSym_mem:
      s = "f";
      s.append("7");
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      sectionTable[cur_section].push_back("05");
      loc_cnt++;

      sym = symTable[cur_symbol];

      if(sym.vis == glb || sym.vis == ext){
        map = relTable[cur_section];
        newRelTable.id = ++cntRelocations;
        newRelTable.addend = -2;
        newRelTable.offset = loc_cnt;
        newRelTable.symbol = sym.name;
        newRelTable.type = pc;
        newRelTable.section = cur_section;
        map[newRelTable.id] = newRelTable;
        relTable[cur_section] = map;
      }

      else if(sym.vis == loc){
        map = relTable[cur_section];
        newRelTable.id = ++cntRelocations;
        newRelTable.addend = sym.offset - 2;
        newRelTable.offset = loc_cnt;
        newRelTable.symbol = sym.section;
        newRelTable.type = pc;
        newRelTable.section = cur_section;
        map[newRelTable.id] = newRelTable;
        relTable[cur_section] = map;
      }

      else{
        map = relTable[cur_section];
        newRelTable.id = ++cntRelocations;
        newRelTable.addend = -2;
        newRelTable.offset = loc_cnt;
        newRelTable.symbol = sym.name;
        newRelTable.type = pc;
        newRelTable.section = cur_section;
        map[newRelTable.id] = newRelTable;
        relTable[cur_section] = map;
      }
      
      s1 = "00";
      s2 = "00";

      sectionTable[cur_section].push_back(s2);
      loc_cnt++;
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      break;
    case regdir_mem:
      s = "f";
      s.append(to_string(regs.first));
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      sectionTable[cur_section].push_back("01");
      loc_cnt++;
      break;
    case regind_mem:
      s = "f";
      s.append(to_string(regs.first));
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      sectionTable[cur_section].push_back("02");
      loc_cnt++;
      break;
    case regindoffDec_mem:
      s = "f";
      s.append(to_string(regs.first));
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      sectionTable[cur_section].push_back("03");
      loc_cnt++;
      stream << std::hex << arg;
      result = stream.str();
      
      for (char c : result) {
        hex_digits.push_back({c});
      }

      cnt = 4 - hex_digits.size();

      if(cnt == 0){
        s1 = hex_digits[0].append(hex_digits[1]);
        s2 = hex_digits[2].append(hex_digits[3]);
      }
      else if(cnt == 1){
        zero = "0";
        s1 = zero.append(hex_digits[0]);
        s2 = hex_digits[1].append(hex_digits[2]);
      }
      else if(cnt == 2){
        s1 = "00";
        s2 = hex_digits[0].append(hex_digits[1]);
      }
      else{
        s1 = "00";
        zero = "0";
        s2 = zero.append(hex_digits[0]);
      }

      sectionTable[cur_section].push_back(s2);
      loc_cnt++;
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      break;
    case regindoffHex_mem:
      s = "f";
      s.append(to_string(regs.first));
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      sectionTable[cur_section].push_back("03");
      loc_cnt++;
      stream << std::hex << arg;
      result = stream.str();
      
      for (char c : result) {
        hex_digits.push_back({c});
      }

      cnt = 4 - hex_digits.size();

      if(cnt == 0){
        s1 = hex_digits[0].append(hex_digits[1]);
        s2 = hex_digits[2].append(hex_digits[3]);
      }
      else if(cnt == 1){
        zero = "0";
        s1 = zero.append(hex_digits[0]);
        s2 = hex_digits[1].append(hex_digits[2]);
      }
      else if(cnt == 2){
        s1 = "00";
        s2 = hex_digits[0].append(hex_digits[1]);
      }
      else{
        s1 = "00";
        zero = "0";
        s2 = zero.append(hex_digits[0]);
      }

      sectionTable[cur_section].push_back(s2);
      loc_cnt++;
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      break;
    case regindoffSym_mem:
      s = "f";
      s.append(to_string(regs.first));
      sectionTable[cur_section].push_back(s);
      loc_cnt++;
      sectionTable[cur_section].push_back("03");
      loc_cnt++;

      sym = symTable[cur_symbol];

      if(sym.vis == glb || sym.vis == ext){
        map = relTable[cur_section];
        newRelTable.id = ++cntRelocations;
        newRelTable.addend = 0;
        newRelTable.offset = loc_cnt;
        newRelTable.symbol = sym.name;
        newRelTable.type = absolute;
        newRelTable.section = cur_section;
        map[newRelTable.id] = newRelTable;
        relTable[cur_section] = map;
      }

      else if(sym.vis == loc){
        map = relTable[cur_section];
        newRelTable.id = ++cntRelocations;
        newRelTable.addend = sym.offset;
        newRelTable.offset = loc_cnt;
        newRelTable.symbol = sym.section;
        newRelTable.type = absolute;
        newRelTable.section = cur_section;
        map[newRelTable.id] = newRelTable;
        relTable[cur_section] = map;
      }

      else{
        map = relTable[cur_section];
        newRelTable.id = ++cntRelocations;
        newRelTable.addend = 0;
        newRelTable.offset = loc_cnt;
        newRelTable.symbol = sym.name;
        newRelTable.type = absolute;
        newRelTable.section = cur_section;
        map[newRelTable.id] = newRelTable;
        relTable[cur_section] = map;
      }

      s1 = "00";
      s2 = "00";

      sectionTable[cur_section].push_back(s2);
      loc_cnt++;
      sectionTable[cur_section].push_back(s1);
      loc_cnt++;
      break;
  }  
}  