# RV32IC-Disassembler

All copy rights © go to Basant Elhussein and Hashem Khaled.

To compile the project from a command line, type these commands:
g++ -c utilityFunc.cpp
g++ -c dissassemblerhb.cpp
g++ utilityFunc.o dissassemblerhb.o -o exe_version

To run the program:
exe_version "name_of_obj_file.bin"

Implementation Details

First: Instruction decoding:
Our logic for decoding the instructions goes the same way as they encoded.
➔ We check whether the instruction is 16 or 32-bit by checking the first two bits of the opcode (32: 11 , 16: otherwise)
➔ Accordingly, we will use the relative function to decode the instruction.

get32, get16:
The two functions are used to decode the 32-bit instructions and the compressed 16-bit instructions.
The logic to get the instruction name and format follows a very similar logic to how they are encoded. We first check the opcode, and then if a more than 1 instruction has the same opcode, we check func3 and so on.
To do this, we used the map data structure to save the instruction name according to the mentioned logic along with if statements to differentiate between instruction formats.
The maps are initialized using createMaps() function that reads the data of the 32-bit instructions from files (opcodes.txt, func3.txt, func7.txt)

Second: Loading registers and immediate values:
We relied on function: orBits() to get the data from the instruction word/half word. This function basically ORs an integer (initialized by zero) with the instruction word/half word passed from a specific start bit to an end bit in the instruction, and you may initialize the start bit of the number as well.
This allowed us to get the register numbers. For the immediate values that are placed in nonsequential bits, we kept ORing with the desired bits until we get the value.

Third: Sign extending the immediate values:
Since we are saving the immediate values in integers, we need to sign extend them to express the negative value in instructions like addi, jal, etc.
To do so we followed this logic:
b → represents the sign bit.
m0 → mask with all 0s
m1 → mask with all 1s starting from the bit after the sign bit (last bit the in the immediate)
if b is 1, then we OR the immediate with m1, otherwise we OR the immediate with m0.

Fourth: Program Counter:
a program counter is created and initialized by 0. It is incremented according to the instruction size (32 bits → increment by 4 bytes, 16 bits → increment by 2 bytes)

Fifth: Creating Labels for instructions that use labels (jal, branch, c.j, …) BONUS
To do so, we first scanned all the instructions used and whenever we find any of these, we saved the targeted address (Program Counter + Immediate) in a map called labels. This map maps the address to a string (“label” + “number of the label”)
Then, the program counter is returned back to 0 and we start decoding the instructions in the output file, and whenever the PC reaches a saved address in the labels, it prints the label first. And whenever an instruction that uses labels is decoded, the name of the label is included beside the address.

Test Cases:
We focused in the test cases to cover wide variety of instructions. Some where to test:
--> Having immediate with negative value (to check sign extension)
--> Different compressed instructions
--> Unkown Instruction (invalid instruction)
--> Illegal Instructions
--> Reserved Instructions

Limitations
The only limitations in our program exist for the compressed instructions (C extension) because we were not able to spot the exact format of compressed instructions from RISC_V specifications document, moreover we tried to search for these formats, but some of them was hard to be found. So, we decided to make assumptions for theses formats by the way we used to in other instructions, following the same pattern. For example: the instruction format for c.add instruction would be c.add rd, rs2

Known Issues
Our program is working properly without any issues, we tested it using all the sample tests provided and using another manually made sample for the compressed instructions and it outputs the right assembly code corresponding to the machine codes in the test cases.

Contributions of Each Team Member
Member 1 (Basant):
- Developing the get32 function and its corresponding maps.
- Loading registers and immediate values.
- Creating the orBits function that eases the excluding of the required bits to be encoded.
- Handling the program counter and creating labels map to perform the bonus part (Using labels for branch and Jal instructions).
- Getting arguments using cmd.

Member 2 (Hashem):
- Developing the get16 function and its corresponding maps.
- Creating files (opcodes.txt, func3.txt, func7.txt).
- Sign extending the immediate values.
- Developing formats for the compressed instructions.
- Handling the program counter and creating labels map to perform the bonus part (Using labels for branch and Jal instructions).

