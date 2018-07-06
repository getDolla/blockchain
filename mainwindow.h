#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFile>
#include <QMessageBox>
#include <QDataStream>
#include <QFileDialog>
#include <QString>
#include <QMainWindow>

#include <vector>
#include <map>

#include "dialog.h"
#include "client.h"
#include "Blockchain.h"
#include "File.h"
#include "server.h"

#include <iostream>
using namespace std;

namespace Ui {
class MainWindow;
}

struct Package, connection_error;

struct Connection {
    QString ipAddr;
    quint16 portAddr;

    Connection(const QString& ip, quint16 port): ipAddr(ip), portAddr(port) {}
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void addNewHost(const vector<Connection>& hosts);

private slots:
    void on_UpdateBlockchain_clicked();

    void on_Store_clicked();

    void on_Save_clicked();

    void on_View_clicked();

    void on_Connect_clicked();

//    void blockChainReceived(const QString& blockChainText, const Blockchain<File>& otherChain);

    void saveConnection(const QString& ip, quint16 port);

    void addText(const QString& updates);

    void displayError(int socketError, const QString &message, const QString& ip, quint16 port);

    void setUpConnection(const QString& ip, quint16 port);

private:
    Ui::MainWindow *ui;
    Server server;
    Client client;
    Blockchain<File>* bChain;
    vector<Connection> connections;
    map<QByteArray, vector<Connection>> hashMap;
    unsigned int _nIndex;
    qint8 mode;
    bool serverWait;
};

#endif // MAINWINDOW_H
