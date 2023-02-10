#include "../inc/linker.hpp"

Linker::Linker(){}

string decodeString(string binaryString) {
  string decodedString;
  int charCode = 0;
  for (int i = 0; i < binaryString.length(); i += 8) {
      for (int j = i; j < i + 8; j++) {
          charCode = (charCode << 1) + (binaryString[j] - '0');
      }
      decodedString += static_cast<char>(charCode);
      charCode = 0;
  }
  return decodedString;
}

int decodeBinaryStringToInt(const std::string& binaryString) {
    int result = 0;
    for (char bit : binaryString) {
        result = result * 2 + (bit - '0');
    }
    return result;
}

bool compareByOldId(const Section &a, const Section &b) {
  return a.oldId < b.oldId;
}

void Linker::readAssemblerOutput(vector<string> outputs){
  ofstream file;
  for(auto o : outputs){
    vector<Section> tempSections;
    vector<Section> tempSectionsByFile;
    Section tempSection;
    std::ifstream file(o);
    std::string line;
    int cnt = 0;

    while (std::getline(file, line)) {
        std::vector<std::string> lines;

        while (!line.empty()) {
            lines.push_back(line);
            std::getline(file, line);
        }
        cnt++;
        if(cnt == 1){
          unordered_map<string, SymbolTableLinker> symbolTables;
          for (auto binaryString : lines) {
            string delimiter = " ";
            size_t pos = 0;
            string token;
            SymbolTableLinker symbolTable;
            pos = binaryString.find(delimiter);
            token = binaryString.substr(0, pos);
            symbolTable.id = decodeBinaryStringToInt(token);
            binaryString.erase(0, pos + delimiter.length());

            pos = binaryString.find(delimiter);
            token = binaryString.substr(0, pos);
            symbolTable.offset = decodeBinaryStringToInt(token);
            binaryString.erase(0, pos + delimiter.length());

            pos = binaryString.find(delimiter);
            token = binaryString.substr(0, pos);
            symbolTable.defined = decodeBinaryStringToInt(token);
            binaryString.erase(0, pos + delimiter.length());

            pos = binaryString.find(delimiter);
            token = binaryString.substr(0, pos);
            symbolTable.vis = (visibilityLinker)decodeBinaryStringToInt(token);
            binaryString.erase(0, pos + delimiter.length());

            pos = binaryString.find(delimiter);
            token = binaryString.substr(0, pos);
            symbolTable.name = decodeString(token);
            binaryString.erase(0, pos + delimiter.length());

            pos = binaryString.find(delimiter);
            token = binaryString.substr(0, pos);
            symbolTable.section = decodeString(token);
            binaryString.erase(0, pos + delimiter.length());
            
            symbolTable.file = o;
            symbolTables[symbolTable.name] = symbolTable;
            if(symbolTable.vis == sectionL){
              tempSection.oldId = symbolTable.id;
              tempSection.name = symbolTable.name;
              tempSection.file = o;
              tempSections.push_back(tempSection);
            }
          }
          std::sort(tempSections.begin(), tempSections.end(), compareByOldId);
          for(int i = 0; i < tempSections.size(); i++){
            tempSections[i].id = ++cntSections;
          }
          allSymTables.push_back(symbolTables);
          SymTableDescriptor std;
          std.id = allSymTables.size() - 1;
          std.file = o;
          descriptors.push_back(std);
        }

        else if(cnt == 2){
          unordered_map<string, vector<string>> result;
          for (const auto& binaryString : lines) {
            string key = decodeString(binaryString.substr(0, binaryString.find(" ")));
            string valueString = binaryString.substr(binaryString.find(" ") + 1);
            vector<string> values;
            size_t pos = 0;
            while ((pos = valueString.find(" ")) != string::npos) {
              string value = decodeString(valueString.substr(0, pos));
              values.push_back(value);
              valueString.erase(0, pos + 1);
            }
            values.push_back(decodeString(valueString));
            result[key] = values;
          }
          for(int i = 0; i < tempSections.size(); i++){
            tempSections[i].data = result[tempSections[i].name];
            tempSections[i].size = tempSections[i].data.size() - 1;
          }
          allSecTables.push_back(result);
        }

        else if(cnt == 3){
          unordered_map<string, unordered_map<int, RelocationTableLinker>> relTables;
          unordered_map<int, RelocationTableLinker> result;
          for (auto binaryString : lines) {
            string delimiter = " ";
            size_t pos = 0;
            string token;
            RelocationTableLinker relTable;
            relTable.file = o;
            pos = binaryString.find(delimiter);
            token = binaryString.substr(0, pos);
            relTable.id = decodeBinaryStringToInt(token);
            binaryString.erase(0, pos + delimiter.length());

            pos = binaryString.find(delimiter);
            token = binaryString.substr(0, pos);
            relTable.offset = decodeBinaryStringToInt(token);
            binaryString.erase(0, pos + delimiter.length());

            pos = binaryString.find(delimiter);
            token = binaryString.substr(0, pos);
            relTable.addend = decodeBinaryStringToInt(token);
            binaryString.erase(0, pos + delimiter.length());

            pos = binaryString.find(delimiter);
            token = binaryString.substr(0, pos);
            relTable.type = (relTypeLinker)decodeBinaryStringToInt(token);
            binaryString.erase(0, pos + delimiter.length());

            pos = binaryString.find(delimiter);
            token = binaryString.substr(0, pos);
            relTable.symbol = decodeString(token);
            binaryString.erase(0, pos + delimiter.length());

            pos = binaryString.find(delimiter);
            token = binaryString.substr(0, pos);
            relTable.section = decodeString(token);
            binaryString.erase(0, pos + delimiter.length());

            result[relTable.id] = relTable;
            relTables[relTable.section] = result;
          }
          allRelTables.push_back(relTables);
          break;
        }
    }

    if(sections.size() == 0){
      for(int i = 0; i < tempSections.size(); i++){
        sections.push_back(tempSections[i]);
      }
      sections[0].end = sections[0].size;
      for(int i = 0; i < sections.size() - 1; i++){
        sections[i+1].end = sections[i].end + sections[i+1].size;
      }
    }
    else{
      for(int i = 0; i < sections.size(); i++){
        for(int j = 0; j < tempSections.size(); j++){
          if(tempSections[j].name == sections[i].name){
            sections[i].size += tempSections[j].size;
            sections[i].end += tempSections[j].size;
            sections[i].data.pop_back();
            for(int k = 0; k < tempSections[j].data.size(); k++){
              sections[i].data.push_back(tempSections[j].data[k]);
            }
            tempSections[j].end = sections[i].end;
            sectionsLost.push_back(tempSections[j]);
            tempSections.erase(tempSections.begin() + j);
          }
        }
      }
      for(int i = 0; i < tempSections.size(); i++){
        tempSections[i].end = sections[sections.size() - 1].end + tempSections[i].size;
        sections.push_back(tempSections[i]);
      }
    }
  }
}

