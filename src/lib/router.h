#ifndef ROUTER_H
#define ROUTER_H

#include "mikrotik_global.h"
#include <QObject>
#include <QHostAddress>

namespace qmikrotik {

class RouterPrivate;

class MIKROTIKSHARED_EXPORT Router : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString address READ addressString WRITE setAddressString)
    Q_PROPERTY(quint16 port READ port WRITE setPort)
    Q_PROPERTY(QString username READ username WRITE setUsername)
    Q_PROPERTY(QString password READ password WRITE setPassword)
    Q_ENUMS(STATE)
    Q_PROPERTY(STATE state READ state NOTIFY stateChanged)
public:
    explicit Router(QObject *parent = 0);
    ~Router();

    void setAddress(const QHostAddress &address);
    QHostAddress address() const;
    void setAddressString(const QString &address);
    QString addressString() const;

    void setPort(quint16 port);
    quint16 port() const;

    void setUsername(const QString &username);
    QString username() const;

    void setPassword(const QString &password);
    QString password() const;

    enum STATE {
        DISCONNECTED,
        CONNECTING,
        LOGGING_IN,
        EXECUTING,
        READY
    };

    STATE state() const;

signals:
    void stateChanged(Router::STATE state);
    void reply(const QStringList &sentence);

public slots:
    void login();
    void logout();
    void request(const QStringList &sentence);

protected:
    RouterPrivate * const d_ptr;
    Router(RouterPrivate &dd, QObject *parent);

private:
    Q_DECLARE_PRIVATE(Router)
    Q_PRIVATE_SLOT(d_func(), void _q_state_changed(QAbstractSocket::SocketState))
    Q_PRIVATE_SLOT(d_func(), void _q_ready_read())
    Q_PRIVATE_SLOT(d_func(), void _q_error(QAbstractSocket::SocketError))
};

} // qmikrotik

#endif // ROUTER_H
