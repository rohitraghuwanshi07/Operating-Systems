#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdint.h>

#define PORT 8080
#define BUFFER_SIZE 4096

static int send_all(int s,const void *buf,size_t n){
    const char*p=buf; while(n){ssize_t r=send(s,p,n,0);if(r<=0)return -1;p+=r;n-=(size_t)r;}return 0;
}
static int recv_all(int s,void *buf,size_t n){
    char*p=buf;while(n){ssize_t r=recv(s,p,n,0);if(r<=0)return -1;p+=r;n-=(size_t)r;}return 0;
}

int main(int argc,char*argv[]){
    if(argc!=2){printf("Usage: %s <server-ip>\n",argv[0]);return 1;}
    int sock=socket(AF_INET,SOCK_STREAM,0);if(sock<0){perror("socket");return 1;}
    struct sockaddr_in server={0};server.sin_family=AF_INET;server.sin_port=htons(PORT);
    if(inet_pton(AF_INET,argv[1],&server.sin_addr)<=0){printf("Invalid server IP address.\n");close(sock);return 1;}

    printf("========================================\n");
    printf("   REMOTE C PROGRAM EXECUTION CLIENT\n");
    printf("========================================\n");
    if(connect(sock,(struct sockaddr*)&server,sizeof(server))<0){perror("connect");close(sock);return 1;}
    printf("Connected to server %s:%d\n",argv[1],PORT);

    char filename[512];
    printf("Enter C source filename: ");fflush(stdout);
    if(!fgets(filename,sizeof(filename),stdin)){close(sock);return 1;}
    filename[strcspn(filename,"\r\n")]='\0';

    FILE*fp=fopen(filename,"rb");if(!fp){perror("fopen");close(sock);return 1;}
    fseek(fp,0,SEEK_END);long fs=ftell(fp);rewind(fp);
    uint64_t size=(uint64_t)fs;
    printf("Sending file: %s\nFile size: %llu bytes\n",filename,(unsigned long long)size);
    send_all(sock,&size,sizeof(size));

    char buf[BUFFER_SIZE];uint64_t left=size;
    while(left){
        size_t want=left>BUFFER_SIZE?BUFFER_SIZE:(size_t)left;
        size_t n=fread(buf,1,want,fp);if(n==0){fclose(fp);close(sock);return 1;}
        if(send_all(sock,buf,n)<0){fclose(fp);close(sock);return 1;}
        left-=(uint64_t)n;
    }
    fclose(fp);
    printf("File sent successfully.\nWaiting for execution result...\n");

    uint32_t len;
    if(recv_all(sock,&len,sizeof(len))<0){printf("Failed to receive result.\n");close(sock);return 1;}
    char*result=malloc((size_t)len+1);if(!result){close(sock);return 1;}
    if(recv_all(sock,result,len)<0){free(result);close(sock);return 1;}
    result[len]='\0';

    printf("\n========================================\n");
    printf("             PROGRAM OUTPUT\n");
    printf("========================================\n");
    printf("%s",result);
    if(len==0||result[len-1]!='\n')printf("\n");
    printf("========================================\n");
    free(result);close(sock);printf("Connection closed.\n");return 0;
}
