//
// Created by Dave Nash on 20/10/2017.
// modified by Yeetkai on 25/6/2018
//

#ifndef TESTCHAIN_BLOCKCHAIN_H
#define TESTCHAIN_BLOCKCHAIN_H

#include <QCoreApplication>
#include <QString>
#include <QMessageBox>

#include <vector>
#include <iostream>
#include <fstream>
#include "Block.h"
#include "base64.h"

using namespace std;

template <typename T>
class Blockchain {
public:
    Blockchain(): _nDifficulty(1), _nIndex(0)
    {
        string path = QCoreApplication::applicationDirPath().toStdString() + "/blockchain";
//        cout << path << endl;

        ifstream blockchain(path);

    	if (!blockchain) {
            QMessageBox messageBox;
            messageBox.critical(0,"Error",QString::fromStdString("Cannot open:\n" + path + "\n"));
            cerr << "Can not open: " << path << " !" << endl;
            exit(1);
        }

    	// string line;
    	// while(getline(blockchain, line)) {
    	// 	cout << line << endl;
    	// }

        //Block attributes
        unsigned int ind;
        string prevHash;
        time_t datTime;
        T dataIn;
        string decode64;
        unsigned int nonce;

        //for checking
        string hash;

        //gets the genesis block:
        if (blockchain >> ind >> datTime >> decode64 >> nonce) {
            dataIn = base64_decode(decode64);
            // cout << "decoded: " << base64_decode(decode64) << endl;
            // cout << "line 49: " << dataIn << endl;

            Block<T> block(ind, "", datTime, dataIn, nonce);
            blockchain >> hash;

            if (block.sHash != hash) {
                QMessageBox messageBox;
                messageBox.critical(0,"Error","Hash inconsistency at genesis block!");
                cerr << "Hash inconsistency at genesis block!" << endl;
                exit(1);
            }

            _nIndex = ind;
            _vChain.push_back(block);

            //gets other blocks
            while (blockchain >> ind >> prevHash >> datTime >> decode64 >> nonce) {
                dataIn = base64_decode(decode64);

                block = Block<T>(ind, prevHash, datTime, dataIn, nonce);
                blockchain >> hash;
                if (block.sHash != hash) {
                    QMessageBox messageBox;
                    messageBox.critical(0,"Error", "Hash inconsistency at block " + QString::number(ind) + "\n");
                    cerr << "Hash inconsistency at block " << ind << "!" << endl;
                    exit(1);
                }

                _nIndex = ind;
                _vChain.push_back(block);
            }
        }
        else {
            _vChain.emplace_back(Block<T>(0, T("Genesis Block")));
        }

        blockchain.close();
    }

    string addBlock(const string& info)
    {
        T data = info;
        Block<T> bNew(++_nIndex, data);
        bNew.sPrevHash = _getLastBlock().sHash;
        string hash = bNew.mineBlock(_nDifficulty);
        _vChain.push_back(bNew);
        return hash;
    }

    string addBlock(const T& data)
    {
        Block<T> bNew(++_nIndex, data);
        bNew.sPrevHash = _getLastBlock().sHash;
        string hash = bNew.mineBlock(_nDifficulty);
        _vChain.push_back(bNew);
        return hash;
    }

    void save() const
    {
        string path = QCoreApplication::applicationDirPath().toStdString() + "/blockchain";
        ofstream blockchain(path);

        if (!blockchain) {
            QMessageBox messageBox;
            messageBox.critical(0,"Error",QString::fromStdString("Cannot open:\n" + path + "\n"));
            cerr << "Can not open: " << path << " !" << endl;
            exit(1);
        }

        for(Block<T> block : _vChain) {
            blockchain << block.getIndex() << " ";
            blockchain << block.sPrevHash << " ";
            blockchain << block.getDatTime() << " ";

            string encode64 = (string) block.getData();
            encode64 = base64_encode(encode64, encode64.length());

            blockchain << encode64 << " ";
            blockchain << block.getNonce() << " ";
            blockchain << block.sHash << "\n";
        }

        blockchain.close();
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

    Block<T> _getLastBlock() const
    {
        return _vChain.back();
    }
};

#endif //TESTCHAIN_BLOCKCHAIN_H
