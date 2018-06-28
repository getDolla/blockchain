#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QMessageBox>
#include <QDataStream>
#include <QFileDialog>
#include <QString>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), _nIndex(0)
{
    ui->setupUi(this);

    ui->label->setText("Blockchain Length: " + QString::number(bChain.length()));
    ui->ipLabel->setText("IP Address: " + server.getIpAddress());
    ui->portLabel->setText("Port: " + QString::number((quint16) (server.getPort())));
}

MainWindow::~MainWindow()
{
    on_UpdateBlockchain_clicked();
    delete ui;
}

void MainWindow::on_UpdateBlockchain_clicked()
{
    ui->textBrowser->append("Saving Blockchain...");
    bChain.save();
    ui->textBrowser->append("Done.");

    ui->textBrowser->append("\n");
}

void MainWindow::on_Store_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open File"), "",
            tr("All Files (*)"));

    if (fileName.length() == 0) {
        return;
    }
    else if (fileName.isEmpty()) {
        ui->textBrowser->append(fileName + " is empty.");
    }
    else {
        string name = fileName.toStdString();
        ifstream ifs(name, ios::binary|ios::ate);

        auto fileSize = ifs.tellg();
        ifs.seekg(ios::beg);
        string content(fileSize,0);
        ifs.read(&content[0],fileSize);

        ifs.close();

        ui->textBrowser->append(QString::fromStdString("Saving " + name + " to block...\n"));

        string hash = bChain.addBlock(File(name, content));
        ui->textBrowser->append(QString::fromStdString("Block mined: " + hash + "\n"));

        ui->label->setText("Blockchain Length: " + QString::number(bChain.length()));
    }
    ui->textBrowser->append("\n");
}

void MainWindow::on_Save_clicked()
{
    if (ui->spinBox_1->value() >= bChain.length()) {
        ui->textBrowser->append("Index cannot be greater than the length of the blockchain!");
        ui->textBrowser->append("\n");
        return;
    }
    _nIndex = ui->spinBox_1->value();


    ui->textBrowser->append("Index " + QString::number(_nIndex) + " selected.\n");

    File file = bChain.viewAt(_nIndex);
    string filename = (file.getFileName() != "") ? file.getFileName() :
                      (QCoreApplication::applicationDirPath().toStdString() + to_string(_nIndex) + ".txt");

    string otherFile = QFileDialog::getSaveFileName(this,
            tr("Save Address Book"), QString::fromStdString(filename),
            tr("All Files (*)")).toStdString();

    ofstream ofs(otherFile);
    ui->textBrowser->append(QString::fromStdString("Saving data from " + filename + " to " + filename + " ...\n"));
    ofs << file.getData();
    ofs.close();

    ui->textBrowser->append("Data successfully saved.");
    ui->textBrowser->append("\n");
}

void MainWindow::on_View_clicked()
{
    if (ui->spinBox_2->value() >= bChain.length()) {
        ui->textBrowser->append("Index cannot be greater than the length of the blockchain!");
        ui->textBrowser->append("\n");
        return;
    }
    _nIndex = ui->spinBox_2->value();


    ui->textBrowser->append("Index " + QString::number(_nIndex) + " selected.\n");
    File file = bChain.viewAt(_nIndex);

    if (file.getFileName() != "") {
        ui->textBrowser->append("Filename:");
        ui->textBrowser->append(QString::fromStdString(file.getFileName()+":\n"));
    }

    ui->textBrowser->append("Data:");
    ui->textBrowser->append(QString::fromStdString(file.getData()));
    ui->textBrowser->append("\n");
}

void MainWindow::on_Connect_clicked()
{

}
