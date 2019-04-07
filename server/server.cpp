#include <iostream>
#include <string.h>
#include <string>
#include <map>
#include <vector>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <unistd.h>
#include <inttypes.h>
using namespace std;

#define PORT 6969
#define MAX_LINE_BUFF 1024

struct client_t {
bool operator_status;
bool kickflag;
vector<string> outbound;
};

struct commands{
    string command;
    string operand1;
    string operand2;
};

pthread_mutex_t room_topic_mutex;
string room_topic;
pthread_mutex_t client_list_mutex;
map<string,client_t> client_list;
void* thread_proc(void *arg);
int readLine(int sock,char*buffer,int buffsize);
commands decodeCommand(const char *buffer);
int join_command(const commands &command,string &message);
int msg_command(const commands &command,const string &nickname,string &message);
int pmsg_command(const commands &command,const string &nickname,string &message);
int op_command(const commands &command,const string &nickname,string &message);
int kick_command(const commands &command,const string &nickname,string &message);
int quit_command(const string &nickname,string &message);
int topic_command(const commands &command,const string &nickname,string &message);

int main(int argc,char *argv[]){
    struct sockaddr_in serv_addr;
    int listensock;
    int connsock;
    int result;
    pthread_t thread_id;
    int flag=1;
    
    listensock=socket(AF_INET,SOCK_STREAM,0);
    setsockopt(listensock,0,SO_REUSEADDR,&flag,sizeof(int));

    serv_addr.sin_addr.s_addr=INADDR_ANY;
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(PORT);

    if((result=bind(listensock,(sockaddr*)&serv_addr,sizeof(sockaddr_in)))<0){
        perror("binding error");
        exit(0);
    }
    if((result=listen(listensock,10))<0){
        perror("listening error");
        exit(1);
    }

    pthread_mutex_init(&room_topic_mutex,NULL);
    pthread_mutex_init(&client_list_mutex,NULL);

    while(1){
        connsock=accept(listensock,(sockaddr*)NULL,NULL);
        result=pthread_create(&thread_id,NULL,thread_proc,(void*)(intptr_t)connsock);
        if(result!=0){
            printf("Thread not created.\n");
            exit(3);
        }
        pthread_detach(thread_id);
        sched_yield();
    }
    return 11;
}



void* thread_proc(void* arg){
    int sock= (intptr_t)arg;
    char BUFFER[MAX_LINE_BUFF];
    int nread;
    int flag=1;
    bool joined=false;
    bool quit=false;
    string nickname;
    struct commands command;
    string return_msg;
    map<string,client_t>::iterator client_iter;

    int status;
    string outstring;
    setsockopt(sock,0,TCP_NODELAY,&flag,sizeof(int));
    ioctl(sock,FIONBIO,&flag);
    while(!quit){
        status=readLine(sock,BUFFER,MAX_LINE_BUFF);
        if(status<0){
            //check this out
            if(joined){
                quit_command(nickname,return_msg);
            }
        return arg;
        }
        else if(status>0){
            command=decodeCommand(BUFFER);
            if(!joined &&command.command!="JOIN"){
                return_msg="203 DENIED - MUST JOIN FIRST";
            }
            else{
                if(command.command=="JOIN"){
                    if(joined){
                        return_msg="203 DENIED - ALREADY JOINED";
                    }
                    else{
                      status = join_command(command,return_msg); 
                      if(status>0){
                          joined=true;
                          nickname=command.operand1;
                      } 
                    }
                }
                else if(command.command=="MSG"){
                    msg_command(command,nickname,return_msg);
                }
                else if(command.command=="PMSG"){
                    pmsg_command(command,nickname,return_msg);
                }
                else if(command.command=="OP"){
                    op_command(command,nickname,return_msg);
                }
                else if(command.command=="KICK"){
                    kick_command(command,nickname,return_msg);
                }
                else if(command.command=="TOPIC"){
                    topic_command(command,nickname,return_msg);
                } 
                else if(command.command=="QUIT"){
                    quit_command(nickname,return_msg);
                    joined=false;
                    quit=true;
                }
                else{
                    return_msg="900 UNKNOWN COMMAND";
                }
                
            }
            return_msg+="\n";
            send(sock,return_msg.c_str(),return_msg.length(),0);
        }
        if(joined){
            pthread_mutex_lock(&client_list_mutex);
            for(int i=0;i<client_list[nickname].outbound.size();i++){
                outstring=client_list[nickname].outbound[i] + "\n";
                send(sock,outstring.c_str(),outstring.length(),0);
            }
            client_list[nickname].outbound.clear();
            if(client_list[nickname].kickflag==true){
                client_iter=client_list.find(nickname);
                if(client_iter!=client_list.end()){
                    client_list.erase(client_iter);
                }
                quit=true;
            }
            pthread_mutex_unlock(&client_list_mutex);
        }
        sched_yield();
    }
    close(sock);
    return arg;
}

