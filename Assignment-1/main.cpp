#include <iostream>
#include <unistd.h>
#include <string>
#include <pthread.h>
#include <algorithm>

struct symbol
{
    char character;
    int frequency;
};

bool compareSymbols(symbol a, symbol b)
{
    if(a.frequency == b.frequency)
    {
        return a.character < b.character; // Ascending ASCII 
    }
    return a.frequency > b.frequency; // Descending order by frequency
}
struct arguments
{
    const std::string* encodedMessage;
    char* decodedMessage;
    char symbol;
    int frequency;
    int skipCount;      //how many positions to ignore before starting to decode
    int *positions;     //stores the 0 based positions
    int totalBitsUsed;
};

void *threadFunction(void *arg_void_ptr)
{
    arguments *arg_ptr = (arguments*)arg_void_ptr;

    int current_index = 0;      //current index in the encoded message
    int number_decoded = 0;     //total elias gamma proccessed so far
    int target_count = arg_ptr->skipCount + arg_ptr->frequency;     //total number decoded (including skipped) before this thread finishes
    int position_index = 0;

    while(number_decoded < target_count)
    {
        int zero_counts = 0;
        while((*arg_ptr->encodedMessage)[current_index] == '0')     //count 0s until hitting the first '1'
        {
            zero_counts++;
            current_index++;
        }

        //extract and convert binary string to decimal integer
        std::string binStr = arg_ptr->encodedMessage->substr(current_index, zero_counts + 1);
        int position = std::stoi(binStr, nullptr, 2);

        int bitUsed = 2 * zero_counts + 1;

        //check if this position should be decoded for this thread or skipped
        if (number_decoded >= arg_ptr->skipCount)
        {
            arg_ptr->totalBitsUsed = arg_ptr->totalBitsUsed + bitUsed;
            arg_ptr->positions[position_index++] = position - 1;        // Store 0-based position
            arg_ptr->decodedMessage[position - 1] = arg_ptr->symbol;    // Place symbol in decoded message
        }
        current_index = current_index + (zero_counts + 1);
        number_decoded++;
    }
    return nullptr;
}

int main() 
{
    int m;
    std::cin >> m;
    symbol *alphabet = new symbol[m];
    int total_length = 0;

    //read the symbols and frequencies
    std::string line;
    std::getline(std::cin, line); //consume the newline after reading m
    for (int i=0; i<m; i++)
    {
        std::getline(std::cin, line);
        alphabet[i].character = line[0];
        alphabet[i].frequency = std::stoi(line.substr(2)); //convert frequency from string
        total_length = total_length + alphabet[i].frequency;
    }

    std::string binaryString;
    std::cin >> binaryString;

    //sort the alphabet by frequency and then by character
    std::sort(alphabet, alphabet + m, compareSymbols);

    pthread_t *tid = new pthread_t[m];
    arguments *arg = new arguments[m];
    char *decodedMessage = new char[total_length];
    for (int i = 0; i < total_length; i++) {
        decodedMessage[i] = ' ';
    }
    int current_skipCount = 0;

    for (int i=0; i<m; i++)
    {
        arg[i].encodedMessage = &binaryString;
        arg[i].decodedMessage = decodedMessage;
        arg[i].frequency = alphabet[i].frequency;
        arg[i].symbol = alphabet[i].character;
        arg[i].skipCount = current_skipCount;
        arg[i].totalBitsUsed = 0;
        arg[i].positions = new int[alphabet[i].frequency];
        current_skipCount = current_skipCount + alphabet[i].frequency;

        //call pthread_create
        if(pthread_create(&tid[i], nullptr, threadFunction, (void*)&arg[i]))
        {
            fprintf (stderr, "Error creating thread\n");
            return 1;
        }
    }

    //wait for all threads to finish
    //call pthread_join
    for (int i=0; i<m; i++)
    {
        pthread_join(tid[i], nullptr);
    }

    for (int i=0; i<m; i++)
    {
        std::cout << "Symbol: " << arg[i].symbol << ", Frequency: " << arg[i].frequency << "\n";
        std::cout << "Positions: ";
        for (int j=0; j<arg[i].frequency; j++)
        {
            std::cout << " " << arg[i].positions[j];
        }
        std::cout << "\nBits to represent the position(s): " << arg[i].totalBitsUsed << "\n\n";
    }
    std::cout << "Decoded message: ";
    for (int i=0; i<total_length; i++)
    {
        std::cout << decodedMessage[i];
    }
    std::cout << "\n";

    for (int i=0; i<m; i++)
    {
        delete[] arg[i].positions;
    }
    delete[] arg;
    delete[] tid;
    delete[] alphabet;
    delete[] decodedMessage;    

    return 0;
}
