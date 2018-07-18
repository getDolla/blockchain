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
    spIndex(0), mode(2), serverWait(false), closing(false),
    bChain(new Blockchain<File>()), server(new Server(this, bChain, &connections, &serverWait)),
    client(server->getPort())
{
    ui->setupUi(this);

    ui->label->setText("Blockchain Length: " + QString::number(bChain->length()));
    ui->ipLabel->setText("IP Address: " + server->getIpAddress());
    ui->portLabel->setText("Port: " + QString::number(server->getPort()));

    connect(server, SIGNAL(addConnection(QString,quint16)), this, SLOT(saveConnection(QString,quint16)));
    connect(server, SIGNAL(updateTextBrowser(QString)), this, SLOT(addText(QString)));
    connect(server, SIGNAL(error(int,QString,QString,quint16)), this, SLOT(displayError(int,QString,QString,quint16)));
    connect(server, SIGNAL(updateBlockchain(Blockchain<File>, QByteArray)), this, SLOT(newBlockchain(Blockchain<File>, QByteArray)));
    connect(server, SIGNAL(lengthAdded()), this, SLOT(updateLengthDisplay()));

    connect(&client, SIGNAL(error(int,QString,QString,quint16)), this, SLOT(displayError(int,QString,QString,quint16)));
    connect(&client, SIGNAL(addConnection(QString,quint16)), this, SLOT(saveConnection(QString,quint16)));

    QString errors = bChain->getErrors();

    QString path = QCoreApplication::applicationDirPath() + "/connections.txt";
    QFile ipaddrresses(path);

    if (ipaddrresses.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream ipStream(&ipaddrresses);

        cerr << "in connections.open()\n";

        QString serverIP;
        quint16 serverPort;
        bChainHash = bChain->hash();

        mode = (!errors.isEmpty()) ? 0 : 2;

        while (!ipStream.atEnd()) {
            if (!(ipStream >> serverIP).atEnd()) {
                ipStream >> serverPort;
                cerr << "serverIP: " << serverIP.toStdString() << endl;
                cerr << "serverPort: " << serverPort << endl;
                setUpConnection(serverIP, serverPort);
            }
        }

        errors = bChain->getErrors();
    }

    ipaddrresses.close();

    if (!errors.isEmpty()) {
        mode = 0;
        ui->textBrowser->append("The blockchain on this computer has the following errors:");
        ui->textBrowser->append("<b>" + errors + "</b>");
        ui->textBrowser->append("Please connect to a node to resolve this issue.<br>");

        ui->Connect->setEnabled(true);
        ui->Store->setEnabled(false);
        ui->UpdateBlockchain->setEnabled(false);
    }

    cerr << "in MainWindow(), checking index... " << bChain->getInd() << endl;
    cerr << "this: " << this << endl;
    cerr << "bChain: " << bChain << endl;
 }

MainWindow::~MainWindow()
{
    if (mode) {
        closing = true;
        on_UpdateBlockchain_clicked();

        QString path = QCoreApplication::applicationDirPath() + "/connections.txt";
        QFile ipaddrresses(path);

        if (ipaddrresses.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream ipStream(&ipaddrresses);

            for (const Connection& c : connections) {
                ipStream << c.ipAddr << " ";
                ipStream << c.portAddr << endl;
            }
        }

        ipaddrresses.close();
    }

    delete bChain;
    delete server;
    delete ui;
}

void MainWindow::updateBlockchain() {
    QByteArray commonHash = checkForUpdates();
    cerr << "In updateBlockchain(): " << endl;
    cerr << commonHash.toStdString() << endl;
    cerr << bChainHash.toStdString() << endl;
    if ((commonHash != bChainHash) && ((hashMap[commonHash].size() > 1) || (!mode))) {
        Connection commonServer = hashMap[commonHash].back();
        mode = 0;
        setUpConnection(commonServer.ipAddr, commonServer.portAddr);
    }
    bChain->save();
    bChainHash = bChain->hash();
}

