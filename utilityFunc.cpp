#include "utilityFunc.h"

#define isON(x,k) (((x)>>(k))&1)
int PC = 0;
ofstream output;

// 32 bits map
map<int,  string> opcodes;  // opcode  ,  type/instruction "r" --> R , "i" --> I , "s" --> S , "b" --> B , "j" --> jalr , "f" --> fence
map<pair<int, string>, string> func3;  // <func3,  type>  ,  sub-type/instruction
map<pair<int, string>, string> func7;    //  <func7,  sub-type>  ,  instruction

// 16 bits map
map<int, string> zero16;

// abi
string abi[32] = {"zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6" };
string abi16[8] = {"s0", "s1", "a0", "a1" , "a2", "a3", "a4", "a5" };

// labels
map<int, string> labels;

// masks:
// 11 : -4096
// 20: -2097152
// 12: -8192
// 5: -64
// 9: -1024
// 17: -262144
// 8: -512

void openOutputFile(string name)
{
    name += ".lsm";
    output.open(name);
    output << "Disassembly of section .text:\n\n";
}

// return number ORed with specified bits from code
int orBits(string& code, int st, int en, int j)
{
    int n = 0;
    for(int i=st; i<=en; i++){
        if(code[i] == '1') n |= (1<<j);
        j++;
    }
    return n;
}

// Get the binary representation in the form of a string
string getBinary(char c)
{
    string str;
    for(int i=0; i<=7; i++){
           if(isON(c,i)) str += "1";
           else str += "0";
    }
    return str;
}



// creating the maps to encode the instructions from files
void createMaps()
{
    int o;
    string c, s;

    ifstream opf;
    opf.open("opcodes.txt");
    while(!opf.eof()){
        opf>>o>>s;
        opcodes[o] = s;
    }
    opf.close();

    ifstream func3f;
    func3f.open("func3.txt");
    while(!func3f.eof()){
        func3f>>o>>c>>s;
        func3[{o,c}] = s;
    }
    func3f.close();

    ifstream func7f;
    func7f.open("func7.txt");
    while(!func7f.eof()){
        func7f>>o>>c>>s;
        func7[{o,c}] = s;
    }
    func7f.close();

    // 16-bits
    zero16[0] = "c.addi4spn";
    zero16[1] = "c.fld";
    zero16[2] = "c.lw";
    zero16[3] = "c.flw";
    zero16[5] = "c.fsd";
    zero16[6] = "c.sw";
    zero16[7] = "c.fsw";
}


// Traverse the instructions to look for needed labels
void getLabels(string& obj)
{
    int i=0, limit;
    for(;i<obj.size();){
        string encode;
        char cur = obj[i++];
        encode += getBinary(cur);

        // check if it is compressed instruction or not
        if(encode[0] == '1' && encode[1] == '1') limit = 4;
        else limit = 2;

        // get the rest of the encode
        for(int j=1; j<limit; j++){
            cur = obj[i++];
            encode += getBinary(cur);
        }
        if(encode[0] == '1' && encode[1] == '1') get32Labels(encode);
        else get16Labels(encode);
        PC += limit;
    }
    PC = 0;
}


// Get 32-bit instructions labels
void get32Labels(string& code)
{
    int op = orBits(code, 0, 6);
    string label = "Label";
    label += labels.size() + '0';
    if(op == 111){                          // jal instruction
        int imm = orBits(code, 12, 19, 12);
        imm |= orBits(code, 20, 20, 11);
        imm |= orBits(code, 21, 30, 1);
        imm |= orBits(code, 31, 31, 20);

        int b = isON(imm, 20);
        int m0 = 0, m1 = -2097152;
        imm |= (b)? m1 : m0;
        labels[PC + imm] = label;           // save the address
    }
    else if(op == 99){                      // branch instructions
        int imm = orBits(code, 7, 7, 11);
        imm |= orBits(code, 8, 11, 1);
        imm |= orBits(code, 25, 30, 5);
        imm |= orBits(code, 31, 31, 12);
        labels[PC + imm] = label;           // save the address
    }
}


