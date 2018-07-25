#include "dialog.h"

//#include <iostream>
using namespace std;


Dialog::Dialog(const vector<Connection>& connections):
    hostLabel(new QLabel(tr("Server Name:"))), portLabel(new QLabel(tr("Server Port:"))),
    button(new QPushButton(tr("Connect to Node"))), portLineEdit(new QLineEdit()),
    hostList(new QTextBrowser()), listLabel(new QLabel("Connected Servers:")), hostLineEdit(nullptr)
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

//    QFont font("MS Shell Dlg", 9);
    setFont(QFont("MS Shell Dlg", 9));

    hostLineEdit = new QLineEdit(ipAddress);
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));
    button->setEnabled(false);

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(listLabel, 0, 0);
    mainLayout->addWidget(hostList, 1, 0, 2, 0);
    mainLayout->addWidget(hostLabel, 3, 0);
    mainLayout->addWidget(hostLineEdit, 3, 1);
    mainLayout->addWidget(portLabel, 4, 0);
    mainLayout->addWidget(portLineEdit, 4, 1);
    mainLayout->addWidget(button, 6, 0, 1, 2);
    setLayout(mainLayout);

    setWindowTitle(tr("Blockchain Client"));
    portLineEdit->setFocus();

    for(const Connection& c : connections) {
        hostList->append("<b>IP Address:</b> " + c.ipAddr + "<br><b>Port:</b> " + QString::number(c.portAddr) + "<br>");
    }

    connect(button, SIGNAL(clicked()), this, SLOT(requestBlockchain()));
    connect(hostLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(enableButton()));
    connect(portLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(enableButton()));
}

Dialog::~Dialog() {
//    cerr << "In destructor..." << endl;
    delete hostLabel;
    delete portLabel;
    delete button;
    delete hostLineEdit;
    delete portLineEdit;
    delete hostList;
    delete listLabel;
    close();
}

void Dialog::requestBlockchain()
{
    button->setEnabled(false);
    emit selectedSettings(hostLineEdit->text(), portLineEdit->text().toInt());
}

void Dialog::enableButton()
{
    bool enable(!hostLineEdit->text().isEmpty() && !portLineEdit->text().isEmpty());
    button->setEnabled(enable);
}

void Dialog::updateServerLists(const std::vector<Connection>& hosts) {
    hostList->clear();
    for(const Connection& c : hosts) {
        hostList->append("<b>IP Address:</b> " + c.ipAddr + "<br><b>Port:</b> " + QString::number(c.portAddr) + "<br>");
    }
}
