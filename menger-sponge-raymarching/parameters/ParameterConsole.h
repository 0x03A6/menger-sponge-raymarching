#pragma once

#include <functional>
#include <iosfwd>
#include <string>

#include "ParameterRegistry.h"
#include "RenderConfig.h"

struct ParameterConsoleContext {
    ConfigState* config_state = nullptr;
    const ParameterRegistry* parameter_registry = nullptr;
    std::function<bool(const AppConfig&, std::string&)> apply_config;
};

void runParameterConsole(
    std::istream& input_stream,
    std::ostream& output_stream,
    ParameterConsoleContext& parameter_console_context
);

bool applyParameterScript(
    std::istream& input_stream,
    std::ostream& output_stream,
    ParameterConsoleContext& parameter_console_context,
    std::string& error_message
);
