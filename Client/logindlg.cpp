
#include "logindlg.h"
#include "chatcli.h"
#include <sys/socket.h>

#include "chatwin.h"
#include <qmessagebox.h>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <qlabel.h>
#include <qpushbutton.h>
cLoginDlg:: cLoginDlg(QWidget *parent) : QDialog(parent){
    this->setMinimumHeight(110);
    this->setMinimumWidth(200);
    this->setWindowTitle("Connect");
    
    QVBoxLayout *main = new QVBoxLayout();
    this->setLayout(main);
    QHBoxLayout *row1 = new QHBoxLayout(this);
    row1->setSpacing(3);
    QLabel* hostlabel = new QLabel();
    hostlabel->setText("Host");
    row1->addWidget(hostlabel);
    hostEdit = new QLineEdit();
    row1->addWidget(hostEdit);
    main->addLayout(row1,0);

    QHBoxLayout *row2 = new QHBoxLayout();
    row2->setSpacing(3);
    QLabel *portlabel = new QLabel();
    portlabel->setText("Port");
    row2->addWidget(portlabel);
    portEdit = new QLineEdit();

    main->addLayout(row2,1);

    QHBoxLayout *row3 = new QHBoxLayout();
    row3->setSpacing(3);
    QLabel* nicklabel = new QLabel();
    nicklabel->setText("Nickname");
    row3->addWidget(nicklabel);
    nickEdit = new QLineEdit();
    row3->addWidget(nickEdit);
    main->addLayout(row3,2);

    QHBoxLayout *row4 = new QHBoxLayout();
    row4->setSpacing(3);
    QPushButton *connbutton = new QPushButton("Connect");
    row4->addWidget(connbutton);
    this->connect(connbutton,SIGNAL(clicked()),this,SLOT(accept()));
    QPushButton *cancelbutton = new QPushButton("Cancel");
    row4->addWidget(cancelbutton);
    this->connect(cancelbutton, SIGNAL(clicked()),this,SLOT(accept()));
    main->addLayout(row4,3);
}

cLoginDlg::~cLoginDlg(){   
}
