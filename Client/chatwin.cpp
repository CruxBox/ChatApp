
#include <sys/socket.h>
#include <chatcli.h>
#include <chatwin.h>
#include <QVBoxLayout>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qinputdialog.h>
#include <Qt>
cChatWin::cChatWin() : QMainWindow(){
    QWidget *big = new QWidget();
    big->setMinimumHeight(330);
    big->setMinimumWidth(400);
    QVBoxLayout *main = new QVBoxLayout();
    main->setSpacing(3);

    QHBoxLayout *row1 = new QHBoxLayout();
    chatEdit = new QTextEdit();
    chatEdit->setMinimumWidth(300);
    chatEdit->setReadOnly(true);
    userList = new QListWidget();
    userList->setMinimumWidth(100);
    userList->setMaximumWidth(100);
    row1->addWidget(chatEdit);
    row1->addWidget(userList);
    main->addLayout(row1);

    QHBoxLayout *row2 = new QHBoxLayout();
    msgEdit = new QLineEdit();
    row2->addWidget(msgEdit);
    msgEdit->setMinimumWidth(250);
    QPushButton *sendbutton = new QPushButton("&Send");
    row2->addWidget(sendbutton);
    this->connect(sendbutton,SIGNAL(clicked()),this,SLOT(sendButtonClicked()));
    QPushButton *privButton = new QPushButton("Send &Private");
    row2->addWidget(privButton);
    this->connect(privButton, SIGNAL(clicked()), this,
    SLOT(pvtButtonClicked()));
    QPushButton *opButton = new QPushButton("&Op");
    row2->addWidget(opButton);
    this->connect(opButton, SIGNAL(clicked()), this, SLOT(opButtonClicked()));
    QPushButton *kickButton = new QPushButton("&Kick");
    row2->addWidget(kickButton);
    this->connect(kickButton, SIGNAL(clicked()), this,
    SLOT(kickButtonClicked()));
    QPushButton *topicButton = new QPushButton("&Topic");
    row2->addWidget(topicButton);
    this->connect(topicButton, SIGNAL(clicked()), this,
    SLOT(topicButtonClicked()));
    QPushButton *quitButton = new QPushButton("&Quit");
    row2->addWidget(quitButton);
    this->connect(quitButton, SIGNAL(clicked()), this,
    SLOT(quitButtonClicked()));
    main->addLayout(row2);
    theTimer = new QTimer(this);
    this->connect(theTimer,SIGNAL(timeout()),this,SLOT(timerFired()));
    theTimer->start(250);

    big->setLayout(main);
    this->setCentralWidget(big);
}

cChatWin::~cChatWin(){

}


void cChatWin::sendButtonClicked(){
    string send_string;
    char buffer[MAX_LINE_BUFF];
    commands cmd;
    int status;
    if(msgEdit->text() == ""){
        return;
    }
    QString temp = msgEdit->text().toAscii();
    send_string = "MSG " + temp.toStdString() + '\n';
    send(client_socket,send_string.c_str(),send_string.length(),0);
    status = readLine(client_socket,buffer, MAX_LINE_BUFF,0);

    if (status < 0) {
        theTimer->stop();
        QMessageBox::critical(NULL, "Lost Connection","The server has closed the connection.");
        this->close();
        return;
    }
    cmd = decodeCommand(buffer);
    if(cmd.command!="100"){
        QMessageBox::critical(NULL, "Unknown Error","An unknown error has occurred.");
        return;
    }
    msgEdit->setText("");

}

void cChatWin::pvtButtonClicked()
{
    string send_string;
    char buffer[MAX_LINE_BUFF];
    commands cmd;
    int status;
    string username;
    if (msgEdit->text() == "") {
        return;
    }
    QString temp;
    temp = userList->currentItem()->text().toAscii();
    if (temp.toStdString() == "") {
        QMessageBox::critical(NULL, "Private Message","You must select a user before sending a private message.");
        return;
    }

    username = temp.toStdString();
    if(username[0]=='@'){
        username = username.substr(1);
    }
    temp = msgEdit->text().toAscii();
    send_string = "PMSG " + username + " " + temp.toStdString() + "\n";
    send(client_socket,send_string.c_str(),send_string.length(),0);
    status = readLine(client_socket,buffer,MAX_LINE_BUFF,0);
    if(status<0){
        theTimer->stop();
        QMessageBox::critical(NULL,"Lost Connection", "The server has closed the connection.");
        this->close();
        return;
    }
    cmd = decodeCommand(buffer);

    if(cmd.command == "100"){
        msgEdit->setText("");
        return;
    }
    else if(cmd.command == "202") {
        QMessageBox::critical(NULL, "Unknown User","The user specified is not in the room.");
        return;
        }
     else {
        QMessageBox::critical(NULL, "Unknown Error","An unknown error has occurred.");
        return;
}
    
}

