#include "mainwindow.h"
#include "ui_mainwindow.h"

/* there are 5 modes FROM CLIENT (all NON-negative):
 * 0 : client blockchain is broken -> server sends its blockchain
 * 1 : client wants server hash -> server sends its hash
 * 2 : client is sending hash -> server compares (sends all good if everything matches, else its blockchain)
 * 3 : client is sending blocks -> new blocks added, server appends to end of blockchain and sends its hash
 * 4 : client is sending entire blockchain -> server checks sent blockchain for errors and compares it to other nodes (if all good, uses that blockchain)
*/

/* there are 4 modes FROM SERVER (all NON-positive):
 * 0 : Server + client blockchain is up to date
 * -1 : Server sends hash
 * -2 : Server sends its blockchain
 * -100 : Error has occured
*/

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow),
    _nIndex(0), mode(2), serverWait(false),
    bChain(new Blockchain<File>()), server(bChain, &connections, &serverWait)
{
    ui->setupUi(this);

    ui->label->setText("Blockchain Length: " + QString::number(bChain->length()));
    ui->ipLabel->setText("IP Address: " + server.getIpAddress());
    ui->portLabel->setText("Port: " + QString::number((quint16) (server.getPort())));

    connect(&server, SIGNAL(addConnection(QString,quint16)), this, SLOT(saveConnection(QString,quint16)));
    connect(&server, SIGNAL(updateTextBrowser(QString)), this, SLOT(addText(QString)));
    connect(&server, SIGNAL(error(int,QString,QString,quint16)), this, SLOT(displayError(int,QString,QString,quint16)));

    connect(&client, SIGNAL(error(int,QString,QString,quint16)), this, SLOT(displayError(int,QString,QString,quint16)));
    connect(&client, SIGNAL(addConnection(QString,quint16)), this, SLOT(saveConnection(QString,quint16)));

    QString errors = bChain->getErrors();
    if (!errors.isEmpty()) {
        mode = 0;
        ui->textBrowser->append("The blockchain on this computer has the following errors:");
        ui->textBrowser->append("<b>" + errors + "</b>");
        ui->textBrowser->append("Please connect to a node to resolve this issue.");

        ui->Connect->setEnabled(true);
        ui->Store->setEnabled(false);
        ui->UpdateBlockchain->setEnabled(false);
    }
 }

MainWindow::~MainWindow()
{
    if (mode) {
        on_UpdateBlockchain_clicked();
    }

    delete bChain;
    delete ui;
}

void MainWindow::on_UpdateBlockchain_clicked()
{
    ui->textBrowser->append("Saving Blockchain...");

    ui->Connect->setEnabled(false);
    ui->Store->setEnabled(false);
    ui->UpdateBlockchain->setEnabled(false);

    bChain->save();
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

        serverWait = true;

        QString hash = bChain->addBlock(File(fileName, content));
        ui->textBrowser->append("Block mined: " + hash + "\n");

        serverWait = false;

        ui->label->setText("Blockchain Length: " + QString::number(bChain->length()));
    }
    ui->textBrowser->append("\n");

    ui->Connect->setEnabled(true);
    ui->Store->setEnabled(true);
    ui->UpdateBlockchain->setEnabled(true);
}

void MainWindow::on_Save_clicked()
{
    if (ui->spinBox_1->value() >= bChain->length()) {
        ui->textBrowser->append("<b>Index cannot be greater than the length of the blockchain!</b>");
        ui->textBrowser->append("");
        return;
    }

    ui->Connect->setEnabled(false);
    ui->Store->setEnabled(false);
    ui->UpdateBlockchain->setEnabled(false);

    _nIndex = ui->spinBox_1->value();


    ui->textBrowser->append("Index " + QString::number(_nIndex) + " selected.\n");

    File file = bChain->viewAt(_nIndex);
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
    if (ui->spinBox_2->value() >= bChain->length()) {
        ui->textBrowser->append("<b>Index cannot be greater than the length of the blockchain!</b>");
        ui->textBrowser->append("");
        return;
    }
    _nIndex = ui->spinBox_2->value();


    ui->textBrowser->append("Index " + QString::number(_nIndex) + " selected.\n");
    File file = bChain->viewAt(_nIndex);

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

    serverWait = true;

    Dialog dialog(connections);

    connect(&dialog, SIGNAL(selectedSettings(QString, quint16)), this, SLOT(setUpConnection(QString, quint16)));
    connect(this, SIGNAL(addNewHost(vector<Connection>)), &dialog, SLOT(updateServerLists(vector<Connection>)));

    dialog.exec();

    serverWait = false;

    ui->Connect->setEnabled(true);
    ui->Store->setEnabled(true);
    ui->UpdateBlockchain->setEnabled(true);
}


