#pragma once

#include <glm/glm.hpp>
#include <algorithm>
#include <deque>
#include <ctime>

using namespace glm;

constexpr int MAX_ITER = 50;
constexpr int BLOCK_AMOUNT = 100;
constexpr int BLOCK_AMOUNT_GPU = 80;

double sdBox(dvec3 p) {
    dvec3 q = abs(p) - 1.5;
    return length(max(q, 0.0) + min(max(q.x, max(q.y, q.z)), 0.0));
}

dvec3 goOut(dvec3 pos, const i8vec3 idx) {
    return (pos / 3.0) + dvec3(idx);
}

dvec3 goIn(dvec3 pos, const i8vec3 idx) {
    return (pos - dvec3(idx)) * 3.0;
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

bool inBlock(dvec3 pos) {
	return pos.x >= -1.5f && pos.x <= 1.5f &&
           pos.y >= -1.5f && pos.y <= 1.5f &&
		   pos.z >= -1.5f && pos.z <= 1.5f;
}

//描述了门格海绵长啥样，pos 代表的单元是实心还是空心，pos 从 0 开始。
bool inSponge(i8vec3 pos) {
    //return false;   //DEBUG
    if (pos.x != 0)
        return (pos.y != 0 || pos.z != 0);
    //else
    return (pos.y != 0 && pos.z != 0);
}

i8vec3 getIdx(dvec3 pos) {
    return i8vec3(pos + 1.5) - (i8vec3)1;
}

i8vec3 getRandIdx() {
    i8vec3 ret;
    do {
        ret = i8vec3(rand() % 3 - 1, rand() % 3 - 1, rand() % 3 - 1);
	} while (!inSponge(ret));
	return ret;
}

std::deque<i8vec3> blocks;

void initBlocks(const unsigned int rand_seed) {
	srand(rand_seed);
    for (int i = 0; i < BLOCK_AMOUNT; i++) {
		blocks.push_back(getRandIdx());
    }
}

void updateBlock(vec3 &pos) {
    while (!inBlock(pos)) {
        pos = goOut(pos, blocks.back());
		blocks.pop_back();
        std::cout << "Out.\n";
        if (blocks.size() < BLOCK_AMOUNT)
			blocks.push_front(getRandIdx());
	}
    for (i8vec3 idx; inSponge(idx = getIdx(pos)); pos = goIn(pos, idx)) {
        blocks.push_back(idx);
		std::cout << "In.\n";
    }
}

void initFractal(const unsigned int rand_seed) {
	initBlocks(rand_seed);
}

void updateFractal(vec3 &pos) {
    updateBlock(pos);
}