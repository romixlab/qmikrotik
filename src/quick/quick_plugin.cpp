#include "quick_plugin.h"
#include "router.h"

#include <qqml.h>

void QuickPlugin::registerTypes(const char *uri)
{
    // @uri com.romixlab.qmlcomponents2
    qmlRegisterType<qmikrotik::Router>(uri, 1, 0, "Router");
}


