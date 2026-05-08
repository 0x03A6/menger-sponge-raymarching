#include "RenderConfig.h"

AppConfig makeDefaultAppConfig(float aspect_ratio) {
    AppConfig app_config = {};
    app_config.runtime.aspect_ratio = aspect_ratio;

    app_config.shader_defines.max_iter_rough = "5";
    app_config.shader_defines.max_iter_relative = "6.0";
    app_config.shader_defines.max_iter_relative_cutoff = "30.0";
    app_config.shader_defines.max_raymarching_iter = "350";
    app_config.shader_defines.cutoff_dist_far = "2000000.0";
    app_config.shader_defines.cutoff_dist_near_rough = "0.0001";
    app_config.shader_defines.cutoff_dist_near_relative = "0.00005";
    app_config.shader_defines.epsilon = "0.00001";
    app_config.shader_defines.base_color_formula = "transpose(mat3(view_inv)) * normal * 0.5 + 0.5";

    return app_config;
}

ConfigState makeDefaultConfigState(float aspect_ratio) {
    ConfigState config_state = {};
    config_state.active_config = makeDefaultAppConfig(aspect_ratio);
    config_state.staged_config = config_state.active_config;
    return config_state;
}
