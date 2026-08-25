#pragma once

#include "core/builder.h"
#include "core/project.h"
#include "stdext/arg_parser.hpp"
#include <optional>

namespace Command {
    enum struct GenFormat {
        Ninja,
        Makefile,
        Shell,
        Batch
    };

    struct SimpleCommand {
        std::string m_name;
        std::string m_project;
        std::optional<std::string> m_package;

        SimpleCommand(const std::string& name);

        SimpleCommand(const std::string& name, stdext::arg_parser& parser);

        void initCommand();

        void handleCommand(stdext::arg_parser& parser);

        void raiseUnknown(stdext::arg_parser& parser);

        int parseNumber(const std::string& argument, const std::string& value);
    };

    struct SelectCommand : public SimpleCommand {
        Project::Profile m_profile;
        Project::StringArray m_environment;

        SelectCommand(const std::string& name);
        SelectCommand(const std::string& name, stdext::arg_parser& parser);

        void initCommand();
        void handleCommand(stdext::arg_parser& parser);
    };

    struct ProjectCommand : public SelectCommand {
        Project::StringArray m_defines;

        ProjectCommand(const std::string& name);
        ProjectCommand(const std::string& name, stdext::arg_parser& parser);

        void initCommand();
        void handleCommand(stdext::arg_parser& parser);
    };

    struct SetupCommand : public ProjectCommand {
        std::string m_output;

        SetupCommand(const std::string& name);
        SetupCommand(const std::string& name, stdext::arg_parser& parser);

        void initCommand();
        void handleCommand(stdext::arg_parser& parser);
    };

    struct GenCommand : public ProjectCommand {
        GenFormat m_format;
        std::string m_output;

        GenCommand(const std::string& name);
        GenCommand(const std::string& name, stdext::arg_parser& parser);

        void initCommand();
        void handleCommand(stdext::arg_parser& parser);
    };

    struct TestCommand : public SelectCommand {
        std::uint16_t m_workers;

        TestCommand(const std::string& name);
        TestCommand(const std::string& name, stdext::arg_parser& parser);

        void initCommand();
        void handleCommand(stdext::arg_parser& parser);
    };

    struct BuildCommand : public ProjectCommand {
        Builder::LogVerbosity m_verbosity;
        uint16_t m_workers;
        bool m_increment;
        bool m_rebuild;

        BuildCommand(const std::string& name);
        BuildCommand(const std::string& name, stdext::arg_parser& parser);

        void initCommand();
        void handleCommand(stdext::arg_parser& parser);
    };
}
