#include "commands.h"

#include "core/builder.h"
#include "core/project.h"
#include "stdext/arg_parser.hpp"
#include "stdext/filesystem.hpp"
#include "stdext/string.hpp"

namespace Command {
    const auto kGenFormats = stdext::hash_map<std::string, GenFormat> {
        { "makefile", GenFormat::Makefile },
        { "ninja", GenFormat::Ninja },
        { "shell", GenFormat::Shell },
        { "batch", GenFormat::Batch },
    };

    /// SimpleCommand

    SimpleCommand::SimpleCommand(const std::string& name) : m_name(name) {
        initCommand();
    }

    SimpleCommand::SimpleCommand(const std::string& name, stdext::arg_parser& parser) : m_name(name) {
        initCommand();

        while (!parser.is_end()) {
            handleCommand(parser);
        }
    }

    void SimpleCommand::initCommand() {
        m_project = stdext::fs::current_dir();
        m_package = std::nullopt;
    }

    void SimpleCommand::handleCommand(stdext::arg_parser& parser) {
        if (parser.match({ "--project" })) {
            m_project = parser.expect("Missing value for --project");
        }
        else if (parser.match({ "--package", "-p" })) {
            m_package = parser.expect("Missing value for --package");
        }
        else {
            raiseUnknown(parser);
        }
    }

    void SimpleCommand::raiseUnknown(stdext::arg_parser& parser) {
        throw std::runtime_error(std::format("Unknown {} argument: {}", m_name, parser.next()));
    }

    int SimpleCommand::parseNumber(const std::string& argument, const std::string& value) {
        try {
            auto number = std::stoi(value);

            if (number <= 0) {
                throw std::runtime_error(std::format("Invalid value for {}: must be > 0", argument));
            }

            return number;
        }
        catch (const std::invalid_argument&) {
            throw std::runtime_error(std::format("Invalid value for {}: not a number", argument));
        }
        catch (const std::out_of_range&) {
            throw std::runtime_error(std::format("Invalid value for {}: number out of range", argument));
        }
    }

    /// SelectCommand

    SelectCommand::SelectCommand(const std::string& name) : SimpleCommand(name) {
        initCommand();
    }

    SelectCommand::SelectCommand(const std::string& name, stdext::arg_parser& parser) : SimpleCommand(name) {
        initCommand();

        while (!parser.is_end()) {
            handleCommand(parser);
        }
    }

    void SelectCommand::initCommand() {
        m_profile = Project::Profile::Complete;

        m_environment.insert(STDEXT_SYSTEM_NAME);
        m_environment.insert(STDEXT_PLATFORM_NAME);
        m_environment.insert(STDEXT_VENDOR_NAME);
        m_environment.insert(STDEXT_ABI_NAME);

        if (STDEXT_PLATFORM_POSIX) {
            if (m_environment.contains("darwin")) {
                m_environment.insert("unix");
            }

            m_environment.insert("posix");
        }

        if (stdext::string::contains(STDEXT_SYSTEM_NAME, "bsd")) {
            m_environment.insert("bsd");
        }

        if (m_environment.contains("windows")) {
            auto platform = std::string(STDEXT_PLATFORM_NAME);

            if (platform == "x86_64" || platform == "arm64" || platform == "riscv64") {
                m_environment.insert("win64");
            }

            m_environment.insert("win32");
        }
    }

    void SelectCommand::handleCommand(stdext::arg_parser& parser) {
        if (parser.match({ "--debug" })) {
            m_profile = Project::Profile::Debug;
        }
        else if (parser.match({ "--preview" })) {
            m_profile = Project::Profile::Preview;
        }
        else if (parser.match({ "--release" })) {
            m_profile = Project::Profile::Release;
        }
        else if (parser.match({ "--env", "-E" })) {
            m_environment.insert(parser.expect("Missing value for --env"));
        }
        else if (parser.match({ "--bare-env", "-B" })) {
            m_environment.clear();
        }
        else {
            SimpleCommand::handleCommand(parser);
        }
    }

    /// ProjectCommand

    ProjectCommand::ProjectCommand(const std::string& name) : SelectCommand(name) {
        initCommand();
    }

    ProjectCommand::ProjectCommand(const std::string& name, stdext::arg_parser& parser) : SelectCommand(name) {
        initCommand();

        while (!parser.is_end()) {
            handleCommand(parser);
        }
    }

    void ProjectCommand::initCommand() {
        m_profile = Project::Profile::Debug;
    }

