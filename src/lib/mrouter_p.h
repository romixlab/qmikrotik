#ifndef MROUTERPRIVATE_H
#define MROUTERPRIVATE_H

#include <QObject>
#include <QHostAddress>

#include "mrouter.h"
#include "mcommand.h"

class QTcpSocket;
class MRouterPrivate
{
    Q_DECLARE_PUBLIC(MRouter)
public:
    MRouterPrivate();
    virtual ~MRouterPrivate();

    void _q_state_changed(QAbstractSocket::SocketState state);
    void _q_ready_read();
    void _q_error(QAbstractSocket::SocketError error);

    QByteArray pack_length(quint32 length);
    quint8 * unpack_length(quint32 &to, quint8 *p, quint32 available, bool *success);

    void write_word(const QString &word);
    void write_sentence(const QStringList &sentence);
    void process_incoming_sentence(const QStringList &sentence);
    void send_command();

    QHostAddress address;
    quint16 port;
    QString username;
    QString password;
    QTcpSocket *tcpSocket;

    QByteArray bytes;
    quint32 pos;
    QStringList sentence;

    QByteArray t;

    MRouter * q_ptr;
    MRouter::STATE state;

    QList<MCommand> commands;
};

#endif // MROUTERPRIVATE_H
