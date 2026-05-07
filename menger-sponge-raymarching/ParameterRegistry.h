#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "RenderConfig.h"

enum class ParameterScope {
    runtime_uniform,
    shader_define,
};

enum class ParameterType {
    integer,
    floating_point,
    glsl_expression,
};

struct ParameterSpec {
    std::string parameter_name;
    std::string shader_define_name;
    ParameterScope scope;
    ParameterType type;
    std::string description;
    std::function<std::string(const AppConfig&)> get_value_text;
    std::function<bool(AppConfig&, const std::string&, std::string&)> set_value_text;
};

class ParameterRegistry {
public:
    ParameterRegistry();

    const ParameterSpec* findParameterSpec(const std::string& parameter_name) const;
    const std::vector<ParameterSpec>& getParameterSpecs() const;

private:
    void registerParameter(ParameterSpec parameter_spec);

    std::vector<ParameterSpec> parameter_specs;
    std::unordered_map<std::string, std::size_t> parameter_specs_by_name;
};

std::string buildShaderDefineBlock(const AppConfig& app_config, const ParameterRegistry& parameter_registry);
