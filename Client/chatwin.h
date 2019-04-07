#ifndef CHATWIN_H
#define CHATWIN_H

#include <qmainwindow.h>
#include <qtextedit.h>
#include <qlineedit.h>
#include <QListWidget>
#include <qtimer.h>

class cChatWin : public QMainWindow
{
    Q_OBJECT
    public:
    cChatWin();
    ~cChatWin();
    int client_socket;
    QTextEdit *chatEdit;
    QLineEdit *msgEdit;
    QListWidget *userList;

    protected slots:
    void sendButtonClicked();
    void pvtButtonClicked();
    void opButtonClicked();
    void kickButtonClicked();
    void topicButtonClicked();
    void quitButtonClicked();
    void timerFired();

    protected:
    QTimer *theTimer;
};
#endif