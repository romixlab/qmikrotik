#ifndef MROUTER_H
#define MROUTER_H

#include <QObject>
#include <QHostAddress>
#include <QSharedData>

#include "mikrotik_global.h"
#include "mcommand.h"

class MRouterPrivate;
/// @file
/**
 * @brief The MRouter class
 */
class MIKROTIKSHARED_EXPORT MRouter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString address READ addressString WRITE setAddressString)
    Q_PROPERTY(quint16 port READ port WRITE setPort)
    Q_PROPERTY(QString username READ username WRITE setUsername)
    Q_PROPERTY(QString password READ password WRITE setPassword)
    Q_ENUMS(STATE)
    Q_PROPERTY(STATE state READ state NOTIFY stateChanged)
public:
    explicit MRouter(QObject *parent = 0);
    ~MRouter();

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

    /**
     * @brief The STATE enum
     */
    enum STATE {
        DISCONNECTED, ///< After creating
        CONNECTING,   ///< After login() called
        LOGGING_IN,   ///< Router answered, login process
        EXECUTING,    ///< Executing one or more commands
        READY         ///< Idle
    };
    /**
     * @brief state
     * @return current state
     */
    STATE state() const;
    /**
     * @brief stateString
     * @return current state as QString
     */
    QString stateString() const;
    /**
     * @brief command Create's MCommand and queues it for sending
     * @param command ["/interfaces/print"] for example
     * @return MCommand
     * Example usage:
     * router->command(QStringList() << "command" << "arg1" << "arg2")
     *      .onCompleted([](const QStringList &answer) {
     *          qDebug() << "Answer received" << answer;
     *      });
     */
    MCommand command(const QStringList &command);

signals:
    void stateChanged(MRouter::STATE state);

public slots:
    void login();
    void logout();

protected:
    MRouterPrivate * const d_ptr;
    MRouter(MRouterPrivate &dd, QObject *parent);

private:
    Q_DECLARE_PRIVATE(MRouter)
    Q_PRIVATE_SLOT(d_func(), void _q_state_changed(QAbstractSocket::SocketState))
    Q_PRIVATE_SLOT(d_func(), void _q_ready_read())
    Q_PRIVATE_SLOT(d_func(), void _q_error(QAbstractSocket::SocketError))
};

#endif // MROUTER_H