// Get 16-bit instructions labels
void get16Labels(string& code)
{
    string label = "Label";
    label += labels.size() + '0';
    if(code[0] == '0' && code[1] == '1'){
        int fun3 = orBits(code, 13, 15);
        if(fun3 == 1 || fun3 == 5 ){        // c.jal  c.j
            int imm = 0;
            imm = orBits(code, 2, 2, 5);
            imm |= orBits(code, 3, 5, 1);
            imm |= orBits(code, 6, 6, 7);
            imm |= orBits(code, 7, 7, 6);
            imm |= orBits(code, 8, 8, 10);
            imm |= orBits(code, 9, 10, 8);
            imm |= orBits(code, 11, 11, 4);
            imm |= orBits(code, 12, 12, 11);

            int b = isON(imm, 11);
            int m0 = 0, m1 = -2048;
            imm |= (b)? m1 : m0;

            labels[PC + imm] = label;       // save address
        }
        else if(fun3 == 6 || fun3 == 7){    // c.bneqz  c.beqz
            int imm = orBits(code, 2, 2, 5);
            imm |= orBits(code, 3, 4, 1);
            imm |= orBits(code, 5, 6, 6);
            imm |= orBits(code, 10, 11, 3);
            imm |= orBits(code, 12, 12, 8);

            int rd = orBits(code, 7, 9);

            int b = isON(imm, 8);
            int m0 = 0, m1 = -256;
            imm |= (b)? m1 : m0;

            labels[PC + imm] = label;       // save address
        }
    }
}


void process(string& obj)
{
    int i=0, limit;
    for(;i<obj.size();){
        string encode;
        char cur = obj[i++];
        encode += getBinary(cur);
        int encodeBi;

        // check if it is compressed instruction or not
        if(encode[0] == '1' && encode[1] == '1') limit = 4;
        else limit = 2;

        // get the rest of the encode
        for(int j=1; j<limit; j++){
            cur = obj[i++];
            encode += getBinary(cur);
        }
        encodeBi = orBits(encode, 0, encode.size()-1);


        // Print the PC and instruction word/half word
        output << std::setfill(' ')<< setw(7) << right << hex << PC;
        output << ":   ";
        output << std::setfill('0') << std::setw(8) << std::hex << encodeBi;
        output << "             ";
        output<< dec;

        // Decode the instruction
        if(encode[0] == '1' && encode[1] == '1') get32(encode);
        else get16(encode);

        // increment the program counter
        PC += limit;

        // check if the next address is labeled
        if(labels.find(PC)!= labels.end())
            output << std::setfill('0') << std::setw(16) << std::hex << PC << "  <" << labels[PC] << ">:\n";
        output<< dec;
    }
}


// Decode 32-bit instructions
void get32(string& code)
{
    int op = orBits(code, 0, 6);
    if(op == 51) printRIns(code, op);
    else if(op == 19) printI1Ins(code, op);
    else if(op == 3 || op == 103) printI2Ins(code, op);
    else if(op == 35) printSIns(code, op);
    else if(op == 111) printJIns(code, op);
    else if(op == 99) printBIns(code, op);
    else if(op == 115) output<< "ecall\n";
    else output<< "Unknown Instruction\n";
}


// Decode 16-bit instructions
void get16(string& code)
{
    if(code[0] == '0' && code[1] == '0') print16Zero(code);
    else if(code[0] == '1' && code[1] == '0') print16One(code);
    else if(code[0] == '0' && code[1] == '1') print16Two(code);
    else output << "Unknown Instruction\n";
}



