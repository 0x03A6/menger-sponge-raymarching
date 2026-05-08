#version 430 core

// 最开始先用一个比较小的迭代次数进行粗略的距离估计
// 之后进行精确计算，迭代次数根据距离远近动态调整

#define LN3 1.0986122886681098  // ln(3)

#define MAX_BLOCKS 80
#define INF 10000000.0

/*
#define MAX_ITER_ROUGH 5        // 粗略估计迭代次数
#define MAX_ITER_RELATIVE 6.0   // 精确计算迭代相对次数
#define MAX_ITER_RELATIVE_CUTOFF 30.0   // 精确计算最大迭代次数
#define MAX_RAYMARCHING_ITER 350 // 最大光线行进步数
#define CUTOFF_DIST_FAR 2000000.0
#define CUTOFF_DIST_NEAR_ROUGH 0.0001
#define CUTOFF_DIST_NEAR_RELATIVE 0.00005
#define EPSILON 0.00001
#define BASE_COLOR_FORMULA transpose(mat3(view_inv)) * normal * 0.5 + 0.5
*/
__CONFIG_DEFINES__

out vec4 FragColor;
in vec2 pos;

uniform mat4 view_inv;
uniform float aspect_ratio;
uniform ivec3 blocks[MAX_BLOCKS];  // 改为ivec3，因为GLSL没有i8vec3