void MainWindow::on_UpdateBlockchain_clicked()
{
    ui->textBrowser->append("Saving Blockchain...");

    ui->Connect->setEnabled(false);
    ui->Store->setEnabled(false);
    ui->UpdateBlockchain->setEnabled(false);

    serverWait = true;

    updateBlockchain();
    ui->textBrowser->append("Done.<br>");

    serverWait = false;

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

        ui->textBrowser->append("Saving " + fileName + " to block...<br>");

        serverWait = true;

        updateBlockchain();

        QString hash = bChain->addBlock(File(fileName, content));
        bChainHash = bChain->hash();

        QString path = QCoreApplication::applicationDirPath() + "/blockchain";
        QFile blockchain(path);

        if (!blockchain.open(QIODevice::ReadOnly)) {
            QMessageBox messageBox;
            messageBox.critical(0,"Error",("Cannot open:\n" + path + "\n"));
            exit(1);
        }

        qint64 pos = blockchain.size() - 1;

        do {
            blockchain.seek(--pos);
        } while (blockchain.peek(1) != "\n");
        blockchain.seek(++pos);

        QByteArray data = blockchain.readLine(blockchain.size() - pos);
        cerr << data.toStdString() << endl;
        blockchain.close();

        cerr << "in store(), checking index... " << bChain->getInd() << endl;

        hashMap.clear();
        size_t ctr = 0;
        while (ctr < connections.size()) {
            Package fromServer = client.talk(connections[ctr].ipAddr, connections[ctr].portAddr, 3, data);

            if (fromServer.mode == -1) {
                hashMap[fromServer.data].push_back(Connection(connections[ctr].ipAddr,connections[ctr].portAddr));
                ++ctr;
            }
            else {
                cerr << "fromServer.mode: " << fromServer.data.toStdString() << endl;
            }
        }

        ui->textBrowser->append("Block mined: " + hash);

        serverWait = false;

        ui->label->setText("Blockchain Length: " + QString::number(bChain->length()));
    }
    ui->textBrowser->append("<br>");

    ui->Connect->setEnabled(true);
    ui->Store->setEnabled(true);
    ui->UpdateBlockchain->setEnabled(true);
}

void MainWindow::on_Save_clicked()
{
    if (ui->spinBox_1->value() > bChain->length()) {
        ui->textBrowser->append("<b>Index cannot be greater than the length of the blockchain!</b><br>");
        return;
    }

    ui->Connect->setEnabled(false);
    ui->Store->setEnabled(false);
    ui->UpdateBlockchain->setEnabled(false);

    spIndex = ui->spinBox_1->value();


    ui->textBrowser->append("Index " + QString::number(spIndex) + " selected.<br>");

    File file = bChain->viewAt(spIndex);
    QString fileName = (file.getFileName() != "") ? file.getFileName() :
                      (QString::number(spIndex) + ".txt");

    QString selectedFile = QFileDialog::getSaveFileName(this,
                                                        tr("Save File"), fileName,
                                                        tr("All Files (*)"));

    if (!selectedFile.isEmpty()) {
        QFile ofs(selectedFile);
        ofs.open(QIODevice::WriteOnly);
        ui->textBrowser->append("Saving data from " + fileName + " to " + selectedFile + " ...<br>");
        ofs.write(file.getData());
        ofs.close();

        ui->textBrowser->append("Data successfully saved.<br>");
    }

    ui->Connect->setEnabled(true);
    ui->Store->setEnabled(true);
    ui->UpdateBlockchain->setEnabled(true);
}

void MainWindow::on_View_clicked()
{
    serverWait = true;

    for (size_t i = 1; i <= bChain->length(); ++i) {
        ui->textBrowser->append("<b>" + QString::number(i) + ":</b>");
        ui->textBrowser->append(bChain->viewAt(i).getFileName()+"<br>");
    }

    serverWait = false;
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

    bChainHash = bChain->hash();

    dialog.exec();

    serverWait = false;

    if (mode) {
        ui->Connect->setEnabled(true);
        ui->Store->setEnabled(true);
        ui->UpdateBlockchain->setEnabled(true);
    }
}


