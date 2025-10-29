#version 330 core

out vec4 FragColor;
in vec2 pos;

uniform mat4 view_inv;
uniform ivec3 blocks[16];  // 改为ivec3，因为GLSL没有i8vec3

#define MAX_ITER 10
#define MAX_BLOCKS 16
#define INF 10000000.0
#define CUTOFF_DIST_FAR 1000.0
#define CUTOFF_DIST_NEAR 0.0001
#define EPSILON 0.0001

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

// 无限层次门格海绵距离函数
float sdInfiniteMenger(vec3 p) {

    float size = 1.0;

    // 从最内层开始
    int currentLevel = 0;
    
    // 应用层次变换，直到找到正确的层次
    while (currentLevel < MAX_BLOCKS - 1) {
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
    
    for(int iter = 0; iter < MAX_ITER; iter++) {
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

// 计算法向量的函数
vec3 calcNormal(vec3 p) {
    
    float dx = sdInfiniteMenger(p + vec3(EPSILON, 0, 0)) - sdInfiniteMenger(p - vec3(EPSILON, 0, 0));
    float dy = sdInfiniteMenger(p + vec3(0, EPSILON, 0)) - sdInfiniteMenger(p - vec3(0, EPSILON, 0));
    float dz = sdInfiniteMenger(p + vec3(0, 0, EPSILON)) - sdInfiniteMenger(p - vec3(0, 0, EPSILON));
    
    return normalize(vec3(dx, dy, dz));
}

void main() {
    vec3 ro_world = vec3(view_inv[3]);
    vec2 uv = pos.xy;
    uv.x *= 1000.0 / 1000.0; // 假设分辨率1000x1000
    
    vec3 rd_camera = normalize(vec3(uv, -1.0));
    vec3 rd_world = normalize(mat3(view_inv) * rd_camera);
    
    float t = 0.0;
    vec3 hitPoint = vec3(0.0);
    bool hit = false;
    
    for(int i = 0; ; i++) {
        vec3 p = ro_world + rd_world * t;
        float d = sdInfiniteMenger(p);
        
        if(d < CUTOFF_DIST_NEAR) {
            hit = true;
            hitPoint = p;
            break;
        }
        
        if(t > CUTOFF_DIST_FAR) break;
        t += d;
    }
    
    if(hit) {
        vec3 normal = abs(calcNormal(hitPoint));
        vec3 color = normal * 0.5 + 0.5;
        FragColor = vec4(color, 1.0);
    } else {
        FragColor = vec4(0.1, 0.1, 0.1, 1.0);
    }
}

//