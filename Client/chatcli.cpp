
#define _XOPEN_SOURCE 600
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string>
#include "logindlg.h"
#include "chatwin.h"
#include "chatcli.h"

#include <qapplication.h>
#include <qmessagebox.h>
using namespace std;

int client_socket;

int readLine(int,char*,int,int);
commands decodeCommand(const char*);
int main(int argc, char** argv){
    cLoginDlg *logindlg;
    QString host;
    int port;
    QString nickname;

    QApplication app(argc,argv);

    do{
        logindlg= new cLoginDlg(NULL);
        if(logindlg->exec()==QDialog::Rejected){
            delete logindlg;
            return 0;
        }
        host = logindlg->hostEdit->text().toAscii();

        port=(logindlg->portEdit->text().toInt());
        nickname=logindlg->nickEdit->text().toAscii();
    
    }while(connectAndJoin(host.toStdString(),port,nickname.toStdString())==0);

    cChatWin *chatwin = new cChatWin();
    chatwin->client_socket = client_socket;
    app.setActiveWindow(chatwin);
    chatwin->setWindowTitle("Chat Test");
    chatwin->show();
    app.connect(&app,SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
    int res = app.exec();
    return res;

}


int connectAndJoin(string host, int port, string nickname){
struct sockaddr_in server;
client_socket=socket(AF_INET,SOCK_STREAM,0);
bzero(&server,sizeof(server));
server.sin_addr.s_addr=INADDR_ANY;
server.sin_port=htons(port);
server.sin_family = AF_INET;
int flag=1;
char buffer[MAX_LINE_BUFF];
string joinString;
commands cmd;
hostent *dotaddr=gethostbyname(host.c_str());
if(dotaddr){
    bcopy((const void*) dotaddr->h_addr_list[0],(void*)&server.sin_addr.s_addr,dotaddr->h_length);
    } 
    else{
        server.sin_addr.s_addr = inet_addr("0.0.0.0");
    }
    if(connect(client_socket,(const sockaddr*)&server,sizeof(server))!=0) {
        QMessageBox::critical(NULL, "Connection Failed","Unable to Connect.");
        close(client_socket);
        return 0;
    }
    setsockopt(client_socket,0,TCP_NODELAY,&flag,sizeof(int));
    ioctl(client_socket,FIONBIO,&flag);
    
    joinString = "JOIN " + nickname + "\n";
    send(client_socket,joinString.c_str(),joinString.length(),0);
    readLine(client_socket,buffer,MAX_LINE_BUFF,0);
    cmd = decodeCommand(buffer);

    if(cmd.command=="100"){
        return 1;
    }    
    else if(cmd.command == "200"){
        QMessageBox::critical(NULL,"Nickname In Use","The nickname you've chosen is already in use.");
        close(client_socket);
        return 0;
    }
    else if(cmd.command == "201"){
        QMessageBox::critical(NULL,"Invalid Nickname","The nickname you've chosen is invalid.");
        close(client_socket);
        return 0;
    }
    else{
        QMessageBox::critical(NULL,"Unkown Error","An unknown error has occured.");
        close(client_socket);
        return 0;
    }

}

int readLine(int sock,char* buffer, int size,int timeout){
    
    char* buffpos=buffer;
    char*buffend=buffer+size;
    int nchars;
    int readlen=0;
    bool complete = false;
    fd_set fd_set;
    struct timeval tv;
    int sockStatus;
    int readSize;
    
    FD_ZERO(&fd_set);
    FD_SET(sock,&fd_set);
    if(timeout>0){
        tv.tv_sec=0;
        tv.tv_usec=timeout;
        sockStatus=select(sock+1,&fd_set,NULL,&fd_set,&tv);
    }
    else{
        sockStatus=select(sock+1,&fd_set,NULL,&fd_set,NULL);
    }
    if(sockStatus<=0){
        return sockStatus;
    }
    buffer[0]='\0';
    while(!complete){
        if((buffend-buffpos)<0){
            readSize = 0;
        }
        else{
            readSize=1;
        }
        FD_ZERO(&fd_set);
        FD_SET(sock,&fd_set);
        tv.tv_sec=5;
        tv.tv_usec=0;
        sockStatus = select(sock+1,&fd_set,NULL,&fd_set,&tv);
        if(sockStatus<0){
            return -1;
        }
        nchars = recv(sock, (char*) buffpos,readSize,MSG_NOSIGNAL);
        readlen+=nchars;
        if(nchars<=0){
            return -1;
        }
        if(buffpos[nchars -1]=='\n'){
            complete=true;
            buffpos[nchars - 1] = '\0';
        }
        buffpos+=nchars;
    }
    return readlen;
    }

commands decodeCommand(const char* buffer){
    struct commands ret_cmd;
    int state;
    state = 0;
    for (int x = 0; x < strlen(buffer); x++) {
    if (buffer[x] == ' ' && state < 2) {
        state++;
        } else {
            switch (state) {
                    case 0:
                            ret_cmd.command += toupper(buffer[x]);
                            break;
                    case 1:
                            ret_cmd.operand1 += buffer[x];
                            break;
                    default:
                            ret_cmd.operand2 += buffer[x];
                            }
                }
    }
    return ret_cmd;
}
