#include "mainwindow.h"
#include "ui_mainwindow.h"

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

    QFile ifs(fileName);
    ifs.open(QIODevice::ReadOnly);
    QByteArray content = ifs.readAll();
    ifs.close();

    if (content.isEmpty()) {
        ui->textBrowser->append(fileName + " is empty.");
    }
    else {
        ui->Connect->setEnabled(false);
        ui->Store->setEnabled(false);
        ui->UpdateBlockchain->setEnabled(false);

        ui->textBrowser->append("Saving " + fileName + " to block...\n");

        QString hash = bChain.addBlock(File(fileName, content));
        ui->textBrowser->append("Block mined: " + hash + "\n");

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
        ui->textBrowser->append("<b>Index cannot be greater than the length of the blockchain!</b>");
        ui->textBrowser->append("");
        return;
    }

    ui->Connect->setEnabled(false);
    ui->Store->setEnabled(false);
    ui->UpdateBlockchain->setEnabled(false);

    _nIndex = ui->spinBox_1->value();


    ui->textBrowser->append("Index " + QString::number(_nIndex) + " selected.\n");

    File file = bChain.viewAt(_nIndex);
    QString fileName = (file.getFileName() != "") ? file.getFileName() :
                      (QString::number(_nIndex) + ".txt");

    QString selectedFile = QFileDialog::getSaveFileName(this,
                                                        tr("Save Address Book"), fileName,
                                                        tr("All Files (*)"));

    if (!selectedFile.isEmpty()) {
        QFile ofs(selectedFile);
        ofs.open(QIODevice::WriteOnly);
        ui->textBrowser->append("Saving data from " + fileName + " to " + selectedFile + " ...\n");
        ofs.write(file.getData());
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
        ui->textBrowser->append("<b>Index cannot be greater than the length of the blockchain!</b>");
        ui->textBrowser->append("");
        return;
    }
    _nIndex = ui->spinBox_2->value();


    ui->textBrowser->append("Index " + QString::number(_nIndex) + " selected.\n");
    File file = bChain.viewAt(_nIndex);

    if (file.getFileName() != "") {
        ui->textBrowser->append("<b>Filename:</b>");
        ui->textBrowser->append(file.getFileName()+":\n");
    }

    ui->textBrowser->append("<b>Data:</b>");
    ui->textBrowser->append(QString(file.getData()));
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