// Decode the R-instructions
void printRIns(string& code, int opcode)
{
    string type = opcodes[opcode];
    int fun3;
    fun3 = orBits(code, 12, 14);

    string sub = func3[{fun3, type}];
    int fun7;
    fun7 = orBits(code, 25, 31);

    string instruction = func7[{fun7, sub}];

    int rd, rs1, rs2;
    rd = orBits(code, 7, 11);
    rs1 = orBits(code, 15, 19);
    rs2 = orBits(code, 20, 24);

    // print the instruction
    output<< instruction << "     "<< abi[rd] << ", "<<abi[rs1]<<", "<<abi[rs2]<<endl;
}


// Decode the I1-instructions
void printI1Ins(string& code, int opcode)
{
     string type = opcodes[opcode];
     int fun3;
     fun3 = orBits(code, 12, 14);
     string instruction = func3[{fun3, type}];
     int rd, rs1, imm;
     if(instruction.length()==1)        // slli , srli, srai
     {
         int temp = orBits(code, 25, 31);
         instruction = func7[{temp, instruction}];
         rd  = orBits(code, 7, 11);
         rs1 = orBits(code, 15, 19);
         imm = orBits(code, 20, 24);
     }
     else
     {
         rd  = orBits(code, 7, 11);
         rs1 = orBits(code, 15, 19);
         imm = orBits(code, 20, 31);

         // sign extending the immediate
        int b = isON(imm, 11);
        int m0 = 0, m1 = -4096;
        imm |= (b)? m1 : m0;

     }
    // print the instruction
    output<< instruction << "     "<< abi[rd] << ", "<<abi[rs1]<<", "<<imm<<endl;
}


// Decode the I-instructions (load & jalr)
void printI2Ins(string& code, int opcode)
{
    string type = opcodes[opcode];
    int fun3;
    fun3 = orBits(code, 12, 14);

    string instruction = func3[{fun3, type}];

    int rd, rs1, imm;
    rd = orBits(code, 7, 11);
    rs1 = orBits(code, 15, 19);
    imm = orBits(code, 20, 31);

    // sign extending the immediate
    int b = isON(imm, 11);
    int m0 = 0, m1 = -4096;
    imm |= (b)? m1 : m0;

    // print the instruction
    if(instruction == "jalr")
        output<< instruction << "     "<< abi[rd] << ", "<<abi[rs1]<<", "<<imm<<endl;
    else output<< instruction << "      " << abi[rd] << ", " << imm << "("<<abi[rs1]<<")"<<endl;
}

// Decode the S-instructions
void printSIns(string& code, int opcode)
{
    string type = opcodes[opcode];
    int fun3;
    fun3 = orBits(code, 12, 14);

    string instruction = func3[{fun3, type}];

    int rs1, rs2, imm;
    rs1 = orBits(code, 15, 19);
    rs2 = orBits(code, 20, 24);
    imm = orBits(code, 7, 11);
    imm |= orBits(code, 25, 31, 5);

    // sign extending the immediate
    int b = isON(imm, 11);
    int m0 = 0, m1 = -4096;
    imm |= (b)? m1 : m0;

    // print the instruction
    output<< instruction << "      " << abi[rs1] << ", " << imm << "("<<abi[rs2]<<")"<<endl;
}

// Decode the J-instructions
void printJIns(string& code, int opcode)
{
    string instruction = opcodes[opcode];

    int rd;
    rd = orBits(code, 7, 11);
    //imm = orBits(code, 12, 31, 12);

    int imm = orBits(code, 12, 19, 12);
    imm |= orBits(code, 20, 20, 11);
    imm |= orBits(code, 21, 30, 1);
    imm |= orBits(code, 31, 31, 20);

    // sign extending the immediate
    int b = isON(imm, 20);
    int m0 = 0, m1 = -2097152;
    imm |= (b)? m1 : m0;
    imm = PC + imm;                     // save the address

    // print the instruction
    output<< instruction << "     "<< abi[rd] << ", ";
    output<< hex<< imm<< "   ";
    output<< "<"<< labels[imm]<<">" <<endl;
    output << dec;
}


