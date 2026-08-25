#include "handlers.h"
#include "help.h"
#include "stdext/arg_parser.hpp"
#include "stdext/console.hpp"
#include "stdext/forge.hpp"

int main(int argc, char* argv[]) {
    stdext::init_console();

    if (argc < 2) {
        Help::PrintMessage();
        return EXIT_SUCCESS;
    }

    try {
        auto arg_parser = stdext::arg_parser(argc, argv);

        if (arg_parser.match({ "version", "--version" })) {
            stdext::forge::print_build_info("Forge Build", true);
        }
        else if (arg_parser.match({ "help", "--help" })) {
            Handlers::HandleHelpCommand(arg_parser);
        }
        else if (arg_parser.match({ "info" })) {
            Handlers::HandleInfoCommand(arg_parser);
        }
        else if (arg_parser.match({ "gen" })) {
            Handlers::HandleGenCommand(arg_parser);
        }
        else if (arg_parser.match({ "setup" })) {
            Handlers::HandleSetupCommand(arg_parser);
        }
        else if (arg_parser.match({ "init" })) {
            Handlers::HandleInitCommand(arg_parser);
        }
        else if (arg_parser.match({ "build" })) {
            Handlers::HandleBuildCommand(arg_parser);
        }
        else if (arg_parser.match({ "test" })) {
            Handlers::HandleTestCommand(arg_parser);
        }
        else if (arg_parser.match({ "copy" })) {
            Handlers::HandleCopyCommand(arg_parser);
        }
        else if (arg_parser.match({ "clean" })) {
            Handlers::HandleCleanCommand(arg_parser);
        }
        else {
            throw std::invalid_argument(std::format("unknown command '{}'. Run 'forge --help' for usage.", arg_parser.next()));
        }
    }
    catch (const std::exception& e) {
        stdext::println(stdext::ansi_color::red, "Error: {}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
