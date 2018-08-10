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
    spIndex(0), mode(2), closing(false),
    bChain(new Blockchain<File>()), server(new Server(bChain)),
    client(server->getPort())
{
    ui->setupUi(this);

    ui->label->setText("区块链长度: " + QString::number(bChain->length()));
    ui->ipLabel->setText("IP地址: " + server->getIpAddress());
    ui->portLabel->setText("端口: " + QString::number(server->getPort()));

    connect(server, SIGNAL(addConnection(QString,quint16)), this, SLOT(saveConnection(QString,quint16)));
    connect(server, SIGNAL(updateTextBrowser(QString)), this, SLOT(addText(QString)));
    connect(server, SIGNAL(error(int,QString,QString,quint16)), this, SLOT(displayError(int,QString,QString,quint16)));
    connect(server, SIGNAL(updateBlockchain(Blockchain<File>, QByteArray)), this, SLOT(newBlockchain(Blockchain<File>, QByteArray)));
    connect(server, SIGNAL(lengthAdded()), this, SLOT(updateLengthDisplay()));
    connect(server, SIGNAL(modeChange()), this, SLOT(changeMode()));

    connect(&client, SIGNAL(error(int,QString,QString,quint16)), this, SLOT(displayError(int,QString,QString,quint16)));
    connect(&client, SIGNAL(addConnection(QString,quint16)), this, SLOT(saveConnection(QString,quint16)));

    QString errors = bChain->getErrors();

    QString path = QCoreApplication::applicationDirPath() + "/connections.txt";
    QFile ipaddrresses(path);

    if (ipaddrresses.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream ipStream(&ipaddrresses);

        // cerr << "in connections.open()\n";

        QString serverIP;
        quint16 serverPort;
        bChainHash = bChain->hash();

        mode = (!errors.isEmpty()) ? 0 : 2;

        if (!ipStream.atEnd()) {
            while (!(ipStream >> serverIP).atEnd()) {
                ipStream >> serverPort;
                // cerr << "serverIP: " << serverIP.toStdString() << endl;
                // cerr << "server端口: " << serverPort << endl;
                setUpConnection(serverIP, serverPort);
            }

            if (connections.size() > 0) {
                errors = bChain->getErrors();
            }
        }
    }

    ipaddrresses.close();

    if (!errors.isEmpty()) {
        mode = 0;
        ui->textBrowser->append("这台机器上的区块链有以下问题：");
        ui->textBrowser->append("<b>" + errors + "</b>");
        ui->textBrowser->append("请链接到一个节点以解决此问题。<br>");

        ui->Connect->setEnabled(true);
        ui->Store->setEnabled(false);
        ui->UpdateBlockchain->setEnabled(false);
    }
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
    bChain->save();
    QByteArray commonHash = checkForUpdates();

    // cerr << "In updateBlockchain(): " << endl;
    // cerr << commonHash.toStdString() << endl;
    // cerr << bChainHash.toStdString() << endl;

    if ((!commonHash.isEmpty()) && ((commonHash != bChainHash) || (!mode))) {
        Connection commonServer = hashMap[commonHash].back();
        mode = (bChain->getErrors().isEmpty()) ? 2 : 0;
        while (!(setUpConnection(commonServer.ipAddr, commonServer.portAddr))) {
            commonHash = checkForUpdates();
            commonServer = hashMap[commonHash].back();
        }

        bChain->save();
        bChainHash = bChain->hash();
    }

    mode = 2;
}

void MainWindow::on_UpdateBlockchain_clicked()
{
    ui->textBrowser->append("在保存区块链...");

    ui->Connect->setEnabled(false);
    ui->Store->setEnabled(false);
    ui->UpdateBlockchain->setEnabled(false);

    updateBlockchain();
    ui->textBrowser->append("完了。<br>");

    ui->Connect->setEnabled(true);
    ui->Store->setEnabled(true);
    ui->UpdateBlockchain->setEnabled(true);
}

