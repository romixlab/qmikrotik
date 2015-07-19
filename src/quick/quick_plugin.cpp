#include "quick_plugin.h"
#include "mrouter.h"

#include <qqml.h>

void QuickPlugin::registerTypes(const char *uri)
{
    // @uri com.marsworks.qmlcomponents2
    qmlRegisterType<MRouter>(uri, 1, 0, "MRouter");
}


