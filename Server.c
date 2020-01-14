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
#include <sys/sendfile.h>
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


int main(int argc, char *argv[])
{
    int sockfd, newsockfd, port, pid, status, childpid;
    socklen_t clilen;
    char buffer[256], keyword[4]="Exit", *execute[4], *output, *history_path;
    struct sockaddr_in serv_addr, cli_addr;
    int n, err, i;
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
        history_path=strcat(getenv("HOME"), "history");
        if ( access( history_path, F_OK ) != -1 ) remove("~/.history");

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
            i=0;
            //close(sockfd);
         do {
           int fd = open("out", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
          // FILE* history=fopen(history_path, "a");
           struct stat file;
           //int size;
          if (i!=0){
             if (stat("out", &file)!=0){
               continue;
             }
             output=malloc(8);
             sprintf(output, "%d", file.st_size);
             if (send(newsockfd, output, 8, 0) == -1)
             {
              perror("send");
              exit(1);
             }
             output=realloc(output, file.st_size);
             read(fd, output, file.st_size);
             //printf("%s\n", output);
             send(newsockfd, output, file.st_size, 0);
          }
          bzero(buffer, 256);

          if (n=recv(newsockfd, buffer, 255, 0) < 0)
          {
            error("ERROR reading from socket");
            end=1;
          } else
          {
            i++;
            buffer[strlen(buffer)-1]=0;
            if (!strncmp(buffer, keyword, 3))
            {
              end=1;
            }
            if (buffer[0]=='c' && buffer[1]=='d' && buffer[2]==' ')
            {
              /*char *path = strtok(buffer, " ");
              path=strtok(NULL, " ");*/
              chdir(buffer);
              sprintf(buffer, "%s", "pwd");
            }
            //ftruncate(fd, 0);
            childpid=fork();
            if (childpid==-1)
            {
              perror("fork:");
              exit(1);
            } else if (childpid!=0)
            {
              //printf("Read number: %d", n);
              while(wait(&status)!=childpid);
              printf("Command executed\n");
              continue;
            }
            else{

              if ( access( "out", F_OK ) != -1 ) remove("out");

              fd = open("out", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
              dup2(fd, 1);
              dup2(fd, 2);

              /*if (!strncmp(buffer, "history", sizeof("history")))
              {
                sprintf(buffer, "%s%s", "cat ", history_path);
              }*/
              execute[0] = "/bin/bash";
              execute[1] = "-c";
              execute[2] = buffer;
              execute[3] = NULL;
              //fprintf(history, "%s", buffer);
              code=execvp(execute[0], execute);
            }

          }

        } while(!end);
        /*int i=0;
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
        }*/
        exit(code);
      }
    }

    close(newsockfd);
    close(sockfd);
    return 0;
}
