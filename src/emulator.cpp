#include "../inc/emulator.hpp"

Emulator::Emulator(){
  memory.resize(SIZE);
  for(int i = 0; i < 8; i++){
    r[i] = 0;
  }
  psw = 0;
  running = true;
}

void Emulator::printOutput(string instruction){
  cout << "-----------------------------------------------------" << endl;
  cout << "Emulated processor executed " << instruction <<  " instruction" << endl;
  cout << "Emulated processor state: psw=0b" << bitset<16>(psw) << endl;
  for (int i = 0; i < 8; i++)
  {
    cout << "r" << i << "=0x";
    cout << setfill('0') << setw(4) << hex << r[i];
    cout << "\t";
    if (i % 4 == 3)
      cout << endl;
  }
}

void Emulator::emulate(string input){

  ifstream in(input, std::ios::binary);
  if (in.is_open()){
    int i = 0;
    while(!in.eof()){
      char charRead;
      in.read(reinterpret_cast<char*>(&charRead), sizeof(char));
      memory[i++] = charRead;
    }
  }
  in.close();

  r[7] |= memory[0];
  r[7] |= (memory[1] << 8);

  while(running){
    int i, j;
    short addr, temp;
    switch(memory[r[7]++]){
      //HALT
      case 0x00:
        running = false;
        printOutput("HALT");
        break;
      //IRET
      case 0x20:
        psw = 0;
        psw |= memory[r[6]++];
        psw |= (memory[r[6]++] << 8);
        r[7] = 0;
        r[7] |= memory[r[6]++];
        r[7] |= (memory[r[6]++] << 8);
        break;
      //RET
      case 0x40:
        r[7] = 0;
        r[7] |= memory[r[6]++];
        r[7] |= (memory[r[6]++] << 8);
        break;
      //INT
      case 0x10:
        memory[--r[6]] = (r[7] & 0xff00) >> 8;
        memory[--r[6]] = (r[7] & 0xff);
        memory[--r[6]] = (psw & 0xff00) >> 8;
        memory[--r[6]] = (psw & 0xff);
        i = (memory[r[7]++] & 0xf0) >> 4;
        addr = (r[i] % 8) * 2;
        r[7] = 0;
        r[7] |= memory[addr++];
        r[7] |= (memory[addr] << 8);
        break;
      //NOT
      case 0x80:
        i = (memory[r[7]++] & 0xf0) >> 4;
        r[i] = ~r[i];
        break;
      //XCHG
      case 0x60: 
        i = (memory[r[7]] & 0xf0) >> 4;
        j = (memory[r[7]++] & 0xf);
        temp = r[i];
        r[i] = r[j];
        r[j] = temp;
        break;
      //ADD
      case 0x70:
        i = (memory[r[7]] & 0xf0) >> 4;
        j = (memory[r[7]++] & 0xf);
        r[i] += r[j];
        break;
      //SUB
      case 0x71:
        i = (memory[r[7]] & 0xf0) >> 4;
        j = (memory[r[7]++] & 0xf);
        r[i] -= r[j];
        break;
      //MUL
      case 0x72:
        i = (memory[r[7]] & 0xf0) >> 4;
        j = (memory[r[7]++] & 0xf);
        r[i] *= r[j];
        break;
      //DIV
      case 0x73:
        i = (memory[r[7]] & 0xf0) >> 4;
        j = (memory[r[7]]++ & 0xf);
        r[i] /= r[j];
        break;
      //CMP
      case 0x74:
        i = (memory[r[7]] & 0xf0) >> 4;
        j = (memory[r[7]++] & 0xf);
        temp = r[i] - r[j];
        if(temp == 0){
          psw |= 1;
          psw &= 0xfff7;
        }
        if(temp < 0){
          psw |= 8;
          psw &= 0xfffe;
        }
        if(temp > 0){
          psw &= 0xfff7;
          psw &= 0xfffe;
        }
        if(r[i] < r[j]){
          psw |= 4;
        }
        else{
          psw &= 0xfffb;
        }
        if(((r[i] > 0 && r[j] < 0) && (r[i] < r[j])) || ((r[i] < 0 && r[j] > 0) && (r[i] > r[j]))){
          psw |= 2;
        }
        else{
          psw &= 0xfffd;
        }
        break;
      //AND
      case 0x81:
        i = (memory[r[7]] & 0xf0) >> 4;
        j = (memory[r[7]++] & 0xf);
        r[i] &= r[j];
        break;
      //OR
      case 0x82:
        i = (memory[r[7]] & 0xf0) >> 4;
        j = (memory[r[7]++] & 0xf);
        r[i] |= r[j];
        break;
      //XOR
      case 0x83:
        i = (memory[r[7]] & 0xf0) >> 4;
        j = (memory[r[7]++] & 0xf);
        r[i] ^= r[j];
        break;
      //TEST
      case 0x84:
        i = (memory[r[7]] & 0xf0) >> 4;
        j = (memory[r[7]++] & 0xf);
        temp = r[i] & r[j];
        if(temp == 0){
          psw |= 1;
          psw &= 0xfff7;
        }
        if(temp > 0){
          psw &= 0xfff7;
          psw &= 0xfffe;
        }
        if(temp < 0){
          psw |= 8;
          psw &= 0xfffe;
        }
        break;
      //SHL
      case 0x90:
        i = (memory[r[7]] & 0xf0) >> 4;
        j = (memory[r[7]++] & 0xf);
        temp = r[i];
        r[i] = r[i] << r[j];
        if(r[i] == 0){
          psw |= 1;
          psw &= 0xfff7;
        }
        if(r[i] > 0){
          psw &= 0xfff7;
          psw &= 0xfffe;
        }
        if(r[i] < 0){
          psw |= 8;
          psw &= 0xfffe;
        }
        if(temp & 0x8000){
          psw |= 4;
        }
        else{
          psw &= 0xfffb;
        }
        break;
      //SHR
      case 0x91:
        i = (memory[r[7]] & 0xf0) >> 4;
        j = (memory[r[7]++] & 0xf);
        temp = r[i];
        r[i] = r[i] >> r[j];
        if(r[i] == 0){
          psw |= 1;
          psw &= 0xfff7;
        }
        if(r[i] > 0){
          psw &= 0xfff7;
          psw &= 0xfffe;
        }
        if(r[i] < 0){
          psw |= 8;
          psw &= 0xfffe;
        }
        if(temp & 0x1){
          psw |= 4;
        }
        else{
          psw &= 0xfffb;
        }
        break;
      //PUSH/STR
      case 0xb0:
        switch(memory[r[7]+1]){
          //PUSH
          case 0x12:
    
            i = (memory[r[7]++] & 0xf0) >> 4;
            memory[--r[6]] = (r[i] & 0xff00) >> 8;
            memory[--r[6]] = (r[i] & 0xff);
            r[7]++;
            break;
          //STR
          default:
  
            i = (memory[r[7]] & 0xf0) >> 4;
            j = (memory[r[7]++] & 0xf);
            dataAddress(0xb0, i, j);
            break;
        }
        break;
      //POP/LDR
      case 0xa0:
        switch(memory[r[7]+1]){
          //POP
          case 0x42:
  
            i = (memory[r[7]++] & 0xf0) >> 4;
            r[i] = 0;
            r[i] |= memory[r[6]++];
            r[i] |= (memory[r[6]++] << 8);
            r[7]++;
            break;
          //LDR
          default:
  
            i = (memory[r[7]] & 0xf0) >> 4;
            j = (memory[r[7]++] & 0xf);
            dataAddress(0xa0, i, j);
            break;
        }
        break;
      //CALL
      case 0x30:

        j = (memory[r[7]++] & 0xf);
        jumpAddress(0x30, j);
        break;
      //JMP
      case 0x50:
        j = (memory[r[7]++] & 0xf);
        jumpAddress(0x50, j);
        break;
      //JEQ
      case 0x51:
        j = (memory[r[7]++] & 0xf);
        jumpAddress(0x51, j);
        break;
      //JNE
      case 0x52:
        j = (memory[r[7]++] & 0xf);
        jumpAddress(0x52, j);
        break;
      //JGT
      case 0x53:
        j = (memory[r[7]++] & 0xf);
        jumpAddress(0x53, j);
        break;
    }
  }
}