    void ProjectCommand::handleCommand(stdext::arg_parser& parser) {
        if (parser.match({ "--define", "-D" })) {
            m_defines.insert(parser.expect("Missing value for --define"));
        }
        else {
            SelectCommand::handleCommand(parser);
        }
    }

    /// SetupCommand

    SetupCommand::SetupCommand(const std::string& name) : ProjectCommand(name) {
        initCommand();
    }

    SetupCommand::SetupCommand(const std::string& name, stdext::arg_parser& parser) : ProjectCommand(name) {
        initCommand();

        while (!parser.is_end()) {
            handleCommand(parser);
        }
    }

    void SetupCommand::initCommand() {
    }

    void SetupCommand::handleCommand(stdext::arg_parser& parser) {
        if (parser.match({ "--output", "-o" })) {
            m_output = parser.expect("Missing value for --output");
        }
        else {
            ProjectCommand::handleCommand(parser);
        }
    }

    /// GenCommand

    GenCommand::GenCommand(const std::string& name) : ProjectCommand(name) {
        initCommand();
    }

    GenCommand::GenCommand(const std::string& name, stdext::arg_parser& parser) : ProjectCommand(name) {
        initCommand();

        while (!parser.is_end()) {
            handleCommand(parser);
        }
    }

    void GenCommand::initCommand() {
        m_format = GenFormat::Ninja;
    }

    void GenCommand::handleCommand(stdext::arg_parser& parser) {
        if (parser.match({ "--format", "-f" })) {
            auto format_name = parser.expect("Missing value for --format");

            if (!kGenFormats.contains(format_name)) {
                throw std::runtime_error(std::format("Unknown format '{}'", format_name));
            }

            m_format = kGenFormats.at(format_name);
        }
        else if (parser.match({ "--output", "-o" })) {
            m_output = parser.expect("Missing value for --output");
        }
        else {
            ProjectCommand::handleCommand(parser);
        }
    }

    /// TestCommand

    TestCommand::TestCommand(const std::string& name) : SelectCommand(name) {
        initCommand();
    }

    TestCommand::TestCommand(const std::string& name, stdext::arg_parser& parser) : SelectCommand(name) {
        initCommand();

        while (!parser.is_end()) {
            handleCommand(parser);
        }
    }

    void TestCommand::initCommand() {
        m_profile = Project::Profile::Debug;
        m_workers = 1;
    }

    void TestCommand::handleCommand(stdext::arg_parser& parser) {
        if (parser.match({ "--workers", "-J" })) {
            m_workers = parseNumber("--workers", parser.expect("Missing value for --workers"));
        }
        else {
            SelectCommand::handleCommand(parser);
        }
    }

    /// BuildCommand

    BuildCommand::BuildCommand(const std::string& name) : ProjectCommand(name) {
        initCommand();
    }

    BuildCommand::BuildCommand(const std::string& name, stdext::arg_parser& parser) : ProjectCommand(name) {
        initCommand();

        while (!parser.is_end()) {
            handleCommand(parser);
        }
    }

    void BuildCommand::initCommand() {
        m_verbosity = Builder::LogVerbosity::Default;
        m_increment = false;
        m_rebuild = false;
        m_workers = 1;
    }

    void BuildCommand::handleCommand(stdext::arg_parser& parser) {
        if (parser.match({ "--rebuild", "-r" })) {
            m_rebuild = true;
        }
        else if (parser.match({ "--no-rebuild", "-nr" })) {
            m_rebuild = false;
        }
        else if (parser.match({ "--debug" })) {
            m_profile = Project::Profile::Debug;
            m_increment = false;
        }
        else if (parser.match({ "--preview" })) {
            m_profile = Project::Profile::Preview;
            m_increment = true;
            m_rebuild = true;
        }
        else if (parser.match({ "--release" })) {
            m_profile = Project::Profile::Release;
            m_increment = true;
            m_rebuild = true;
        }
        else if (parser.match({ "--increment", "-i" })) {
            m_increment = true;
        }
        else if (parser.match({ "--no-increment", "-ni" })) {
            m_increment = false;
        }
        else if (parser.match({ "--verbose", "-v" })) {
            m_verbosity = Builder::LogVerbosity::Verbose;
        }
        else if (parser.match({ "--quiet", "-q" })) {
            m_verbosity = Builder::LogVerbosity::Quiet;
        }
        else if (parser.match({ "--workers", "-J" })) {
            m_workers = parseNumber("--workers", parser.expect("Missing value for --workers"));
        }
        else {
            ProjectCommand::handleCommand(parser);
        }
    }
}
