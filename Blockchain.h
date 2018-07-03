//
// Created by Dave Nash on 20/10/2017.
// modified by Yeetkai on 25/6/2018
//

#ifndef TESTCHAIN_BLOCKCHAIN_H
#define TESTCHAIN_BLOCKCHAIN_H

#include <QCoreApplication>
#include <QString>
#include <QMessageBox>
#include <QByteArray>
#include <QTextStream>
#include <QFile>

#include <vector>

#include "Block.h"

//#include <iostream>
//#include <string>
using namespace std;

template <typename T>
class Blockchain {
public:
    QString errors;

    Blockchain(): _nDifficulty(2), _nIndex(0)
    {
        QString path = QCoreApplication::applicationDirPath() + "/blockchain";
//        cout << path << endl;

        QFile blockchain(path);

        if (!blockchain.open(QIODevice::ReadOnly)) {
            QMessageBox messageBox;
            messageBox.critical(0,"Error",("Cannot open:\n" + path + "\n"));
//            cerr << "Can not open: " << path.toStdString() << " !" << endl;
            exit(1);
        }

    	// string line;
    	// while(getline(blockchain, line)) {
    	// 	cout << line << endl;
    	// }

        QTextStream blockStr(&blockchain);
        readFromStream(blockStr);

        blockchain.close();
    }

    Blockchain(const QByteArray& chainString):_nDifficulty(2), _nIndex(0) {
//        cerr << "In blockchain 2nd constr\n";
        if (!chainString.isEmpty()) {
            QTextStream chainStr(chainString);
            readFromStream(chainStr, 0);
        }
    }

    QString addBlock(const QByteArray& info)
    {
        T data = info;
        Block<T> bNew(++_nIndex, data);
        bNew.sPrevHash = _vChain.back().sHash;
        QString hash = bNew.mineBlock(_nDifficulty);
        _vChain.push_back(bNew);
        return hash;
    }

    QString addBlock(const T& data)
    {
        Block<T> bNew(++_nIndex, data);
        bNew.sPrevHash = _vChain.back().sHash;
        QString hash = bNew.mineBlock(_nDifficulty);
        _vChain.push_back(bNew);
        return hash;
    }

    void save() const
    {
        QString path = QCoreApplication::applicationDirPath() + "/blockchain";
        QFile blockchain(path);

        if (!blockchain.open(QIODevice::WriteOnly)) {
            QMessageBox messageBox;
            messageBox.critical(0,"Error",("Cannot open:\n" + path + "\n"));
//            cerr << "Can not open: " << path << " !" << endl;
            exit(1);
        }

        QTextStream blockStr(&blockchain);

        for(Block<T> block : _vChain) {
            blockStr << block.getIndex() << " ";
//            cerr << block.getIndex() << endl;
            blockStr << block.sPrevHash << " ";
//            cerr << block.sPrevHash.toStdString() << endl;
            blockStr << ((quint64) block.getDatTime()) << " ";
//            cerr << ((quint64) block.getDatTime()) << endl;

//            string data_str = ((QByteArray) block.getData()).data();
//            blockStr << base64_encode(data_str, data_str.length()).c_str() << " ";
            blockStr << ((QByteArray) block.getData()).toBase64() << " ";

            blockStr << block.getNonce() << " ";
//            cerr << block.getNonce() << endl;
            blockStr << block.sHash << "\n";
//            cerr << block.sHash.toStdString() << endl;
        }

        blockchain.close();
    }

    QString equals(const Blockchain<T>& rhs) {
        if (rhs.length() != length()) {
            return "Length inconsistent between nodes!<br>Length of blockchain on this computer: " + QString::number(rhs.length())
                    + "<br>Length of blockchain on connected node: " + QString::number(length());
        }

        for(size_t i = 0; i < rhs.length(); ++i) {
            if (_vChain[i] != rhs._vChain[i]) {
                return "Block inconsistent at index <b>" + QString::number(i) + "</b>!";
            }
        }

        return "Blockchain is up to date with connected node :)";
    }

    T viewAt(unsigned long index) {
        return _vChain[index].getData();
    }

    unsigned long length() const {
        return _vChain.size();
    }

private:
    unsigned int _nDifficulty;
    unsigned long _nIndex;
    vector<Block<T>> _vChain;

    template <typename S>
    bool readFromStream(S& chainStr, bool flag = 1) {
        bool success = true; //doesn't fail silently

        //Block attributes
        unsigned int ind;
        QString prevHash;
        quint64 datTime;
        QByteArray decode64;
        unsigned int nonce;

        //for checking
        QString hash;

        //gets the genesis block:
        if(!(chainStr >> ind >> datTime >> decode64 >> nonce).atEnd()) {
//            T dataIn = (QByteArray(base64_decode(decode64.data()).c_str()));
            T dataIn = (QByteArray::fromBase64(decode64));
            // cout << "decoded: " << base64_decode(decode64) << endl;
            // cout << "line 49: " << dataIn << endl;

            Block<T> block(ind, prevHash, datTime, dataIn, nonce);
            chainStr >> hash;

            if (block.sHash != hash) {
                errors += "Hash inconsistency at genesis block!\n";
//                cerr << ind << endl;
//                cerr << block.getDatTime() << endl;
//                cerr << nonce << endl;
//                cerr << block.sHash.toStdString() << "\n" << hash.toStdString() << endl;
                success = false;
                if (flag) {
                    QMessageBox messageBox;
                    messageBox.critical(0,"Error","Hash inconsistency at genesis block!");
                    exit(1);
                }
            }

            _nIndex = ind;
            _vChain.push_back(block);

            //gets other blocks
            while (!(chainStr >> ind >> prevHash >> datTime >> decode64 >> nonce).atEnd()) {
//                cerr << decode64.toStdString() << endl;
//                T dataIn = (QByteArray(base64_decode(decode64.data()).c_str()));
                T dataIn = (QByteArray::fromBase64(decode64));

                block = Block<T>(ind, prevHash, datTime, dataIn, nonce);
                chainStr >> hash;
                if (block.sHash != hash) {
                    errors += "Hash inconsistency at block " + QString::number(ind) + "!\n";
//                    cerr << "Hash inconsistency at block " << ind << "!" << endl;
//                    cerr << ind << endl;
//                    cerr << block.sPrevHash.toStdString() << endl;
//                                    cerr << block.getDatTime() << endl;
//                                    cerr << nonce << endl;
//                                    cerr << block.sHash.toStdString() << "\n" << hash.toStdString() << endl;
                    success = false;
                    if (flag) {
                        QMessageBox messageBox;
                        messageBox.critical(0,"Error", "Hash inconsistency at block " + QString::number(ind) + "\n");
                        exit(1);
                    }
                }

                _nIndex = ind;
                _vChain.push_back(block);
            }
        }
        else {
            _vChain.emplace_back(Block<T>(0, T(QByteArray("Genesis Block"))));
            return true;
        }
        return success;
    }
};

#endif //TESTCHAIN_BLOCKCHAIN_H
