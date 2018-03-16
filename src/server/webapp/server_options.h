#pragma once

#include <primitives/filesystem.h>

struct ServerProgramOptions
{
    path getDbPath() const;
};

ServerProgramOptions &getProgramOptions();
