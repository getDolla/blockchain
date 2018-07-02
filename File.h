#ifndef FILE_H
#define FILE_H

#include <QString>
#include <QByteArray>
#include <QDataStream>

//#include <iostream>
//using namespace std;

class File {
    friend bool operator ==(const File& lhs, const File& rhs) {
        return (lhs.filename == rhs.filename) && (lhs.data == rhs.data);
    }

private:
    QString filename;
    QByteArray data;

public:
    operator QByteArray() const {
        QByteArray temp;
        temp.append("NAME: ");
        temp.append(filename.toUtf8());
        temp.append(" DATA: ");
        return temp += data;
    }

    File& operator=(const QByteArray& rhs) {
        int foundName = rhs.indexOf("NAME: ");
        int foundData = rhs.indexOf(" DATA: ");
        if ((foundName == 0) && (foundData - 6 > 0)) {
            for(size_t i = 6; i < foundData; ++i) {
                filename += rhs[i];
            }
        }
        if (foundData >= 0) {
            for(size_t i = foundData + 7; i < rhs.length(); ++i) {
                data += rhs[i];
            }
        }
        else {
            data = rhs;
        }

//         cout << foundName << endl;
//         cout << foundData << endl;
//         cerr << "Filename: " << filename.toStdString() << endl;
//         cerr << "Data: " << data.toStdString() << endl;

        return *this;
    }

    File() {}
    File(const QByteArray& content) {
        operator=(content);
    }
    File(const QString& name, const QByteArray& content): filename(name), data(content) {}

    QString getFileName() const { return filename; }
    QByteArray getData() const { return data; }
};
#endif
