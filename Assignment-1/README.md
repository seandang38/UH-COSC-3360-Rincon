# UH-COSC-3360-Assignment-1
Problem:
You need to develop the decoder for a new compression algorithm created by Dr. Rincon to evaluate the effectiveness of Elias-Gamma encoding for larger integer values. The proposed compression algorithm works as follows:
   1. Determine the frequency of symbols in the message.
   2. Sort the alphabet symbols in descending order based on their frequency. If two or more symbols have the same frequency, sort them according to lexicographical        order (ascending based on ASCII value).
   3. Create the encoded message by appending the positions of the symbols (based on the sorted alphabet) in the original message. Since Elias-Gamma encoding is used       to represent the positions, the values for the positions start from one.
Given the following original message: AACABDDADABC
 
The alphabet is sorted as A, D, B, and C, and the positions in the encoded message are: 1, 2, 4, 8, 10, 6, 7, 9, 5, 11, 3, and 12 (represented as Elias-Gamma codes).
 
Your decoder takes the following information as input from the standard input (STDIN):
   1. An integer representing the alphabet size (m).
   2. m symbols. Each symbol is represented by a single character and an integer value indicating its frequency.
   3. A binary string with positions encoded using Elias-Gamma code. The position values are stored according to the order of symbols in the sorted alphabet.
For this assignment, you will create a multithreaded program to find the positions of the symbols in the original message, reconstruct the original message (based on the encoded message), and determine the number of bits required to represent the positions using Elias-Gamma encoding.

Given the following input:
4
A 5
C 2
B 2
D 3
10100010000010000001010001100011100010010010100010110110001100

The expected output is:

Symbol: A, Frequency: 5
Positions: 0 1 3 7 9 
Bits to represent the position(s): 23

Symbol: D, Frequency: 3
Positions: 5 6 8 
Bits to represent the position(s): 17

Symbol: B, Frequency: 2
Positions: 4 10 
Bits to represent the position(s): 12

Symbol: C, Frequency: 2
Positions: 2 11 
Bits to represent the position(s): 10

Decoded message: AACABDDADABC

Process:
 
Your solution must execute the following steps:
 
    1. Read the input lines from STDIN.
    2. Create m POSIX threads (where m is the alphabet size). Each child thread performs the following tasks:
      - Receives the encoded message, the symbol to decode, and the memory location to store the decoding results.
      - Determines the positions of the assigned symbol in the original message.
      - Calculates the number of bits used to represent the symbol's position using Elias Gamma encoding.
      - Stores the assigned symbol's information in a memory location accessible by the main thread.
      - Stores the assigned symbol, using the determined positions, into the memory address shared with the main thread that contains the decoded message.
 
    3. Print the information for each symbol to STDOUT (sorted according to the compression algorithm rules).
    4. Print the decoded message.
