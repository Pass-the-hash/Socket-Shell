#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

typedef struct sockaddr server;

void handler(int signum)
{
  printf("\nCaught signal %d\nGoodbye! :)\n", signum);
  exit(signum);
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int game(int number)
{
  srand(time(NULL));
  printf("Guessing numbers\n");
  if ((rand()%20+1)==number)
  {
    return 1;
  }
  return 0;
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, port, pid, status, childpid;
    socklen_t clilen;
    char buffer[256], keyword[4]="END", *execute[4];
    struct sockaddr_in serv_addr, cli_addr;
    int n, out, err;
    char str[INET_ADDRSTRLEN];
    signal(SIGINT, handler);
    if (argc < 2)
    {
        fprintf(stderr, "No port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

   if (sockfd < 0)
        error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    port = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    if (bind(sockfd,(server *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

      int end=0;
      while(1){
        int code;
        if (listen(sockfd, 5) == -1) {
            error("listen");
            exit(1);
        }
        printf("Listening for connections...\n");
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        printf("Socket: %d\n", newsockfd);
        if (newsockfd < 0)
            error("ERROR on accept");
        else printf("Accepted connection\n");
        if (inet_ntop(AF_INET, &cli_addr.sin_addr, str, INET_ADDRSTRLEN) == NULL) {
            fprintf(stderr, "Could not convert byte to address\n");
            exit(1);
        }

        fprintf(stdout, "The client address is :%s\n", str);

          pid=fork();
          if (pid==-1)
          {
            error("fork:");
            exit(1);
          } else if (pid!=0)
          {
            close(newsockfd);
            while(wait(&status)!=pid);
            printf("Client diconnected\n");
          } else
          {
            close(sockfd);
         do {

          bzero(buffer, 256);

          if (read(newsockfd, buffer, 255) < 0)
          {
            error("ERROR reading from socket");
            end=1;
          } else
          {
            if (!strncmp(buffer, keyword, 3))
            {
              end=1;
              int i=0;
              while(1)
              {
                if (read(newsockfd, buffer, 255) < 0)
                {
                  error("ERROR reading from socket");
                }
                if (!strncmp(buffer, keyword, 3))
                {
                  bzero(buffer, 256);
                  if (i!=0) sprintf(buffer, "%s", "echo 'Server won.'");
                  break;
                }
                if(game(atoi(buffer)))
                {
                  sprintf(buffer, "%s", "echo 'You won!'");
                  break;
                }
                i++;
              }
            }

            childpid=fork();
            if (childpid==-1)
            {
              perror("fork:");
              exit(1);
            } else if (childpid!=0)
            {
              while(wait(&status)!=childpid);
              printf("Command executed\n");
              //exit(0);
            }
            else{
              //out=dup(STDOUT_FILENO);
              dup2( newsockfd, STDOUT_FILENO );
              dup2( newsockfd, STDERR_FILENO );
              /*close(newsockfd);*/
              execute[0] = "/bin/bash";
              execute[1] = "-c";
              execute[2] = buffer;
              execute[3] = NULL;
              code=execvp(execute[0], execute);
              //exit(code);
            }

          }

        } while(!end);
        exit(code);
      }
    }

    close(newsockfd);
    close(sockfd);
    return 0;
}