void Emulator::dataAddress(unsigned short ins, int i, int j){
  short operand, toAdd, temp;
  switch(memory[r[7]++]){
    //IMMED
    case 0x00:
      switch(ins){
        //LDR
        case 0xa0:
          operand = 0;
          operand |= memory[r[7]++];
          operand |= (memory[r[7]++] << 8);
          r[i] = operand;
          break;
        //STR
        case 0xb0:
          cout << "ERROR! IMMED NOT SUPPORTED FOR STR!";
          break;
      } 
      break;
    //REGDIR
    case 0x01:
      switch(ins){
        //LDR
        case 0xa0:
          r[i] = r[j];
          break;
        //STR
        case 0xb0:
          r[j] = r[i];
          break;
      }
      break;
    //REGIND
    case 0x02:
      switch(ins){
        //LDR
        case 0xa0:
          operand = r[j];
          r[i] = 0;
          r[i] |= memory[operand++];
          r[i] |= (memory[operand] << 8);
          break;
        //STR
        case 0xb0:
          operand = r[j];
          memory[operand] = 0;
          memory[operand + 1] = 0;
          memory[operand++] |= r[i] & 0xff;
          memory[operand] |= (r[i] >> 8) & 0xff;
          break;
      }
      break;
    //REGINDPOM
    case 0x03:
      switch(ins){
        //LDR
        case 0xa0:
          toAdd = 0;
          toAdd |= memory[r[7]++];
          toAdd |= (memory[r[7]++] << 8);
          operand = toAdd + r[j];
          r[i] = 0;
          r[i] |= memory[operand++];
          r[i] |= (memory[operand] << 8);
          break;
        //STR
        case 0xb0:
          toAdd = 0;
          toAdd |= memory[r[7]++];
          toAdd |= (memory[r[7]++] << 8);
          operand = toAdd + r[j];
          memory[operand] = 0;
          memory[operand + 1] = 0;
          memory[operand++] |= r[i] & 0xff;
          memory[operand] |= (r[i] >> 8) & 0xff;
          break;
      }
      break;
    //MEMDIR
    case 0x04:
      switch(ins){
        //LDR
        case 0xa0:
          operand = 0;
          operand |= memory[r[7]++];
          operand |= (memory[r[7]++] << 8);
          r[i] = 0;
          r[i] |= memory[operand++];
          r[i] |= (memory[operand] << 8);
          break;
        //STR
        case 0xb0:
          operand = 0;
          operand |= memory[r[7]++];
          operand |= (memory[r[7]++] << 8);
          memory[operand] = 0;
          memory[operand + 1] = 0;
          memory[operand++] |= r[i] & 0xff;
          memory[operand] |= (r[i] >> 8) & 0xff;
          break;
      }
      break;
    //PCREL
    case 0x05:
      switch(ins){
        //LDR
        case 0xa0:
          toAdd = 0;
          toAdd |= memory[r[7]++];
          toAdd |= (memory[r[7]++] << 8);
          temp = r[7] ;
          operand = temp + toAdd;
          r[i] = 0;
          r[i] |= memory[operand++];
          r[i] |= (memory[operand] << 8);
          break;
        //STR
        case 0xb0:
          toAdd = 0;
          toAdd |= memory[r[7]++];
          toAdd |= (memory[r[7]++] << 8);
          temp = r[7];
          operand = temp + toAdd;
          memory[operand] = 0;
          memory[operand + 1] = 0;
          memory[operand++] |= r[i] & 0xff;
          memory[operand] |= (r[i] >> 8) & 0xff;
          break;
      }
      break;
  }
}

