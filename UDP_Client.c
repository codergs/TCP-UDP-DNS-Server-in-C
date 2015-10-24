//**************************************************************//
//                                                              //
//                     PROJECT: DNS DOSSIER                     //
//                                                              //
//**************************************************************//
//                                                              //
//                  CLIENT END UDP/IP APPLICATION               //
//                                                              //
//**************************************************************//

#include <stdio.h>                  // for printf() and fprintf()
#include <sys/socket.h>             // for socket(), connect(), send(), and recv()
#include <arpa/inet.h>              // for sockaddr_in and inet_addr()
#include <stdlib.h>                 // for atoi() and exit()
#include <string.h>                 // for memset()
#include <unistd.h>                 // for close()
#include <stdbool.h>                // for ip check
#include <ctype.h>

#define RCVBUFSIZE 100              // Size of receive buffer

//***************       FUNCTION PROTYPE        *****************//

void DieWithError(char *errorMessage);

char * toString(char str[], int num);

/* Function to validate the IP address entered by Client */
bool isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}

int main(int argc, char *argv[])
{
    int sock;                           // Socket descriptor
    struct sockaddr_in echoServAddr;    // Echo server address
    struct sockaddr_in fromAddr;        // Source address of echo
    unsigned short  serverPort;         // Echo server port
    unsigned int fromSize;              // In-out of address size for recvfrom()
    char *servIP;                       // IP address of server
    char echoString[RCVBUFSIZE];        // String to send to echo server
    char echoBuffer[RCVBUFSIZE];        // Buffer for receiving echoed string
    unsigned long echoStringLen;        // Length of string to echo
    int respStringLen;                  // Length of received response
    char *domainName;
    char *ipToAdd;
    char *action;
    char str[2];
    
    if ((argc < 4) || (argc > 6))       // Test for correct number of arguments
    {
        printf("\n\t\tNo of command line parametes aren't enough and proper for the request");
        exit(1);
    }
    
    servIP = argv[1];                   // First arg: server IP address (dotted quad)
    serverPort = atoi(argv[2]);         // Use given port, if any
    action = argv[3];
    
    strcpy(echoString,toString(str,argc));
    strcat(echoString, "#");            // Formatting the string to be sent with "#" in between each argument
    strcat(echoString, action);
    strcat(echoString, "#");            // Check if valid action code is entered
    
    if (atoi(action)> 6 || atoi(action) <0)
        DieWithError("Invalid request code entered by the client");
    
    switch (argc){                      // Check for the number of args entered by the Client
        case 5:     domainName = argv[4];
                    strcat(echoString,domainName);
                    strcat(echoString,"#");
                    printf("\nCommand Sent: %s %s %s %s",argv[1],argv[2],argv[3],argv[4] );
                    break;
        
        case 6:
                    //To validate the IP Address format
                    if (isValidIpAddress(argv[5])){
                        domainName = argv[4];
                        strcat(echoString,domainName);  // Concatenate the Domain name to the string
                        strcat(echoString," ");
                        ipToAdd = argv[5];
                        strcat(echoString,ipToAdd);     // Concatenate the IP to the string
                        strcat(echoString,"#");
                        printf("\nCommand Sent: %s %s %s %s %s",argv[1],argv[2],argv[3],argv[4], argv[5]);
                        break;
                    }
                    else
                        DieWithError("Invalid IP Address entered by the client");
            
        default:    printf("\nCommand Sent: %s %s %s",argv[1],argv[2],argv[3]);
                    break;
            
    }
    echoStringLen = strlen(echoString);
    
    /* Create a datagram/UDP socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");
    
    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));     // Zero out structure
    echoServAddr.sin_family = AF_INET;                  // Internet addr family
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);   // Server IP address
    echoServAddr.sin_port   = htons( serverPort);       // Server port
    
    /* Send the string to the server */
    if (sendto(sock, echoString, echoStringLen, 0, (struct sockaddr *)
               &echoServAddr, sizeof(echoServAddr)) != echoStringLen)
        DieWithError("sendto() sent a different number of bytes than expected");
    
    /* Recv a response */
    fromSize = sizeof(fromAddr);
    if ((respStringLen = recvfrom(sock, echoBuffer, RCVBUFSIZE, 0,
                                  (struct sockaddr *) &fromAddr, &fromSize)) < 0)
        DieWithError("recvfrom() failed");
    
    if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
    {
        fprintf(stderr,"Error: received a packet from unknown source.\n");
        exit(1);
    }
       echoBuffer[respStringLen] ='\0';
    
    /* null-terminate the received data */
    printf("\nReceived: %s\n", echoBuffer);             // Print the echoed arg
    
    close(sock);
    exit(0);
}

// Error handling function - Die with error
void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

//toString function - converts integer into string
char * toString(char * str, int num)
{
    int i, rem, len = 0, n;
    
    n = num;
    while (n != 0)                  // Count the number of digits
    {
        len++;
        n /= 10;
    }
    for (i = 0; i < len; i++)       // Convert each digit to a char in the character array
    {
        rem = num % 10;
        num = num / 10;
        str[len - (i + 1)] = rem +'0' ;
    }
    str[len] = '\0';
    
    return str;                     // Return the converted string
}
