#ifndef FILE_H
#define FILE_H

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

class File {
    friend bool operator ==(const File& lhs, const File& rhs) {
        return (lhs.filename == rhs.filename) && (lhs.data == rhs.data);
    }

private:
    string filename;
    string data;

public:
    friend ostream& operator<<(ostream& os, const File& rhs) {
        os << rhs.filename << ":\n";
        os << rhs.data << endl;
        return os;
    }

    operator string() const {
        return "NAME: " + filename + " DATA: " + data;
    }

    File& operator=(const string& rhs) {
        size_t foundName = rhs.find("NAME: ");
        size_t foundData = rhs.find(" DATA: ");
        if ((foundName != string::npos) && (foundName == 0) && (foundData - 6 > 0)) {
            filename = rhs.substr(6, foundData - 6);
        }
        if (foundData != string::npos) {
            data = rhs.substr(foundData + 7);
        }
        else {
            data = rhs;
        }

        // cout << foundName << endl;
        // cout << foundData << endl;
        // cout << filename << endl;
        // cout << data << endl;

        return *this;
    }

    File() {}
    File(const string& content): data(content) {}
    File(const string& name, const string& content): filename(name), data(content) {}

    string getFileName() const { return filename; }
    string getData() const { return data; }
};
#endif
