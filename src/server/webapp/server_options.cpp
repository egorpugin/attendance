#include "server_options.h"

#include <Wt/WServer.h>

ServerProgramOptions &getProgramOptions()
{
    static ServerProgramOptions program_options;
    return program_options;
}

static String getVar(const String &v)
{
    String s;
    Wt::WServer::instance()->readConfigurationProperty(v, s);
    return s;
}

path ServerProgramOptions::getDbPath() const
{
    return getVar("db-path");
}
