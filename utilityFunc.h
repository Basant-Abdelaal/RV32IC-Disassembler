#ifndef UTILITYFUNC_H
#define UTILITYFUNC_H
#include<bits/stdc++.h>
using namespace std;


void openOutputFile(string name);      // create file for the output

void createMaps();              // creating 32-bit & 16-bit instructions maps

int orBits(string& code, int st, int en, int j = 0);    // extract bits from instruction word
string getBinary(char c);                               // Get the binary representation in the form of a string

void getLabels(string& obj);    // get all the addresses of the labels in the program
void get32Labels(string& code); // helper function to get labels of 32-bit instructions
void get16Labels(string& code); // helper function to get labels of 16-bit instructions

void process(string& obj);      // The main function that starts processing the object file
void get32(string& code);       // decode 32-bit instruction
void get16(string& code);       // decode 16-bit instruction

//Decoding 32-bit Instructions helper functions:
void printRIns(string& code, int opcode);
void printI1Ins(string& code, int opcode);
void printI2Ins(string& code, int opcode);
void printSIns(string& code, int opcode);
void printJIns(string& code, int opcode);
void printBIns(string& code, int opcode);


//Decoding 16-bit Instructions helper functions:
void print16Zero(string& code);
void print16One(string& code);
void print16Two(string& code);



#endif
