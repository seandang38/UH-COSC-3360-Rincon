#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h> 
#include <sys/wait.h> 
#include <signal.h>  

//fireman function
//this code is provided by Dr.Rincon
void fireman(int) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main( int argc, char *argv[]) {
    //this code is provided by Dr.Rincon
    int sockfd, newsockfd, portno, clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    // Creating a signal to execute the fireman function when a child process end its execution
    signal(SIGCHLD, fireman);

    // Check for the port number from the command line
    //this code is provided by Dr.Rincon
    if(argc < 2) {
        std::cerr << "ERROR no port provided\n";
        exit(1);
    }

    //create a socket descriptor
    //this code is provided by Dr.Rincon
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "ERROR opening socket\n";
        exit(1);
    }
    
    //this code is provided by Jason-TA
    int yes=1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("setsockopt");
        exit(1);
    }
    
    //Populate the sockaddr_in structure
    //this code is provided by Dr.Rincon
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    // Bind the socket descriptor with the sockaddr_in structure
    //this code is provided by Dr.Rincon
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "ERROR on binding\n";
        exit(1);
    }

    // Set the maximum number of concurrent connections
    //this code is provided by Dr.Rincon
    listen(sockfd, 20);
    clilen = sizeof(cli_addr);

    //this code is provided by Dr.Rincon
    while (true) {
        // Accept a request from the client. A new socket descriptor is created to handle the request
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
        
        //this code is provided by Dr.Rincon
        if (fork() == 0) {
            if (newsockfd < 0) {
                std::cerr << "ERROR on accept";
                exit(1);
            }

            //this code is provided by Dr.Rincon
            int msgSize = 0;
            n = read(newsockfd, &msgSize, sizeof(int));
            if (n <= 0) {
                std::cerr << "ERROR reading size\n";
                _exit(1);
            }

            //this code is provided by Dr.Rincon
            char *tempBuffer = new char[msgSize + 1];
            bzero(tempBuffer, msgSize + 1);
            n = read(newsockfd, tempBuffer, msgSize);
            if (n <= 0) {
                std::cerr << "ERROR reading from socket\n";
                _exit(1);
            }

            std::string requestStr = tempBuffer;
            delete[] tempBuffer;

            std::stringstream ss(requestStr);
            int asciiVal;
            int frequency, skipCount;
            std::string encodedMassage;

            ss >> asciiVal >> frequency >> skipCount >> encodedMassage;
            
            char symbol = (char)asciiVal;

            //Elias-gamma decodeing logic from assignment 1
            int totalBitsUsed = 0;
            int current_index = 0;
            int number_decoded = 0;
            int target_count = skipCount + frequency;

            std::string responseStr = "";

            while (number_decoded < target_count) {
                int zero_counts = 0;
                while (encodedMassage[current_index] == '0') {
                    zero_counts++;
                    current_index++;
                }

                std::string binStr = encodedMassage.substr(current_index, zero_counts + 1);
                int position = stoi(binStr, nullptr, 2);
                int bitUsed = 2 * zero_counts + 1;

                //check if this position should be decoded for this thread or skipped
                if (number_decoded >= skipCount) {
                    totalBitsUsed = totalBitsUsed + bitUsed;
                    responseStr = responseStr + std::to_string(position) + " ";
                }

                current_index = current_index + (zero_counts + 1);
                number_decoded++;
            }

            responseStr = std::to_string(totalBitsUsed) + " " + responseStr;

            //this code is provided by Dr.Rincon
            msgSize = responseStr.size();
            n = write(newsockfd, &msgSize, sizeof(int)); // Send size
            if (n < 0) { 
                std::cerr << "Error writing size\n"; 
                _exit(1); 
            }

            //this code is provided by Dr.Rincon
            n = write(newsockfd, responseStr.c_str(), msgSize); // Send string
            if (n < 0) { 
                std::cerr << "Error writing data\n"; 
                _exit(1); 
            }

            // Clean up and kill the child process
            close(newsockfd);
            _exit(0);
        }
        close(newsockfd);
    }
    close(sockfd);
    return 0;
}