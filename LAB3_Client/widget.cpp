#include "widget.h"

#include <QHostInfo>
#include <QNetworkInterface>
#include <QGridLayout>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , hostCombo(new QComboBox)
    , portLineEdit(new QLineEdit)
    , getMessageButton(new QPushButton("Get Message"))
    , setMessageButton(new QPushButton("Set Message"))
    , messageArea(new QTextEdit)
    , tcpSocket(new QTcpSocket(this))
{
    qDebug() << "Constructor is called";
    hostCombo->setEditable(true);
    // find out name of this machine
    QString name = QHostInfo::localHostName();
    if (!name.isEmpty()) {
        hostCombo->addItem(name);
        QString domain = QHostInfo::localDomainName();
        if (!domain.isEmpty())
            hostCombo->addItem(name + QChar('.') + domain);
    }
    if (name != QLatin1String("localhost"))
        hostCombo->addItem(QString("localhost"));
    // find out IP addresses of this machine
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // add non-localhost addresses
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (!ipAddressesList.at(i).isLoopback())
            hostCombo->addItem(ipAddressesList.at(i).toString());
    }
    // add localhost addresses
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i).isLoopback())
            hostCombo->addItem(ipAddressesList.at(i).toString());
    }

    portLineEdit->setValidator(new QIntValidator(1, 65535, this));

    auto hostLabel = new QLabel("Server name:");
    hostLabel->setBuddy(hostCombo);
    auto portLabel = new QLabel("Server port:");
    portLabel->setBuddy(portLineEdit);

    auto messagesLabel = new QLabel("Messages:");
    messagesLabel->setBuddy(messageArea);
    messageArea = new QTextEdit("This examples requires that you run the "
                                "Message Server example as well.");


    auto messageLabel = new QLabel("Message:");
    messageLabel->setBuddy(messageLineEdit);
    messageLineEdit = new QLineEdit("Message");

    getMessageButton->setDefault(true);
    getMessageButton->setEnabled(false);

    auto quitButton = new QPushButton("Quit");

    in.setDevice(tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);

    connect(hostCombo, &QComboBox::editTextChanged,
            this, &Widget::enableMessageButtons);
    connect(portLineEdit, &QLineEdit::textChanged,
            this, &Widget::enableMessageButtons);
    connect(messageLineEdit, &QLineEdit::textChanged,
            this, &Widget::enableMessageButtons);
    connect(getMessageButton, &QAbstractButton::clicked,
            this, &Widget::getMessage);
    connect(setMessageButton, &QAbstractButton::clicked,
            this, &Widget::setMessage);
    connect(quitButton, &QAbstractButton::clicked, this, &QWidget::close);

    connect(tcpSocket, &QAbstractSocket::connected, this, &Widget::requestMessagesList);
    connect(tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
            this, &Widget::displayError);


    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->addWidget(hostLabel, 0, 0);
    mainLayout->addWidget(hostCombo, 0, 1);
    mainLayout->addWidget(portLabel, 1, 0);
    mainLayout->addWidget(portLineEdit, 1, 1);
    mainLayout->addWidget(messagesLabel, 2, 0);
    mainLayout->addWidget(messageArea, 2, 1);
    mainLayout->addWidget(messageLabel, 3, 0);
    mainLayout->addWidget(messageLineEdit, 3, 1);
    mainLayout->addWidget(getMessageButton, 4, 1, 1, 1);
    mainLayout->addWidget(setMessageButton, 5, 1, 1, 1);
    mainLayout->addWidget(quitButton, 4, 0, 1, 1);

    portLineEdit->setFocus();

    enableMessageButtons();
}

Widget::~Widget()
{

}

void Widget::getMessage()
{
    getMessageFlag = true;
    openConnection();
}

void Widget::setMessage()
{
    setMessageFlag = true;
    openConnection();
}

void Widget::openConnection()
{
    qDebug() << "Open connection is called";
    getMessageButton->setEnabled(false);
    setMessageButton->setEnabled(false);
    tcpSocket->abort();
    tcpSocket->connectToHost(hostCombo->currentText(),
                             portLineEdit->text().toInt());
}

void Widget::requestMessagesList()
{
    qDebug() << "Messages are requested";
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_10);

    if (getMessageFlag) {
        getMessageFlag = false;
        out << READ_FORTUNE_MARKER;
        connect(tcpSocket, &QAbstractSocket::readyRead,
                this, &Widget::readMessages);
    } else if (setMessageFlag) {
        setMessageFlag = false;
        out << WRITE_FORTUNE_MARKER;
        out << messageLineEdit->text();
    } else {
        qDebug() << "No action is required";
    }


    tcpSocket->write(block);
    tcpSocket->flush();
}

void Widget::readMessages()
{
    qDebug() << "Read messages is called";
    in.startTransaction();

    QString Messages;
    in >> Messages;

    if (!in.commitTransaction())
        return;

    currentMessages = Messages;

    messageArea->setText(currentMessages);
    getMessageButton->setEnabled(true);
    setMessageButton->setEnabled(true);
    disconnect(tcpSocket, &QAbstractSocket::readyRead,
               this, &Widget::readMessages);
}

void Widget::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("Message Client"),
                                 tr("The host was not found. Please check the "
                                    "host name and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("Message Client"),
                                 tr("The connection was refused by the peer. "
                                    "Make sure the message server is running, "
                                    "and check that the host name and port "
                                    "settings are correct."));
        break;
    default:
        QMessageBox::information(this, tr("Message Client"),
                                 tr("The following error occurred: %1.")
                                 .arg(tcpSocket->errorString()));
    }

    getMessageButton->setEnabled(true);
    setMessageButton->setEnabled(true);
}

void Widget::enableMessageButtons()
{
    getMessageButton->setEnabled(!hostCombo->currentText().isEmpty() &&
                                 !portLineEdit->text().isEmpty());
    setMessageButton->setEnabled(!hostCombo->currentText().isEmpty() &&
                                 !portLineEdit->text().isEmpty() &&
                                 !messageLineEdit->text().isEmpty());

}