// Decode the B-instructions
void printBIns(string& code, int opcode)
{
    string type = opcodes[opcode];
    int fun3;
    fun3 = orBits(code, 12, 14);

    string instruction = func3[{fun3, type}];

    int rs1, rs2, imm;
    rs1 = orBits(code, 15, 19);
    rs2 = orBits(code, 20, 24);
    imm = orBits(code, 7, 7, 11);
    imm |= orBits(code, 8, 11, 1);
    imm |= orBits(code, 25, 30, 5);
    imm |= orBits(code, 31, 31, 12);

    // sign extending the immediate
    int b = isON(imm, 12);
    int m0 = 0, m1 = -8192;
    imm |= (b)? m1 : m0;
    imm = PC + imm;                     // save the address

    // print the instruction
    output<< instruction << "      " << abi[rs1] << ", " << abi[rs2] << ", ";
    output<< hex<< imm<< "   ";
    output<< "<"<< labels[imm]<<">" <<endl;
    output << dec;
}



void print16Zero(string& code)
{
    int n = orBits(code, 0, 15);
    if(n == 0){
        // print illegal
        output<< "Illegal instruction\n";
        return;
    }
    int rd = 0, rs = 0;
    unsigned int imm = 0;
    int fun3 = orBits(code, 13, 15);
    string instruction = zero16[fun3];

    rd = orBits(code, 2, 4);
    rs = orBits(code, 7, 9);

    if(fun3 == 0){

        imm = orBits(code, 5, 5, 3);
        imm |= orBits(code, 6, 6, 2);
        imm |= orBits(code, 7, 10, 6);
        imm |= orBits(code, 11, 12, 4);

        output<< instruction << "     "<< abi16[rd]<<", "<<imm<<endl;
        return;
    }
    else if(fun3 == 1 || fun3 == 5) {
        imm = orBits(code, 10, 12, 3);
        imm |= orBits(code, 5, 6, 6);
        if(fun3 == 1)
            output<< instruction << "     "<< abi16[rd]<<", "<<abi16[rs] << ", " << imm<<endl;
        else output<< instruction << "     "<< abi16[rs]<<", "<<abi16[rd] << ", " << imm<<endl;
        return;
    }
    else if(fun3 == 2 || fun3 == 3 || fun3 == 7 || fun3 == 6){
        imm = orBits(code, 10, 12, 3);
        imm |= orBits(code, 5, 5, 6);
        imm |= orBits(code, 6, 6, 2);
        if(fun3 == 2 || fun3 == 3)
            output<< instruction << "     "<< abi16[rd]<<", "<<abi16[rs] << ", " << imm<<endl;
        else output<< instruction << "     "<< abi16[rs]<<", "<<abi16[rd] << ", " << imm<<endl;
        return;
    }
    else if(fun3 == 4){
        // print reserved
        output << "Reserved\n";
        return;
    }

}



