#include "mcommand.h"
#include "mcommand_p.h"

MCommandPrivate::MCommandPrivate()
{ }

MCommandPrivate::~MCommandPrivate()
{ }

MCommandPrivate::MCommandPrivate(const MCommandPrivate &other) :
    QSharedData(other),
    command(other.command),
    lambda(other.lambda)
{ }

MCommand::MCommand() :
    d(new MCommandPrivate)
{ }

MCommand::MCommand(const QStringList &command) :
    d(new MCommandPrivate)
{
    d->command = command;
}

MCommand::MCommand(const MCommand &other) :
    d(other.d)
{ }

MCommand &MCommand::operator=(const MCommand &other)
{
    d = other.d;
    return *this;
}

MCommand::~MCommand()
{ }

void MCommand::onCompleted(std::function<void (const QStringList &)> lambda)
{
    d->lambda = lambda;
}

void MCommand::onCompleted(quint16 id, std::function<void (quint16, const QStringList &)> lambda)
{
    d->id = id;
    d->lambda_id = lambda;
}
