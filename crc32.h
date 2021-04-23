#ifndef CRC32_H
#define CRC32_H

#include <QObject>
#include <QByteArray>

class CRC32 : public QObject
{
public:
    static quint32 encode(QByteArray& bytes);

    CRC32();
};

#endif // CRC32_H
