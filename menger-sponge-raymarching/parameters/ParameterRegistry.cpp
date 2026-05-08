#include "ParameterRegistry.h"

#include <cctype>
#include <cstdlib>
#include <utility>

namespace {

std::string trim(const std::string& text) {
    std::size_t begin = 0;
    while (begin < text.size() && std::isspace(static_cast<unsigned char>(text[begin])))
        begin++;

    std::size_t end = text.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1])))
        end--;

    return text.substr(begin, end - begin);
}

bool parseDoubleStrict(const std::string& value_text, double* value) {
    const std::string trimmed_value = trim(value_text);
    if (trimmed_value.empty())
        return false;

    char* end = nullptr;
    const double parsed_value = std::strtod(trimmed_value.c_str(), &end);
    if (end == trimmed_value.c_str())
        return false;

    while (*end != '\0') {
        if (!std::isspace(static_cast<unsigned char>(*end)))
            return false;
        end++;
    }

    *value = parsed_value;
    return true;
}

bool isIntegerLiteral(const std::string& value_text) {
    const std::string trimmed_value = trim(value_text);
    if (trimmed_value.empty())
        return false;

    std::size_t index = 0;
    if (trimmed_value[index] == '+' || trimmed_value[index] == '-')
        index++;
    if (index == trimmed_value.size())
        return false;

    for (; index < trimmed_value.size(); index++) {
        if (!std::isdigit(static_cast<unsigned char>(trimmed_value[index])))
            return false;
    }

    return true;
}

bool setPositiveIntegerText(std::string& target, const std::string& value_text, std::string& error_message) {
    const std::string trimmed_value = trim(value_text);
    if (!isIntegerLiteral(trimmed_value)) {
        error_message = "value must be an integer literal";
        return false;
    }

    double value = 0.0;
    if (!parseDoubleStrict(trimmed_value, &value) || value <= 0.0) {
        error_message = "value must be greater than 0";
        return false;
    }

    target = trimmed_value;
    return true;
}

bool setNonNegativeFloatText(std::string& target, const std::string& value_text, std::string& error_message) {
    const std::string trimmed_value = trim(value_text);

    double value = 0.0;
    if (!parseDoubleStrict(trimmed_value, &value)) {
        error_message = "value must be a floating-point literal";
        return false;
    }
    if (value < 0.0) {
        error_message = "value must be non-negative";
        return false;
    }

    target = trimmed_value;
    return true;
}

bool setPositiveFloatText(std::string& target, const std::string& value_text, std::string& error_message) {
    const std::string trimmed_value = trim(value_text);

    double value = 0.0;
    if (!parseDoubleStrict(trimmed_value, &value)) {
        error_message = "value must be a floating-point literal";
        return false;
    }
    if (value <= 0.0) {
        error_message = "value must be greater than 0";
        return false;
    }

    target = trimmed_value;
    return true;
}

bool setGlslExpressionText(std::string& target, const std::string& value_text, std::string& error_message) {
    const std::string trimmed_value = trim(value_text);
    if (trimmed_value.empty()) {
        error_message = "value must not be empty";
        return false;
    }

    target = trimmed_value;
    return true;
}

} // namespace