void print16One(string& code)
{
    int fun3 = orBits(code, 13, 15);
    int n = orBits(code, 2, 13);
    string instruction;

    if(!n){
        instruction = "c.nop";
        // print c.nop
        output << instruction << endl;

    }
    else if(fun3 == 0 || fun3 == 2){
        if(! fun3)
            instruction = "c.addi";
        else instruction = "c.li";
        int imm = 0;
        imm = orBits(code, 2, 6);
        imm |= orBits(code, 12, 12, 5);

        // sign extending the immediate
        int b = isON(imm, 5);
        int m0 = 0, m1 = -64;
        imm |= (b)? m1 : m0;

        int rd = orBits(code, 7, 11);
        output<< instruction << "     "<< abi[rd]<<", "<<imm<<endl;
    }
    else if(fun3 == 1 || fun3 == 5){
        if(fun3 == 1) instruction = "c.jal";
        else instruction = "c.j";
        int imm = 0;
        imm = orBits(code, 2, 2, 5);
        imm |= orBits(code, 3, 5, 1);
        imm |= orBits(code, 6, 6, 7);
        imm |= orBits(code, 7, 7, 6);
        imm |= orBits(code, 8, 8, 10);
        imm |= orBits(code, 9, 10, 8);
        imm |= orBits(code, 11, 11, 4);
        imm |= orBits(code, 12, 12, 11);

        // sign extending the immediate
        int b = isON(imm, 11);
        int m0 = 0, m1 = -4096;
        imm |= (b)? m1 : m0;

        imm = PC + imm;         // save address

        output<< instruction << "     ";
        output<< hex<< imm<< "   ";
        output<< "<"<< labels[imm]<<">" <<endl;
        output << dec;
    }
    else if(fun3 == 3){
        int rd = orBits(code, 7, 11);
        int imm;
        if(rd == 2){
            instruction = "c.addi16sp";
            imm = orBits(code, 2, 2, 5);
            imm |= orBits(code, 3, 4, 7);
            imm |= orBits(code, 5, 5, 6);
            imm |= orBits(code, 6, 6, 4);
            imm |= orBits(code, 12, 12, 9);

            int b = isON(imm, 9);
            int m0 = 0, m1 = -1024;
            imm |= (b)? m1 : m0;

            output<< instruction << "     "<<imm<<endl;
        }
        else{
            instruction = "c.lui";
            imm = orBits(code, 2, 6, 12);
            imm |= orBits(code, 12, 12, 17);

            int b = isON(imm, 17);
            int m0 = 0, m1 = -262144;
            imm |= (b)? m1 : m0;


            output<< instruction << "     "<< abi[rd]<<", "<<imm<<endl;
        }

    }
    else if(fun3 == 4){
        int fun7 = 0;
        int imm = 0;
        fun7 = orBits(code, 10, 11);
        if(!fun7){
            instruction = "c.slri";
            imm = orBits(code, 2, 6);
            imm |= orBits(code, 12, 12, 5);
            int rd = orBits(code, 7, 9);

            output<< instruction << "     "<< abi16[rd]<<", "<<imm<<endl;
        }
        else if(fun7 == 1){
            imm = orBits(code, 2, 6);
            imm |= orBits(code, 12, 12, 5);
            if(!imm){
                instruction == "c.srai64";
                int rd = orBits(code, 7, 9);
                output<< instruction << "     "<< abi16[rd]<<endl;
            }
            else{
                instruction = "c.srai";
                int rd = orBits(code, 7, 9);
                output<< instruction << "     "<< abi16[rd]<<", "<<imm<<endl;
            }
        }
        else if(fun7 == 2){
            imm = orBits(code, 2, 6);
            imm |= orBits(code, 12, 12, 5);
            instruction = "c.andi";

            int rd = orBits(code, 7, 9);

            int b = isON(imm, 5);
            int m0 = 0, m1 = -64;
            imm |= (b)? m1 : m0;

            output<< instruction << "     "<< abi16[rd]<<", "<<imm<<endl;
        }
        else if(fun7 == 3){
            int rs2 = orBits(code, 2, 4);
            int rd = orBits(code, 7, 9);

            int fun4 = orBits(code, 5, 6);
            int fun2 = orBits(code, 12, 12);
            if(!fun2){
                switch(fun4){
                case 0:
                    instruction = "c.sub";
                    break;
                case 1:
                    instruction = "c.xor";
                    break;
                case 2:
                    instruction = "c.or";
                    break;
                case 3:
                    instruction = "c.and";
                    break;
                }
            }
            else{
                switch(fun4){
                case 0:
                    instruction = "c.subw";
                    break;
                case 1:
                    instruction = "c.addw";
                    break;
                case 2:
                    //print reserved
                    output << "Reserved\n";
                    return;
                    break;
                case 3:
                    // print reserved
                    output << "Reserved\n";
                    return;
                    break;
                }
            }
            output<< instruction << "     "<< abi16[rd]<<",  "<<abi16[rs2]<<endl;
        }
    }
    else if(fun3 == 6 || fun3 == 7){
        if(fun3 == 6) instruction = "c.beqz";
        else instruction = "c.bneqz";

        int imm = orBits(code, 2, 2, 5);
        imm |= orBits(code, 3, 4, 1);
        imm |= orBits(code, 5, 6, 6);
        imm |= orBits(code, 10, 11, 3);
        imm |= orBits(code, 12, 12, 8);

        int rd = orBits(code, 7, 9);

        int b = isON(imm, 8);
        int m0 = 0, m1 = -512;
        imm |= (b)? m1 : m0;

        imm = PC + imm;         // save address

        output<< instruction << "     "<<abi16[rd]<< ",  ";
        output<< hex<< imm<< "   ";
        output<< "<"<< labels[imm]<<">" <<endl;
        output << dec;
    }

}


