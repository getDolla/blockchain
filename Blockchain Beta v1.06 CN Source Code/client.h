#ifndef CLIENT_H
#define CLIENT_H

#include <QDialog>
#include <QtNetwork>
#include <QByteArray>
#include <QString>

#include <iostream>
using namespace std;

struct Package {
    QByteArray data;
    qint8 mode;
    Package(const QByteArray& theData, qint8 theMode): data(theData), mode(theMode) {}
};

class Client : public QDialog
{
    Q_OBJECT

    friend class MainWindow;
public:
    Client(quint16 port);
    ~Client();

signals:
    void error(int socketError, const QString &message, const QString& ip, quint16 port);
    void addConnection(const QString& ip, quint16 port);

private:
    quint16 serverPort;

    Package talk(const QString &hostName, quint16 port, qint8 theMode, const QByteArray& theData);
};

#endif
