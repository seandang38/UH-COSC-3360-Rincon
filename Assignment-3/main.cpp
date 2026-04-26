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
    
    int thread_id; 
    
    pthread_mutex_t *arg_mutex;
    pthread_cond_t *arg_cv;
    bool *arg_copied;

    pthread_mutex_t *print_mutex;
    pthread_cond_t *print_cv;
    int *current_turn;
};

void *threadFunction(void *arg_void_ptr)
{
    arguments *arg_ptr = (arguments*)arg_void_ptr;

    // read arguments
    pthread_mutex_lock(arg_ptr->arg_mutex);

    //store local copies
    const std::string* encodedMessage = arg_ptr->encodedMessage;
    char* decodedMessage = arg_ptr->decodedMessage;
    char symbol = arg_ptr->symbol;
    int frequency = arg_ptr->frequency;
    int skipCount = arg_ptr->skipCount;
    int thread_id = arg_ptr->thread_id;
    
    pthread_mutex_t *print_mutex = arg_ptr->print_mutex;
    pthread_cond_t *print_cv = arg_ptr->print_cv;
    int *current_turn = arg_ptr->current_turn;

    // Signal main thread 
    *(arg_ptr->arg_copied) = true;
    pthread_cond_signal(arg_ptr->arg_cv);
    pthread_mutex_unlock(arg_ptr->arg_mutex);

   
    int current_index = 0;      //current index in the encoded message
    int number_decoded = 0;     //total elias gamma proccessed so far
    int target_count = skipCount + frequency;     //total number decoded (including skipped) before this thread finishes
    int position_index = 0;
    
    int totalBitsUsed = 0;
    int *positions = new int[frequency]; // Moved inside thread so each thread has its own array

    while(number_decoded < target_count)
    {
        int zero_counts = 0;
        while((*encodedMessage)[current_index] == '0')     //count 0s until hitting the first 1
        {
            zero_counts++;
            current_index++;
        }

        //extract and convert binary string to decimal integer
        std::string binStr = encodedMessage->substr(current_index, zero_counts + 1);
        int position = std::stoi(binStr, nullptr, 2);

        int bitUsed = 2 * zero_counts + 1;

        //check if this position should be decoded for this thread or skipped
        if (number_decoded >= skipCount)
        {
            totalBitsUsed = totalBitsUsed + bitUsed;
            positions[position_index++] = position - 1;        // Store 0-based position
            decodedMessage[position - 1] = symbol;    // Place symbol in decoded message
        }
        current_index = current_index + (zero_counts + 1);
        number_decoded++;
    }

    //Oder printing
    pthread_mutex_lock(print_mutex);

    // Wait until it's this thread turn
    while (*current_turn != thread_id) {
        pthread_cond_wait(print_cv, print_mutex);
    }

    std::cout << "Symbol: " << symbol << ", Frequency: " << frequency << "\n";
    std::cout << "Positions:";
    for (int j = 0; j < frequency; j++)
    {
        std::cout << " " << positions[j];
    }
    std::cout << "\nBits to represent the position(s): " << totalBitsUsed << "\n\n";

    // Pass the turn to the next thread
    (*current_turn)++;
    pthread_cond_broadcast(print_cv);
    pthread_mutex_unlock(print_mutex);

    delete[] positions;
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
    char *decodedMessage = new char[total_length];
    for (int i = 0; i < total_length; i++) {
        decodedMessage[i] = ' ';
    }
    
    int current_skipCount = 0;


    arguments arg; 
    pthread_mutex_t arg_mutex;
    pthread_cond_t arg_cv;
    bool arg_copied;

    pthread_mutex_t print_mutex;
    pthread_cond_t print_cv;
    int current_turn = 0;

    // create sync tools
    pthread_mutex_init(&arg_mutex, NULL);
    pthread_cond_init(&arg_cv, NULL);
    pthread_mutex_init(&print_mutex, NULL);
    pthread_cond_init(&print_cv, NULL);

    // Map struct pointers to the local variables
    arg.arg_mutex = &arg_mutex;
    arg.arg_cv = &arg_cv;
    arg.arg_copied = &arg_copied;
    arg.print_mutex = &print_mutex;
    arg.print_cv = &print_cv;
    arg.current_turn = &current_turn;

    for (int i=0; i<m; i++)
    {
        pthread_mutex_lock(&arg_mutex);

        arg.encodedMessage = &binaryString;
        arg.decodedMessage = decodedMessage;
        arg.frequency = alphabet[i].frequency;
        arg.symbol = alphabet[i].character;
        arg.skipCount = current_skipCount;
        arg.thread_id = i;
        arg_copied = false; 

        //call pthread_create 
        if(pthread_create(&tid[i], nullptr, threadFunction, (void*)&arg))
        {
            fprintf (stderr, "Error creating thread\n");
            return 1;
        }

        // Wait for thread to copy data
        while (!arg_copied) {
            pthread_cond_wait(&arg_cv, &arg_mutex);
        }
        pthread_mutex_unlock(&arg_mutex);

        current_skipCount = current_skipCount + alphabet[i].frequency;
    }

    //wait for all threads to finish
    //call pthread_join
    for (int i=0; i<m; i++)
    {
        pthread_join(tid[i], nullptr);
    }

    // Main thread prints the decoded message
    std::cout << "Decoded message: ";
    for (int i=0; i<total_length; i++)
    {
        std::cout << decodedMessage[i];
    }
    std::cout << "\n";

    pthread_mutex_destroy(&arg_mutex);
    pthread_cond_destroy(&arg_cv);
    pthread_mutex_destroy(&print_mutex);
    pthread_cond_destroy(&print_cv);

    delete[] tid;
    delete[] alphabet;
    delete[] decodedMessage;    

    return 0;
}