void print16Two(string& code)
{
    int fun3 = orBits(code, 13, 15);
    string instruction;
    if(!fun3){
        instruction = "c.slli";
        int rd = orBits(code, 7, 11);
        int imm = orBits(code, 2, 6);
        imm |= orBits(code, 12, 12, 5);
        output<< instruction << "     "<< abi[rd]<<", "<<imm<<endl;
    }
    else if(fun3 == 1){
        instruction = "c.fldsb";
        int rd = orBits(code, 7, 11);
        int imm = orBits(code, 2, 4, 6);
        imm |= orBits(code, 5, 6, 3);
        imm |= orBits(code, 12, 12, 5);
        output<< instruction << "     "<< abi[rd]<<", "<<imm<<endl;
    }
    else if(fun3 == 2){
        instruction = "c.lwsp";
        int rd = orBits(code, 7, 11);
        int imm = orBits(code, 2, 3, 6);
        imm |= orBits(code, 4, 6, 2);
        imm |= orBits(code, 12, 12, 5);
        output<< instruction << "     "<< abi[rd]<<", "<<imm<<endl;
    }
    else if(fun3 == 3){
        instruction = "c.flwsp";
        int rd = orBits(code, 7, 11);
        int imm = orBits(code, 2, 3, 6);
        imm |= orBits(code, 4, 6, 2);
        imm |= orBits(code, 12, 12, 5);
        output<< instruction << "     "<< abi[rd]<<", "<<imm<<endl;
    }
    else if(fun3 == 4){
        int fun7 = orBits(code, 12, 12);
        int rd = orBits(code, 7, 11);
        if(!rd){
            instruction = "c.ebreak";
            // print
            output << instruction << endl;
        }
        if(!fun7){
            int rs2 = orBits(code, 2, 6);
            if(!rs2){
                instruction = "c.jr";
                // print
                output<< instruction << "     "<< abi[rd]<<endl;
            }
            else {
                instruction = "c.mv";
                // print
                output<< instruction << "     "<< abi[rd] << ", "<< abi[rs2]<<endl;
            }
        }
        else{
            int rs2 = orBits(code, 2, 6);
            if(!rs2){
                instruction = "c.jalr";
                // print
                output<< instruction << "     "<< abi[rd]<<endl;
            }
            else {
                instruction = "c.add";
                // print
                output<< instruction << "     "<< abi[rd] << ", "<< abi[rs2]<<endl;
            }
        }

    }
    else if(fun3 == 5){
        instruction = "c.fsdsp";
        int imm = orBits(code, 7, 9, 6);
        imm |= orBits(code, 10, 12, 3);
        int rs2 = orBits(code, 2, 6);
        //print
        output<< instruction << "     "<< abi[rs2] << ", "<< imm<<endl;
    }
    else if(fun3 == 6 || fun3 == 7){
        if(fun3 == 6) instruction = "c.swsp";
        else instruction = "c.fswsp";
        int rs2 = orBits(code, 2, 6);
        int imm = orBits(code, 7, 8, 6);
        imm |= orBits(code, 9, 12, 2);
        //print
        output<< instruction << "     "<< abi[rs2] << ", "<< imm<<endl;
    }
}


