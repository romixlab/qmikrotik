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
public:
    explicit Router(QObject *parent = 0);
    ~Router();

    void setAddress(const QHostAddress &address);
    QHostAddress address() const;

    void setPort(quint16 port);
    quint16 port() const;

    void setUsername(const QString &username);
    QString username() const;

    void setPassword(const QString &password);
    QString password() const;

    enum class STATE {
        DISCONNECTED,
        CONNECTING,
        LOGGING_IN,
        READY
    };

signals:
    void stateChanged(STATE state);
    //void reply(const Sentence &sentence);

public slots:
    void login();
    void logout();
    //void request(const Sentence &sentence);

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
