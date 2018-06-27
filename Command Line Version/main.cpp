#include "Blockchain.h"
#include "File.h"

int main()
{
    Blockchain<File> bChain;

    cout << "Blockchain data is stored in base64 format." << endl;
    cout << "Enter help for available commands." << endl;
    cout << "Enter exit to exit the program." << endl;

    string input;
    getline(cin, input);

    while(input != "exit") {
        if (input == "store") {
            cout << "Enter data (or filename):" << endl;
            getline(cin, input);

            ifstream ifs(input, ios::binary|ios::ate);

            if (ifs) {
                cout << "File " << input << " found!" << endl;
                auto fileSize = ifs.tellg();
                ifs.seekg(ios::beg);
                string content(fileSize,0);
                ifs.read(&content[0],fileSize);
                cout << "Saving " << input << " to block..." << endl;
                bChain.addBlock(File(input, content));
            }
            else {
                cout << "Storing " << input << " to block..." << endl;
                bChain.addBlock(input);
            }
            ifs.close();
        }
        else if (input == "view") {
            cout << "Enter index:" << endl;
            unsigned long index;
            cin >> index;
            cout << endl;
            cout << bChain.viewAt(index) << endl;
            cin.ignore();
        }
        else if (input == "save") {
            cout << "Enter index:" << endl;
            unsigned long index;
            cin >> index;

            File file = bChain.viewAt(index);
            string filename = file.getFileName();

            if (file.getFileName() == "") {
                filename = to_string(index) + ".txt";
            }

            ofstream ofs(filename);
            ofs << file.getData();
            cout << "Saving data to " << filename << " ..." << endl;
            ofs.close();

            cout << "Data saved." << endl;
            cin.ignore();
        }
        else if (input == "length") {
            cout << "There are " << bChain.length() - 1 << " blocks." << endl;
        }
        else if (input == "help") {
            cout << "Enter exit to exit the program." << endl;
            cout << "Enter store to store data." << endl;
            cout << "Enter view to view data at a specific index." << endl;
            cout << "Enter save to store the data in hard drive (non-files will be stored as a .txt file)." << endl;
            cout << "Enter length to see how many blocks of data exists." << endl;
        }

        cout << endl;
        getline(cin, input);
    }

    bChain.save();

    return 0;
}