bool MainWindow::setUpConnection(const QString &ip, quint16 port) {
    if ((ip == server->getIpAddress()) && (port == server->getPort())) {
        cerr << "LMAO can't connect to yourself bro\n";
        removeConnectection(ip, port);
        return false;
    }

    cerr << "In setUpConnection\n";
    QByteArray data;
    if (mode == 2) {
        cerr << "mode == 2" << endl;
        data = bChainHash;
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

    Package fromServer = client.talk(ip, port, mode, data);
    if (!fromServer.mode) {
        hashMap[bChainHash].push_back(Connection(ip,port));
    }
    else if (fromServer.mode == -1) {
        hashMap[fromServer.data].push_back(Connection(ip,port));
    }
    else if (fromServer.mode == -2) {
        Blockchain<File> importedChain = fromServer.data;
        QString errors = importedChain.getErrors();

        if (errors.isEmpty()) {
            newBlockchain(importedChain, fromServer.data);
        }
        else {
            ui->textBrowser->append("There were errors <b>from the connected node:</b><br>" + errors);
            ui->textBrowser->append("Sending blockchain on this computer to connected node...<br>");
            mode = 4;
            setUpConnection(ip, port);
        }
    }
    else if (fromServer.mode == -100) {
        cerr << "fromServer.mode: " << fromServer.data.toStdString() << endl;
        return false;
    }
    return true;
}

void MainWindow::newBlockchain(const Blockchain<File>& importedChain, const QByteArray &packet) {
    cerr << "In newBlockchain" << endl;
    QByteArray importedHash = QCryptographicHash::hash(packet, QCryptographicHash::Sha3_512).toHex();
    QByteArray commonHash = checkForUpdates();

    cerr << "ImportedHash: " << importedHash.toStdString() << endl;
    cerr << "CommonHash: " << commonHash.toStdString() << endl;

    if ((commonHash != bChainHash) && ((hashMap[commonHash].size() > 1) || (!mode))) {
        if (commonHash == importedHash) {
            bChain->operator =(importedChain);
            bChain->save();
            bChainHash = bChain->hash();
            ui->textBrowser->append("<b>Note:</b> Blockchain Updated!");
            ui->textBrowser->append("Using blockchain from another node!<br>");
        }
        else if (!commonHash.isEmpty()) {
            Connection commonServer = hashMap[commonHash].back();
            mode = 0;
            setUpConnection(commonServer.ipAddr, commonServer.portAddr);
        }
    }

    mode = 2;
    ui->label->setText("Blockchain Length: " + QString::number(bChain->length()));
}

QByteArray MainWindow::checkForUpdates() {
    hashMap.clear();
    size_t ctr = 0;
    bChainHash = bChain->hash();
    qint8 oldMode = mode;

    while (ctr < connections.size()) {
           mode = 1;
           if (setUpConnection(connections[ctr].ipAddr, connections[ctr].portAddr)) {
                ++ctr;
           }
    }

    mode = oldMode;
    QByteArray commonHash;
    size_t maxSize = 0;
    for (const auto& iter : hashMap) {
        cerr << "iter.first: " << iter.first.toStdString() << endl;
        if (iter.second.size() > maxSize) {
            maxSize = iter.second.size();
            commonHash = iter.first;
        }
    }

    return commonHash;
}

void MainWindow::saveConnection(const QString& ip, quint16 port) {
    for(const Connection& c : connections) {
        if ((ip == c.ipAddr) && (port == c.portAddr)) {
            return;
        }
    }

    connections.push_back(Connection(ip, port));
    QString text = "Connected to:<br>";
    text += "<b>IP Address:</b> " + ip + "<br><b>Port:</b> ";
    text += QString::number(port) + "<br>";
    ui->textBrowser->append(text);

    emit addNewHost(connections);
}

void MainWindow::addText(const QString& updates) {
    ui->textBrowser->append(updates);
}

void MainWindow::displayError(int socketError, const QString &message, const QString& ip, quint16 port)
{
    if (!closing) {
        QMessageBox messageBox;
        switch (socketError) {
        case QAbstractSocket::HostNotFoundError:
            messageBox.information(this, tr("Blockchain Client"),
                                     ("IP: " + ip + " Port: " + QString::number(port) + " not found. Please check the " +
                                        "host and port settings."));
            break;
        case QAbstractSocket::ConnectionRefusedError:
            messageBox.information(this, tr("Blockchain Client"),
                                     ("The connection was refused by: IP: " + ip + " Port: " + QString::number(port) +
                                        ". Make sure the blockchain server is running, " +
                                        "and check that the host name and port " +
                                        "settings are correct."));
            break;
        default:
            messageBox.information(this, tr("Blockchain Client"),
                                     ("Attempted connection: IP: " + ip + " Port: " + QString::number(port) + ". The following error occurred: %1.")
                                     .arg(message));
        }
    }

    cerr << "still here in display error...\n";
    removeConnectection(ip, port);
}

bool MainWindow::removeConnectection(const QString& ip, quint16 port) {
    for(size_t i = 0; i < connections.size(); ++i) {
        if ((connections[i].ipAddr == ip) && (connections[i].portAddr == port)) {
            connections[i] = connections.back();
            connections.pop_back();
            cerr << "removing connection...\n";
            return true;
        }
    }

    return false;
}

void MainWindow::updateLengthDisplay() {
    ui->label->setText("Blockchain Length: " + QString::number(bChain->length()));
}
