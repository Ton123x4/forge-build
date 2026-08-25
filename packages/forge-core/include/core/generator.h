#pragma once

#include "context.h"

namespace Generator {
    void GenerateNinja(const Build::Context& context, const std::string& output);

    void GenerateMakefile(const Build::Context& context, const std::string& output);

    void GenerateShellScript(const Build::Context& context, const std::string& output);

    void GenerateBatchFile(const Build::Context& context, const std::string& output);

    void GenerateCompileCommands(const Build::Context& context, const std::string& output);
}
