#pragma once

#include "stdext/arg_parser.hpp"

namespace Handlers {
    void HandleHelpCommand(stdext::arg_parser& arg_parser);

    void HandleInfoCommand(stdext::arg_parser& arg_parser);

    void HandleInitCommand(stdext::arg_parser& arg_parser);

    void HandleGenCommand(stdext::arg_parser& arg_parser);

    void HandleSetupCommand(stdext::arg_parser& arg_parser);

    bool HandleBuildCommand(stdext::arg_parser& arg_parser);

    bool HandleTestCommand(stdext::arg_parser& arg_parser);

    void HandleCopyCommand(stdext::arg_parser& arg_parser);

    void HandleCleanCommand(stdext::arg_parser& arg_parser);
}
