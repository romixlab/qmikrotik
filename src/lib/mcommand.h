#ifndef MCOMMAND_H
#define MCOMMAND_H

#include <functional>

#include <QSharedData>

#include "mikrotik_global.h"

class MRouter;
class MRouterPrivate;
class MCommandPrivate;
class MIKROTIKSHARED_EXPORT MCommand
{
public:
    MCommand();
    MCommand(const QStringList &command);
    MCommand(const MCommand &other);
    MCommand &operator=(const MCommand &other);
    ~MCommand();

    void onCompleted(std::function<void (const QStringList &answer)> lambda);
    void onCompleted(quint16 id,
                     std::function<void (quint16 id,
                                         const QStringList &answer)> lambda);

    friend class MRouter;
    friend class MRouterPrivate;
private:
    QExplicitlySharedDataPointer<MCommandPrivate> d;
};

#endif // MCOMMAND_H
