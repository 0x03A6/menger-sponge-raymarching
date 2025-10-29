#version 330 core

out vec4 FragColor;
in vec2 pos;

uniform mat4 view_inv;

#define MAX_ITER 10

float sdBox(vec3 p) {
    vec3 q = abs(p) - 1.5;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

// 门格海绵距离函数
float sdMenger(vec3 p) {
    float size = 1.0;
    
    for(int iter = 0; iter < MAX_ITER; iter++) {
        
        p = abs(p);
        if(p.y > p.x) p.yx = p.xy;
        if(p.z > p.y) p.zy = p.yz;
        
        // 门格海绵的核心变换
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
    float eps = 0.001; // 一个小偏移量用于计算梯度
    
    // 在三个方向上计算距离场的梯度
    float dx = sdMenger(p + vec3(eps, 0, 0)) - sdMenger(p - vec3(eps, 0, 0));
    float dy = sdMenger(p + vec3(0, eps, 0)) - sdMenger(p - vec3(0, eps, 0));
    float dz = sdMenger(p + vec3(0, 0, eps)) - sdMenger(p - vec3(0, 0, eps));
    
    // 归一化梯度得到法向量
    return normalize(vec3(dx, dy, dz));
}

void main() {
    // 使用逆视图矩阵计算世界空间中的射线
    // 相机在世界空间中的位置 = 逆视图矩阵的平移部分
    vec3 ro_world = vec3(view_inv[3]);
    
    // 在相机空间中的射线方向（标准化设备坐标到相机空间）
    vec3 rd_camera = normalize(vec3(pos.x, pos.y, -1.0));
    
    // 使用逆视图矩阵的旋转部分将射线方向转换到世界空间
    // 注意：我们只使用3x3旋转部分，忽略平移
    vec3 rd_world = normalize(mat3(view_inv) * rd_camera);
    
    // Ray Marching - 在世界空间中进行
    float t = 0.0;
    vec3 hitPoint = vec3(0.0);
    bool hit = false;
    
    for(int i = 0; i < 100; i++) {
        vec3 p = ro_world + rd_world * t;
        float d = sdMenger(p);
        
        if(d < 0.001) {
            hit = true;
            hitPoint = p; // 保存命中点位置
            break;
        }
        
        if(t > 20.0) break;
        t += d;
    }
    
    if(hit) {
        // 计算命中点的法向量
        vec3 normal = abs(calcNormal(hitPoint));
        
        // 将法向量从[-1,1]范围映射到[0,1]范围用于颜色
        // 法向量的x,y,z分量分别对应R,G,B通道
        vec3 color = normal * 0.5 + 0.5;
        
        FragColor = vec4(color, 1.0);
    } else {
        // 背景色 - 深灰色而不是纯黑，以便更好地与物体区分
        FragColor = vec4(0.1, 0.1, 0.1, 1.0);
    }
}

//