void MainWindow::on_Store_clicked()
{
    QString fileName;

    do {
        fileName = QFileDialog::getOpenFileName(this,
                tr("打开文件"), "",
                tr("生产在文件名中 (*)"));

        if (fileName.length() == 0) {
            return;
        }
    } while (!fileName.split("/").last().contains("生产"));

    QFile ifs(fileName);
    ifs.open(QIODevice::ReadOnly);
    QByteArray content = ifs.readAll();
    ifs.close();

    if (content.isEmpty()) {
        ui->textBrowser->append(fileName + " 不能是空白。");
    }
    else {
        ui->Connect->setEnabled(false);
        ui->Store->setEnabled(false);
        ui->UpdateBlockchain->setEnabled(false);

        ui->textBrowser->append("保存 " + fileName + " 至区块中...<br>");

        updateBlockchain();

        QString hash = bChain->addBlock(File(fileName, content));
        bChainHash = bChain->hash();

        QString path = QCoreApplication::applicationDirPath() + "/blockchain";
        QFile blockchain(path);

        if (!blockchain.open(QIODevice::ReadOnly)) {
            QMessageBox messageBox;
            messageBox.critical(0,"错误",("不能打开:\n" + path + "\n"));
            exit(1);
        }

        qint64 pos = blockchain.size() - 2;

        do {
            blockchain.seek(--pos);
        } while (blockchain.peek(1) != "\n");
        blockchain.seek(++pos);

        QByteArray data = blockchain.readLine(blockchain.size() - pos);
        // cerr << data.toStdString() << endl;
        blockchain.close();

        // cerr << "in store(), checking index... " << bChain->getInd() << endl;

        hashMap.clear();
        size_t ctr = 0;
        while (ctr < connections.size()) {
            Package fromServer = client.talk(connections[ctr].ipAddr, connections[ctr].portAddr, 3, data);

            if (fromServer.mode == -1) {
                hashMap[fromServer.data].push_back(Connection(connections[ctr].ipAddr,connections[ctr].portAddr));
                ++ctr;
            }
            else {
                // cerr << "fromServer.mode: " << fromServer.data.toStdString() << endl;
            }
        }

        ui->textBrowser->append("区块开采: " + hash);
        ui->label->setText("区块链长度: " + QString::number(bChain->length()));
    }
    ui->textBrowser->append("<br>");

    ui->Connect->setEnabled(true);
    ui->Store->setEnabled(true);
    ui->UpdateBlockchain->setEnabled(true);
}

void MainWindow::on_Save_clicked()
{
    if (ui->spinBox_1->value() > bChain->length()) {
        ui->textBrowser->append("<b>错误：指数过头了！</b><br>");
        return;
    }

    ui->Connect->setEnabled(false);
    ui->Store->setEnabled(false);
    ui->UpdateBlockchain->setEnabled(false);

    spIndex = ui->spinBox_1->value();


    ui->textBrowser->append("指数 " + QString::number(spIndex) + " 已选中。<br>");

    File file = bChain->viewAt(spIndex);
    QString fileName = (file.getFileName() != "") ? file.getFileName() :
                      (QString::number(spIndex) + ".txt");

    QString selectedFile = QFileDialog::getSaveFileName(this,
                                                        tr("保存文件"), fileName,
                                                        tr("所有文件 (*)"));

    if (!selectedFile.isEmpty()) {
        QFile ofs(selectedFile);
        ofs.open(QIODevice::WriteOnly);
        ui->textBrowser->append("保存数据从 " + fileName + " 到 " + selectedFile + " ...<br>");
        ofs.write(file.getData());
        ofs.close();

        ui->textBrowser->append("数据已成功保存。<br>");
    }

    ui->Connect->setEnabled(true);
    ui->Store->setEnabled(true);
    ui->UpdateBlockchain->setEnabled(true);
}

void MainWindow::on_View_clicked()
{
    for (size_t i = 1; i <= bChain->length(); ++i) {
        ui->textBrowser->append("<b>指数 " + QString::number(i) + ":</b>");
        ui->textBrowser->append(bChain->viewAt(i).getFileName()+"<br>");
    }
}

void MainWindow::on_Connect_clicked()
{
    ui->Connect->setEnabled(false);
    ui->Store->setEnabled(false);
    ui->UpdateBlockchain->setEnabled(false);

    Dialog dialog(connections);

    connect(&dialog, SIGNAL(selectedSettings(QString, quint16)), this, SLOT(setUpConnection(QString, quint16)));
    connect(this, SIGNAL(addNewHost(vector<Connection>)), &dialog, SLOT(updateServerLists(vector<Connection>)));

    bChainHash = bChain->hash();

    dialog.exec();

    if (mode) {
        ui->Store->setEnabled(true);
        ui->UpdateBlockchain->setEnabled(true);
    }
    ui->Connect->setEnabled(true);
}


