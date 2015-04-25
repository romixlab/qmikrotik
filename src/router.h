#ifndef ROUTER_H
#define ROUTER_H

#include <QObject>
#include <QHostAddress>

namespace qmikrotik {

class Router : public QObject
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
    QString password();

    enum class STATE {
        DISCONNECTED,
        CONNECTING,
        LOGGING_IN,
        READY
    };

signals:
    void stateChanged(STATE state);
    void reply(const Sentence &sentence);

public slots:
    void login();
    void logout();
    void request(const Sentence &sentence);
};

} // qmikrotik

#endif // ROUTER_H
