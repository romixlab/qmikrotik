#ifndef ROUTERPRIVATE_H
#define ROUTERPRIVATE_H

#include "router.h"
#include <QObject>
#include <QHostAddress>

class QTcpSocket;

namespace qmikrotik {

class RouterPrivate
{
    Q_DECLARE_PUBLIC(Router)
public:
    RouterPrivate();
    virtual ~RouterPrivate();

    void _q_state_changed(QAbstractSocket::SocketState state);
    void _q_ready_read();
    void _q_error(QAbstractSocket::SocketError error);

    QByteArray pack_length(quint32 length);
    quint8 * unpack_length(quint32 &to, quint8 *p, quint8 available, bool *success);
    void write_word(const QString &word);

    void test();

    QHostAddress address;
    quint16 port;
    QString username;
    QString password;
    QTcpSocket *tcpSocket;

    QByteArray bytes;
    quint32 pos;
    QStringList sentence;

    QByteArray t;

    Router * q_ptr;
};

} // qmikrotik
#endif // ROUTERPRIVATE_H
