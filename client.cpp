/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

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

Client::Client() {}

Client::~Client() {}

Package Client::talk(const QString &hostName, quint16 port, qint8 theMode, const QByteArray& theData)
{
    QString serverName = hostName;
    quint16 serverPort = port;
    qint8 clientMode = theMode;
    QByteArray dataToSend = theData;

    const int Timeout = 10 * 1000;

    QTcpSocket socket;
    socket.connectToHost(serverName, serverPort);
    connect(&socket, SIGNAL(disconnected()), &socket, SLOT(deleteLater()));

    if (!socket.waitForConnected(Timeout)) {
        emit error(socket.error(), socket.errorString(), serverName, serverPort);
        throw connection_error();
    }
    emit addConnection(serverName, serverPort);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (quint64) 0;
    out << clientMode;

    if (clientMode > 1) {
        out << dataToSend;
    }

    out.device()->seek(0);
    out << (quint64)(block.size() - sizeof(quint64));

    if(socket.state() == QAbstractSocket::ConnectedState) {
        socket.write(block);
    }

    if (!socket.waitForBytesWritten(Timeout)) {
        emit error(socket.error(), socket.errorString(), serverName, serverPort);
        throw connection_error();
    }

    while (socket.bytesAvailable() < (quint64)sizeof(quint64)) {
        if (!socket.waitForReadyRead(Timeout)) {
            emit error(socket.error(), socket.errorString(), serverName, serverPort);
            throw connection_error();
        }
    }

    quint64 blockSize;
    QDataStream in(&socket);
    in >> blockSize;

    while (socket.bytesAvailable() < blockSize) {
        if (!socket.waitForReadyRead(Timeout)) {
            emit error(socket.error(), socket.errorString(), serverName, serverPort);
            throw connection_error();
        }
    }

    qint8 serverMode;
    in >> serverMode;

    if ((!serverMode) || (serverMode == -100)) {
        return Package("", serverMode);
    }

    QByteArray blockchain;
    in >> blockchain;
    return Package(blockchain, serverMode);
}
