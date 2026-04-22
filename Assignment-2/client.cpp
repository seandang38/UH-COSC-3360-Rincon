#include <unistd.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <sstream> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <strings.h>
#include <pthread.h>

struct Symbol
{
    char character;
    int frequency;
};

bool compareSymbols(Symbol a, Symbol b)
{
    if(a.frequency == b.frequency)
    {
        return a.character < b.character; // Ascending ASCII 
    }
    return a.frequency > b.frequency; // Descending order by frequency
}
struct arguments
{
    const char *hostname;           //server's address
    int port;                       //server's port number

    const std::string* encodedMessage;
    char* decodedMessage;

    char symbol;
    int frequency;
    int skipCount;      //how many positions to ignore before starting to decode

    int *positions;     //stores the 0 based positions
    int totalBitsUsed;
};

void *communicateWithServer (void *arg_void_ptr)
{
    arguments *arg_ptr = (arguments *)arg_void_ptr;
    int n;

    //create stream socket
    //this code is provided by Dr.Rincon
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "ERROR opening socket\n";
        exit(0);
    }

    //setup server address
    //this code is provided by Dr.Rincon
    struct hostent *server = gethostbyname(arg_ptr->hostname);
    if (server == NULL) {
        std::cerr << "ERROR, no such host\n";
        exit(0);
    }

    //this code is provided by Dr.Rincon
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(arg_ptr->port);

    //connect to server
    //this code is provided by Dr.Rincon
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "ERROR connecting\n");
        exit(1);
    }

    std::string request = std::to_string((int)arg_ptr->symbol) + " " + 
                          std::to_string(arg_ptr->frequency) + " " +
                          std::to_string(arg_ptr->skipCount) + " " +
                          *(arg_ptr->encodedMessage);
    
    //this code is provided by Dr.Rincon
    int msgSize = request.size();
    //send size
    n = write(sockfd, &msgSize, sizeof(int));
    if (n < 0) {
        std::cerr << "ERROR writing to socket" << std::endl;
        exit(0);
    }

    //send string
    //this code is provided by Dr.Rincon
    n = write(sockfd, request.c_str(), msgSize);
    if (n < 0) {
        std::cerr << "ERROR writing to socket" << std::endl;
        exit(0);
    }

    //read size
    //this code is provided by Dr.Rincon
    int respSize = 0;
    n = read(sockfd, &respSize, sizeof(int));
    if (n <= 0) {
        std::cerr << "ERROR reading from socket" << std::endl;
        exit(0);
    }

    //read string
    //this code is provided by Dr.Rincon
    char *tempBuffer = new char[respSize + 1];
    bzero(tempBuffer, respSize + 1);
    n = read(sockfd, tempBuffer, respSize);
    if (n <= 0) {
        std::cerr << "ERROR reading from socket" << std::endl;
        exit(0);
    }

    std::string responseStr = tempBuffer;
    delete[] tempBuffer;

    //store data
    std::stringstream ss(responseStr);
    ss >> arg_ptr->totalBitsUsed; 

    for (int i = 0; i < arg_ptr->frequency; i++) {
        int pos;
        ss >> pos;
        arg_ptr->positions[i] = pos;
        arg_ptr->decodedMessage[pos - 1] = arg_ptr->symbol;
    }

    close(sockfd);
    return NULL;
};

int main(int argc, char *argv[]) 
{
    //this code is provided by Dr.Rincon
    if (argc != 3) {
        std::cerr << "usage " << argv[0] << "hostname port\n";
        exit(1);
    }

    const char *hostname = argv[1];
    int port = std::stoi(argv[2]);

    int m;
    std::cin >> m;
    Symbol *alphabet = new Symbol[m];
    int total_length = 0;

    //read the symbols and frequencies
    std::string line;
    std::getline(std::cin, line); //consume the newline after reading m
    for (int i=0; i<m; i++)
    {
        std::getline(std::cin, line);
        alphabet[i].character = line[0];
        alphabet[i].frequency = std::stoi(line.substr(1)); //convert frequency from string
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
        arg[i].hostname = hostname;
        arg[i].port = port;
        arg[i].encodedMessage = &binaryString;
        arg[i].decodedMessage = decodedMessage;
        arg[i].frequency = alphabet[i].frequency;
        arg[i].symbol = alphabet[i].character;
        arg[i].skipCount = current_skipCount;
        arg[i].totalBitsUsed = 0;
        arg[i].positions = new int[alphabet[i].frequency];
        current_skipCount = current_skipCount + alphabet[i].frequency;

        //call pthread_create
        if(pthread_create(&tid[i], nullptr, communicateWithServer, (void*)&arg[i]))
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
        for (int j = 0; j < arg[i].frequency; j++)
        {
            std::cout << " " << arg[i].positions[j] - 1;
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
