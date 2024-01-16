
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <wait.h>
#include <stdbool.h>
#include <time.h>

#define PORT 2024
#define LOGIN_FILE "login.txt"

extern int errno;

void sighandler(int signal) {
  while (wait(NULL));
}

bool usernameIsRegistered(char username[50]) {
  FILE *file;
  file = fopen(LOGIN_FILE, "r");

  char fileUsername[50];
  bool usernameFound = false;

  printf("Username is: %s\n", username);

  while (fgets(fileUsername, 50, file) != NULL && usernameFound == false) {
    usernameFound = strncmp(fileUsername, username, strlen(username)) == 0;
  }

  fclose(file);
  return usernameFound;
}

bool registerUser(char username[50]) {
  if(usernameIsRegistered(username)) {
    printf("Username already registered!\n");
    return false;
  } else {
    FILE *file;
    file = fopen(LOGIN_FILE, "a");
    fputs(username, file);
    fputs("\n", file);
    fclose(file);

    printf("Register successfully!\n");
    return true;
  }
}

bool sendMessageToReciever(char username[50], char userReceiver[50], char message[200]) {
  char fullMessage[400] = {'\0'}; 

  const char ARROW[10] = {" -> "};
  const char TEXTSCRIPT[15] = {" message: "};
  const char ENTER[5] = {"\n"};

  strcat(fullMessage, username);
  strcat(fullMessage, ARROW);
  strcat(fullMessage, userReceiver);
  strcat(fullMessage, TEXTSCRIPT);
  strcat(fullMessage, message);
  strcat(fullMessage, ENTER);

  FILE * file;
  file = fopen("emailbox.txt", "a");
  fputs(fullMessage, file);
  fclose(file);
  return true;
}

bool youAreTheReceiver(char username[50], char message[500]) {
  char messageAuxiliary[500];
  strcpy(messageAuxiliary, message);

  int numberOfWords = 0;
  int position = 0;

  while(numberOfWords < 2) { 
    if(messageAuxiliary[position] == ' ') {
      numberOfWords++;
    }
    position++;
  }
  
  strcpy(messageAuxiliary, messageAuxiliary + position);
  
  return strncmp(messageAuxiliary, username, strlen(username)) == 0;
} 
  
bool needToSendNotification(int totalSecondsForNotification) {

  time_t my_time;
  struct tm * localTime; 
  time (&my_time);
  localTime = localtime(&my_time);
  int hour = localTime->tm_hour;
  int minute = localTime->tm_min;
  int second = localTime->tm_sec;
  printf("Current time is: %02d:%02d:%02d\n", hour, minute, second);

  int hoursForNotification = hour + totalSecondsForNotification / 3600;
  int minutesForNotification = minute + (totalSecondsForNotification % 3600) / 60;
  int secondsForNotification = second + (totalSecondsForNotification % 3600) % 60;

  if(secondsForNotification > 60) {
    int aux = secondsForNotification / 60;
    minutesForNotification = minutesForNotification + aux;
    secondsForNotification = secondsForNotification % 60;
  }

  if(minutesForNotification > 60) {
    int aux = minutesForNotification / 60;
    hoursForNotification = hoursForNotification + aux;
    minutesForNotification = minutesForNotification % 60;
  }
  printf("Notification time is: %02d:%02d:%02d\n", hoursForNotification, minutesForNotification, secondsForNotification);

  int totalSeconds = hour * 3600 + minute * 60 + second;

  bool timeIsEqual = false;


  while(timeIsEqual == false) {
    if(hour == hoursForNotification && minute == minutesForNotification) {
      timeIsEqual = true;
      printf("Time is equal!\n");
    }

    time_t my_time;
    struct tm * localTime; 
    time (&my_time);
    localTime = localtime(&my_time);
    hour = localTime->tm_hour;
    minute = localTime->tm_min;
    second = localTime->tm_sec;
  }
  
  return timeIsEqual; 
}

