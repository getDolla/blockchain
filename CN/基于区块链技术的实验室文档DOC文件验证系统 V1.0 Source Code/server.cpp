#include "server.h"

//#include <iostream>

Server::Server(Blockchain<File>* chainPtr):
    blockChainPtr(chainPtr), tcpServer(nullptr), networkSession(nullptr), port(0)
{
    QNetworkConfigurationManager manager;
    if (manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired) {
        // Get saved network configuration
        QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
        settings.beginGroup(QLatin1String("QtNetwork"));
        const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();
        settings.endGroup();

        // If the saved network configuration is not currently discovered use the system default
        QNetworkConfiguration config = manager.configurationFromIdentifier(id);
        if ((config.state() & QNetworkConfiguration::Discovered) !=
            QNetworkConfiguration::Discovered) {
            config = manager.defaultConfiguration();
        }

        networkSession = new QNetworkSession(config, this);
        connect(networkSession, SIGNAL(opened()), this, SLOT(sessionOpened()));

        networkSession->open();
    } else {
        sessionOpened();
    }

    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(handleConnection()));
}

Server::~Server() {
    delete networkSession;
    delete tcpServer;
}

void Server::sessionOpened()
{
    // Save the used configuration
    if (networkSession) {
        QNetworkConfiguration config = networkSession->configuration();
        QString id;
        if (config.type() == QNetworkConfiguration::UserChoice)
            id = networkSession->sessionProperty(QLatin1String("UserChoiceConfiguration")).toString();
        else
            id = config.identifier();

        QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
        settings.beginGroup(QLatin1String("QtNetwork"));
        settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
        settings.endGroup();
    }

    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen()) {
        QMessageBox::critical(0, tr("区块链服务器"),
                              tr("无法启动服务器: %1.")
                              .arg(tcpServer->errorString()));
        close();
        return;
    }

    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
            ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    // if we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();

    port = tcpServer->serverPort();
//    cout << ipAddress.toStdString() << " 端口: " << port << endl;
}

void Server::handleConnection() {
    while (tcpServer->hasPendingConnections())
        {
            // cerr << "In handleConnection\n";
            QTcpSocket* socket = tcpServer->nextPendingConnection();
            connect(socket, SIGNAL(readyRead()), this, SLOT(readBlocks()));
            connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
        }
}

