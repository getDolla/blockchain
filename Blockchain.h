//
// Created by Dave Nash on 20/10/2017.
// modified by Yeetkai on 25/6/2018
//

#ifndef TESTCHAIN_BLOCKCHAIN_H
#define TESTCHAIN_BLOCKCHAIN_H

#include <vector>
#include <iostream>
#include <fstream>
#include "Block.h"

using namespace std;

template <typename T>
class Blockchain {
public:
    Blockchain(): _nDifficulty(6)
    {
        _vChain.emplace_back(Block<T>(0, "Genesis Block"));

        ifstream blockchain("blockchain");

    	if (!blockchain) {
            cerr << "Can not open file!" << endl;
            exit(1);
        }



        //gets the genesis block:
        if (blockchain >> ) {
            blockData.push_back();
            for(size_t i = 1; i << 4; ++i) {

            }

            Block block = Block<T>(blockData[0], block)
        }
        while(blockchain >> ) {

            for(size_t i = 1; i < 5; ++i) {

            }
        }
    }

    void addBlock(Block<T> bNew)
    {
        bNew.sPrevHash = _getLastBlock().sHash;
        bNew.mineBlock(_nDifficulty);
        _vChain.push_back(bNew);
    }

    void save() const
    {
        ofstream blockchain("blockchain");

        if (!blockchain) {
            cerr << "Can not open file!" << endl;
            exit(1);
        }

        for(Block<T> block : _vChain) {
            blockchain << block.getIndex() << " ";
            blockchain << block.sPrevHash << " ";
            blockchain << block.getDatTime() << " ";
            blockchain << block.getData() << " ";
            blockchain << block.getNonce() << " ";
            blockchain << block.sHash << "\n";
        }

        blockchain.close();
    }

private:
    unsigned int _nDifficulty;
    vector<Block<T>> _vChain;

    Block<T> _getLastBlock() const
    {
        return _vChain.back();
    }
};

#endif //TESTCHAIN_BLOCKCHAIN_H
