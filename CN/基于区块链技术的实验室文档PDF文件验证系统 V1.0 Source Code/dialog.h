#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QtWidgets>
#include <QFont>
#include <QTextBrowser>
#include <QtNetwork>
#include <QString>
#include <QByteArray>

#include <vector>

#include "mainwindow.h"

#include "Blockchain.h"
#include "File.h"

struct Connection;

class Dialog : public QDialog
{
    Q_OBJECT

    friend class MainWindow;
public:
    Dialog(const vector<Connection>& connections);
    ~Dialog();

signals:
    void selectedSettings(const QString &hostName, quint16 port);

private slots:
    void requestBlockchain();
    void enableButton();
    void updateServerLists(const vector<Connection>& hosts);

private:
    QLabel *hostLabel;
    QLabel *portLabel;
    QLineEdit *hostLineEdit;
    QLineEdit *portLineEdit;
    QPushButton *button;
    QTextBrowser* hostList;
    QLabel* listLabel;
};

#endif
