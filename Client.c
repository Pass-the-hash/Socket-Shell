#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

#define BUFFER_SIZE 100000

int sockfd;
char *keyword="END";

void handler(int signum)
{
  //close(sockfd);
  //printf("\nCaught signal %d\nGoodbye! :)\n", signum);
  send(sockfd, "END", strlen("END"), 0);
  close(sockfd);
  exit(0);
}

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    signal(SIGINT, handler);
    char out[BUFFER_SIZE], buffer[256];
    if (argc < 3)
    {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
            (char *)&serv_addr.sin_addr.s_addr,
            server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    srand(time(NULL));
    int end=1;
    int count=0;
    while(printf("%s_> ", argv[1]), fgets(buffer, 256, stdin), !feof(stdin)){
      if (send(sockfd, buffer, strlen(buffer), 0) == -1)
      {
          perror("send");
          exit(1);
      }
      if (strncmp(buffer, keyword, 3)==0)
      {
        end=0;
        for (int i=0; i<count; i++)
        {
          sprintf(buffer, "%d", rand()%20+1);
          if (send(sockfd, buffer, strlen(buffer), 0) == -1)
          {
              perror("send");
              exit(1);
          }
        }
        sprintf(buffer, "%s", "END");
        if (send(sockfd, buffer, strlen(buffer), 0) == -1)
        {
            perror("send");
            exit(1);
        }
      }
      if ((n=recv(sockfd, out, 256, 0)) > 0)
      {
          out[n] = '\0';
          for (int i=0; i<=n; i++){
            putchar(out[i]);
          }
          putchar('\n');
      } else
      {
          if (n < 0) perror("recv");
          else printf("Server closed connection\n");
          exit(1);
      }

      if (end==0) break;
      //bzero(buffer, 256);
      count++;
    }

    close(sockfd);
    return 0;
}
