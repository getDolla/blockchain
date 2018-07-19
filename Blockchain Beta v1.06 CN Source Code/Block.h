//
// Created by Dave Nash on 20/10/2017.
// modified by Yeetkai on 25/6/2018
//

#ifndef TESTCHAIN_BLOCK_H
#define TESTCHAIN_BLOCK_H

#include <QDataStream>
#include <QString>
#include <QByteArray>
#include <QCryptographicHash>

#include <ctime>
#include <iostream>
using namespace std;

template <typename T>
class Block {
    friend bool operator ==(const Block<T>& lhs, const Block<T>& rhs) {
//        cerr << "lhs.sHash: " << lhs.sHash.toStdString() << endl;
//        cerr << "rhs.sHash: " << rhs.sHash.toStdString() << endl;
//        cerr << "(lhs.sPrevHash == rhs.sPrevHash): " << (lhs.sPrevHash == rhs.sPrevHash) << endl;
//        cerr << "(lhs._nIndex == rhs._nIndex): " << (lhs._nIndex == rhs._nIndex) << endl;
//        cerr << "(lhs._nNonce == rhs._nNonce): " << (lhs._nNonce == rhs._nNonce) << endl;
//        cerr << "lhs._tTime: " << lhs._tTime << endl;
//        cerr << "rhs._tTime: " << rhs._tTime << endl;
//        cerr << "(lhs._data == rhs._data): " << (lhs._data == rhs._data) << endl;
        return (lhs.sHash == rhs.sHash) && (lhs.sPrevHash == rhs.sPrevHash) && (lhs._nIndex == rhs._nIndex)
                && (lhs._nNonce == rhs._nNonce) && (lhs._tTime == rhs._tTime) && (lhs._data == rhs._data);
    }

public:
    QString sHash;
    QString sPrevHash;

    Block(quint64 nIndexIn, const T& dataIn) : _nIndex(nIndexIn), _data(dataIn), _nNonce(0), _tTime(time(0))
    {
        sHash = _calculateHash();
    }

    Block(quint64 ind, const QString& prevHash, const quint64& datTime, const T& dataIn, quint64 nonce) :
    _nIndex(ind), sPrevHash(prevHash), _data(dataIn), _nNonce(nonce), _tTime(datTime)
    {
        sHash = _calculateHash();
    }

    QString mineBlock(quint64 nDifficulty)
    {
        char cstr[nDifficulty + 1];
        for (size_t i = 0; i < nDifficulty; ++i)
        {
            cstr[i] = '0';
        }
        cstr[nDifficulty] = '\0';

        QString str(cstr);

        while (sHash.left(nDifficulty) != str)
        {
            ++_nNonce;
            sHash = _calculateHash();
        }

        return sHash;
    }

    QString _calculateHash() const
    {
        QByteArray toHash;
        QDataStream bytestr(&toHash, QIODevice::WriteOnly);
        bytestr << _nIndex << sPrevHash << QString::fromStdString(to_string(_tTime)) << _data << _nNonce;

        return QString(QCryptographicHash::hash(toHash, QCryptographicHash::Sha3_512).toHex());
    }


    T getData() const
    {
        return _data;
    }

    quint64 getIndex() const
    {
        return _nIndex;
    }

    quint64 getNonce() const
    {
        return _nNonce;
    }

    time_t getDatTime() const
    {
        return _tTime;
    }

private:
    quint64 _nIndex;
    quint64 _nNonce;
    T _data;
    time_t _tTime;
};

template <typename T>
bool operator !=(const Block<T>& lhs, const Block<T>& rhs) {
    return !(lhs == rhs);
}

#endif //TESTCHAIN_BLOCK_H