void Emulator::jumpAddress(unsigned short ins, int j){
  short operand, temp, toAdd;
  switch(memory[r[7]++]){
    //IMMED
    case 0x00:
      switch(ins){
        //CALL
        case 0x30:
          operand = 0;
          operand |= memory[r[7]++];
          operand |= (memory[r[7]++] << 8);
          memory[--r[6]] = (r[7] & 0xff00) >> 8;
          memory[--r[6]] = (r[7] & 0xff);
          r[7] = operand;
          break;
        //JMP
        case 0x50:
          operand = 0;
          operand |= memory[r[7]++];
          operand |= (memory[r[7]++] << 8);
          r[7] = operand;
          break;
        //JEQ
        case 0x51:
          if(psw & 0x1){
            operand = 0;
            operand |= memory[r[7]++];
            operand |= (memory[r[7]++] << 8);
            r[7] = operand;
          }
          else{
            r[7] += 2;
          }
          break;
        //JNE
        case 0x52:
          if(!(psw & 0x1)){
            operand = 0;
            operand |= memory[r[7]++];
            operand |= (memory[r[7]++] << 8);
            r[7] = operand;
          }
          else{
            r[7] += 2;
          }
          break;
        //JGT
        case 0x53:
          if(!(psw & 0x1) && !(psw & 0x8)){
            operand = 0;
            operand |= memory[r[7]++];
            operand |= (memory[r[7]++] << 8);
            r[7] = operand;
          }
          else{
            r[7] += 2;
          }
          break;
      } 
      break;
    //REGDIR
    case 0x01:
      switch(ins){
        //CALL
        case 0x30:
          memory[--r[6]] = (r[7] & 0xff00) >> 8;
          memory[--r[6]] = (r[7] & 0xff);
          r[7] = r[j];
          break;
        //JMP
        case 0x50:
          r[7] = r[j];
          break;
        //JEQ
        case 0x51:
          if(psw & 0x1){
            r[7] = r[j];
          }
          break;
        //JNE
        case 0x52:
          if(!(psw & 0x1)){
            r[7] = r[j];
          }
          break;
        //JGT
        case 0x53:
          if(!(psw & 0x1) && !(psw & 0x8)){
            r[7] = r[j];
          }
          break;
      } 
      break;
    //REGIND
    case 0x02:
      switch(ins){
        //CALL
        case 0x30:
          memory[--r[6]] = (r[7] & 0xff00) >> 8;
          memory[--r[6]] = (r[7] & 0xff);
          operand = r[j];
          r[7] = 0;
          r[7] |= memory[operand++];
          r[7] |= (memory[operand] << 8);
          break;
        //JMP
        case 0x50:
          operand = r[j];
          r[7] = 0;
          r[7] |= memory[operand++];
          r[7] |= (memory[operand] << 8);
          break;
        //JEQ
        case 0x51:
          if(psw & 0x1){
            operand = r[j];
            r[7] = 0;
            r[7] |= memory[operand++];
            r[7] |= (memory[operand] << 8);
          }
          break;
        //JNE
        case 0x52:
          if(!(psw & 0x1)){
            operand = r[j];
            r[7] = 0;
            r[7] |= memory[operand++];
            r[7] |= (memory[operand] << 8);
          }
          break;
        //JGT
        case 0x53:
          if(!(psw & 0x1) && !(psw & 0x8)){
            operand = r[j];
            r[7] = 0;
            r[7] |= memory[operand++];
            r[7] |= (memory[operand] << 8);
          }
          break;
      } 
      break;
    //REGINDPOM
    case 0x03:
      switch(ins){
        //CALL
        case 0x30:
          toAdd = 0;
          toAdd |= memory[r[7]++];
          toAdd |= (memory[r[7]++] << 8);
          operand = toAdd + r[j];
          memory[--r[6]] = (r[7] & 0xff00) >> 8;
          memory[--r[6]] = (r[7] & 0xff);
          r[7] = 0;
          r[7] |= memory[operand++];
          r[7] |= (memory[operand] << 8);
          break;
        //JMP
        case 0x50:
          toAdd = 0;
          toAdd |= memory[r[7]++];
          toAdd |= (memory[r[7]++] << 8);
          operand = toAdd + r[j];
          r[7] = 0;
          r[7] |= memory[operand++];
          r[7] |= (memory[operand] << 8);
          break;
        //JEQ
        case 0x51:
          if(psw & 0x1){
            toAdd = 0;
            toAdd |= memory[r[7]++];
            toAdd |= (memory[r[7]++] << 8);
            operand = toAdd + r[j];
            r[7] = 0;
            r[7] |= memory[operand++];
            r[7] |= (memory[operand] << 8);
          }
          else{
            r[7] += 2;
          }
          break;
        //JNE
        case 0x52:
          if(!(psw & 0x1)){
            toAdd = 0;
            toAdd |= memory[r[7]++];
            toAdd |= (memory[r[7]++] << 8);
            operand = toAdd + r[j];
            r[7] = 0;
            r[7] |= memory[operand++];
            r[7] |= (memory[operand] << 8);
          }
          else{
            r[7] += 2;
          }
          break;
        //JGT
        case 0x53:
          if(!(psw & 0x1) && !(psw & 0x8)){
            toAdd = 0;
            toAdd |= memory[r[7]++];
            toAdd |= (memory[r[7]++] << 8);
            operand = toAdd + r[j];
            r[7] = 0;
            r[7] |= memory[operand++];
            r[7] |= (memory[operand] << 8);
          }
          else{
            r[7] += 2;
          }
          break;
      } 
      break;
    //MEMDIR
    case 0x04:
      switch(ins){
        //CALL
        case 0x30:
          operand = 0;
          operand |= memory[r[7]++];
          operand |= (memory[r[7]++] << 8);
          memory[--r[6]] = (r[7] & 0xff00) >> 8;
          memory[--r[6]] = (r[7] & 0xff);
          r[7] = 0;
          r[7] |= memory[operand++];
          r[7] |= (memory[operand] << 8);
          break;
        //JMP
        case 0x50:
          operand = 0;
          operand |= memory[r[7]++];
          operand |= (memory[r[7]++] << 8);
          r[7] = 0;
          r[7] |= memory[operand++];
          r[7] |= (memory[operand] << 8);
          break;
        //JEQ
        case 0x51:
          if(psw & 0x1){
            operand = 0;
            operand |= memory[r[7]++];
            operand |= (memory[r[7]++] << 8);
            r[7] = 0;
            r[7] |= memory[operand++];
            r[7] |= (memory[operand] << 8);
          }
          else{
            r[7] += 2;
          }
          break;
        //JNE
        case 0x52:
          if(!(psw & 0x1)){
            operand = 0;
            operand |= memory[r[7]++];
            operand |= (memory[r[7]++] << 8);
            r[7] = 0;
            r[7] |= memory[operand++];
            r[7] |= (memory[operand] << 8);
          }
          else{
            r[7] += 2;
          }
          break;
        //JGT
        case 0x53:
          if(!(psw & 0x1) && !(psw & 0x8)){
            operand = 0;
            operand |= memory[r[7]++];
            operand |= (memory[r[7]++] << 8);
            r[7] = 0;
            r[7] |= memory[operand++];
            r[7] |= (memory[operand] << 8);
          }
          else{
            r[7] += 2;
          }
          break;
      } 
      break;
    //PCREL
    case 0x05:
      switch(ins){
        //CALL
        case 0x30:
          toAdd = 0;
          toAdd |= memory[r[7]++];
          toAdd |= (memory[r[7]++] << 8);
          temp = r[7];
          operand = temp + toAdd;
          memory[--r[6]] = (r[7] & 0xff00) >> 8;
          memory[--r[6]] = (r[7] & 0xff);
          r[7] = 0;
          r[7] = operand;
          break;
        //JMP
        case 0x50:
          toAdd |= memory[r[7]++];
          toAdd |= (memory[r[7]++] << 8);
          temp = r[7];
          operand = temp + toAdd;
          r[7] = 0;
          r[7] = operand;
          break;
        //JEQ
        case 0x51:
          if(psw & 0x1){
            toAdd |= memory[r[7]++];
            toAdd |= (memory[r[7]++] << 8);
            temp = r[7];
            operand = temp + toAdd;
            r[7] = 0;
            r[7] = operand;
          }
          else{
            r[7] += 2;
          }
          break;
        //JNE
        case 0x52:
          if(!(psw & 0x1)){
            toAdd |= memory[r[7]++];
            toAdd |= (memory[r[7]++] << 8);
            temp = r[7];
            operand = temp + toAdd;
            r[7] = 0;
            r[7] = operand;
          }
          else{
            r[7] += 2;
          }
          break;
        //JGT
        case 0x53:
          if(!(psw & 0x1) && !(psw & 0x8)){
            toAdd |= memory[r[7]++];
            toAdd |= (memory[r[7]++] << 8);
            temp = r[7];
            operand = temp + toAdd;
            r[7] = 0;
            r[7] = operand;
          }
          else{
            r[7] += 2;
          }
          break;
      } 
      break;
  }
}