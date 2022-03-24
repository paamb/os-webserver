#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include "bbuffer.h"
#include "sem.h"
#include <sys/stat.h>

#define DEFAULT_PORT 8000
#define MAXREQ (4096 * 1024)

char request_buffer[MAXREQ], msg[MAXREQ], body_buffer[MAXREQ / 2];
char *absolute_path;
char *dirPath;
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

// Checking if path points to a file
int valid_path(char *absolute_path)
{
    struct stat status;
    stat(absolute_path, &status);
    return S_ISREG(status.st_mode);
}

// Reads file from input
void read_file(char *absolute_path, char *body_buffer)
{
    FILE *fp;
    fp = fopen(absolute_path, "r");
    if (fp == NULL || !valid_path(absolute_path))
    {
        fp = fopen("404response.html", "r");
        fread(body_buffer, sizeof(int), MAXREQ, fp);
        fclose(fp);
    }
    else
    {
        fread(body_buffer, sizeof(int), MAXREQ, fp);
        fclose(fp);
    }
}

void *work(void *args)
{
    BNDBUF *bbuffer = (BNDBUF *)args;
    int n;
    while (1)
    {
        int connectsockfd = bb_get(bbuffer);
        n = read(connectsockfd, request_buffer, sizeof(request_buffer) - 1);
        char *phc;
        phc = strtok(request_buffer, " "); // Cycling through GET
        phc = strtok(NULL, " ");           // phc is now the given URL

        if (phc != NULL)
        {
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
        printf("Connected!\n");
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

    printf("Running in directory: %s\n", dirPath);

    // Sockfd - Socketen to be made
    int sockfd, connectsockfd;

    // Size of address
    socklen_t clilen;

    struct sockaddr_in serv_addr, cli_addr;

    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("Error opening socket");

    // Binding for easier use
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        error("setsockopt(SO_REUSEADDR) failed");

    bzero((char *)&serv_addr, sizeof(serv_addr));

    // Setting up
    serv_addr.sin_family = AF_INET;

    serv_addr.sin_addr.s_addr = INADDR_ANY;

    serv_addr.sin_port = htons(port);

    // Binder
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("ERROR on binding");
    }

    listen(sockfd, 1);

    pthread_t thr[num_threads];
    for (int i = 0; i < num_threads; i++)
    {
        pthread_create(&thr[i], NULL, work, (void *)bbuffer);
    }

    printf("Listening...\n");
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
    }
    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(thr[i], NULL);
    }
}
