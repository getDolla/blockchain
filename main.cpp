#include "Blockchain.h"

int main()
{
    Blockchain<string> bChain;

    cout << "Blockchain data is stored in base64 format." << endl;
    cout << "Enter help for available commands." << endl;
    cout << "Enter exit to exit the program." << endl;

    string input;
    getline(cin, input);

    while(input != "exit") {
        if (input == "store") {
            cout << "Enter data:" << endl;
            getline(cin, input);
            cout << "Storing " << input << " to block..." << endl;
            bChain.addBlock(input);
        }
        else if (input == "view") {
            cout << "Enter index:" << endl;
            unsigned long index;
            cin >> index;
            cout << bChain.viewAt(index) << endl;
            cin.ignore();
        }
        else if (input == "length") {
            cout << "There are " << bChain.length() - 1 << " blocks." << endl;
        }
        else if (input == "help") {
            cout << "Enter exit to exit the program." << endl;
            cout << "Enter store to store data." << endl;
            cout << "Enter view to view data at a specific index (files on the blockchain will be saved)" << endl;
            cout << "Enter length to see how many blocks of data exists." << endl;
        }
        getline(cin, input);
    }

    bChain.save();

    return 0;
}