void MainWindow::setUpConnection(const QString &ip, quint16 port) {
    QByteArray data;
    if (mode == 2) {
        data = bChain->hash();
    }
    else if (mode > 2) {
        QString path = QCoreApplication::applicationDirPath() + "/blockchain";
        QFile blockchain(path);

        if (!blockchain.open(QIODevice::ReadOnly)) {
            QMessageBox messageBox;
            messageBox.critical(0,"Error",("Cannot open:\n" + path + "\n"));
            exit(1);
        }

        if (mode == 3) {
            qint64 pos = blockchain.size() - 1;

            do {
                blockchain.seek(--pos);
            } while (blockchain.peek(1) != "\n");
            blockchain.seek(++pos);

            data = blockchain.readLine(blockchain.size() - pos);
            cerr << data.toStdString() << endl;
        }
        else {
            data = blockchain.readAll();
        }

        blockchain.close();
    }

    try {
        Package fromServer = client.talk(ip, port, mode, data);
        if (!fromServer.mode) {
            return;
        }
        else if (fromServer.mode == -1) {
            hashMap[fromServer.data].push_back(Connection(ip,port));
        }
        else {

        }
    } catch (...) {
        cerr << "Error connecting..." << endl;
    }
}

/*
void MainWindow::blockChainReceived(const QString& blockChainText, const Blockchain<File>& otherChain) {
//    cerr << "In MainWindow slot\n";
    ui->textBrowser->append("There were errors <b>from the connected node:</b>");
    ui->textBrowser->append("");

    if (blockChainText.isEmpty()) {
        ui->textBrowser->append("None :)");
        ui->textBrowser->append("\n");
        ui->textBrowser->append("Comparing this blockchain to the node's...");

        ui->textBrowser->append("<b>" + bChain->equals(otherChain) + "</b>");
    }
    else {
        ui->textBrowser->append(blockChainText);
    }

    ui->textBrowser->append("\n");
} */

void MainWindow::saveConnection(const QString& ip, quint16 port) {
    for(const Connection& c : connections) {
        if ((ip == c.ipAddr) && (port == c.portAddr)) {
            return;
        }
    }

    connections.push_back(Connection(ip, port));
    emit addNewHost(connections);
}

void MainWindow::addText(const QString& updates) {
    ui->textBrowser->append(updates);
}

void MainWindow::displayError(int socketError, const QString &message, const QString& ip, quint16 port)
{
    switch (socketError) {
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("Blockchain Client"),
                                 tr("IP: " + ip + " Port: " + QString::number((quint16) port) + " not found. Please check the "
                                    "host and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("Blockchain Client"),
                                 tr("The connection was refused by: " + "IP: " + ip + " Port: " + QString::number((quint16) port) +
                                    ". Make sure the blockchain server is running, "
                                    "and check that the host name and port "
                                    "settings are correct."));
        break;
    default:
        QMessageBox::information(this, tr("Blockchain Client"),
                                 tr("Attempted connection: " + "IP: " + ip + " Port: " + QString::number((quint16) port) + ". The following error occurred: %1.")
                                 .arg(message));
    }

    for(size_t i = 0; i < connections.size(); ++i) {
        if ((connections[i].ipAddr == ip) && (connections[i].portAddr == port)) {
            connections[i] = connections.back();
            connections.pop_back();
            return;
        }
    }
}
