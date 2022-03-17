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
#define MAXREQ (4096 * 1024)

// Oppretter max lengde paa buffrene
char request_buffer[MAXREQ], msg[MAXREQ], body_buffer[MAXREQ];
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

// Funksjon som burde fungerer, men gjorde det ikke så bare gjorde det i main istedenfor
void assign_absolutPath(char *absolutePath, char dirPath[], char webpagePath[])
{
    absolutePath = malloc(strlen(dirPath) + strlen(webpagePath) + 1);
    strcpy(absolutePath, dirPath);
    strcat(absolutePath, webpagePath);
    printf("%s", absolutePath);
    printf("%p", &absolutePath);

    // FILE *fp;
    // fp = fopen(absolutePath, "r");
    printf("Ikke feil HER \n\n\n");
    // return absolutePath;
}

// Kopiert fra stackoverflow
void func()
{
    char cwd[200];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        printf("Current working dir: %s\n", cwd);
    }
    else
    {
        perror("getcwd() error");
    }
}

int main()
{
    char dirPathArr[60];
    char *dirPath;
    int port[10];
    FILE *fp;
    char *absolute_path;
    // memset(absolute_path, '\0', sizeof(dirPathArr) + 50);
    // Finds current directory
    func();
    printf("Skriv inn path ");
    scanf("%59s", dirPathArr);

    dirPath = dirPathArr;

    // Sockfd - Socketen som blir opprettet
    int sockfd, connectsockfd;

    // socklengt type for connection
    // Size of address
    socklen_t clilen;

    struct sockaddr_in serv_addr, cli_addr;
    int n;
    // struct sockaddr_iN
    // IPV4
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("Error opening socket");

    // Socket-option for easier reusing of port
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        error("setsockopt(SO_REUSEADDR) failed");

    // Tar opp plass i minnet og setter verdien til 0
    bzero((char *)&serv_addr, sizeof(serv_addr));

    // Adresse familien til transport adressen
    serv_addr.sin_family = AF_INET;

    // INADDR_ANY Binder socketen til hvilken som helst adresse. Koble til hvilken som helst IP - vi vet ikke IP til maskinen.
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    // Hosten sitt portnummer, definert i toppen
    serv_addr.sin_port = htons(PORT);

    // Binder
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("ERROR on binding");
    }

    // listen(sockfd, 5);
    listen(sockfd, 1);

    while (1)
    {
        // Finds current directory
        func();
        bzero(request_buffer, sizeof(request_buffer));

        clilen = sizeof(cli_addr);
        connectsockfd = accept(sockfd, (struct sockaddr *)&cli_addr,
                               &clilen);
        printf("\nfeil i toppen?");
        if (connectsockfd < 0)
            error("ERROR on accept");
        n = read(connectsockfd, request_buffer, sizeof(request_buffer) - 1);
        printf("\nfeeeeeeeeil");
        // printf("%s", buffer);
        // char *test;
        // test = strtok(buffer, "/");
        printf("%s", request_buffer);


        // Denne bør gjøres litt bedre. Skal ikke alltid endres. "malloc-koden" under blir stygg.
        char *phc;
        phc = strtok(request_buffer, " "); // Cycling through GET
        phc = strtok(NULL, " ");   // phc is now the given URL
        // char webpagePath[30];
        printf("Ikke feil forst i main");
        // const peker direkte til stringen

        // Oppretter en absolutt path dirPath + url path.
        // if (strcmp(phc, "/index.html") == 0){
        
        // bzero(absolute_path,strlen(dirPath) + strlen(phc) + 1);
        
        printf("\n\n\n%s", phc);
        printf("%s\n\n\n", dirPath);

        // Kjørte requests innimellom og da var phc null. Fikk derfor segfault naar man prøvde aa malloc noe med lengde null.
        if(phc != NULL){
            free(absolute_path);
            absolute_path = malloc(strlen(dirPath) + strlen(phc) + 1);
            strcpy(absolute_path, &(*dirPath));
            strcat(absolute_path, &(*phc));
        }

        printf("%s", absolute_path);

        printf("andre gang %s", absolute_path);
        FILE *fp;
        fp = fopen(absolute_path, "r");
        if (fp == NULL)
        {
            fp = fopen("404response.html", "r");
            fread(body_buffer, sizeof(int), MAXREQ, fp);
            fclose(fp);
        }
        else{
            fread(body_buffer, sizeof(int), MAXREQ, fp);
            fclose(fp);
            printf("%s", body_buffer);

            // Printer filen til webpage
            // snprintf(body, sizeof(body), fpbuffer);


        }

        snprintf(msg, sizeof(msg),
        "HTTP/0.9 200 OK\n"
        "Content-Type: text/html\n"
        "Content-Length: %d\n\n%s",
        strlen(body_buffer), body_buffer);

        n = write(connectsockfd, msg, strlen(msg));
        if (n < 0)
            error("ERROR writing to socket");
        close(connectsockfd);
        printf("Yo kom gjennom hele jo uten å få en eneste segmentation fault jævel");
        // }
        

        // assign_absolutPath(absolutePath, dirPath, phc);




        // if (fp = fopen(absolutePath, "r") == NULL)
        // {
        //     printf("Fil kunne ikke aapnes");
        // }

    }
}