void Linker::printSections()
{
  for(int i = 0; i < sections.size(); i++){
    cout << sections[i].id << " " << sections[i].name << " " << sections[i].size << " " << sections[i].end << " " << sections[i].file << endl;
    for(int j = 0; j < sections[i].data.size(); j++){
      cout << sections[i].data[j] << " ";
    }
    cout << endl;
  }
  cout << endl;
}

void Linker::printSectionsByFile()
{
  for(int i = 0; i < sectionsLost.size(); i++){
    cout << sectionsLost[i].id << " " << sectionsLost[i].name << " " << sectionsLost[i].file << " " << sectionsLost[i].size << " " << sectionsLost[i].end << " " << sectionsLost[i].file << endl;
    for(int j = 0; j < sectionsLost[i].data.size(); j++){
      cout << sectionsLost[i].data[j] << " ";
    }
    cout << endl;
  }
  cout << endl;
}

void Linker::printAllSymTables()
{
  for(int i = 0; i < allSymTables.size(); i++){
    for (const auto& item : allSymTables[i]) {
    string strKey = item.first;
    SymbolTableLinker myVal = item.second;
    cout << myVal.id << " " << myVal.name << " " << myVal.vis << " " << myVal.section << " " << myVal.offset << " " << myVal.defined << endl;
    }
    cout << endl;
  }
}

void Linker::printAllSecTables(){
  for(int i = 0; i < allSecTables.size(); i++){
    for (const auto& item : allSecTables[i]) {
    string strKey = item.first;
    vector<string> myVal = item.second;

    cout << strKey << ":" << endl;

    for(string s : myVal){
      cout << s << " ";
    }
    cout << endl;
  }
  }
}

void Linker::printAllRelocTables()
{
  for(int i = 0; i < allRelTables.size(); i++){
    unordered_map<string, unordered_map<int, RelocationTableLinker>>::iterator outerIter;
    for (outerIter = allRelTables[i].begin(); outerIter != allRelTables[i].end(); ++outerIter) {
      string outerKey = outerIter->first;
      unordered_map<int, RelocationTableLinker> innerMap = outerIter->second;
      unordered_map<int, RelocationTableLinker>::iterator innerIter;
      cout << outerKey << ":" << endl;
      for (innerIter = innerMap.begin(); innerIter != innerMap.end(); ++innerIter) {
        int innerKey = innerIter->first;
        RelocationTableLinker relocationTable = innerIter->second;
        cout << relocationTable.offset << " " << relocationTable.type << " " << relocationTable.symbol << " " << relocationTable.section << " " << relocationTable.addend << endl;
      }
    } 
    cout << endl;
  }
}

