#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#define PORT 8000
#define MAXREQ (4096*1024)

// Oppretter max lengde paa buffrene
char buffer[MAXREQ], body[MAXREQ], msg[MAXREQ], fpbuffer[MAXREQ];
void error(const char *msg) { perror(msg); exit(1); }

void open_file_from_path(char *absolutePath, char dirPath[], char webpagePath[]){
    absolutePath = malloc(strlen(dirPath)+strlen(webpagePath)+1);
    strcpy(absolutePath, dirPath);
    strcat(absolutePath, webpagePath);
    printf("%s", absolutePath);
    printf("%p", &absolutePath);
    
    // FILE *fp;
    // fp = fopen(absolutePath, "r");
    printf("Ikke feil HER \n\n\n");
    // return absolutePath;
}

void func(){  
    char cwd[200];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working dir: %s\n", cwd);
    } else {
        perror("getcwd() error");
    }
}

int main() {
    char dirPath[60];
    int port;
    FILE *fp;
    
    // Finds current directory
    func();
    printf("Skriv inn path ");
    scanf("%59s", dirPath);
    
    //Sockfd - Socketen som blir opprettet
    int sockfd, connectsockfd;

    //socklengt type for connection
    //Size of address
    socklen_t clilen;

    struct sockaddr_in serv_addr, cli_addr;
    int n;
    // struct sockaddr_iN
    // IPV4
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("Error opening socket");

    // Socket-option for easier reusing of port
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        error("setsockopt(SO_REUSEADDR) failed");
    
    // Tar opp plass i minnet og setter verdien til 0 
    bzero((char *) &serv_addr, sizeof(serv_addr));

    //Adresse familien til transport adressen
    serv_addr.sin_family = AF_INET;

    // INADDR_ANY Binder socketen til hvilken som helst adresse. Koble til hvilken som helst IP - vi vet ikke IP til maskinen.
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    // Hosten sitt portnummer, definert i toppen
    serv_addr.sin_port = htons(PORT);

    // Binder 
    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        error("ERROR on binding");
    }

    // listen(sockfd, 5);
    listen(sockfd, 1);

    while(1){
        // Finds current directory
        func();
        printf("hei");
        bzero(buffer,sizeof(buffer));

        clilen = sizeof(cli_addr);
        connectsockfd = accept (sockfd, (struct sockaddr *) &cli_addr,
            &clilen);

        if (connectsockfd < 0) error("ERROR on accept");
        n = read (connectsockfd,buffer,sizeof(buffer)-1);

        // printf("%s", buffer);
        // char *test;
        // test = strtok(buffer, "/");
        char *phc;
        
        phc = strtok(buffer," "); // Cycling through GET
        phc = strtok(NULL," "); // phc is now the given URL
        // char webpagePath[30];
        printf("Ikke feil forst i main");

        // const peker direkte til stringen
        char *absolutePath;
        absolutePath = malloc(strlen(dirPath)+strlen(phc)+1);
        strcpy(absolutePath, dirPath);
        strcat(absolutePath, phc);
        printf("%s", absolutePath);
        // absolutePath = open_file_from_path(absolutePath, dirPath, phc);
        // open_file_from_path(absolutePath, dirPath, phc);
        printf("andre gang %s", absolutePath);
        FILE *fp;

        if ((fp = fopen(absolutePath, "r")) == NULL){
            printf("Fil kunne ikke aapnes");
        }
        printf("Den kan faktisk leses");
        // printf("Mest sannsynlig her");
        fgets(fpbuffer, sizeof(fpbuffer)-1, fp);
        // printf("\n\nfds");
        // printf("\n\n\n%s", test);
        snprintf (body, sizeof (body),
            "<html>\n<body>\n"
            "<h1>Hello web browser</h1>\nYour request was\n"
            "<pre>%s</pre>\n\n\n"
            "Your URL was\n"
            "<pre>%s</pre>"
            "</body>\n</html>\n", phc, fpbuffer);
        
        snprintf (msg, sizeof (msg),
            "HTTP/1.0 200 OK\n"
            "Content-Type: text/html\n"
            "Content-Length: %d\n\n%s", strlen (body), body);
        n = write (connectsockfd,msg,strlen(msg));
        if (n < 0) error("ERROR writing to socket");
        close (connectsockfd);
    }


}