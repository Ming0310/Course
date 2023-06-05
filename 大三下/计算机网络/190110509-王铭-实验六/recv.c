#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define MAX_SIZE 65535

char buf[MAX_SIZE+1];

#define swap16(x) ((((x)&0xFF) << 8) | (((x) >> 8) & 0xFF)) //为16位数据交换大小端


void printResponse(int fd){
    int r_size;
    if ((r_size = recv(fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; // Do not forget the null terminator
    printf("%s", buf);
}

void recv_mail()
{
    const char* host_name = "pop.qq.com"; // TODO: Specify the mail server domain name
    const unsigned short port = 110; // POP3 server port
    const char* user = "user ***\r\n"; // TODO: Specify the user
    const char* pass = "pass ***\r\n"; // TODO: Specify the password
    char dest_ip[16];
    int s_fd; // socket file descriptor
    struct hostent *host;
    struct in_addr **addr_list;
    int i = 0;
    int r_size;

    // Get IP from domain name
    if ((host = gethostbyname(host_name)) == NULL)
    {
        herror("gethostbyname");
        exit(EXIT_FAILURE);
    }

    addr_list = (struct in_addr **) host->h_addr_list;
    while (addr_list[i] != NULL)
        ++i;
    strcpy(dest_ip, inet_ntoa(*addr_list[i-1]));

    // TODO: Create a socket,return the file descriptor to s_fd, and establish a TCP connection to the POP3 server
    s_fd = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = swap16(port);
    struct in_addr sin_addr;
    sin_addr.s_addr = inet_addr(dest_ip);
    servaddr.sin_addr = sin_addr;
    bzero(&(servaddr.sin_zero),8);
    if (connect(s_fd,&servaddr,sizeof(servaddr)) == -1){
        perror("connect error");
        exit(1);
    }
    // Print welcome message
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; // Do not forget the null terminator
    printf("%s", buf);

    // TODO: Send user and password and print server response
    send(s_fd,user,strlen(user),0);
    printResponse(s_fd);
    send(s_fd,pass,strlen(pass),0);
    printResponse(s_fd);
    // TODO: Send STAT command and print server response
    const char *STAT = "stat\r\n";
    send(s_fd,STAT,strlen(STAT),0);
    printResponse(s_fd);
    // TODO: Send LIST command and print server response
    const char *LIST = "list\r\n";
    send(s_fd,LIST,strlen(LIST),0);
    printResponse(s_fd);
    // TODO: Retrieve the first mail and print its content
    const char *RETR = "retr 1\r\n";
    send(s_fd,RETR,strlen(RETR),0);
    printResponse(s_fd);
    // TODO: Send QUIT command and print server response
    const char *QUIT = "quit\r\n";
    send(s_fd,QUIT,strlen(QUIT),0);
    printResponse(s_fd);
    close(s_fd);
}

int main(int argc, char* argv[])
{
    recv_mail();
    exit(0);
}
