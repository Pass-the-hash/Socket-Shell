#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>


int sockfd;
char *keyword="Exit";

void handler(int signum)   //Συνάρτηση για διαχείριση σημάτων από το πληκτρολόγιο
{
  send(sockfd, "Exit", strlen("Exit"), 0);
  close(sockfd);
  exit(0);
}

void error(const char *msg)     //Συνάρτηση για εκτύπωση σφαλμάτων στην οθόνη
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int portno, n, size;  
    struct sockaddr_in serv_addr; // Διεύθυνση διακομιστή
    struct hostent *server; 
    signal(SIGINT, handler); // Διαχειριστής σημάτων
    char *out=malloc(8), buffer[256];
    if (argc < 3) // Έλεγχος για την παροχή των αναμενόμενων ορισμάτων
    {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }
 
    portno = atoi(argv[2]); // Αριθμός θύρας
    sockfd = socket(AF_INET, SOCK_STREAM, 0);   // Περιγραφέας υποδοχής
    if (sockfd < 0) // Έλεγχος για επίτευξη υποδοχής
        error("ERROR opening socket");
    server = gethostbyname(argv[1]); // Μετατρέπει το όνομα σε διεύθυνση
    if (server == NULL);
    server = gethostbyname(argv[1]);
    if (server == NULL) 
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr)); // Πάει σε κάθε θέση του buffer και βάζει μηδενικά για να ανοίξει το socket 
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,  // Η διαδικασία δημιουργίας δομής που αντιπροσωπεύει την διεύθυνση του διακομιστή
            (char *)&serv_addr.sin_addr.s_addr,
            server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    int end=1;
    int count=0;
    int received;

    while(printf("%s_> ", argv[1]), fgets(buffer, 256, stdin), !feof(stdin)){   //Προτροπή για είσοδο εντολών από το χρήστη
      
      if (buffer[0]=='\n') continue;    //Αν δοθεί κενό, τότε επανέρχεται στην προτροπή χρήστη
      if (send(sockfd, buffer, strlen(buffer), 0) == -1) //Η εντολή στέλνεται στο διακομιστή μέσω υποδοχής
      {
          perror("send");
          exit(1);
      }
      if (strncmp(buffer, keyword, 4)==0) //Αν δοθεί η "Exit", τερματίζεται το πρόγραμμα
      {
        end=0;
        
      } 
      if(n=recv(sockfd, out, 8, 0)>0) //Λήψη του μεγέθους εξόδου της εντολής από το διακομιστή
      {
        int size=atoi(out); //Μετατροπή σε ακέραιο
        out=realloc(out, size);
        recv(sockfd, out, size, 0); //Λήψη της εξόδου της εντολής 
       
        printf("%s", out);

          

        putchar('\n');
       
      } else
      {
        if (n < 0) perror("recv");
        else printf("Server closed connection\n");
        exit(1);
      }

      if (end==0) break;
     
      count++;
    }

    close(sockfd);
    return 0;
}
