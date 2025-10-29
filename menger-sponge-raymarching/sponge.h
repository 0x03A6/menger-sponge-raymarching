#pragma once

#include <glm/glm.hpp>
#include <algorithm>

using namespace glm;

constexpr int MAX_ITER = 15;

float sdBox(dvec3 p) {
    dvec3 q = abs(p) - 1.5;
    return length(max(q, 0.0) + min(max(q.x, max(q.y, q.z)), 0.0));
}

// 门格海绵距离函数
double sdMenger(dvec3 p) {
    double size = 1.0;

    for (int iter = 0; iter < MAX_ITER; iter++) {

        p = abs(p);
        if (p.y > p.x) std::swap(p.x, p.y);
		if (p.z > p.y) std::swap(p.y, p.z);

        // 门格海绵的核心变换
        if (p.z > 0.5)
            p -= dvec3(1.0, 1.0, 1.0);
        else
            p -= dvec3(1.0, 1.0, 0.0);

        p *= 3.0;
        size /= 3.0;
    }

    return sdBox(p) * size;
}