/*
    TRANSFERMODE:
    0 - ALL
    1 - PDF
    2 - JPEG
    3 - DOC
    4 - ACCESS
    5 - EXCEL
    6 - TXT
    7 - 质控
    8 - 生产
    9 - 库存
*/

#ifndef FILE_H
#define FILE_H

#define TRANSFERMODE 1

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
            filename += rhs.mid(6, foundData - 6);
        }
        if (foundData >= 0) {
            data += rhs.right(rhs.length() - (foundData + 7));
        }
        else {
            data = rhs;
        }

//         cout << foundName << endl;
//         cout << foundData << endl;
//         cerr << "Filename: " << filename.toStdString() << endl;
//         cerr << "Data:" << data.toStdString() << endl;

        return *this;
    }

    File() {}
    File(const QByteArray& content) {
//        cout << content.toStdString() << endl;
        operator=(content);
    }
    File(const QString& name, const QByteArray& content): filename(name), data(content) {}

    QString getFileName() const { return filename; }
    QByteArray getData() const { return data; }
};
#endif
