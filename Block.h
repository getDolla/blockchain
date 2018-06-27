//
// Created by Dave Nash on 20/10/2017.
// modified by Yeetkai on 25/6/2018
//

#ifndef TESTCHAIN_BLOCK_H
#define TESTCHAIN_BLOCK_H

#include <iostream>
#include <sstream>
#include <time.h>
#include "sha256.h"

using namespace std;

template <typename T>
class Block {
public:
    string sHash;
    string sPrevHash;

    Block(unsigned int nIndexIn, const T& dataIn) : _nIndex(nIndexIn), _data(dataIn), _nNonce(0), _tTime(time(nullptr))
    {
        sHash = _calculateHash();
    }

    Block(unsigned int ind, const string& prevHash, const time_t& datTime, const T& dataIn, unsigned int nonce) :
    _nIndex(ind), sPrevHash(prevHash), _data(dataIn), _nNonce(nonce), _tTime(datTime)
    {
        sHash = _calculateHash();
    }

    string mineBlock(unsigned int nDifficulty)
    {
        char cstr[nDifficulty + 1];
        for (size_t i = 0; i < nDifficulty; ++i)
        {
            cstr[i] = '0';
        }
        cstr[nDifficulty] = '\0';

        string str(cstr);

        while (sHash.substr(0, nDifficulty) != str)
        {
            ++_nNonce;
            sHash = _calculateHash();
        }

        return sHash;
    }

    string _calculateHash() const
    {
        stringstream ss;
        ss << _nIndex << sPrevHash << _tTime << _data << _nNonce;

        return sha256(ss.str());
    }


    T getData() const
    {
        return _data;
    }

    unsigned int getIndex() const
    {
        return _nIndex;
    }

    unsigned int getNonce() const
    {
        return _nNonce;
    }

    time_t getDatTime() const
    {
        return _tTime;
    }

private:
    unsigned int _nIndex;
    unsigned int _nNonce;
    T _data;
    time_t _tTime;
};

#endif //TESTCHAIN_BLOCK_H