void Server::readBlocks() {
    QTcpSocket* socket = static_cast<QTcpSocket*>(sender());
    const int Timeout = 8 * 1000;

    QString peerAddress = (socket->peerAddress().toString().contains("::ffff:")) ? socket->peerAddress().toString().mid(7) : socket->peerAddress().toString();

    while (socket->bytesAvailable() < (quint64)sizeof(quint64)) {
        if (!(socket->waitForReadyRead(2 * Timeout))) {
            emit error(socket->error(), socket->errorString(), peerAddress, socket->peerPort());
            return;
        }
    }

    quint64 blockSize;
    QDataStream in(socket);
    in >> blockSize;
    // cerr << blockSize << endl;

    while (socket->bytesAvailable() < blockSize) {
        if (!(socket->waitForReadyRead(Timeout))) {
            emit error(socket->error(), socket->errorString(), peerAddress, socket->peerPort());
            return;
        }
    }

    quint8 transfer;
    in >> transfer;

    if (transfer != TRANSFERMODE) {
        sendBlocks(socket, -100);
        socket->disconnectFromHost();
        return;
    }

    quint16 otherPort;
    in >> otherPort;

    emit addConnection(peerAddress, otherPort);

//    QString text = "Connected to:<br>";
//    text += "<b>IP地址:</b> " + peerAddress + "<br><b>端口:</b> ";
//    text += QString::number(otherPort) + "<br>";

//    emit updateTextBrowser(text);

    qint8 mode;
    in >> mode;
    // cerr << QString::number(mode).toStdString() << endl;
    qint8 serverMode;

    // cerr << "read contents\n";

    /* there are 5 modes FROM CLIENT (all NON-negative):
     * 0 : client blockchain is broken -> server sends its blockchain
     * 1 : client wants server hash -> server sends its hash
     * 2 : client is sending hash -> server compares (sends all good if everything matches, else its blockchain)
     * 3 : client is sending blocks -> new blocks added, server appends to end of blockchain and sends its hash
     * 4 : client is sending entire blockchain -> server checks sent blockchain for errors and compares it to other nodes (if all good, uses that blockchain)
    */

    if (mode > 1) {
        QByteArray packet;
        in >> packet;

        // cerr << packet.toStdString() << endl;

        if (mode == 2) {
            // cerr << "In mode == 2, checking on index... " << blockChainPtr->getInd() << endl;
            serverMode = (packet == (blockChainPtr->hash())) ? 0 : -2;
            // cerr << QString::number(serverMode).toStdString() << endl;
        }
        else if (mode == 3) {
            // cerr << "In mode == 3\n";
            // cerr << "checking on index... " << blockChainPtr->getInd() << endl;

            QString text = "从: ";
            text += "<b>IP地址:</b> " + peerAddress + " <b>端口:</b> ";
            text +=  QString::number(otherPort) + "收到了新块。<br>";
            emit updateTextBrowser(text);
            if(!(blockChainPtr->addBlocks(packet))) {
                emit updateTextBrowser("节点 <b>IP:</b> " + peerAddress + " <b>端口:</b> " + QString::number(otherPort)
                                       + " 有以下问题:<br>" + (blockChainPtr->getErrors()));
                serverMode = -2;
            }
            else {
                serverMode = -1;
            }
            emit lengthAdded();
        }
        else {
            /* blockchain is also stored in RAM */
            blockChainPtr->save();
            Blockchain<File> importedChain(packet);
            QString errors = importedChain.getErrors();

            if (errors.isEmpty()) {
                if (blockChainPtr->hash() !=
                        QCryptographicHash::hash(packet, QCryptographicHash::Sha3_512).toHex()) {
                    if (!(blockChainPtr->equals(importedChain))) {
                        if (blockChainPtr->length() <= importedChain.length()) {
                            blockChainPtr->operator =(importedChain);
                            blockChainPtr->save();
                            emit updateTextBrowser("<b>注意：</b> 区块链已更新！<br>使用来自<b>IP:</b> "
                                                   + peerAddress + " <b>端口:</b> " + QString::number(otherPort) + " 的区块链<br>");
                            emit modeChange();
                            serverMode = 0;
                        }
                        else {
                            emit updateTextBrowser("将此计算机上的区块链发送到: <b>IP:</b> "
                                                   + peerAddress + " <b>端口:</b> " + QString::number(otherPort) + "<br>");
                            serverMode = -2;
                        }
                    }
                    else {
                        blockChainPtr->save();
                        emit updateTextBrowser("<b>注意：</b> 区块链与 <b>IP:</b> "
                                               + peerAddress + " <b>端口:</b> " + QString::number(otherPort) + " 同步。<br>");
                        emit modeChange();
                        serverMode = 0;
                    }
                }
            }
            else {
                emit updateTextBrowser("节点 <b>IP:</b> " + peerAddress + " <b>端口:</b> " + QString::number(otherPort)
                                       + " 有以下问题:<br>" + errors);

                emit updateTextBrowser("将此计算机上的区块链发送到节点...<br>");
                serverMode = -2;
            }

            emit lengthAdded();
        }
    }
    else if (!mode) {
        serverMode = -2;
    }
    else if (mode == 1) {
        serverMode = -1;
    }

    /* there are 4 modes FROM SERVER (all NON-positive):
     * 0 : Server + client blockchain is up to date
     * -1 : Server sends hash
     * -2 : Server sends its blockchain
     * -100 : Error has occured
    */

    sendBlocks(socket, serverMode);
    // cerr << "gonna disconnect this bad boy\n";
    // cerr << "checking on index... " << blockChainPtr->getInd() << endl;
    socket->disconnectFromHost();
    // cerr << "bad boy disconnected\n";
}

void Server::sendBlocks(QTcpSocket *socket, qint8 mode)
{
    // cerr << "In sendBlocks\n";
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (quint64) 0;
    out << mode;

    QByteArray message;
    if (mode && (mode != -100)) {
        // cerr << "if (mode)" << endl;
        if (mode == -1) {
            message = blockChainPtr->hash();
        }
        else if (mode == -2) {
            QString path = QCoreApplication::applicationDirPath() + "/blockchain";
            QFile ifs(path);

            if (!ifs.open(QIODevice::ReadOnly)) {
                QMessageBox messageBox;
                messageBox.critical(0,"错误",("不能打开:\n" + path + "\n"));
                exit(1);
            }

            message = ifs.readAll();
            ifs.close();
        }

        out << message;
 //    // cerr << (quint64)(block.size() - sizeof(quint64)) << endl;
    }

    out.device()->seek(0);
    out << (quint64)(block.size() - sizeof(quint64));
    // cerr << "Writing size in server..." << endl;
    // cerr << (quint64)(block.size() - sizeof(quint64)) << endl;
    socket->write(block);
}
