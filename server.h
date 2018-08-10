/*
    TRANSFERMODE:
    0 - ALL
    1 - PDF
    2 - JPEG
    3 - DOC
    4 - ACCESS
    5 - EXCEL
    6 - TXT
    7 - 质控
    8 - 生产
    9 - 库存
*/

#ifndef SERVER_H
#define SERVER_H

#include <QDialog>
#include <QtNetwork>
#include <QMessageBox>
#include <QString>
#include <QCryptographicHash>

#include <exception>

#include "Blockchain.h"
#include "File.h"

#include <iostream>
#include <stdlib.h>
using namespace std;

struct Package;
struct connection_error;

class Server : public QDialog
{
    Q_OBJECT

public:
    Server(Blockchain<File>* chainPtr);
    ~Server();

    QString getIpAddress() const {
        return ipAddress;
    }

    quint16 getPort() const {
        return port;
    }

signals:
    void updateTextBrowser(const QString& updates);
    void addConnection(const QString& ip, quint16 port);
    void error(int socketError, const QString &message, const QString& ip, quint16 port);
    void updateBlockchain(const Blockchain<File>& importedChain, const QByteArray& packet);
    void lengthAdded();
    void modeChange();

private slots:
    void sessionOpened();
    void handleConnection();
    void readBlocks();

private:
    QTcpServer *tcpServer;
    QNetworkSession *networkSession;

    Blockchain<File>* blockChainPtr;

    QString ipAddress;
    quint16 port;

    void sendBlocks(QTcpSocket* socket, qint8 mode);
};

#endif
