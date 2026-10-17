#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdint.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define SOURCE_FILE "received_program.c"
#define EXECUTABLE_FILE "received_program"

static int send_all(int s, const void *buf, size_t n) {
    const char *p = buf;
    while (n) { ssize_t r=send(s,p,n,0); if(r<=0)return -1; p+=r; n-=(size_t)r; }
    return 0;
}
static int recv_all(int s, void *buf, size_t n) {
    char *p=buf;
    while(n){ ssize_t r=recv(s,p,n,0); if(r<=0)return -1; p+=r; n-=(size_t)r; }
    return 0;
}
static void send_result(int s, const char *msg) {
    uint32_t n=(uint32_t)strlen(msg);
    send_all(s,&n,sizeof(n));
    send_all(s,msg,n);
}

int main(void) {
    int server=socket(AF_INET,SOCK_STREAM,0);
    if(server<0){perror("socket");return 1;}
    int opt=1; setsockopt(server,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    struct sockaddr_in addr={0};
    addr.sin_family=AF_INET; addr.sin_addr.s_addr=INADDR_ANY; addr.sin_port=htons(PORT);
    if(bind(server,(struct sockaddr*)&addr,sizeof(addr))<0){perror("bind");close(server);return 1;}
    if(listen(server,5)<0){perror("listen");close(server);return 1;}

    printf("========================================\n");
    printf("   REMOTE C PROGRAM EXECUTION SERVER\n");
    printf("========================================\n");
    printf("Server started on port %d\nWaiting for clients...\n",PORT);

    while(1){
        struct sockaddr_in client={0}; socklen_t len=sizeof(client);
        int c=accept(server,(struct sockaddr*)&client,&len);
        if(c<0)continue;
        printf("\nClient connected: %s\n",inet_ntoa(client.sin_addr));

        uint64_t size;
        if(recv_all(c,&size,sizeof(size))<0){close(c);continue;}
        printf("Receiving C source file (%llu bytes)...\n",(unsigned long long)size);

        FILE *fp=fopen(SOURCE_FILE,"wb");
        if(!fp){send_result(c,"ERROR: Could not create source file on server.");close(c);continue;}

        uint64_t left=size; char buf[BUFFER_SIZE]; int ok=1;
        while(left){
            size_t want=left>BUFFER_SIZE?BUFFER_SIZE:(size_t)left;
            ssize_t n=recv(c,buf,want,0);
            if(n<=0){ok=0;break;}
            if(fwrite(buf,1,(size_t)n,fp)!=(size_t)n){ok=0;break;}
            left-=(uint64_t)n;
        }
        fclose(fp);
        if(!ok){remove(SOURCE_FILE);close(c);continue;}
        printf("Source file received successfully.\n");

        printf("Compiling program with gcc...\n");
        int status=system("gcc received_program.c -o received_program 2> compile_errors.txt");
        if(status!=0){
            printf("Compilation failed.\n");
            FILE *ef=fopen("compile_errors.txt","rb");
            if(!ef){send_result(c,"COMPILATION_ERROR\nCompilation failed.");}
            else{
                fseek(ef,0,SEEK_END); long es=ftell(ef); rewind(ef);
                char *e=malloc((size_t)es+1);
                if(!e){send_result(c,"COMPILATION_ERROR\nMemory allocation failed.");}
                else{
                    size_t nr=fread(e,1,(size_t)es,ef); e[nr]='\0';
                    size_t total=nr+32; char *msg=malloc(total);
                    if(msg){snprintf(msg,total,"COMPILATION_ERROR\n%s",e);send_result(c,msg);free(msg);}
                    free(e);
                }
                fclose(ef);
            }
            remove(SOURCE_FILE);remove(EXECUTABLE_FILE);remove("compile_errors.txt");
            close(c);printf("Client session ended.\nWaiting for new clients...\n");continue;
        }
        remove("compile_errors.txt");
        printf("Compilation successful.\nExecuting program...\n");

        int pipefd[2];
        if(pipe(pipefd)<0){perror("pipe");send_result(c,"ERROR: Could not create execution pipe.");remove(SOURCE_FILE);remove(EXECUTABLE_FILE);close(c);continue;}

        pid_t pid=fork();
        if(pid<0){perror("fork");close(pipefd[0]);close(pipefd[1]);send_result(c,"ERROR: fork failed.");remove(SOURCE_FILE);remove(EXECUTABLE_FILE);close(c);continue;}

        if(pid==0){
            close(pipefd[0]);
            dup2(pipefd[1],STDOUT_FILENO); dup2(pipefd[1],STDERR_FILENO);
            close(pipefd[1]);
            execl("./received_program","./received_program",(char*)NULL);
            perror("exec"); _exit(127);
        }

        close(pipefd[1]);
        char *out=NULL; size_t outn=0, cap=0;
        while(1){
            ssize_t n=read(pipefd[0],buf,sizeof(buf));
            if(n<=0)break;
            if(outn+(size_t)n+1>cap){
                size_t nc=cap?cap*2:8192;
                while(nc<outn+(size_t)n+1)nc*=2;
                char *tmp=realloc(out,nc);
                if(!tmp){free(out);out=NULL;outn=0;cap=0;break;}
                out=tmp;cap=nc;
            }
            memcpy(out+outn,buf,(size_t)n);outn+=(size_t)n;out[outn]='\0';
        }
        close(pipefd[0]);
        waitpid(pid,NULL,0);

        if(!out)send_result(c,"ERROR: Could not capture program output.");
        else{printf("Program execution completed.\n");send_result(c,out);free(out);}
        remove(SOURCE_FILE);remove(EXECUTABLE_FILE);
        close(c);
        printf("Result sent to client.\nClient session ended.\nWaiting for new clients...\n");
    }
    close(server);
    return 0;
}
