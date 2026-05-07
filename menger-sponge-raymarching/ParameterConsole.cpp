#include "ParameterConsole.h"

#include <cctype>
#include <iostream>
#include <sstream>

namespace {

enum class ConsoleCommandMode {
    interactive,
    startup_script,
};

bool isBlankLine(const std::string& line) {
    for (char ch : line) {
        if (!std::isspace(static_cast<unsigned char>(ch)))
            return false;
    }

    return true;
}

std::string trim(const std::string& text) {
    std::size_t begin = 0;
    while (begin < text.size() && std::isspace(static_cast<unsigned char>(text[begin])))
        begin++;

    std::size_t end = text.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1])))
        end--;

    return text.substr(begin, end - begin);
}

std::string formatParameterValue(const ParameterSpec& parameter_spec, const AppConfig& app_config) {
    return parameter_spec.get_value_text(app_config);
}

void printConsoleHelp(std::ostream& output_stream, const ParameterRegistry& parameter_registry) {
    output_stream << "Commands:\n";
    output_stream << "  help               Show this help text.\n";
    output_stream << "  show               Show current staged parameter values.\n";
    output_stream << "  <name> <value>     Stage a parameter update.\n";
    output_stream << "  apply              Apply staged changes and rebuild shaders if needed.\n";
    output_stream << "  cancel             Leave the parameter console without applying changes.\n";
    output_stream << "Parameters:\n";

    for (const ParameterSpec& parameter_spec : parameter_registry.getParameterSpecs())
        output_stream << "  " << parameter_spec.parameter_name << "  " << parameter_spec.description << '\n';
}

void printParameterValues(
    std::ostream& output_stream,
    const AppConfig& app_config,
    const ConfigState& config_state,
    const ParameterRegistry& parameter_registry
) {
    output_stream << "Parameters:\n";

    for (const ParameterSpec& parameter_spec : parameter_registry.getParameterSpecs()) {
        output_stream << "  " << parameter_spec.parameter_name << " = " << formatParameterValue(parameter_spec, app_config);

        const std::string active_value = parameter_spec.get_value_text(config_state.active_config);
        const std::string staged_value = parameter_spec.get_value_text(app_config);
        if (active_value != staged_value)
            output_stream << "  [pending]";

        output_stream << '\n';
    }
}

bool hasShaderDefineChanges(const AppConfig& left, const AppConfig& right, const ParameterRegistry& parameter_registry) {
    for (const ParameterSpec& parameter_spec : parameter_registry.getParameterSpecs()) {
        if (parameter_spec.scope != ParameterScope::shader_define)
            continue;
        if (parameter_spec.get_value_text(left) != parameter_spec.get_value_text(right))
            return true;
    }

    return false;
}

bool hasPendingParameterChanges(const ConfigState& config_state, const ParameterRegistry& parameter_registry) {
    for (const ParameterSpec& parameter_spec : parameter_registry.getParameterSpecs()) {
        if (parameter_spec.get_value_text(config_state.active_config) != parameter_spec.get_value_text(config_state.staged_config))
            return true;
    }

    return false;
}

bool applyStagedConfig(ParameterConsoleContext& parameter_console_context, std::string& error_message) {
    ConfigState& config_state = *parameter_console_context.config_state;
    const ParameterRegistry& parameter_registry = *parameter_console_context.parameter_registry;

    if (hasShaderDefineChanges(config_state.active_config, config_state.staged_config, parameter_registry)) {
        if (!parameter_console_context.apply_config(config_state.staged_config, error_message))
            return false;
    }

    config_state.active_config = config_state.staged_config;
    error_message.clear();
    return true;
}

