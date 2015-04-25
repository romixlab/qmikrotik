#include "router.h"
#include "router_p.h"
#include "sysdep.h"
#include <QTcpSocket>
#include <QTimer>

using namespace qmikrotik;

RouterPrivate::RouterPrivate() : pos(0) {}
RouterPrivate::~RouterPrivate() {}

void RouterPrivate::_q_state_changed(QAbstractSocket::SocketState state)
{
    qDebug() << "new state:" << state;
    if (state == QTcpSocket::ConnectedState) {
        write_word("/login");
        write_word("");
    }
}

void RouterPrivate::_q_ready_read()
{
    //bytes += t;
    bytes += tcpSocket->readAll();

    quint8 *p = (quint8 *)bytes.data() + pos;
    quint8 *start = p;
    quint32 len;
    bool ok;
    qDebug() << "bytes size" << bytes.length();
    qDebug() << "pos" << pos;
    while (p - start < bytes.length()) {
        quint8 *pmoved = unpack_length(len, p, bytes.length() - (p - start), &ok);
        if (!ok) {
            qDebug() << "waiting for next chunk";
            pos = p - start;
            return;
        }
        qDebug() << "want" << len;
        if (p + len >= p + bytes.length()) {
            qDebug() << "waiting(2) for next chunk";
            pos = p - start;
            return;
        }
        p = pmoved;
        sentence << QString::fromUtf8((const char *)p, len);
        p += len;
    }
    qDebug() << sentence;

    sentence.clear();
    bytes.clear();
    pos = 0;


    tcpSocket->disconnect();
}

void RouterPrivate::_q_error(QAbstractSocket::SocketError error)
{
    qDebug() << error;
}

QByteArray RouterPrivate::pack_length(quint32 length)
{
    QByteArray packed;
    quint8 *p = (quint8 *)packed.data();
    if (length < 0x80) {
        packed.resize(1);
        p[0] = (quint8)length;
    } else if (length < 0x4000) {
        packed.resize(2);
        _msgpack_store16(p, (quint16)length);
        p[0] |= 0x80;
    } else if (length < 0x200000) {
        packed.resize(3);
        quint32 val = _msgpack_be32(length);
        memcpy(p, &val, 3);
        p[0] |= 0xc0;
    } else if (length < 0x10000000) {
        packed.resize(4);
        _msgpack_store32(p, length);
        p[0] |= 0xe0;
    } else {
        qWarning() << "Word is too big";
    }

    return packed;
}

quint8 *RouterPrivate::unpack_length(quint32 &to, quint8 *p, quint8 available, bool *success)
{
    if (available == 0) {
        *success = false;
        return p;
    }
    *success = true;
    if ((p[0] & 0xe0) == 0xe0) {
        if (available < 4) {
            *success = false;
            return p;
        }
        quint32 val = _msgpack_load32(quint32, p);
        val &= 0x1fffffff;
        to = val;
        return p + 4;
    } else if ((p[0] & 0xc0) == 0xc0) {
        if (available < 3) {
            *success = false;
            return p;
        }
        quint32 val = (((quint32)p[0]) << 24) |
                      (((quint32)p[1]) << 16) |
                      (((quint32)p[2]) <<  8);
        val &= 0x3fffff;
        to = val;
        return p + 3;
    } else if ((p[0] & 0x80) == 0x80) {
        if (available < 2) {
            *success = false;
            return p;
        }
        quint16 val = _msgpack_load16(quint16, p);
        val &= 0x7fff;
        to = val;
        return p + 2;
    } else {
        to = p[0];
        return p + 1;
    }
    qWarning() << "Failed to unpack word length";
    return p;
}

void RouterPrivate::write_word(const QString &word)
{
    qDebug() << tcpSocket->write(pack_length(word.length()));
    qDebug() << tcpSocket->write(word.toUtf8().constData(), word.length());
}

void RouterPrivate::test()
{
    QByteArray len = pack_length(3);

    t = len;
    _q_ready_read();
    t = QByteArray("ab", 2);
    _q_ready_read();
    t = QByteArray("c", 1);
    _q_ready_read();
}

Router::Router(QObject *parent) : QObject(parent), d_ptr(new RouterPrivate)
{
    Q_D(Router);
    d->q_ptr = this;
    d->tcpSocket = new QTcpSocket(this);
    connect(d->tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(_q_state_changed(QAbstractSocket::SocketState)));
    connect(d->tcpSocket, SIGNAL(readyRead()),
            this, SLOT(_q_ready_read()));
    connect(d->tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(_q_error(QAbstractSocket::SocketError)));
}

Router::Router(RouterPrivate &dd, QObject *parent) : QObject(parent), d_ptr(&dd)
{
    Q_D(Router);
    d->q_ptr = this;
}

Router::~Router()
{
    Q_D(Router);
    if (d) {
        delete d;
    }
}

void Router::setAddress(const QHostAddress &address)
{
    Q_D(Router);
    d->address = address;
}

QHostAddress Router::address() const
{
    Q_D(const Router);
    return d->address;
}

void Router::setPort(quint16 port)
{
    Q_D(Router);
    d->port = port;
}

quint16 Router::port() const
{
    Q_D(const Router);
    return d->port;
}

void Router::setUsername(const QString &username)
{
    Q_D(Router);
    d->username = username;
}

QString Router::username() const
{
    Q_D(const Router);
    return d->username;
}

void Router::setPassword(const QString &password)
{
    Q_D(Router);
    d->password = password;
}

QString Router::password() const
{
    Q_D(const Router);
    return d->password;
}

void Router::login()
{
    Q_D(Router);
    d->tcpSocket->connectToHost(d->address, d->port);
    //d->test();
}

void Router::logout()
{

}

#include "moc_router.cpp"
