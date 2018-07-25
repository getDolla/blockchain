#include "client.h"

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

Client::Client(quint16 port): serverPort(port) {}

Client::~Client() {}

Package Client::talk(const QString &hostName, quint16 port, qint8 theMode, const QByteArray& theData)
{
    const int Timeout = 8 * 1000;

    QTcpSocket socket;
    socket.connectToHost(hostName, port);
    connect(&socket, SIGNAL(disconnected()), &socket, SLOT(deleteLater()));

    // cerr << "Attempted connection in client::talk\n";

    if (!socket.waitForConnected(Timeout)) {
        emit error(socket.error(), socket.errorString(), hostName, port);
        // cerr << "throwing error in client...\n";
        return Package("Error has occured", -100);
    }
    emit addConnection(hostName, port);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (quint64) 0;
    out << serverPort;
    out << theMode;

    // cerr << QString::number(theMode).toStdString() << endl;

    if (theMode > 1) {
        out << theData;
        // cerr << theData.toStdString() << endl;
    }

    out.device()->seek(0);
    out << (quint64)(block.size() - sizeof(quint64));
    // cerr << (quint64)(block.size() - sizeof(quint64)) << endl;

    if(socket.state() == QAbstractSocket::ConnectedState) {
        socket.write(block);
    }

    if (!socket.waitForBytesWritten(Timeout)) {
        emit error(socket.error(), socket.errorString(), hostName, port);
        // cerr << "throwing error in client...\n";
        return Package("Error has occured", -100);
    }

    while (socket.bytesAvailable() < (quint64)sizeof(quint64)) {
        if (!socket.waitForReadyRead(2 * Timeout)) {
            // cerr << "In the socket.bytesAvail loop (3rd one)\n";
            emit error(socket.error(), socket.errorString(), hostName, port);
            // cerr << "throwing error in client...\n";
            return Package("Error has occured", -100);
        }
    }
    // cerr << "out of the socket.bytesAvail loop (3rd one)\n";

    quint64 blockSize;
    QDataStream in(&socket);
    in >> blockSize;
    // cerr << blockSize << endl;

    while (socket.bytesAvailable() < blockSize) {
        if (!socket.waitForReadyRead(2 * Timeout)) {
            emit error(socket.error(), socket.errorString(), hostName, port);
            // cerr << "throwing error in client...\n";
            return Package("Error has occured", -100);
        }
    }

    qint8 serverMode;
    in >> serverMode;
    // cerr << QString::number(serverMode).toStdString() << endl;

    if ((!serverMode) || (serverMode == -100)) {
        return Package("", serverMode);
    }

    QByteArray blockchain;
    in >> blockchain;
    return Package(blockchain, serverMode);
}
