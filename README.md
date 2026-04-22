Objective:\
This assignment will introduce you to interprocess communication mechanisms in UNIX using sockets.\
Problem:\
You must write two programs to implement a distributed version of the multithreaded program you created for programming assignment 1. These programs should find the positions of the symbols in the original message, reconstruct the original message (based on the encoded message), and determine the number of bits required to represent the positions using Elias-Gamma encoding. 

These programs are:

The server program:
 
The user will execute this program using the following syntax:
 
./exec_filename port_no       
 
where exec_filename is the name of your executable file, and port_no is the port number to create the socket. The port number will be available to the server program as a command-line argument.
 
The server program receives no input from STDIN and prints no messages to STDOUT.
 
The server program executes the following task:
    Receive multiple requests from the client program using sockets. Therefore, the server program creates a child process for each request to handle them simultaneously. For this reason, the parent process must handle zombie processes by implementing the fireman() function call (unless you can determine the number of requests the server program receives from the client program). 

Each child process executes the following tasks:\
    1. Receive the encoded message, the assigned symbol, and the sum of the frequencies for the symbols assigned to the previous threads on the client program.\
    2. Determines the positions of the assigned symbol in the original message.\
    3. Calculates the number of bits used to represent the symbol's position using Elias Gamma encoding.\
    4. Send the positions and the number of bits used to represent the symbol's positions to the client program using sockets.

The client program:
 
The user will execute this program using the following syntax:
 
./exec_filename hostname port_no < input_filename
 
where exec_filename is the name of your executable file, hostname is the address where the server program is located, port_no is the port number used by the server program, and input_filename is the name of the input file. The hostname and the port number will be available to the client as command-line arguments.
 
The client program receives multiline input in accordance with the specifications for Programming Assignment 1.
 
Input example:
4                       \
A 5                    \
C 2                   \
B 2                    \
D 3                       \
10100010000010000001010001100011100010010010100010110110001100 
 
After reading the information from STDIN, the program must create n child threads (where n is the alphabet size). Each child thread executes the following tasks: \
    1. Receives the encoded message, the symbol to decode, and the information needed to find the positions of the assigned symbol in the encoded message.\
    2. Send the received information to the server program using sockets.\
    3. Receive the positions and number of bits used to represent the symbol's positions from the server program using sockets.\
    4. Store the received information in a memory location accessible by the main thread.\
    5. Stores the assigned symbol, using the determined positions, into the memory address shared with the main thread that contains the decoded message.\
Finally, the main thread prints the information for each symbol to STDOUT (sorted according to the compression algorithm rules) and the decoded message.
 
The expected output (given the input above) is:
 
Symbol: A, Frequency: 5                                          \
Positions: 0 1 3 7 9                                             \
Bits to represent the position(s): 23                            

Symbol: D, Frequency: 3                                          \
Positions: 5 6 8                                                 \
Bits to represent the position(s): 17                            

Symbol: B, Frequency: 2                                          \
Positions: 4 10                                                  \
Bits to represent the position(s): 12                             

Symbol: C, Frequency: 2                                          \
Positions: 2 11                                                   \
Bits to represent the position(s): 10

Decoded message: AACABDDADABC

Notes:
 
The position values for the symbols must start from 1 (not 0), since Elias Gamma can only represent positive integers starting from 1.\
You can safely assume that the input will always be in the proper format.\
You must use the output statement format based on the example above.\
For the client program, you must use POSIX Threads and stream sockets. A penalty of 100% will be applied to submissions not using POSIX Threads and Stream Sockets.\
For the server program, you must use multiple processes (fork) and stream sockets. Submissions that do not use multiple processes and Stream Sockets will be penalized by 100%.\
The Moodle server will kill your server program after executing each test case.\
You must present code that is readable and has comments explaining the logic of your solution. A 10% penalty will be applied to submissions that do not follow this guideline.\
You cannot use global variables. A 100% penalty will be applied to submissions using global variables.\
A 100% penalty will be applied to solutions that do not compile.\
A 100% penalty will be applied to solutions that hardcode the output.\