int readLine(int sock,char*buffer, int size){
    buffer[0]='\0';
    char *buffpos=buffer;
    char *buffend=buffer+size;
    int nchars,readlen=0;
    bool complete=false;
    fd_set fd_set;
    struct timeval tv;
    int sockStatus;
    int readSize;
    FD_ZERO(&fd_set);
    FD_SET(sock,&fd_set);
    tv.tv_sec=0;
    tv.tv_usec=50;
    if((sockStatus=select(sock+1,&fd_set,NULL,&fd_set,&tv))<=0){
        return sockStatus;
    }
    while(!complete){
        if((buffend-buffpos)<0){
            readSize=0;
        }
        else{
            readSize=1;
        }
        FD_ZERO(&fd_set);
        FD_SET(sock,&fd_set);
        tv.tv_sec=5;
        tv.tv_usec=0;
        sockStatus=select(sock+1,&fd_set,NULL,&fd_set,&tv);
        if(sockStatus<0){
            return -1;
        }   
    nchars=recv(sock,(char*)buffpos,readSize,MSG_NOSIGNAL);
    if(nchars<=0){
        return -1;
    }
    readlen+=nchars;

    if(buffpos[nchars-1]=='\n'){
        complete=true;
        buffpos[nchars-1]='\0';
    }
        buffpos+=nchars;
    }
    return readlen;
}

commands decodeCommand(const char* buffer){
    struct commands new_cmd;
    int state=0;

    for(int i=0;i<strlen(buffer);i++){
        if(buffer[i]==' ' && state<2){
            state++;
        }
        else{
            switch(state){
                case 0:     new_cmd.command+=toupper(buffer[i]);
                            break;
                case 1:     new_cmd.operand1+=buffer[i];
                            break;
                default:    new_cmd.operand2+=buffer[i];
                            break;
            }
        }
    }
    return new_cmd;
}

int join_command(const commands &cmd,string &msg){
    int retval;
    map<string,client_t>::iterator client_iter;
    if(cmd.operand1.length()==0 || cmd.operand2.length()>0){
        msg="201 INVALID NICKNAME";
        return 0;
    }
    else{
        pthread_mutex_lock(&client_list_mutex);
        client_iter=client_list.find(cmd.operand1);
        if(client_iter==client_list.end()){
            if(client_list.size()==0){
                client_list[cmd.operand1].operator_status=true;
            }
            else{
                client_list[cmd.operand1].operator_status=false;
            }
            client_list[cmd.operand1].kickflag=false;

            for(client_iter=client_list.begin();client_iter!=client_list.end();client_iter++){
                //Telling others about new user
                if((*client_iter).first!=cmd.operand1){
                    (*client_iter).second.outbound.push_back("JOIN "+cmd.operand1);
                }
                //tell the new client which users are already in room
                client_list[cmd.operand1].outbound.push_back("JOIN "+(*client_iter).first);
                //tell new client who has operator status
                if((*client_iter).second.operator_status==true){
                    client_list[cmd.operand1].outbound.push_back("OP "+(*client_iter).first);
                }
            }

            //tell new client the room topic
            pthread_mutex_lock(&room_topic_mutex);
            client_list[cmd.operand1].outbound.push_back("TOPIC * "+ room_topic);
            pthread_mutex_unlock(&room_topic_mutex);
            msg="100 OK";
            retval = 1;
        }
        else{
            msg="200 NICKNAME IN USE";
            retval=0;
        }
        pthread_mutex_unlock(&client_list_mutex);
    }
    return retval;
}
int msg_command(const struct commands &cmd,const string &nickname,string &msg){
    map<string,client_t>::iterator client_iter;

    pthread_mutex_lock(&client_list_mutex);
    for(client_iter=client_list.begin();client_iter!=client_list.end();client_iter++){
        (*client_iter).second.outbound.push_back("MSG "+nickname+ " "+cmd.operand1+" "+cmd.operand2);
    }
    pthread_mutex_unlock(&client_list_mutex);
    msg="100 OK";

    return 1;
}

