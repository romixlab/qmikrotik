#include "router.h"
#include "router_p.h"
#include "sysdep.h"
#include <QTcpSocket>
#include <QCryptographicHash>
#include <QTimer>

using namespace qmikrotik;

RouterPrivate::RouterPrivate() : pos(0), state(Router::DISCONNECTED) {}
RouterPrivate::~RouterPrivate() {}

void RouterPrivate::_q_state_changed(QAbstractSocket::SocketState state)
{
    qDebug() << "new state:" << state;
    if (state == QTcpSocket::ConnectedState) {
        write_sentence(QStringList() << "/login");
        this->state = Router::LOGGING_IN;
        Q_Q(Router);
        emit q->stateChanged(this->state);
    }
}

void RouterPrivate::_q_ready_read()
{
    bytes += tcpSocket->readAll();

    quint8 *p = (quint8 *)bytes.data() + pos;
    quint8 *start = p;
    quint32 len;
    bool ok;
//    qDebug() << "bytes size" << bytes.length();
//    qDebug() << "pos" << pos;
    while (p - start < bytes.length()) {
        quint8 *pmoved = unpack_length(len, p, bytes.length() - (p - start), &ok);
        if (!ok) {
 //           qDebug() << "waiting for next chunk";
            pos = p - start;
            return;
        }
 //       qDebug() << "want" << len;
        if (p + len >= p + bytes.length()) {
 //           qDebug() << "waiting(2) for next chunk";
            pos = p - start;
            return;
        }
        p = pmoved;
        sentence << QString::fromUtf8((const char *)p, len);
        p += len;
    }
    process_incoming_sentence(sentence);

    sentence.clear();
    bytes.clear();
    pos = 0;
}

void RouterPrivate::_q_error(QAbstractSocket::SocketError error)
{
    qDebug() << error;
    Q_Q(Router);
    state = Router::DISCONNECTED;
    emit q->stateChanged(state);
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

quint8 *RouterPrivate::unpack_length(quint32 &to, quint8 *p, quint32 available, bool *success)
{
    if (available == 0) {
        qDebug() << "s1";
        *success = false;
        return p;
    }
    *success = true;
    if ((p[0] & 0xe0) == 0xe0) {
        if (available < 4) {
            qDebug() << "s2";
            *success = false;
            return p;
        }
        quint32 val = _msgpack_load32(quint32, p);
        val &= 0x1fffffff;
        to = val;
        return p + 4;
    } else if ((p[0] & 0xc0) == 0xc0) {
        if (available < 3) {
            qDebug() << "s3";
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
            qDebug() << "s4";
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
    tcpSocket->write(pack_length(word.length()));
    tcpSocket->write(word.toUtf8().constData(), word.length());
}

void RouterPrivate::write_sentence(const QStringList &sentence)
{
    foreach (const QString &word, sentence)
        write_word(word);
    write_word("");
}

void RouterPrivate::process_incoming_sentence(const QStringList &sentence)
{
    qDebug() << "processing" << sentence;
    Q_Q(Router);
    if (state == Router::EXECUTING) {
        emit q->reply(sentence);
        if (!requestList.isEmpty()) {
            write_sentence(requestList.first());
            requestList.pop_front();
        } else {
            state = Router::READY;
            emit q->stateChanged(state);
        }
    } else if (state == Router::LOGGING_IN) {
        if (sentence.length() == 2 && sentence[0] == "!done") {
            state = Router::READY;
            emit q->stateChanged(state);
            return;
        } else if ((sentence.length() != 3) ||
                   (sentence.length() == 3 && sentence[0] != "!done")) {
            tcpSocket->close();
            state = Router::DISCONNECTED;
            emit q->stateChanged(state);
            return;
        }
        QCryptographicHash md5(QCryptographicHash::Md5);
        md5.addData("", 1);
        md5.addData(QByteArray(password.toLocal8Bit()));
        QString challenge = sentence[1].split("=")[2];
        md5.addData(QByteArray::fromHex(challenge.toLocal8Bit()));
        QString digest = md5.result().toHex();
        write_sentence(QStringList() << "/login"
                       << "=name=" + username
                       << "=response=00" + digest);
    } else {
        qWarning() << "Recieved data, that not expected" << sentence;
    }
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
    Q_D(Router);
    d->tcpSocket->close();
    d->state = DISCONNECTED;
    emit stateChanged(d->state);
}

void Router::request(const QStringList &sentence)
{
    Q_D(Router);
    if (d->state == EXECUTING) {
        d->requestList.append(sentence);
        return;
    }
    d->state = EXECUTING;
    emit stateChanged(d->state);
    d->write_sentence(sentence);
}

#include "moc_router.cpp"
