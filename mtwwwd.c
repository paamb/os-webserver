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
#include <pthread.h>
#include "bbuffer.h"
#include "sem.h"

#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>

#define DEFAULT_PORT 8000
#define MAXREQ (4096 * 1024)

// Oppretter max lengde paa buffrene
char request_buffer[MAXREQ], msg[MAXREQ], body_buffer[MAXREQ/2];
char *absolute_path;
char *dirPath;
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
}


void read_file(char *absolute_path, char *body_buffer){
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
    }
}

void *work(void *args){
    BNDBUF *bbuffer = (BNDBUF *)args;
    int n;
    pid_t x = syscall(__NR_gettid);
    while(1){
        int connectsockfd = bb_get(bbuffer);
        printf("CONNECTED %d \n", x);
        n = read(connectsockfd, request_buffer, sizeof(request_buffer) - 1);
        char *phc;
        phc = strtok(request_buffer, " "); // Cycling through GET
        phc = strtok(NULL, " ");   // phc is now the given URL

        if(phc != NULL){
            absolute_path = malloc(strlen(dirPath) + strlen(phc) + 1);
            strcpy(absolute_path, dirPath);
            strcat(absolute_path, phc);
        }
        read_file(absolute_path, body_buffer);
        snprintf(msg, sizeof(msg),
        "HTTP/0.9 200 OK\n"
        "Content-Type: text/html\n"
        "Content-Length: %li\n\n%s",
        strlen(body_buffer), body_buffer);

        n = write(connectsockfd, msg, strlen(msg));
        if (n < 0)
            error("ERROR writing to socket");
        close(connectsockfd);
    }
}

int main(int argc, char *argv[])
{
    dirPath = argv[1];
    int port = atoi(argv[2]);
    int num_threads = atoi(argv[3]);
    int buffer_slots = atoi(argv[4]);
    BNDBUF *bbuffer = bb_init(buffer_slots);
    

    printf("Running in directory: %s\n %d", dirPath, port);

    // Sockfd - Socketen som blir opprettet
    int sockfd, connectsockfd;

    // socklengt type for connection
    // Size of address
    socklen_t clilen;

    struct sockaddr_in serv_addr, cli_addr;
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
    serv_addr.sin_port = htons(port);

    // Binder
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("ERROR on binding");
    }
    
    // listen(sockfd, 5);
    listen(sockfd, 1);
    
    pthread_t thr[num_threads];
    for (int i = 0; i < num_threads; i++){
        pthread_create(&thr[i], NULL, work, (void *)bbuffer);
    }



    while (1)
    {
        // Finds current directory
        bzero(request_buffer, sizeof(request_buffer));

        clilen = sizeof(cli_addr);
        connectsockfd = accept(sockfd, (struct sockaddr *)&cli_addr,
                               &clilen);
        if (connectsockfd < 0)
            error("ERROR on accept");

        bb_add(bbuffer, connectsockfd);
        pid_t e = syscall(__NR_gettid);
        printf("MAIN WRITER %d \n", e);
    }
    for (int i = 0; i < num_threads; i++){
        pthread_join(thr[i], NULL);
    }
}
