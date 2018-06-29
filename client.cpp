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

//#include <iostream>
//using namespace std;


Client::Client():
    hostLabel(new QLabel(tr("Server name:"))), portLabel(new QLabel(tr("Server port:"))),
    button(new QPushButton(tr("Connect to Node"))), portLineEdit(new QLineEdit()),
    hostLineEdit(nullptr)
{
    // find out which IP to connect to
    QString ipAddress;
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

    hostLineEdit = new QLineEdit(ipAddress);
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));
    button->setEnabled(false);

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(hostLabel, 0, 0);
    mainLayout->addWidget(hostLineEdit, 0, 1);
    mainLayout->addWidget(portLabel, 1, 0);
    mainLayout->addWidget(portLineEdit, 1, 1);
    mainLayout->addWidget(button, 3, 0, 1, 2);
    setLayout(mainLayout);

    setWindowTitle(tr("Blockchain Client"));
    portLineEdit->setFocus();

    connect(button, SIGNAL(clicked()), this, SLOT(requestBlockchain()));
    connect(hostLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(enableButton()));
    connect(portLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(enableButton()));

//! [0]
    connect(&thread, SIGNAL(newBlockchain(QString)),
            this, SLOT(showblockchain(QString)));
//! [0] //! [1]
    connect(&thread, SIGNAL(error(int,QString)),
            this, SLOT(displayError(int,QString)));
//! [1]

//    cerr << "In client constructor\n";
}

Client::~Client() {
//    cerr << "In destructor..." << endl;
    delete hostLabel;
    delete portLabel;
    delete button;
    delete hostLineEdit;
    delete portLineEdit;
    close();
}

//! [2]
void Client::requestBlockchain()
{
    button->setEnabled(false);
    thread.requestBlockchain(hostLineEdit->text(),
                             portLineEdit->text().toInt());
}
//! [2]

//! [3]
void Client::showblockchain(const QString &nextblockchain)
{
    Blockchain<File> importedChain = nextblockchain;


//       cerr << "In showblockchain\n";
//       cerr << nextblockchain.toStdString() << endl;

    emit newBlockchain(QString::fromStdString(importedChain.errors), importedChain);
}
//! [4]

void Client::displayError(int socketError, const QString &message)
{
    switch (socketError) {
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("Blockchain Client"),
                                 tr("The host was not found. Please check the "
                                    "host and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("Blockchain Client"),
                                 tr("The connection was refused by the peer. "
                                    "Make sure the blockchain server is running, "
                                    "and check that the host name and port "
                                    "settings are correct."));
        break;
    default:
        QMessageBox::information(this, tr("Blockchain Client"),
                                 tr("The following error occurred: %1.")
                                 .arg(message));
    }
}

void Client::enableButton()
{
    bool enable(!hostLineEdit->text().isEmpty() && !portLineEdit->text().isEmpty());
    button->setEnabled(enable);
}