void Linker::printSymbolTable(){
  for (const auto& item : symTable) {
    string strKey = item.first;
    SymbolTableLinker myVal = item.second;
    cout << myVal.id << " " << myVal.name << " " << myVal.vis << " " << myVal.section << " " << myVal.offset << " " << myVal.defined << endl;
  }
}

void Linker::createGlobalSymbolTable(){
  SymbolTableLinker toAdd;
  for(int i = 0; i < allSymTables.size(); i++){
    for(auto item : allSymTables[i]){
      if(item.second.vis == glbL || item.second.vis == extL){
        toAdd.vis = item.second.vis;
        toAdd.defined = item.second.defined;
        toAdd.name = item.second.name;
        toAdd.section = item.second.section;
        toAdd.file = item.second.file;
        toAdd.offset = -1;
        string file = descriptors[i].file;
        for(int j = 0; j < sectionsLost.size(); j++){
          if(file == sectionsLost[j].file && toAdd.section == sectionsLost[j].name){
            toAdd.offset = item.second.offset + (sectionsLost[j].end - sectionsLost[j].size);
          }
        }
        if(toAdd.offset == -1){
          for(int j = 0; j < sections.size(); j++){
            if(toAdd.section == sections[j].name){
              toAdd.offset = item.second.offset + (sections[j].end - sections[j].size);
            }
          }
        }
        if(toAdd.offset != -1){
          toAdd.id = ++cntSymbols;
          symTable[toAdd.name] = toAdd;
        }
        else{
          //cout << "Error: " << toAdd.name << " not defined";
        }
      }
    }
  }
}

