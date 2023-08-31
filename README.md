# SIMP_Simulator
A simulator for SIMP processor (invented processor - university project)

This simulator is build in C language. 
The simulator simulate the fetch-decode-execute loop of the processor.

**This is how the Simulator works**:

The simulator loads the information needed from the memin.txt (which contains all the instructions in machine language (Hexa).

Than, the simulator goes over the instructions and execute each instruction according to the definition of the instructions.

While the code runs there is stop options in case of an illegal instruction is given (like writing to $zero or $imm registers.

The output of the simulator is four files:

1. Cycles – the number of instructions the simulator executed for the whole simulation.
2. Regout – The information that each register holds in the end of the simulation.
3. Trace – every line in the trace file holds a single execution and shows its number (cycle), the instruction (opcode, rd, rs, rt, immeditate), all the data that each register currently holds.
4. Memout – the memory at the end of the simulation.
   
The simulator ends when it gets an HALT instruction.

**The structure of the code:**

•	Structs defining: defining two structs that will be used in the simulator:

  1.	**Inst**: contains five fields: opcode, rd, rs, rt, immediate. The fields are updated while the simulator runs according the the current instruction.
  2.	**Files**: holds the five files the simulator works with: memin, memout, regout, trace, cycles.

•	Decleration for the function we are using in the code

•	main function:

creation of three arrays that will be used in the simulator:
  1.	int array called “R” that will hold the data of the registers, initialized with zeros.
  2.	Int array called “mem” which is dynamically allocated and holds the memory of the processor for all the simulation and prints its content at the end in the memout file.
  3.	Inst Instructions array: an array which holds all the instructions from the memin file.

Than, the files are opened and checked to be opened correctly.

In case the files are opened correctly, the simulator starts with the execution of the instructions.
At the end, the desired data is printed to the output files, the files are closed and the memory which was dynamiccaly allocated is free.
The last part of the code is helping function used in the code.  