int pmsg_command(const commands &cmd, const string &nickname, string &msg)
{
map<string, client_t>::iterator client_iter;
pthread_mutex_lock(&client_list_mutex);
client_iter = client_list.find(cmd.operand1);
if (client_iter == client_list.end()) {
msg = "202 UNKNOWN NICKNAME";
} else {
(*client_iter).second.outbound.push_back("PMSG " + nickname + " " +
cmd.operand2);
msg = "100 OK";
}
pthread_mutex_unlock(&client_list_mutex);
return 1;
}

int op_command(const commands &cmd,const string &nickname,string &msg){
    map<string,client_t>::iterator client_iter;

    pthread_mutex_lock(&client_list_mutex);
    client_iter=client_list.find(nickname);
    if(client_iter==client_list.end()){
        msg="999 UNKNOWN";
    }
    else{
        if((*client_iter).second.operator_status==false){
            msg="203 DENIED";
        }
        else{
            client_iter=client_list.find(cmd.operand1);
            if(client_iter == client_list.end()){
                msg= "202 UNKNOWN NICKNAME";
            }
            else{
                (*client_iter).second.operator_status=true;
                for(client_iter=client_list.begin();client_iter!=client_list.end();client_iter++){
                    (*client_iter).second.outbound.push_back("OP "+cmd.operand1);
                }
                msg="100 OK";
            }
        }
    }
    pthread_mutex_unlock(&client_list_mutex);
    return 1;
}

int kick_command(const commands &cmd,const string &nickname,string &msg){
    map<string, client_t>::iterator client_iter;

    pthread_mutex_lock(&client_list_mutex);
    client_iter = client_list.find(nickname);
    if(client_iter==client_list.end()){
        msg="999 UNKOWN";
    }
    else{
        if((*client_iter).second.operator_status==false){
            msg="203 DENIED";
        }
        else{
            (*client_iter).second.kickflag=true;
            for(client_iter=client_list.begin();client_iter!=client_list.end();client_iter++){
                (*client_iter).second.outbound.push_back("KICK "+ cmd.operand1+" "+nickname);
            }
            msg="100 OK";
        }
    }

pthread_mutex_unlock(&client_list_mutex);
return 1;
}

int topic_command(const commands &cmd,const string &nickname, string &msg){

    map<string, client_t>::iterator client_iter;
    pthread_mutex_lock(&client_list_mutex);
    client_iter = client_list.find(nickname);
    if (client_iter == client_list.end()){
        msg = "999 UNKNOWN";
    } 
    else{
        if((*client_iter).second.operator_status == false) {
            msg = "203 DENIED";
        }   
        else{
        pthread_mutex_lock(&room_topic_mutex);
        room_topic = cmd.operand1;
        if(cmd.operand2.length() != 0) {
            room_topic += " " + cmd.operand2;
        }
        for(client_iter = client_list.begin();client_iter != client_list.end(); client_iter++) {
            (*client_iter).second.outbound.push_back("TOPIC " + nickname+ " " + room_topic);
        }
        pthread_mutex_unlock(&room_topic_mutex);
        msg = "100 OK";
        }
    }
    pthread_mutex_unlock(&client_list_mutex);
    return 1;
}

int quit_command(const string &nickname,string &msg){

    map<string, client_t>::iterator client_iter;
    pthread_mutex_lock(&client_list_mutex);
    client_iter = client_list.find(nickname);
    if (client_iter == client_list.end()) {
        msg = "999 UNKNOWN";
    } 
    else {
        client_list.erase(client_iter);
        for (client_iter = client_list.begin(); client_iter != client_list.end();client_iter++) {
            (*client_iter).second.outbound.push_back("QUIT " + nickname);
        }
        msg = "100 OK";
    }
    pthread_mutex_unlock(&client_list_mutex);
    return 1;
}