void cChatWin::opButtonClicked()
{
string send_string;
char buffer[MAX_LINE_BUFF];
commands cmd;
int status;
string username;
    QString temp = userList->currentItem()->text().toAscii();
    if ( temp.toStdString() == "") {
        QMessageBox::critical(NULL, "Op Error","You must select a user before making them an operator.");
        return;
    }
send_string = "OP " + username + "\n";
send(client_socket, send_string.c_str(), send_string.length(), 0);
status = readLine(client_socket, buffer, MAX_LINE_BUFF,0);

if (status < 0) {
    theTimer->stop();
    QMessageBox::critical(NULL, "Lost Connection","The server has closed the connection.");
    this->close();
    return;
    }
cmd = decodeCommand(buffer);

if (cmd.command == "100") {
    return;
}
else if(cmd.command=="202"){
    QMessageBox::critical(NULL, "Unknown User","The user specified is not in the room.");
    return;
}
else if (cmd.command == "203") {
    QMessageBox::critical(NULL, "Denied","Only room operators may op other users.");
    return;
} 
else {
    QMessageBox::critical(NULL, "Unknown Error","An unknown error has occurred.");
    return;
    }

}    

void cChatWin::kickButtonClicked(){
    string send_string;
    char buffer[MAX_LINE_BUFF];
    commands cmd;
    int status;
    string username;
    QString temp = userList->currentItem()->text().toAscii();
    if ( temp.toStdString() == "") {
        QMessageBox::critical(NULL, "Private Message","You must select a user before sending a private message.");
        return;
    }
    username = temp.toStdString();
    if(username[0]=='@'){
        username = username.substr(1);
    }
    send_string = "KICK "+ username + "\n";
    send(client_socket,buffer,MAX_LINE_BUFF,0);
    status = readLine(client_socket,buffer,MAX_LINE_BUFF,0);
    if(status<0){
        theTimer->stop();
        QMessageBox::critical(NULL,"Lost Connection","The serer has closed the connection.");
        this->close();
        return;
    }
    cmd = decodeCommand(buffer);

    if (cmd.command == "100") {
        return;
    }
    else if(cmd.command=="202"){
        QMessageBox::critical(NULL, "Unknown User","The user specified is not in the room.");
        return;
    }
    else if (cmd.command == "203") {
        QMessageBox::critical(NULL, "Denied","Only room operators may may kick out other users.");
        return;
    } 
    else {
        QMessageBox::critical(NULL, "Unknown Error","An unknown error has occurred.");
        return;
        }
}
void cChatWin::topicButtonClicked(){
    string send_string;
    char buffer[MAX_LINE_BUFF];
    commands cmd;
    int status;
    bool ok;
    QString topic = QInputDialog::getText(this,"Chat Client", "Enter the new topic:",QLineEdit::Normal,QString::null,&ok);

    if(ok==false || topic.isEmpty()){
        return;
    }
    QString temp;
    temp = topic.toAscii();
    send_string = "TOPIC " + temp.toStdString()+"\n";
    send(client_socket, send_string.c_str(),send_string.length(),0);
    status = readLine(client_socket,buffer,MAX_LINE_BUFF,0);
    if(status<0){
        theTimer->stop();
        QMessageBox::critical(NULL, "Lost Connection", "The server has closed the connection.");
        this->close();
        return;
    }
    cmd = decodeCommand(buffer);

    if (cmd.command == "100") {
        return;
    } 
    else if (cmd.command == "203") {
        QMessageBox::critical(NULL, "Denied","Only room operators may change the topic.");
        return;
        }
     else {
        QMessageBox::critical(NULL, "Unknown Error","An unknown error has occurred.");
        return;
        }
} 

