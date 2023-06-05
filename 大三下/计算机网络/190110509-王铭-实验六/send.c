#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <getopt.h>
#include "base64_utils.h"

#define MAX_SIZE 4095

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


// receiver: mail address of the recipient
// subject: mail subject
// msg: content of mail body or path to the file containing mail body
// att_path: path to the attachment
void send_mail(const char* receiver, const char* subject, const char* msg, const char* att_path)
{
    const char* end_msg = "\r\n.\r\n";
    const char* host_name = "smtp.qq.com"; // TODO: Specify the mail server domain name
    const unsigned short port = 25; // SMTP server port
    const char* user = "***"; // TODO: Specify the user
    const char* pass = "***"; // TODO: Specify the password
    const char* from = "***"; // TODO: Specify the mail address of the sender
    char dest_ip[16]; // Mail server IP address
    int s_fd; // socket file descriptor
    struct hostent *host;
    struct in_addr **addr_list;
    int i = 0;
    int r_size;
    const char* ctrf = "\r\n";

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

    // TODO: Create a socket, return the file descriptor to s_fd, and establish a TCP connection to the mail server
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
    printResponse(s_fd);

    // Send EHLO command and print server response
    const char* EHLO = "EHLO qq.com\r\n"; // TODO: Enter EHLO command here
    send(s_fd, EHLO, strlen(EHLO), 0);
    printResponse(s_fd);
    // TODO: Authentication. Server response should be printed out.
    const char* AUT = "AUTH login\r\n";
    send(s_fd,AUT,strlen(AUT),0);
    printResponse(s_fd);

    char* user64 = encode_str(user);
    char* pass64 = encode_str(pass);
    send(s_fd,user64,strlen(user64),0);
    send(s_fd,ctrf,strlen(ctrf),0);
    printResponse(s_fd);
    send(s_fd,pass64,strlen(pass64),0);
    send(s_fd,ctrf,strlen(ctrf),0);
    printResponse(s_fd);
    free(user64);
    free(pass64);
    // TODO: Send MAIL FROM command and print server response
    const char *FROM = "MAIL FROM:<***>\r\n";
    send(s_fd,FROM,strlen(FROM),0);
    printResponse(s_fd);

    // TODO: Send RCPT TO command and print server response
    char TO[50] = "RCPT TO:<";
    char temp[] = ">\r\n";
    strcpy(TO+9,receiver);
    strcpy(TO+9+strlen(receiver),temp);
    send(s_fd,TO,strlen(TO),0);
    printResponse(s_fd);

    // TODO: Send DATA command and print server response
    const char *DATA = "DATA\r\n";
    send(s_fd,DATA,strlen(DATA),0);
    printResponse(s_fd);

    // TODO: Send message data
    const char *SUBJECT = "subject:";
    send(s_fd,SUBJECT,strlen(SUBJECT),0);
    send(s_fd,subject,strlen(subject),0);
    send(s_fd,ctrf,strlen(ctrf),0);
 
    const char* f = "from:";
    send(s_fd,f,strlen(f),0);
    send(s_fd,from,strlen(from),0);
    send(s_fd,ctrf,strlen(ctrf),0);
    const char* header = "MIME-Version:1.0\r\nContent-Type:multipart/mixed;boundary=#BOUNDARY#\r\n\r\n";
    send(s_fd,header,strlen(header),0);
    if(msg){
        const char* cheader1 = "--#BOUNDARY#\r\nContent-Type:text/plain\r\nContent-Description:body\r\n\r\n";
        send(s_fd,cheader1,strlen(cheader1),0);
        FILE *t = fopen(msg,"r+");
        if(!t){
            send(s_fd,msg,strlen(msg),0);
        }
        else{
            int nCount;
            while( (nCount = fread(buf, 1, MAX_SIZE, t)) > 0 ){
                send(s_fd, buf, nCount, 0);
            }
            fclose(t);
        }
    }
    if(att_path){
        FILE *rawfile = fopen(att_path,"r+");
        if(!rawfile){
            perror("file error!\n");
            exit(1);
        }
        FILE *file = fopen("temp","w+");
        encode_file(rawfile,file);
        const char* cheader2 = "\r\n--#BOUNDARY#\r\nContent-Type:application/octet-stream;name=";
        send(s_fd,cheader2,strlen(cheader2),0);
        send(s_fd,att_path,strlen(att_path),0);
        const char* cheader3 = "\r\nContent-Transfer-Encoding: base64\r\nContent-Description:attachment\r\n\r\n";
        send(s_fd,cheader3,strlen(cheader3),0);
        fclose(file);
        fclose(rawfile);
        int nCount;
        file = fopen("temp","r+");
        while( (nCount = fread(buf, 1, MAX_SIZE, file)) > 0 ){
            send(s_fd, buf, nCount, 0);
        }
        fclose(file);
    }
    // TODO: Message ends with a single period
    send(s_fd,end_msg,strlen(end_msg),0);
    printResponse(s_fd);

    // TODO: Send QUIT command and print server response
    const char *QUIT = "quit\r\n";
    send(s_fd,QUIT,strlen(QUIT),0);
    printResponse(s_fd);

    close(s_fd);
}

int main(int argc, char* argv[])
{
    int opt;
    char* s_arg = NULL;
    char* m_arg = NULL;
    char* a_arg = NULL;
    char* recipient = NULL;
    const char* optstring = ":s:m:a:";
    while ((opt = getopt(argc, argv, optstring)) != -1)
    {
        switch (opt)
        {
        case 's':
            s_arg = optarg;
            break;
        case 'm':
            m_arg = optarg;
            break;
        case 'a':
            a_arg = optarg;
            break;
        case ':':
            fprintf(stderr, "Option %c needs an argument.\n", optopt);
            exit(EXIT_FAILURE);
        case '?':
            fprintf(stderr, "Unknown option: %c.\n", optopt);
            exit(EXIT_FAILURE);
        default:
            fprintf(stderr, "Unknown error.\n");
            exit(EXIT_FAILURE);
        }
    }

    if (optind == argc)
    {
        fprintf(stderr, "Recipient not specified.\n");
        exit(EXIT_FAILURE);
    }
    else if (optind < argc - 1)
    {
        fprintf(stderr, "Too many arguments.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        recipient = argv[optind];
        send_mail(recipient, s_arg, m_arg, a_arg);
        exit(0);
    }
}