float sdBox(vec3 p) {
    vec3 q = abs(p) - 1.5;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

// 判断点是否在块内
bool inBlock(vec3 pos) {
    return pos.x >= -1.5 && pos.x <= 1.5 &&
           pos.y >= -1.5 && pos.y <= 1.5 &&
           pos.z >= -1.5 && pos.z <= 1.5;
}

// 判断块索引是否在门格海绵中
bool inSponge(ivec3 idx) {
    if (idx.x != 0)
        return (idx.y != 0 || idx.z != 0);
    return (idx.y != 0 && idx.z != 0);
}

// 向外层变换
vec3 goOut(vec3 pos, ivec3 idx) {
    return (pos / 3.0) + vec3(idx);
}

// 向内层变换
vec3 goIn(vec3 pos, ivec3 idx) {
    return (pos - vec3(idx)) * 3.0;
}

// 从坐标获取块索引
ivec3 getIdx(vec3 pos) {
    return ivec3(floor(pos + 1.5)) - 1;
}

float current_size = 1.0;

// 无限层次门格海绵距离函数
float sdInfiniteMenger(vec3 p, int max_iter) {

    float size = 1.0;

    // 从最内层开始
    int currentLevel = 0;
    
    // 应用层次变换，直到找到正确的层次
    while (currentLevel < MAX_BLOCKS) {
        if (inBlock(p)) {
            break;
        }
        // 向外移动一层
        p = goOut(p, blocks[currentLevel]);
        currentLevel++;
        size *= 3.0;
    }
    
    // 如果超出了所有层次，返回大距离
    if (!inBlock(p)) {
        return INF;
    }

    current_size = size;
    
    // 向内移动，直到找到最深的实心块
    /*
    for (int level = currentLevel; level >= 0; level--) {
        ivec3 idx = getIdx(p);
        if (!inSponge(idx)) {
            break;
        }
        p = goIn(p, idx);
    }
    */
    
    for(int iter = -currentLevel; iter < max_iter; iter++) {
        p = abs(p);
        if(p.y > p.x) p.yx = p.xy;
        if(p.z > p.y) p.zy = p.yz;
        
        if(p.z > 0.5) 
            p -= vec3(1.0, 1.0, 1.0);
        else 
            p -= vec3(1.0, 1.0, 0.0);
        
        p *= 3.0;
        size /= 3.0;
    }
    
    return sdBox(p) * size;
}

float sdInfiniteMengerRelative(vec3 p, float max_iter) {
    int x = int(floor(max_iter) + EPSILON);
    float t = max_iter - x;
    return sdInfiniteMenger(p, x) * (1.0 - t) + sdInfiniteMenger(p, x + 1) * t;
}

// 计算法向量的函数
vec3 calcNormal(vec3 p, float epsilon, float max_iter) {
    
    float dx = sdInfiniteMengerRelative(p + vec3(epsilon, 0, 0), max_iter) - sdInfiniteMengerRelative(p - vec3(epsilon, 0, 0), max_iter);
    float dy = sdInfiniteMengerRelative(p + vec3(0, epsilon, 0), max_iter) - sdInfiniteMengerRelative(p - vec3(0, epsilon, 0), max_iter);
    float dz = sdInfiniteMengerRelative(p + vec3(0, 0, epsilon), max_iter) - sdInfiniteMengerRelative(p - vec3(0, 0, epsilon), max_iter);
    
    return normalize(vec3(dx / epsilon, dy / epsilon, dz / epsilon));
}

float calcRelativeIter(float dist) {
    return min(MAX_ITER_RELATIVE_CUTOFF, max(0.0, MAX_ITER_RELATIVE - log(dist) / LN3));
}

void main() {
    vec3 ro_world = vec3(view_inv[3]);
    vec2 uv = pos.xy;
    uv.x *= aspect_ratio;
    
    vec3 rd_camera = normalize(vec3(uv, -1.0));
    vec3 rd_world = normalize(mat3(view_inv) * rd_camera);
    
    float t = 0.0;
    bool hit = false;

    vec3 p = ro_world;

    float dist = sdInfiniteMenger(p, int(MAX_ITER_RELATIVE_CUTOFF));
    
    // ROUGH RAY MARCHING
    for (int i = 0; ; i++) {
        float d = sdInfiniteMenger(vec3(p), MAX_ITER_ROUGH);
        
        if(d < CUTOFF_DIST_NEAR_ROUGH * t) {
            hit = true;
            break;
        }

        vec3 temp_p = p + rd_world * d;
        if (temp_p == p)    // 浮点精度不够了，于是停止
            break;
        p = temp_p;
        t += d;

        if (i == MAX_RAYMARCHING_ITER) {
            hit = true;
            break;
        }

        if(t > CUTOFF_DIST_FAR) break;
    }
    
    if(hit) {
        // ACCURATE RAY MARCHING
        hit = false;
        for (int i = 0; ; i++) {
            float d = sdInfiniteMengerRelative(vec3(p), calcRelativeIter(t));
        
            if (d < CUTOFF_DIST_NEAR_RELATIVE * t) {
                hit = true;
                break;
            }

            vec3 temp_p = p + rd_world * d;
            if (temp_p == p)    // 浮点精度不够了，于是停止
                break;
            p = temp_p;
            t += d;

            if (t > CUTOFF_DIST_FAR) break;

            if (i == MAX_RAYMARCHING_ITER) {
                hit = true;
                break;
            }
        }
        vec3 normal = calcNormal(vec3(p), EPSILON * max(1.0, current_size), calcRelativeIter(t));
        if (any(isnan(p))) {
            FragColor = vec4(0.0, 1.0, 1.0, 1.0);
            return;
        }
        vec3 base_color = BASE_COLOR_FORMULA;
        // vec3 base_color = abs(normal) * 0.6 + 0.4;
        float dot_result = max(0.0, dot(normal, -rd_world));
        float relative_dist = dist / t;
        const vec3 ambient = base_color;
        const vec3 diffuse = max(base_color * dot_result, vec3(0.0));
        const vec3 specular = vec3(pow(dot_result, 8.0));
        vec3 color = 0.6 * ambient + (0.5 * diffuse + 0.1 * specular) * relative_dist * relative_dist;
        // vec3 color = base_color;
        FragColor = vec4(color, 1.0);
    } else {
        FragColor = vec4(0.1, 0.1, 0.1, 1.0);
    }

}

//
