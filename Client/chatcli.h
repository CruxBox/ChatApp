/*chatcli.h*/
#ifndef CHATCLI_H
#define CHATCLI_H

#include<string>
using namespace std;


//#define's
#define MAX_LINE_BUFF 1024

//structures

struct commands{
    string command;
    string operand1;
    string operand2;
};

//function declarations
int connectAndJoin(string host,int port,string nickname);
int readLine(int sock,char*buffer,int size);
commands decodeCommand(const char *buffer);

#endif