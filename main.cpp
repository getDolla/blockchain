#include "Blockchain.h"

int main()
{
    Blockchain<string> bChain;

    cout << "Blockchain data is stored in base64 format." << endl;
    cout << "Enter help for available commands." << endl;
    cout << "Enter exit to exit the program." << endl;

    string input;



    cout << "Mining block 1..." << endl;
    bChain.addBlock("Block 1 Data");

    bChain.save();

    // cout << "Mining block 2..." << endl;
    // bChain.addBlock(Block<string>(2, "Block 2 Data"));
    //
    // cout << "Mining block 3..." << endl;
    // bChain.addBlock(Block<string>(3, "Block 3 Data"));



    return 0;
}
