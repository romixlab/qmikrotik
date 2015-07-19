#include <QTcpSocket>
#include <QCryptographicHash>
#include <QTimer>

#include "mrouter.h"
#include "mrouter_p.h"
#include "mcommand_p.h"
#include "endianhelper.h"


MRouterPrivate::MRouterPrivate() :
    port(8728),
    pos(0),
    state(MRouter::DISCONNECTED)
{ }

MRouterPrivate::~MRouterPrivate() { }

void MRouterPrivate::_q_state_changed(QAbstractSocket::SocketState state)
{
    if (state == QTcpSocket::ConnectedState) {
        write_sentence(QStringList() << "/login");
        this->state = MRouter::LOGGING_IN;
        Q_Q(MRouter);
        emit q->stateChanged(this->state);
    }
}

void MRouterPrivate::_q_ready_read()
{
    bytes += tcpSocket->readAll();

    quint8 *p = (quint8 *)bytes.data() + pos;
    quint8 *start = p;
    quint32 len;
    bool ok;
    while (p - start < bytes.length()) {
        quint8 *pmoved = unpack_length(len, p, bytes.length() - (p - start), &ok);
        if (!ok) {
            pos = p - start;
            return;
        }
        if (p + len >= p + bytes.length()) {
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

void MRouterPrivate::_q_error(QAbstractSocket::SocketError error)
{
    Q_Q(MRouter);
    Q_UNUSED(error);
    state = MRouter::DISCONNECTED;
    emit q->stateChanged(state);
}

QByteArray MRouterPrivate::pack_length(quint32 length)
{
    QByteArray packed;
    quint8 *p = (quint8 *)packed.data();
    if (length < 0x80) {
        packed.resize(1);
        p[0] = (quint8)length;
    } else if (length < 0x4000) {
        packed.resize(2);
        endian_store16(p, (quint16)length);
        p[0] |= 0x80;
    } else if (length < 0x200000) {
        packed.resize(3);
        quint32 val = endian_be32(length);
        memcpy(p, &val, 3);
        p[0] |= 0xc0;
    } else if (length < 0x10000000) {
        packed.resize(4);
        endian_store32(p, length);
        p[0] |= 0xe0;
    } else {
        qWarning() << "Word is too big";
    }

    return packed;
}

quint8 *MRouterPrivate::unpack_length(quint32 &to, quint8 *p, quint32 available, bool *success)
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
        quint32 val = endian_load32(quint32, p);
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
        quint16 val = endian_load16(quint16, p);
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

void MRouterPrivate::write_word(const QString &word)
{
    tcpSocket->write(pack_length(word.length()));
    tcpSocket->write(word.toUtf8().constData(), word.length());
}

void MRouterPrivate::write_sentence(const QStringList &sentence)
{
    foreach (const QString &word, sentence)
        write_word(word);
    write_word("");
}

void MRouterPrivate::process_incoming_sentence(const QStringList &sentence)
{
    Q_Q(MRouter);
    if (state == MRouter::EXECUTING) {
        MCommand cmd = commands.front();
        if (cmd.d->lambda)
            cmd.d->lambda(sentence);
        if (cmd.d->lambda_id)
            cmd.d->lambda_id(cmd.d->id, sentence);
        commands.removeFirst();
        send_command();
    } else if (state == MRouter::LOGGING_IN) {
        if (sentence.length() == 2 && sentence[0] == "!done") {
            state = MRouter::READY;
            emit q->stateChanged(state);
            send_command();
            return;
        } else if ((sentence.length() != 3) ||
                   (sentence.length() == 3 && sentence[0] != "!done")) {
            tcpSocket->close();
            state = MRouter::DISCONNECTED;
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

void MRouterPrivate::send_command()
{
    Q_Q(MRouter);
    if (!commands.isEmpty()) {
        if (state != MRouter::EXECUTING) {
            state = MRouter::EXECUTING;
            emit q->stateChanged(state);
        }
        MCommand cmd = commands.front();
        if (cmd.d->command.isEmpty()) {
            commands.removeFirst();
            send_command();
        }
        write_sentence(cmd.d->command);
    } else {
        if (state == MRouter::READY)
            return;
        state = MRouter::READY;
        emit q->stateChanged(state);
    }
}

MRouter::MRouter(QObject *parent) : QObject(parent), d_ptr(new MRouterPrivate)
{
    Q_D(MRouter);
    d->q_ptr = this;
    d->tcpSocket = new QTcpSocket(this);
    connect(d->tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(_q_state_changed(QAbstractSocket::SocketState)));
    connect(d->tcpSocket, SIGNAL(readyRead()),
            this, SLOT(_q_ready_read()));
    connect(d->tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(_q_error(QAbstractSocket::SocketError)));
}

MRouter::MRouter(MRouterPrivate &dd, QObject *parent) : QObject(parent), d_ptr(&dd)
{
    Q_D(MRouter);
    d->q_ptr = this;
}

MRouter::~MRouter()
{
    Q_D(MRouter);
    if (d) {
        delete d;
    }
}

void MRouter::setAddress(const QHostAddress &address)
{
    Q_D(MRouter);
    d->address = address;
}

QHostAddress MRouter::address() const
{
    Q_D(const MRouter);
    return d->address;
}

void MRouter::setAddressString(const QString &address)
{
    Q_D(MRouter);
    d->address = QHostAddress(address);
}

QString MRouter::addressString() const
{
    Q_D(const MRouter);
    return d->address.toString();
}

void MRouter::setPort(quint16 port)
{
    Q_D(MRouter);
    d->port = port;
}

quint16 MRouter::port() const
{
    Q_D(const MRouter);
    return d->port;
}

void MRouter::setUsername(const QString &username)
{
    Q_D(MRouter);
    d->username = username;
}

QString MRouter::username() const
{
    Q_D(const MRouter);
    return d->username;
}

void MRouter::setPassword(const QString &password)
{
    Q_D(MRouter);
    d->password = password;
}

QString MRouter::password() const
{
    Q_D(const MRouter);
    return d->password;
}

MRouter::STATE MRouter::state() const
{
    Q_D(const MRouter);
    return d->state;
}

QString MRouter::stateString() const
{
    Q_D(const MRouter);
    switch (d->state) {
    case DISCONNECTED:
        return QStringLiteral("Disconnected");
    case CONNECTING:
        return QStringLiteral("Connecting");
    case LOGGING_IN:
        return QStringLiteral("Logging in");
    case EXECUTING:
        return QStringLiteral("Executing");
    case READY:
        return QStringLiteral("Ready");
    }
    return QStringLiteral("");
}

MCommand MRouter::command(const QStringList &command)
{
    Q_D(MRouter);
    MCommand cmd(command);
    d->commands.append(cmd);
    return cmd;
}

void MRouter::login()
{
    Q_D(MRouter);
    d->tcpSocket->connectToHost(d->address, d->port);
}

void MRouter::logout()
{
    Q_D(MRouter);
    d->tcpSocket->close();
    d->state = DISCONNECTED;
    emit stateChanged(d->state);
}

#include "moc_mrouter.cpp" // intentionally, for private slots to work