ParameterRegistry::ParameterRegistry() {
    registerParameter({
        "max_iter_rough",
        "MAX_ITER_ROUGH",
        ParameterScope::shader_define,
        ParameterType::integer,
        "Rough ray-marching iteration count.",
        [](const AppConfig& app_config) {
            return app_config.shader_defines.max_iter_rough;
        },
        [](AppConfig& app_config, const std::string& value_text, std::string& error_message) {
            return setPositiveIntegerText(app_config.shader_defines.max_iter_rough, value_text, error_message);
        },
    });

    registerParameter({
        "max_iter_relative",
        "MAX_ITER_RELATIVE",
        ParameterScope::shader_define,
        ParameterType::floating_point,
        "Relative iteration baseline used for accurate ray marching.",
        [](const AppConfig& app_config) {
            return app_config.shader_defines.max_iter_relative;
        },
        [](AppConfig& app_config, const std::string& value_text, std::string& error_message) {
            return setNonNegativeFloatText(app_config.shader_defines.max_iter_relative, value_text, error_message);
        },
    });

    registerParameter({
        "max_iter_relative_cutoff",
        "MAX_ITER_RELATIVE_CUTOFF",
        ParameterScope::shader_define,
        ParameterType::floating_point,
        "Maximum relative iteration count used by the accurate pass.",
        [](const AppConfig& app_config) {
            return app_config.shader_defines.max_iter_relative_cutoff;
        },
        [](AppConfig& app_config, const std::string& value_text, std::string& error_message) {
            return setNonNegativeFloatText(app_config.shader_defines.max_iter_relative_cutoff, value_text, error_message);
        },
    });

    registerParameter({
        "max_raymarching_iter",
        "MAX_RAYMARCHING_ITER",
        ParameterScope::shader_define,
        ParameterType::integer,
        "Maximum number of ray-marching steps for each pass.",
        [](const AppConfig& app_config) {
            return app_config.shader_defines.max_raymarching_iter;
        },
        [](AppConfig& app_config, const std::string& value_text, std::string& error_message) {
            return setPositiveIntegerText(app_config.shader_defines.max_raymarching_iter, value_text, error_message);
        },
    });

    registerParameter({
        "cutoff_dist_far",
        "CUTOFF_DIST_FAR",
        ParameterScope::shader_define,
        ParameterType::floating_point,
        "Maximum ray distance before a ray is treated as a miss.",
        [](const AppConfig& app_config) {
            return app_config.shader_defines.cutoff_dist_far;
        },
        [](AppConfig& app_config, const std::string& value_text, std::string& error_message) {
            return setPositiveFloatText(app_config.shader_defines.cutoff_dist_far, value_text, error_message);
        },
    });

    registerParameter({
        "cutoff_dist_near_rough",
        "CUTOFF_DIST_NEAR_ROUGH",
        ParameterScope::shader_define,
        ParameterType::floating_point,
        "Hit threshold scale for the rough ray-marching pass.",
        [](const AppConfig& app_config) {
            return app_config.shader_defines.cutoff_dist_near_rough;
        },
        [](AppConfig& app_config, const std::string& value_text, std::string& error_message) {
            return setPositiveFloatText(app_config.shader_defines.cutoff_dist_near_rough, value_text, error_message);
        },
    });

    registerParameter({
        "cutoff_dist_near_relative",
        "CUTOFF_DIST_NEAR_RELATIVE",
        ParameterScope::shader_define,
        ParameterType::floating_point,
        "Hit threshold scale for the accurate ray-marching pass.",
        [](const AppConfig& app_config) {
            return app_config.shader_defines.cutoff_dist_near_relative;
        },
        [](AppConfig& app_config, const std::string& value_text, std::string& error_message) {
            return setPositiveFloatText(app_config.shader_defines.cutoff_dist_near_relative, value_text, error_message);
        },
    });

    registerParameter({
        "epsilon",
        "EPSILON",
        ParameterScope::shader_define,
        ParameterType::floating_point,
        "Finite-difference epsilon used for the normal calculation.",
        [](const AppConfig& app_config) {
            return app_config.shader_defines.epsilon;
        },
        [](AppConfig& app_config, const std::string& value_text, std::string& error_message) {
            return setPositiveFloatText(app_config.shader_defines.epsilon, value_text, error_message);
        },
    });

    registerParameter({
        "base_color_formula",
        "BASE_COLOR_FORMULA",
        ParameterScope::shader_define,
        ParameterType::glsl_expression,
        "GLSL vec3 expression used to calculate base_color.",
        [](const AppConfig& app_config) {
            return app_config.shader_defines.base_color_formula;
        },
        [](AppConfig& app_config, const std::string& value_text, std::string& error_message) {
            return setGlslExpressionText(app_config.shader_defines.base_color_formula, value_text, error_message);
        },
    });
}

const ParameterSpec* ParameterRegistry::findParameterSpec(const std::string& parameter_name) const {
    const auto iterator = parameter_specs_by_name.find(parameter_name);
    if (iterator == parameter_specs_by_name.end())
        return nullptr;

    return &parameter_specs[iterator->second];
}

const std::vector<ParameterSpec>& ParameterRegistry::getParameterSpecs() const {
    return parameter_specs;
}

void ParameterRegistry::registerParameter(ParameterSpec parameter_spec) {
    const std::size_t parameter_index = parameter_specs.size();
    parameter_specs_by_name[parameter_spec.parameter_name] = parameter_index;
    parameter_specs.push_back(std::move(parameter_spec));
}

std::string buildShaderDefineBlock(const AppConfig& app_config, const ParameterRegistry& parameter_registry) {
    std::string define_block;

    for (const ParameterSpec& parameter_spec : parameter_registry.getParameterSpecs()) {
        if (parameter_spec.scope != ParameterScope::shader_define)
            continue;
        if (parameter_spec.shader_define_name.empty())
            continue;

        define_block += "#define ";
        define_block += parameter_spec.shader_define_name;
        define_block += ' ';
        define_block += parameter_spec.get_value_text(app_config);
        define_block += '\n';
    }

    return define_block;
}
