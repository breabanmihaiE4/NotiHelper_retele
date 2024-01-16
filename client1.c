
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>

extern int errno;

int port;

void clearBuffer() {
    char buffer[100];
    fgets(buffer, sizeof(buffer), stdin);
}

void receivedMassagesFromServer(int socketDescriptor, bool clientIsOnline) {
    char messages[500];
    char serverMessage[50];
    bool haveMessageToReceive = true;

    while(haveMessageToReceive) {
        if(read(socketDescriptor, messages, 500) <= 0) {
            perror ("[client]Read error at line 225\n");
            clientIsOnline = false;
        }

        if(strcmp(messages, "No more messages!") == 0) {
            haveMessageToReceive = false;
        } else {
            printf("~Message~ %s", messages);
        }

        strcpy(serverMessage, "Message received!");
        if (write(socketDescriptor, serverMessage, 50) <= 0) {
            perror("[client] Write error at line 234");
            clientIsOnline = false;
        }
    }
}

int main (int argc, char *argv[]) {
    int socketDescriptor;
    struct sockaddr_in server; 
    char clientMessage[50];
    char serverMessage[50];

    if (argc != 3) {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

    port = atoi (argv[2]);

    if ((socketDescriptor = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
      perror ("Socket error.\n");
      return errno;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons (port);

    if (connect (socketDescriptor, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1) {
        perror ("[client] Connection error.\n");
        return errno;
    } 

    bool commandIsLogin = false;
    bool commandIsRegister = false;
    bool commandIsExit = false;

    bool commandIsSetNotification = false;
    bool commandIsSendMessage = false;
    bool commandIsSeeSentMessages = false;
    bool commandIsSeeReceivedMessages = false;
    bool commandIsDisconnect = false;

    char command[10] = {'0'};

    char commandLogin[10] = {'1'};
    char commandRegister[10] = {'2'};
    char commandExit[10] = {'3'};
    
    char commandSetNotification[10] = {'1'};
    char commandSendMessage[10] = {'2'};
    char commandSeeSentMessages[10] = {'3'};
    char commandSeeReceivedMessages[10] = {'4'};
    char commandDisconnect[10] = {'5'};

    char username[50] = {'\0'};

    bool loginIsSuccessful = false;
    bool clientIsOnline = true;

    while(true) {
        if(loginIsSuccessful) {
            printf("\nYou are logged in as %s\n", username);
            printf("\n1. Set notification\n");
            printf("2. Send message\n");
            printf("3. See Sent messages\n");
            printf("4. See Received messages\n");
            printf("5. Disconnect\n");
            printf("Select your command: ");
        } else {
            printf("\n1. Login\n");
            printf("2. Register\n");
            printf("3. Exit\n");
            printf("Select your command: ");
        }

        scanf("%c", command);
        clearBuffer();
        
        if (write (socketDescriptor, command, 10) <= 0) {
            perror ("[client]Write error at line 88.\n");
        }

        if (loginIsSuccessful) {
            commandIsLogin = false;
            commandIsRegister = false;
            commandIsExit = false;

            commandIsSetNotification = strcmp(command, commandSetNotification) == 0;
            commandIsSendMessage = strcmp(command, commandSendMessage) == 0;
            commandIsSeeSentMessages = strcmp(command, commandSeeSentMessages) == 0;
            commandIsSeeReceivedMessages = strcmp(command, commandSeeReceivedMessages) == 0;
            commandIsDisconnect = strcmp(command, commandDisconnect) == 0;
        } else {
            commandIsLogin = strcmp(command, commandLogin) == 0;
            commandIsRegister = strcmp(command, commandRegister) == 0;
            commandIsExit = strcmp(command, commandExit) == 0;

            commandIsSetNotification = false;
            commandIsSendMessage = false;
            commandIsSeeSentMessages = false;
            commandIsSeeReceivedMessages = false;
            commandIsDisconnect = false;
        }

        if(commandIsLogin) {
            printf("\nCommand is Login! \n");

            printf("Type your username: ");
            scanf("%s", username);
            clearBuffer();

            if (write (socketDescriptor, username, 50) <= 0) {
                perror ("[client]Write error at line 119.\n");
            }

            if(read(socketDescriptor, serverMessage, 50) <= 0){
                perror ("[client]Read error at line 123\n");
                clientIsOnline = false;
            }

            loginIsSuccessful = strcmp(serverMessage, "Login is successful!") == 0;

          } else if(commandIsRegister) {
            printf("\nCommand is Register!\n");

            printf("Type your wanted username: ");
            scanf("%s", username);
            clearBuffer();

            if (write (socketDescriptor, username, 50) <= 0) {
                perror ("[client]Write error at line 144.\n");
            }

            if(read(socketDescriptor, serverMessage, 50) <= 0){
                perror ("[client]Read error at line 148\n");
                clientIsOnline = false;
            }

            loginIsSuccessful = strcmp(serverMessage, "Username was created successfuly") == 0;

            if(loginIsSuccessful) {
                printf("This username is good:)))\n");
            } else {
                printf("This username already exist:(\n");
            }
            

          } else if(commandIsExit) {
            printf("\nCommand is Exit!\n");
            close(socketDescriptor);
            return 0;
            
          } else if(commandIsSetNotification) {
            printf("\nCommand is SetNotification!\n");

            printf("In how many seconds do you want to be notified? ");

            char secondsString[50];
            scanf("%s", secondsString);
            clearBuffer();

            if(write(socketDescriptor, secondsString, 50) <= 0) {
                perror ("[client]Write error at line 213.\n");
                clientIsOnline = false;
            }

            int seconds;
            int minutes;
            seconds = atoi(secondsString);
            
            if(seconds % 60 == 0) {
                minutes = seconds / 60;
                seconds = 0;
            } else {
                minutes = seconds / 60;
                seconds = seconds % 60;
            }

            if(read(socketDescriptor, serverMessage, 50) <= 0) {
                perror ("[client]Read error at line 218\n");
                clientIsOnline = false;
            }

            if(strcmp(serverMessage, "Time for notification was set successfuly!") == 0) {
                printf("Time for notification was set successfuly!\n");
            }
            
            printf("You will be notified in %d minutes and %d seconds\n", minutes, seconds);

            printf("Type your message for notification: ");
            char messageForNotification[500];
            fgets(messageForNotification, 500, stdin);
            messageForNotification[strlen(messageForNotification) - 1] = '\0';

            printf("message is: %s\n", messageForNotification);

            if(write(socketDescriptor, messageForNotification, 500) <= 0) {
                perror ("[client]Write error at line 239.\n");
                clientIsOnline = false;
            }

            if(read(socketDescriptor, serverMessage, 50) <= 0) {
                perror ("[client]Read error at line 244\n");
                clientIsOnline = false;
            }

            if(strcmp(serverMessage, "Message for notification was set successfuly!") == 0) {
                printf("Message for notification was set successfuly!\n");
            }

            printf("Waiting for notification...\n");
            printf("\n");

            if(read(socketDescriptor, serverMessage, 50) <= 0) {
                perror ("[client]Read error at line 255\n");
                clientIsOnline = false;
            }

            if(strcmp(serverMessage, "You have a new notfication!") == 0) {
                printf("Your notification!\n");
                printf("Message: %s", messageForNotification);
            }

            printf("\n");
          } else if(commandIsSendMessage) {
            printf("\nCommand is SendMessage!\n");

            char userReceiver[50];
            char message[200];
            bool userReceiverIsFound = false;
            bool messageWasSent = false;

            printf("Type to whom you want to send a message: ");
            scanf("%s", userReceiver);
            clearBuffer();

            if (write(socketDescriptor, userReceiver, 50) <= 0) {
                perror("[client] Write error at line 177");
                clientIsOnline = false;
            }

            if(read(socketDescriptor, serverMessage, 50) <= 0) {
                perror ("[client]Read error at line 148\n");
                clientIsOnline = false;
            }

            userReceiverIsFound = strcmp(serverMessage, "Reciever user was found") == 0;

            if(userReceiverIsFound) {
                printf("Type your message for %s: ", userReceiver);
                fgets(message, 200, stdin);
                message[strlen(message) - 1] = '\0';
                clearBuffer();

                if (write(socketDescriptor, message, 200) <= 0) {
                    perror("[client] Write error at line 196");
                    clientIsOnline = false;
                }

                if(read(socketDescriptor, serverMessage, 50) <= 0) {
                    perror ("[client]Read error at line 148\n");
                    clientIsOnline = false;
                }

                messageWasSent = strcmp(serverMessage, "Message successfuly sent!") == 0;

                if(messageWasSent) {
                    printf("Message successfuly sent!\n");
                } else {
                    printf("Message was not sent!\n");
                }
            } else {
                printf("This username for your reciever can not be found :((( \n");
            }

          } else if(commandIsSeeSentMessages) {
            printf("\nCommand is SeeMessages!\n");
            receivedMassagesFromServer(socketDescriptor, clientIsOnline);
            printf("\n");
          }else if(commandIsSeeReceivedMessages) {
            printf("\nCommand is SeeReceivedMessages!\n");
            receivedMassagesFromServer(socketDescriptor, clientIsOnline);
            printf("\n");
          } else if(commandIsDisconnect) {
            printf("\nCommand is Disconnect!\n");  
            
            loginIsSuccessful = false;        
            username[0] = '\0';          
          }
  }
}



// ./client 10.0.2.15 2024
// ./server 
// gcc client.c -o client
// gcc server.c -o server