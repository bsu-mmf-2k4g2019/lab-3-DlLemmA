#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QDataStream>
#include <QTcpSocket>
#include <QTextEdit>


// int is used for this constants
#define READ_FORTUNE_MARKER (0u)
#define WRITE_FORTUNE_MARKER (1u)

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void getMessage();
    void setMessage();
    void openConnection();
    void requestMessagesList();
    void readMessages();
    void displayError(QAbstractSocket::SocketError socketError);
    void enableMessageButtons();

private:
    QComboBox *hostCombo = nullptr;
    QLineEdit *portLineEdit = nullptr;
    QLineEdit *messageLineEdit = nullptr;
    QPushButton *getMessageButton = nullptr;
    QPushButton *setMessageButton = nullptr;
    QTextEdit *messageArea = nullptr;

    QTcpSocket *tcpSocket = nullptr;
    QDataStream in;
    QString currentMessage;
    QString currentMessages;
    bool getMessageFlag = 0;
    bool setMessageFlag = 0;
};

#endif // WIDGET_H