bool handleConsoleCommand(
    const std::string& command_line,
    std::ostream& output_stream,
    ParameterConsoleContext& parameter_console_context,
    ConsoleCommandMode command_mode,
    bool* should_exit_session
) {
    ConfigState& config_state = *parameter_console_context.config_state;
    const ParameterRegistry& parameter_registry = *parameter_console_context.parameter_registry;

    std::istringstream command_stream(command_line);
    std::string operation;
    if (!(command_stream >> operation))
        return false;

    if (operation == "help") {
        printConsoleHelp(output_stream, parameter_registry);
        return true;
    }

    if (operation == "show") {
        printParameterValues(output_stream, config_state.staged_config, config_state, parameter_registry);
        return true;
    }

    if (operation == "cancel") {
        if (command_mode == ConsoleCommandMode::startup_script) {
            output_stream << "The 'cancel' command is not supported in startup config scripts.\n";
            return false;
        }

        config_state.staged_config = config_state.active_config;
        output_stream << "Discarded staged changes.\n";
        *should_exit_session = true;
        return true;
    }

    if (operation == "apply") {
        if (!hasPendingParameterChanges(config_state, parameter_registry)) {
            output_stream << "No pending parameter changes.\n";
            if (command_mode == ConsoleCommandMode::interactive)
                *should_exit_session = true;
            return true;
        }

        std::string error_message;
        if (!applyStagedConfig(parameter_console_context, error_message)) {
            output_stream << error_message << std::endl;
            return false;
        }

        output_stream << "Applied staged parameters.\n";
        if (command_mode == ConsoleCommandMode::interactive)
            *should_exit_session = true;
        return true;
    }

    const ParameterSpec* parameter_spec = parameter_registry.findParameterSpec(operation);
    if (parameter_spec == nullptr) {
        output_stream << "Unknown command or parameter: " << operation << '\n';
        return false;
    }

    std::string value_text;
    std::getline(command_stream, value_text);
    value_text = trim(value_text);
    if (value_text.empty()) {
        output_stream << "Expected a value after parameter name.\n";
        return false;
    }

    std::string error_message;
    if (!parameter_spec->set_value_text(config_state.staged_config, value_text, error_message)) {
        output_stream << "Failed to set " << parameter_spec->parameter_name << ": " << error_message << '\n';
        return false;
    }

    output_stream << "Staged " << parameter_spec->parameter_name << " = "
                  << formatParameterValue(*parameter_spec, config_state.staged_config) << '\n';
    return true;
}

} // namespace

void runParameterConsole(
    std::istream& input_stream,
    std::ostream& output_stream,
    ParameterConsoleContext& parameter_console_context
) {
    ConfigState& config_state = *parameter_console_context.config_state;

    config_state.staged_config = config_state.active_config;

    output_stream << "\nEntered parameter console.\n";
    printConsoleHelp(output_stream, *parameter_console_context.parameter_registry);
    printParameterValues(
        output_stream,
        config_state.staged_config,
        config_state,
        *parameter_console_context.parameter_registry
    );

    for (;;) {
        output_stream << "config> " << std::flush;

        std::string command_line;
        if (!std::getline(input_stream, command_line)) {
            input_stream.clear();
            output_stream << "\nLeaving parameter console.\n";
            break;
        }

        bool should_exit_session = false;
        handleConsoleCommand(
            command_line,
            output_stream,
            parameter_console_context,
            ConsoleCommandMode::interactive,
            &should_exit_session
        );
        if (should_exit_session)
            break;
    }
}

bool applyParameterScript(
    std::istream& input_stream,
    std::ostream& output_stream,
    ParameterConsoleContext& parameter_console_context,
    std::string& error_message
) {
    ConfigState& config_state = *parameter_console_context.config_state;
    config_state.staged_config = config_state.active_config;

    std::string command_line;
    int line_number = 0;
    while (std::getline(input_stream, command_line)) {
        line_number++;

        if (isBlankLine(command_line))
            continue;

        bool should_exit_session = false;
        const bool command_handled = handleConsoleCommand(
            command_line,
            output_stream,
            parameter_console_context,
            ConsoleCommandMode::startup_script,
            &should_exit_session
        );

        if (!command_handled) {
            error_message = "startup config script error on line " + std::to_string(line_number);
            return false;
        }
    }

    if (!hasPendingParameterChanges(config_state, *parameter_console_context.parameter_registry)) {
        error_message.clear();
        return true;
    }

    if (!applyStagedConfig(parameter_console_context, error_message)) {
        error_message = "failed to apply startup config script:\n" + error_message;
        return false;
    }

    output_stream << "Applied startup config script.\n";
    error_message.clear();
    return true;
}
