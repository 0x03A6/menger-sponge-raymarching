#pragma once

#include <string>

struct RuntimeConfig {
    float aspect_ratio;
};

struct ShaderDefineConfig {
    std::string max_iter_rough;
    std::string max_iter_relative;
    std::string max_iter_relative_cutoff;
    std::string max_raymarching_iter;
    std::string cutoff_dist_far;
    std::string cutoff_dist_near_rough;
    std::string cutoff_dist_near_relative;
    std::string epsilon;
    std::string base_color_formula;
};

struct AppConfig {
    RuntimeConfig runtime;
    ShaderDefineConfig shader_defines;
};

struct ConfigState {
    AppConfig active_config;
    AppConfig staged_config;
};

AppConfig makeDefaultAppConfig(float aspect_ratio);
ConfigState makeDefaultConfigState(float aspect_ratio);