void Linker::relocate(){
  int addr = -1;
  int oldAddrStart = -1;
  int newAddrStart = -1;
  int oldAddrReloc = -1;
  int newAddrReloc = -1;
  int value;
  int index;
  for(int i = 0; i < allRelTables.size(); i++){
    for(auto item1 : allRelTables[i]){
      for(auto item2 : item1.second){
        if(item2.second.type == pcL){
          if(symTable.find(item2.second.symbol) == symTable.end()){
            for(int j = 0; j < sectionsLost.size(); j++){
              if(sectionsLost[j].name == item2.second.symbol && item2.second.file == sectionsLost[j].file){
                oldAddrStart = sectionsLost[j].end - sectionsLost[j].size;
              }

              if(sectionsLost[j].name == item2.second.section && item2.second.file == sectionsLost[j].file){
                oldAddrReloc = sectionsLost[j].end - sectionsLost[j].size;
              }
            }
            for(int j = 0; j < sections.size(); j++){
              if(sections[j].name == item2.second.symbol && item2.second.file == sections[j].file){
                newAddrStart = sections[j].end - sections[j].size;
              }
              if(sections[j].name == item2.second.section && item2.second.file == sections[j].file){
                newAddrReloc = sections[j].end - sections[j].size;
                index = j;
              }
            }
            if(oldAddrStart == -1){
              if(oldAddrReloc == -1){
                addr  = item2.second.offset;
                value = newAddrStart + item2.second.addend - (item2.second.offset + newAddrReloc);
              }
              else{
                addr  = item2.second.offset + oldAddrReloc - newAddrReloc;
                value = newAddrStart + item2.second.addend - (item2.second.offset + oldAddrReloc);
              }
            }
            else{
              if(oldAddrReloc == -1){
                addr = item2.second.offset;
                value = oldAddrStart + item2.second.addend - (item2.second.offset + newAddrReloc);
              }
              else{
                addr = item2.second.offset + oldAddrReloc - newAddrReloc;
                value = oldAddrStart + item2.second.addend - (item2.second.offset + oldAddrReloc);
              }
            }
          }
          else{
            for(int j = 0; j < sectionsLost.size(); j++){
              if(sectionsLost[j].name == item2.second.section && item2.second.file == sectionsLost[j].file){
                oldAddrReloc = sectionsLost[j].end - sectionsLost[j].size;
              }
            }
            for(int j = 0; j < sections.size(); j++){
              if(sections[j].name == item2.second.section && item2.second.file == sections[j].file){
                newAddrReloc = sections[j].end - sections[j].size;
                index = j;
              }
            }
            if(oldAddrReloc == -1){
              addr = item2.second.offset;
              value = symTable[item2.second.symbol].offset - (newAddrReloc + item2.second.offset) + item2.second.addend;
            }
            else{
              addr = item2.second.offset + oldAddrReloc - newAddrReloc;
              value = symTable[item2.second.symbol].offset - (oldAddrReloc + item2.second.offset) + item2.second.addend;
            }
          }
        }
        else{
          if(symTable.find(item2.second.symbol) == symTable.end()){
            // apsolutno kad je lokalan simbol
            for(int j = 0; j < sectionsLost.size(); j++){
              if(sectionsLost[j].name == item2.second.symbol && item2.second.file == sectionsLost[j].file){
                oldAddrStart = sectionsLost[j].end - sectionsLost[j].size;
              }

              if(sectionsLost[j].name == item2.second.section && item2.second.file == sectionsLost[j].file){
                oldAddrReloc = sectionsLost[j].end - sectionsLost[j].size;
              }
            }
            for(int j = 0; j < sections.size(); j++){
              if(sections[j].name == item2.second.symbol && item2.second.file == sections[j].file){
                newAddrStart = sections[j].end - sections[j].size;
              }
              if(sections[j].name == item2.second.section && item2.second.file == sections[j].file){
                newAddrReloc = sections[j].end - sections[j].size;
                index = j;
              }
            }
            if(oldAddrStart == -1){
              addr  = item2.second.offset;
              value = newAddrStart + item2.second.addend;
            }
            else{
              if(oldAddrReloc == -1){
                addr = item2.second.offset;
                value = newAddrStart + item2.second.addend;
              }
              else{
                addr = item2.second.offset + oldAddrReloc - newAddrReloc;
                value = oldAddrStart + item2.second.addend;
              }
            }
          }
          else{
            // apsolutni sa globalnim
            for(int j = 0; j < sectionsLost.size(); j++){
              if(sectionsLost[j].name == item2.second.section && item2.second.file == sectionsLost[j].file){
                oldAddrReloc = sectionsLost[j].end - sectionsLost[j].size;
              }           
            }
            for(int j = 0; j < sections.size(); j++){
              if(sections[j].name == item2.second.section && item2.second.file == sections[j].file){
                newAddrReloc = sections[j].end - sections[j].size;
                index = j;
              }
            }
            if(oldAddrReloc == -1){
              addr = item2.second.offset;
              value = symTable[item2.second.symbol].offset;
            }
            else{
              addr = item2.second.offset + oldAddrReloc - newAddrReloc;
              value = symTable[item2.second.symbol].offset;
            }
          }
        }
        std::stringstream stream;
        std::vector<std::string> hex_digits;
        int cnt = 0;
        string s, s1, s2;
        string result;
        string zero;

        unsigned int abs_value = abs(value);
        stream << std::hex << abs_value;
        result = stream.str();

        for (char c : result) {
          hex_digits.push_back({c});
        }

        cnt = 4 - hex_digits.size();

        if (cnt == 0) {
          s1 = hex_digits[0].append(hex_digits[1]);
          s2 = hex_digits[2].append(hex_digits[3]);
        } else if (cnt == 1) {
          zero = "0";
          s1 = zero.append(hex_digits[0]);
          s2 = hex_digits[1].append(hex_digits[2]);
        } else if (cnt == 2) {
          s1 = "00";
          s2 = hex_digits[0].append(hex_digits[1]);
        } else {
          s1 = "00";
          zero = "0";
          s2 = zero.append(hex_digits[0]);
        }

        if (value < 0) {
          // Convert to two's complement
          int s1_int = std::stoi(s1, nullptr, 16);
          int s2_int = std::stoi(s2, nullptr, 16);
          s1_int = 0xFFFF - s1_int;
          s2_int = 0xFFFF - s2_int;
          s1_int += 1;
          s2_int += (s1_int == 0x10000);
          
          std::stringstream ss1, ss2;
          ss1 << std::setfill('0') << std::hex << s1_int;
          ss2 << std::setfill('0') << std::hex << s2_int;
          s1 = ss1.str();
          s2 = ss2.str();
          string s21 = s2.substr(0, 2);
          string s22 = s2.substr(2, 4);
          sections[index].data[addr+1] = s21;
          sections[index].data[addr] = s22;
        }
        else{
          sections[index].data[addr+1] = s1;
          sections[index].data[addr] = s2;
        }
      }
    }
  }
}

string stringArray;

void Linker::writeOutput(string outp) {

string stringArray;
for (int i = 0; i < sections.size(); i++) {
  for (int j = 0; j < sections[i].data.size(); j++) {
    for(int k = 0; k < sections[i].data[j].size(); k++){
      if (isxdigit(sections[i].data[j][k])) {
        stringArray.push_back(sections[i].data[j][k]);
      }
    }
  }
}

vector<char> send;
for(int i = 0; i < stringArray.length(); i = i + 2){ 
  string pair = stringArray.substr(i, 2);
  unsigned long decimal_value = std::stoul(pair, nullptr, 16);
  short arg = decimal_value & 0xffff;
  char c = (char)arg;

  send.push_back(c);
}

ofstream out;

/*for(char c: send){
  cout << hex << (c & 0xff) << " ";
}*/

out.open(outp, ios::binary);

if(out.is_open()){

  for(char character: send){
    out.write(reinterpret_cast<char*>(&character),sizeof(char));
  }
}
  out.close();
}