#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QMessageBox>
#include <QDataStream>
#include <QFileDialog>
#include <QString>

#include "client.h"

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

    ui->Connect->setEnabled(false);
    ui->Store->setEnabled(false);
    ui->UpdateBlockchain->setEnabled(false);

    bChain.save();
    ui->textBrowser->append("Done.");

    ui->textBrowser->append("\n");

    ui->Connect->setEnabled(true);
    ui->Store->setEnabled(true);
    ui->UpdateBlockchain->setEnabled(true);
}

void MainWindow::on_Store_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open File"), "",
            tr("All Files (*)"));

    if (fileName.length() == 0) {
        return;
    }

    string name = fileName.toStdString();
    ifstream ifs(name, ios::binary|ios::ate);

    auto fileSize = ifs.tellg();
    ifs.seekg(ios::beg);
    string content(fileSize,0);
    ifs.read(&content[0],fileSize);

    ifs.close();

    if (fileSize <= 0) {
        ui->textBrowser->append(fileName + " is empty.");
    }
    else {
        ui->Connect->setEnabled(false);
        ui->Store->setEnabled(false);
        ui->UpdateBlockchain->setEnabled(false);

        ui->textBrowser->append(QString::fromStdString("Saving " + name + " to block...\n"));

        string hash = bChain.addBlock(File(name, content));
        ui->textBrowser->append(QString::fromStdString("Block mined: " + hash + "\n"));

        ui->label->setText("Blockchain Length: " + QString::number(bChain.length()));
    }
    ui->textBrowser->append("\n");

    ui->Connect->setEnabled(true);
    ui->Store->setEnabled(true);
    ui->UpdateBlockchain->setEnabled(true);
}

void MainWindow::on_Save_clicked()
{
    if (ui->spinBox_1->value() >= bChain.length()) {
        ui->textBrowser->append("Index cannot be greater than the length of the blockchain!");
        ui->textBrowser->append("\n");
        return;
    }

    ui->Connect->setEnabled(false);
    ui->Store->setEnabled(false);
    ui->UpdateBlockchain->setEnabled(false);

    _nIndex = ui->spinBox_1->value();


    ui->textBrowser->append("Index " + QString::number(_nIndex) + " selected.\n");

    File file = bChain.viewAt(_nIndex);
    string filename = (file.getFileName() != "") ? file.getFileName() :
                      (QCoreApplication::applicationDirPath().toStdString() + to_string(_nIndex) + ".txt");

    QString selectedFile = QFileDialog::getSaveFileName(this,
                                                        tr("Save Address Book"), QString::fromStdString(filename),
                                                        tr("All Files (*)"));

    if (!selectedFile.isEmpty()) {
        string otherFile = selectedFile.toStdString();

        ofstream ofs(otherFile);
        ui->textBrowser->append(QString::fromStdString("Saving data from " + filename + " to " + filename + " ...\n"));
        ofs << file.getData();
        ofs.close();

        ui->textBrowser->append("Data successfully saved.");
        ui->textBrowser->append("\n");
    }

    ui->Connect->setEnabled(true);
    ui->Store->setEnabled(true);
    ui->UpdateBlockchain->setEnabled(true);
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
    ui->Connect->setEnabled(false);
    ui->Store->setEnabled(false);
    ui->UpdateBlockchain->setEnabled(false);

//    cerr << "creating client\n";

    Client client;

    connect(&client, SIGNAL(newBlockchain(QString, Blockchain<File>)), this, SLOT(blockChainReceived(QString, Blockchain<File>)));

    client.exec();

//    while(client.getUpdateInfo().isEmpty()) { cerr << "in while loop\n"; }

    ui->Connect->setEnabled(true);
    ui->Store->setEnabled(true);
    ui->UpdateBlockchain->setEnabled(true);
}

void MainWindow::blockChainReceived(const QString& blockChainText, const Blockchain<File>& otherChain) {
//    cerr << "In MainWindow slot\n";
    ui->textBrowser->append("Errors <b>from the client's blockchain:</b>");
    ui->textBrowser->append("");

    if (blockChainText.isEmpty()) {
        ui->textBrowser->append("None :)");
        ui->textBrowser->append("\n");
        ui->textBrowser->append("Comparing this blockchain to the node's...");

        ui->textBrowser->append("<b>" + bChain.equals(otherChain) + "</b>");
    }
    else {
        ui->textBrowser->append(blockChainText);
    }

    ui->textBrowser->append("\n");
}
