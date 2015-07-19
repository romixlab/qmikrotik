#ifndef MCOMMAND_P
#define MCOMMAND_P

#include <functional>

#include "mcommand.h"

class MCommandPrivate : public QSharedData
{
public:
    MCommandPrivate();
    MCommandPrivate(const MCommandPrivate &other);
    ~MCommandPrivate();

    QStringList command;
    std::function<void (const QStringList &answer)> lambda;
    quint16 id;
    std::function<void (quint16 id, const QStringList &answer)> lambda_id;
};

#endif // MCOMMAND_P

