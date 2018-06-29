#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Blockchain.h"
#include "File.h"
#include "server.h"
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_UpdateBlockchain_clicked();

    void on_Store_clicked();

    void on_Save_clicked();

    void on_View_clicked();

    void on_Connect_clicked();

    void blockChainReceived(const QString& blockChainText, const Blockchain<File>& otherChain);

private:
    Ui::MainWindow *ui;
    Server server;
    Blockchain<File> bChain;
    unsigned int _nIndex;
};

#endif // MAINWINDOW_H