int main()
{
  struct sockaddr_in server;
  struct sockaddr_in from;
  char clientMessage[100];
  char serverMessage[100];
  int socketDescriptor;

  if ((socketDescriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("[server]Socket error().\n");
    return errno;
  }

  bzero(&server, sizeof(server));
  bzero(&from, sizeof(from));

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(PORT);

  if (bind(socketDescriptor, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
    perror("[server]Eroare la bind().\n");
    return errno;
  }

  if (listen(socketDescriptor, 5) == -1) {
    perror("[server]Eroare la listen().\n");
    return errno;
  }

  while (true) {
    int clientSocketDescriptor;
    int length = sizeof(from);

    printf("[server]Wait at port: %d...\n", PORT);
    fflush(stdout);

    clientSocketDescriptor = accept(socketDescriptor, (struct sockaddr *)&from, &length);

    if (clientSocketDescriptor < 0) {
      perror("[server]Client acceptance error ()).\n");
      continue;
    }

    int pid = fork();

    if (pid < 0) {
      fprintf(stderr, "Fork error!");
      exit(1);
    }

    if (pid == 0) {
        printf("[server] Accepted as clientSocketDescriptor\n");
        printf("\n");

        bool commandIsLogin = false;
        bool commandIsRegister = false;
        bool commandIsExit = false;

        bool commandIsSetNotification = false;
        bool commandIsSendMessage = false;
        bool commandIsSeeSentMessages = false;
        bool commandIsSeeReceivedMessages = false;
        bool commandIsDisconnect = false;

        char clientCommand[10] = {'0'};

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

        while (clientIsOnline) {
          
          if (read(clientSocketDescriptor, clientCommand, 10) <= 0) {
            perror("[server]Read error on line 158. \n");
            clientIsOnline = false;
          }

          if (loginIsSuccessful) {
            commandIsLogin = false;
            commandIsRegister = false;
            commandIsExit = false;

            commandIsSetNotification = strcmp(clientCommand, commandSetNotification) == 0;
            commandIsSendMessage = strcmp(clientCommand, commandSendMessage) == 0;
            commandIsSeeSentMessages = strcmp(clientCommand, commandSeeSentMessages) == 0;
            commandIsSeeReceivedMessages = strcmp(clientCommand, commandSeeReceivedMessages) == 0;
            commandIsDisconnect = strcmp(clientCommand, commandDisconnect) == 0;
          } else {
            commandIsLogin = strcmp(clientCommand, commandLogin) == 0;
            commandIsRegister = strcmp(clientCommand, commandRegister) == 0;
            commandIsExit = strcmp(clientCommand, commandExit) == 0;

            commandIsSetNotification = false;
            commandIsSendMessage = false;
            commandIsSeeSentMessages = false;
            commandIsSeeReceivedMessages = false;
            commandIsDisconnect = false;
          }
          
          if(commandIsLogin) {
            printf("Command is Login!\n");

            if (read(clientSocketDescriptor, username, 50) <= 0) {
              perror("[server]Read error on line 179. \n");
              clientIsOnline = false;
            }

            if (usernameIsRegistered(username)) {
              printf("Connection successful for client: %s\n", username);
              strcpy(clientMessage, "Login is successful!");
              loginIsSuccessful = true;
            } else {
              printf("Connection failed for client: %s\n", username);
              strcpy(clientMessage, "Login failed!");
            }

            if (write(clientSocketDescriptor, clientMessage, 50) <= 0) {
              perror("[server] Write error on line 195.\n");
              clientIsOnline = false;
            }
            printf("\n");
          } else if(commandIsRegister) {
            printf("Command is Register!\n");

            if (read(clientSocketDescriptor, username, 50) <= 0) {
              perror("[server] Read error on line 201.\n");
              clientIsOnline = false;
            }

            if (registerUser(username)) {
              printf("Client %s was registered successfuly\n", username);
              strcpy(clientMessage, "Username was created successfuly");
              loginIsSuccessful = true;
            } else {
              printf("Client %s already exist.\n", username);
              strcpy(clientMessage, "Username already exist");
            }

            if (write(clientSocketDescriptor, clientMessage, 50) <= 0) {
              perror("[server] Write error on line 214.\n");
              clientIsOnline = false;
            }
            printf("\n");
          } else if(commandIsExit) {
            printf("Command is Exit!\n");
            clientIsOnline = false;
            printf("\n");
          } else if(commandIsSetNotification) {
            printf("Command is SetNotification!\n");

            char notificationMessage[50];
            char secondsNotification[50];

            if (read(clientSocketDescriptor, secondsNotification, 50) <= 0) {
              perror("[server] Read error on line 266.\n");
              clientIsOnline = false;
            }
            printf("Seconds notification is: %s\n", secondsNotification);

            int seconds = atoi(secondsNotification);
            int totalSeconds = seconds;
            int minutes = 0;

            if(seconds % 60 == 0) {
              minutes = seconds / 60;
              seconds = 0;
              strcpy(clientMessage, "Time for notification was set successfuly!");
              printf("Notification is set in: %d minutes\n", minutes);
            } else {
              minutes = seconds / 60;
              seconds = seconds % 60;
              strcpy(clientMessage, "Time for notification was set successfuly!");
              printf("Notification is set in: %d minutes and %d seconds\n", minutes, seconds);
            }

            if (write(clientSocketDescriptor, clientMessage, 50) <= 0) {
              perror("[server] Write error on line 284.\n");
              clientIsOnline = false;
            }

            char messageForNotification[500];
            if(read(clientSocketDescriptor, messageForNotification, 500) <= 0) {
              perror ("[client]Read error at line 294\n");
              clientIsOnline = false;
            }
            printf("Message for notification is: %s\n", messageForNotification);
            strcpy(clientMessage, "Message for notification was set successfuly!");

            if(write(clientSocketDescriptor, clientMessage, 50) <= 0) {
              perror ("[client]Write error at line 301\n");
              clientIsOnline = false;
            }
            printf("The notification is set successfuly!\n");
            
            if(needToSendNotification(totalSeconds)) {
              strcpy(clientMessage, "You have a new notfication!");
              printf("It's time for notification!\n");
              if(write(clientSocketDescriptor, clientMessage, 50) <= 0) {
                perror ("[client]Write error at line 309\n");
                clientIsOnline = false;
              }
            }

            printf("\n");
          } else if(commandIsSendMessage) {
            printf("Command is SendMessage!\n");
            char userReceiver[50];
            char message[200];

            bool userReceiverIsFound = false;
            bool messageWasSent = false;

            if (read(clientSocketDescriptor, userReceiver, 50) <= 0) {
              perror("[server] Read error on line 219.\n");
              clientIsOnline = false;
            }

            if(usernameIsRegistered(userReceiver)) {
              printf("[server] The user receiver is %s\n", userReceiver);
              strcpy(clientMessage, "Reciever user was found");
              userReceiverIsFound = true;
            } else {
              printf("[server] This user receiver can not be found:))");
              strcpy(clientMessage, "Reciever user was not found:(");
            }

            if (write(clientSocketDescriptor, clientMessage, 50) <= 0) {
              perror("[server] Write error on line 264.\n");
              clientIsOnline = false;
            }

            printf("[server] userReceiverIsFound: %d\n", userReceiverIsFound);
            if (userReceiverIsFound) {
              if (read(clientSocketDescriptor, message, 200) <= 0) {
                perror("[server] Read error on line 271.\n");
                clientIsOnline = false;
              }

              printf("[server] The message is: %s\n", message);
              printf("[server] The size of message is: %ld\n", strlen(message));

              messageWasSent = sendMessageToReciever(username, userReceiver, message);
              printf("[server] message was sent %d\n", messageWasSent);

              if(messageWasSent) {
                strcpy(clientMessage, "Message successfuly sent!");
                printf("[server] Message successfuly sent!\n");
              } else {
                strcpy(clientMessage, "Message was not sent!");
                printf("[server] Message was not sent!\n");
              }

              if (write(clientSocketDescriptor, clientMessage, 50) <= 0) {
                perror("[server] Write error on line 291.\n");
                clientIsOnline = false;
              }
            }
            printf("\n");
          } else if(commandIsSeeSentMessages) {
            printf("Command is SeeMessages!\n");

            char message[500];
            char messageSend[500];

            FILE * file;
            file = fopen("emailbox.txt", "r");

            while(fgets(message, 500, file) != NULL) {
              if(strncmp(message, username, strlen(username)) == 0) {
                strcpy(messageSend, message);
                printf("MessageSend is: %s", messageSend);

                if (write(clientSocketDescriptor, messageSend, 500) <= 0) {
                  perror("[server] Write error on line 307.\n");
                  clientIsOnline = false;
                }

                if(read(clientSocketDescriptor, clientMessage, 50) <= 0) {
                  perror ("[client]Read error at line 148\n");
                  clientIsOnline = false;
                }
              }
            }

            fclose(file);

            strcpy(messageSend, "No more messages!");
            
            if (write(clientSocketDescriptor, messageSend, 500) <= 0) {
              perror("[server] Write error on line 307.\n");
              clientIsOnline = false;
            }

            if(read(clientSocketDescriptor, clientMessage, 50) <= 0) {
              perror ("[client]Read error at line 148\n");
              clientIsOnline = false;
            }
            printf("\n");
          } else if(commandIsSeeReceivedMessages) {
            printf("Command is SeeReceivedMessages!\n");

            char message[500];
            char messageSend[500];

            FILE * file;
            file = fopen("emailbox.txt", "r");

            while(fgets(message, 500, file) != NULL) {
              if(youAreTheReceiver(username, message)) {
                strcpy(messageSend, message);
                printf("MessageSend is: %s", messageSend);

                if (write(clientSocketDescriptor, messageSend, 500) <= 0) {
                  perror("[server] Write error on line 307.\n");
                  clientIsOnline = false;
                }

                if(read(clientSocketDescriptor, clientMessage, 50) <= 0) {
                  perror ("[client]Read error at line 148\n");
                  clientIsOnline = false;
                }
              }
            }

            fclose(file);

            strcpy(messageSend, "No more messages!");
            
            if (write(clientSocketDescriptor, messageSend, 500) <= 0) {
              perror("[server] Write error on line 307.\n");
              clientIsOnline = false;
            }

            if(read(clientSocketDescriptor, clientMessage, 50) <= 0) {
              perror ("[client]Read error at line 148\n");
              clientIsOnline = false;
            }

            printf("\n");
          } else if(commandIsDisconnect) {
            printf("Command is Disconnect!\n");    
            loginIsSuccessful = false;        
            username[0] = '\0';
          }
        } 

      close(clientSocketDescriptor);
    } 

    if (signal(SIGCHLD, sighandler) == SIG_ERR) {  
      perror("signal()");
      return 1;
    }
  }

  return 0;
}
