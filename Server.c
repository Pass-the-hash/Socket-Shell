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

void handler(int signum)  //Συνάρτηση για τη διαχείριση SIGINT
{
    if (signum==2){
        printf("\nCaught signal %d\nGoodbye! :)\n", signum);
        exit(signum);
    }
}

void error(const char *msg) //Συνάρτηση για εκτύπωση σφαλμάτων στην οθόνη
{
    perror(msg);
    exit(1);
}


int main(int argc, char *argv[])
{
    
    int sockfd, newsockfd;  //Περιγραφείς υποδοχών
    int port, pid, status, childpid; //Μεταβλητές για κατάσταση και αριθμούς διεργασιών
    socklen_t clilen;
    char buffer[256], keyword[4]="Exit", *execute[4], *output; //Buffers και λέξεις-κλειδιά
    struct sockaddr_in serv_addr, cli_addr; //Δομές διευθυνσιοδότησης
    int n, err, i, fd; //Περιγραφείς αρχείων, μετρητές και επιστροφή διαδικασιών 
    char str[INET_ADDRSTRLEN];
    signal(SIGINT, handler);    //Διαχειριστής σημάτων
    
    if (argc < 2)   // 
    {
        fprintf(stderr, "No port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

   if (sockfd < 0)
        error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));  // Εκκίνηση διαδικασίας για επίτυεξη επικοινωνίας με υποδοχές, όπως δείξατε και στο εργαστήριο
    port = atoi(argv[1]); // Αριθμός θύρας
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    if (bind(sockfd,(server *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

      int end=0;

      while(1){ //Αρχική επαναληπτική δομή για διατήρηση του server στο παρασκήνιο
        

        int code;
        if (listen(sockfd, 5) == -1) {    //Υποστήριξη μέχρι 5 πελατών
            error("listen");
            exit(1);
        }
        printf("Listening for connections...\n");
        clilen = sizeof(cli_addr); 
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen); // Ο πελάτης συνδέθηκε με τον διακομιστή
        printf("Socket: %d\n", newsockfd);
        if (newsockfd < 0) // Εαν κατι πηγε λαθος με την σύνδεση, εμφανίζει μήνυμα, αλλιώς εμφανίζει μήνυμα επιτυχούς σύνδεσης
            error("ERROR on accept");
        else printf("Accepted connection\n");
        if (inet_ntop(AF_INET, &cli_addr.sin_addr, str, INET_ADDRSTRLEN) == NULL) { // Έλεγχος τύχον κάποιας μετατροπής της διεύθυνσης του πελάτη, απο εργαστήριο
            fprintf(stderr, "Could not convert byte to address\n");
            exit(1);
        }

        fprintf(stdout, "The client address is :%s\n", str);

          pid=fork();   //Εκκίνηση θυγατρικής διεργασίας για το νέο πελάτη
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
            int i=0; //Αρχικοποίηση μετρητή επαναλήψεων
            
         do {   //Ο πελάτης θα εκτελεί πολλές εντολές και γι'αυτό χρειάζεται ακόμη μια δομή επανάληψης
           fd = open("out", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR); //Δημιουργία/Άνοιγμα αρχείου για ανακατεύθυνση εξόδου εντολών
         
           struct stat file;
           
          if (i!=0){  //Ανάγνωση και αποστολή του αρχείου μέσω υποδοχής μετά την 1η επανάληψη 
             if (stat("out", &file)!=0){ //Αν δεν υπάρχει το αρχείο, δε γίνεται αποστολή
               continue;
             }
             output=malloc(8);  //Δέσμευση χώρου για αποστολή του μεγέθους της εξόδου της εντολής
             sprintf(output, "%d", file.st_size);
             if (send(newsockfd, output, 8, 0) == -1)
             {
              perror("send");
              exit(1);
             }
             output=realloc(output, file.st_size);  
             read(fd, output, file.st_size);  //Ανάγνωση του αρχείου και στη συνέχεια, αποστολή του μέσω υποδοχής
            
             send(newsockfd, output, file.st_size, 0);
          }
          bzero(buffer, 256); //Εκκαθάριση προσωρινής μνήμης εντολών

          if (n=recv(newsockfd, buffer, 255, 0) < 0) //Λήψη εντολής από τον πελάτη
          {
            error("ERROR reading from socket");
            end=1;
          } else
          {
            i++;
            buffer[strlen(buffer)-1]=0;
            if (!strncmp(buffer, keyword, sizeof(keyword)) )//Αν η εντολή είναι η "Exit", τερματίζεται η επικοινωνία
            {
              end=1;
            }
            if (buffer[0]=='c' && buffer[1]=='d')   //Αν η εντολή ξεκινάει με "cd ...", τότε καλείται διαδικασία για
            {                                                       // αλλαγή καταλόγου
              
              chdir(buffer);
              sprintf(buffer, "%s", "pwd");   //Εμφάνιση του τωρινού καταλόγου μετά την εκτέλεση της εντολής
            }
           
            childpid=fork();  //Δημιουργία διεργασίας για την εκτέλεση της εντολής
            if (childpid==-1)
            {
              perror("fork:");
              exit(1);
            } else if (childpid!=0)
            {
              
              while(wait(&status)!=childpid);
              printf("Command executed\n");
              continue;
            }
            else{

              if ( access( "out", F_OK ) != -1 ) remove("out"); //Εκκαθάριση του περιεχομένου του αρχείου, αν υπάρχει

              fd = open("out", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);    
              dup2(fd, 1);  //Ανακατεύθυνση εξόδου
              dup2(fd, 2);  //και σφαλμάτων σε αρχείο

              execute[0] = "/bin/bash";   //Δημιουργία εντολής για εκτέλεση από την execvp()
              execute[1] = "-c";
              execute[2] = buffer;
              execute[3] = NULL;
              execvp(execute[0], execute);
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
