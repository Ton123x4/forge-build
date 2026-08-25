#include "help.h"

#include "stdext/console.hpp"
#include "stdext/handler.hpp"
#include "stdext/hash_map.hpp"

namespace Help {
    constexpr const char kHelpMessage[] = {
        #embed "./resources/help/help_forge.txt"
        , 0
    };

    constexpr const char kHelpInfoMessage[] = {
        #embed "./resources/help/help_info.txt"
        , 0
    };

    constexpr const char kHelpInitMessage[] = {
        #embed "./resources/help/help_init.txt"
        , 0
    };

    constexpr const char kHelpSetupMessage[] = {
        #embed "./resources/help/help_setup.txt"
        , 0
    };

    constexpr const char kHelpGenMessage[] = {
        #embed "./resources/help/help_gen.txt"
        , 0
    };

    constexpr const char kHelpBuildMessage[] = {
        #embed "./resources/help/help_build.txt"
        , 0
    };

    constexpr const char kHelpTestMessage[] = {
        #embed "./resources/help/help_test.txt"
        , 0
    };

    constexpr const char kHelpCopyMessage[] = {
        #embed "./resources/help/help_copy.txt"
        , 0
    };

    constexpr const char kHelpCleanMessage[] = {
        #embed "./resources/help/help_clean.txt"
        , 0
    };

    const stdext::hash_map<std::string, std::string_view> k_help_messages = {
        { "help", kHelpMessage },
        { "info", kHelpInfoMessage },
        { "init", kHelpInitMessage },
        { "setup", kHelpSetupMessage },
        { "gen", kHelpGenMessage },
        { "build", kHelpBuildMessage },
        { "test", kHelpTestMessage },
        { "copy", kHelpCopyMessage },
        { "clean", kHelpCleanMessage }
    };

    void PrintMessage() {
        stdext::println("{}", kHelpMessage);
    }

    void PrintMessage(const std::string& subcommand) {
        if (!k_help_messages.contains(subcommand)) {
            stdext::println(stdext::ansi_color::red, "Unknown command '{}'. Use --help for a list of commands.", subcommand);
            return;
        }

        stdext::println("{}", k_help_messages.at(subcommand));
    }
}
