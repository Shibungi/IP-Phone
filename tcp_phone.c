#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DATA_SIZE 4096

void die(int n,char *mess){
    if(n == -1){
        perror(mess);
        exit(EXIT_FAILURE);
    }
}

int main(int argc,char **argv){
if(argc == 2){//サーバー ./a.out port
    int port = atoi(argv[1]);

    int ss = socket(PF_INET,SOCK_STREAM,0);
    die(ss,"socket");
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    //bind
    int is_bind_ok = bind(ss, (struct sockaddr *)&addr, sizeof(addr));
    die(is_bind_ok,"bind");
    listen(ss,10);

    //accept
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    int s = accept(ss,(struct sockaddr *)&client_addr,&len);
    die(s,"accept");

    //accept終了後録音等開始
    FILE *fp1;
    char *cmd1 = "rec -t raw -b 16 -c 1 -e s -r 44100 -";
    if((fp1 = popen(cmd1,"r")) == NULL){
        fprintf(stderr,"cannot popen1\n");
        return 0;
    }
    int fn1 = fileno(fp1);

    FILE *fp2;
    char *cmd2 = "play -t raw -b 16 -c 1 -e s -r 44100 -";
    if((fp2 = popen(cmd2,"w")) == NULL){
        fprintf(stderr,"cannot popen2\n");
        return 0;
    }
    int fn2 = fileno(fp2);

    // // エラーログ用
    // FILE *fp3;
    // if((fp3 = fopen("errorlog.txt","w")) == NULL){
    //     fprintf(stderr,"cannot popen2\n");
    //     return 0;
    // }

    //データの送受信
    // unsigned char send_data[DATA_SIZE];
    // unsigned char recv_data[DATA_SIZE];
    short send_data[DATA_SIZE];
    short recv_data[DATA_SIZE];
    while(1){
        int n1 = read(fn1,send_data,DATA_SIZE*sizeof(short),0);
        die(n1,"read");
        if(n1 == 0)close(s);

        int m1 = send(s,send_data,DATA_SIZE*sizeof(short),0);
        die(m1,"send");

        int n2 = recv(s,recv_data,DATA_SIZE*sizeof(short),0);
        die(n2,"recv");
        if(n2 == 0)close(s);
        //fft
        int m2 = write(fn2,recv_data,DATA_SIZE*sizeof(short));
        die(m2,"write");
    }

    pclose(fp1);
    pclose(fp2);
    // close(s);
    // close(fp3);
}
else if(argc == 3){//クライアント ./a.out port ip
    int port = atoi(argv[1]);

    int s = socket(PF_INET,SOCK_STREAM,0);
    die(s,"socket");
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    inet_aton(argv[2],&addr.sin_addr);
    addr.sin_port = htons(port);

    //connect
    int con = connect(s,(struct sockaddr *)&addr, sizeof(addr));
    die(con,"connect");

    //connect完了後録音等開始
    FILE *fp1;
    char *cmd1 = "rec -t raw -b 16 -c 1 -e s -r 44100 -";
    if((fp1 = popen(cmd1,"r")) == NULL){
        fprintf(stderr,"cannot popen1\n");
        return 0;
    }
    int fn1 = fileno(fp1);

    FILE *fp2;
    char *cmd2 = "play -t raw -b 16 -c 1 -e s -r 44100 -";
    if((fp2 = popen(cmd2,"w")) == NULL){
        fprintf(stderr,"cannot popen2\n");
        return 0;
    }
    int fn2 = fileno(fp2);

    //データ送受信
    // unsigned char send_data[DATA_SIZE];
    // unsigned char recv_data[DATA_SIZE];
    short send_data[DATA_SIZE];
    short recv_data[DATA_SIZE];
    while(1){
        int n2 = recv(s,recv_data,DATA_SIZE*sizeof(short),0);
        die(n2,"recv");
        if(n2 == 0)close(s);
        //fft
        int m2 = write(fn2,recv_data,DATA_SIZE*sizeof(short));
        die(m2,"write");//read,sendとrecv,writeがサーバーと逆(？)

        int n1 = read(fn1,send_data,DATA_SIZE*sizeof(short),0);
        die(n1,"read");
        if(n1 == 0)close(s);

        int m1 = send(s,send_data,DATA_SIZE*sizeof(short),0);
        die(m1,"send");
    }
    pclose(fp1);
    pclose(fp2);
    // close(s);
}else{
    printf("NEED 1 or 2 INPUT\n");
    printf("Server : ./a.out port\n");
    printf("Client : ./a.out port ip\n");
}
    return 0;
}