bool MainWindow::setUpConnection(const QString &ip, quint16 port) {
    if ((ip == server->getIpAddress()) && (port == server->getPort())) {
        // cerr << "LMAO can't connect to yourself bro\n";
        removeConnectection(ip, port);
        return false;
    }

    // cerr << "In setUpConnection\n";
    QByteArray data;
    if (mode == 2) {
        // cerr << "mode == 2" << endl;
        data = bChainHash;
    }
    else if (mode > 2) {
        bChain->save();
        QString path = QCoreApplication::applicationDirPath() + "/blockchain";
        QFile blockchain(path);

        if (!blockchain.open(QIODevice::ReadOnly)) {
            QMessageBox messageBox;
            messageBox.critical(0,"错误",("不能打开:\n" + path + "\n"));
            exit(1);
        }

        if (mode == 3) {
            qint64 pos = blockchain.size() - 2;

            do {
                blockchain.seek(--pos);
            } while (blockchain.peek(1) != "\n");
            blockchain.seek(++pos);

            data = blockchain.readLine(blockchain.size() - pos);
            // cerr << data.toStdString() << endl;
        }
        else {
            data = blockchain.readAll();
        }

        blockchain.close();
    }

    Package fromServer = client.talk(ip, port, mode, data);
    if (!fromServer.mode) {
        if (hashMap.count(bChainHash)) {
            for(const Connection& c : hashMap[bChainHash]) {
                if ((ip == c.ipAddr) && (port == c.portAddr)) {
                    return true;
                }
            }
        }

        hashMap[bChainHash].push_back(Connection(ip,port));
    }
    else if (fromServer.mode == -1) {
        if (hashMap.count(fromServer.data)) {
            for(const Connection& c : hashMap[fromServer.data]) {
                if ((ip == c.ipAddr) && (port == c.portAddr)) {
                    return true;
                }
            }
        }

        hashMap[fromServer.data].push_back(Connection(ip,port));
    }
    else if (fromServer.mode == -2) {
        Blockchain<File> importedChain(fromServer.data);
        // cerr << "fromServer.data: " << fromServer.data.toStdString() << endl;
        QString errors = importedChain.getErrors();

        if (errors.isEmpty() && (importedChain.length() >= bChain->length())) {
            newBlockchain(importedChain, fromServer.data);
        }
        else {
            if (!(errors.isEmpty())) {
                ui->textBrowser->append("节点 <b>IP:</b> " + ip + " <b>端口:</b> " + QString::number(port) + " 有以下问题:<br>" + errors);
                ui->textBrowser->append("将此计算机上的区块链发送到连接节点...<br>");
            }
            else if (importedChain.length() < bChain->length()) {
                ui->textBrowser->append("将此计算机上的区块链发送到: <b>IP:</b> " + ip + " <b>端口:</b> " + QString::number(port) + "<br>");
            }
            mode = 4;
            setUpConnection(ip, port);
            mode = 2;
        }
    }
    else if (fromServer.mode == -100) {
        // cerr << "fromServer.mode: " << fromServer.data.toStdString() << endl;
        return false;
    }
    return true;
}

void MainWindow::newBlockchain(const Blockchain<File>& importedChain, const QByteArray &packet) {
    // cerr << "In newBlockchain" << endl;
    QByteArray importedHash = QCryptographicHash::hash(packet, QCryptographicHash::Sha3_512).toHex();
    QByteArray commonHash = checkForUpdates();

    // cerr << "ImportedHash: " << importedHash.toStdString() << endl;
    // cerr << "CommonHash: " << commonHash.toStdString() << endl;

    if (commonHash != bChainHash) {
        if ((hashMap[commonHash].size() > 1) || (!mode)) {
            if (commonHash == importedHash) {
                bChain->operator =(importedChain);
                bChain->save();
                bChainHash = bChain->hash();
                ui->textBrowser->append("<b>注意：</b> 区块链已更新！");
                ui->textBrowser->append("使用来自另一个节点的区块链！<br>");
            }
            else {
                Connection commonServer = hashMap[commonHash].back();
                mode = 0;
                setUpConnection(commonServer.ipAddr, commonServer.portAddr);
            }
        }
        else if (bChain->equals(importedChain)) {
            bChain->save();
            bChainHash = bChain->hash();
            ui->textBrowser->append("<b>注意：</b> 区块链与另一个节点同步。<br>");
        }
    }

    mode = 2;
    ui->label->setText("区块链长度: " + QString::number(bChain->length()));
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
        // cerr << "iter.first: " << iter.first.toStdString() << endl;
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
    QString text = "连接到：<br>";
    text += "<b>IP地址:</b> " + ip + "<br><b>端口:</b> ";
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
            messageBox.information(this, tr("连接对方计算机"),
                                     ("IP: " + ip + " 端口: " + QString::number(port) + " 没被找到。 请检查主机和端口设置。"));
            break;
        case QAbstractSocket::ConnectionRefusedError:
            messageBox.information(this, tr("连接对方计算机"),
                                     ("连接被: IP: " + ip + " 端口: " + QString::number(port) +
                                        "拒绝了。 请检查主机和端口设置。"));
            break;
        default:
            messageBox.information(this, tr("连接对方计算机"),
                                     ("试图连接: IP: " + ip + " 端口: " + QString::number(port) + ". 发生以下错误: %1.")
                                     .arg(message));
        }
    }

    // cerr << "still here in display error...\n";
    removeConnectection(ip, port);
}

bool MainWindow::removeConnectection(const QString& ip, quint16 port) {
    for(size_t i = 0; i < connections.size(); ++i) {
        if ((connections[i].ipAddr == ip) && (connections[i].portAddr == port)) {
            connections[i] = connections.back();
            connections.pop_back();
            // cerr << "removing connection...\n";
            return true;
        }
    }

    return false;
}

void MainWindow::updateLengthDisplay() {
    ui->label->setText("区块链长度: " + QString::number(bChain->length()));
}

void MainWindow::changeMode() {
    if (!mode) {
        ui->Connect->setEnabled(true);
        ui->Store->setEnabled(true);
        ui->UpdateBlockchain->setEnabled(true);
        mode = 2;
    }
}