void cChatWin::quitButtonClicked()
{
    string send_string;
    char buffer[MAX_LINE_BUFF];
    commands cmd;
    int status;
    send_string = "QUIT\n";
    send(client_socket, send_string.c_str(), send_string.length(), 0);
    status = readLine(client_socket, buffer, MAX_LINE_BUFF,0);
    if (status < 0) {
        theTimer->stop();
        QMessageBox::critical(NULL, "Lost Connection","The server has closed the connection.");
        this->close();
        return;
    }
    cmd = decodeCommand(buffer);
    if(cmd.command!="100"){
        QMessageBox::critical(NULL, "Unknown Error","An unknown error has occurred.");
    }
    this->close();
}

void cChatWin::timerFired()
{
    int status;
    char buffer[MAX_LINE_BUFF];
    commands cmd;
    string str;
    while((status = readLine(client_socket,buffer,MAX_LINE_BUFF,100))!=0){

        qApp->processEvents();
        if(status<0){
            QMessageBox::critical(NULL,"Lost Connection", "The server has closed the connection.");
            this->close();
            return;
        }
        else if(status>0){
        cmd = decodeCommand(buffer);
        if (cmd.command == "JOIN") {
            const QString q = QString::fromStdString(cmd.operand1);
            userList->insertItem(0,q);
            str = cmd.operand1 + " has joined the room.\n";
        }
        else if(cmd.command=="MSG"){
            str = cmd.operand1 + ": " + cmd.operand2 + "\n";
            chatEdit->append(str.c_str());
        }
        else if(cmd.command == "PMSG"){
            str = cmd.operand1+ " has sent you a private message:\n\n";
            str+= cmd.operand2;
            QMessageBox::information(NULL, "Private Message",str.c_str());
        }
        else if(cmd.command=="OP"){
            bool check = false;
            QString temp;
            for(int i=0;i< userList->count();i++){
                temp=userList->item(i)->text().toAscii();
                if( temp.toStdString()== cmd.operand1){
                    userList->item(i)->setText(("@"+cmd.operand1).c_str());
                    str = cmd.operand1 + " has been made a room operator.\n";
                    
                    check = true;
                    break;
                }
            }
            if(!check) str = "No such user present.\n";
            chatEdit->append(str.c_str());
        }
        else if(cmd.command=="KICK"){
            bool check = false;
            QString temp;
            for(int i=0;i<userList->count();i++){
                temp = userList->item(i)->text().toAscii();
                if( temp.toStdString() == cmd.operand1){
                    userList->removeItemWidget(userList->item(i));
                    check=true;
                    break;
                }
                else if(temp.toStdString() == ((cmd.operand1+"@").c_str())){
                    userList->removeItemWidget(userList->item(i));
                    check=true;
                    break;
                }
            }
            if(!check) str="No such user found.\n";
            else str = cmd.operand1 + " was kicked out of the room by "+cmd.operand2 +"\n";

            chatEdit->append(str.c_str());
        }
        else if(cmd.command =="TOPIC"){
            if(cmd.operand1!="*"){
                str = "The topic has been changed to \"" + cmd.operand2 + "\" by " + cmd.operand1 + "\n";
                chatEdit->append(str.c_str());
            }
            str = "Chat Client - Topic: " + cmd.operand2;
            this->setWindowTitle(str.c_str());
        }
        else if(cmd.command=="QUIT"){
            bool check=false;
            QString temp;
            for(int i=0;i<userList->count();i++){
                temp = userList->item(i)->text().toAscii();
                if( temp.toStdString()== cmd.operand1){
                    userList->removeItemWidget(userList->item(i));
                    check=true;
                    break;
                }
                else if(temp.toStdString() == ((cmd.operand1+"@").c_str())){
                    userList->removeItemWidget(userList->item(i));
                    check=true;
                    break;
                }
            }
            if(!check) str="No such user found.\n";
            else str = cmd.operand1 + " has left the room.\n";
            chatEdit->append(str.c_str());
            }
        }
    }
    theTimer->start(250);
}

