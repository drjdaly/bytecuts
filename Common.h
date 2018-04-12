/*
 * MIT License
 *
 * Copyright (c) 2017 by J. Daly at Michigan State University
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#pragma once
#ifndef  COMMON_H
#define  COMMON_H
#include <vector>
#include <queue>
#include <list>
#include <set>
#include <iostream>
#include <algorithm>
#include <random>
#include <numeric>
#include <memory>
#include <chrono> 
#include <array>

#define NumDims 5

#define FieldSA 0
#define FieldDA 1
#define FieldSP 2
#define FieldDP 3
#define FieldProto 4

#define LowDim 0
#define HighDim 1
 
#define POINT_SIZE_BITS 32

typedef uint32_t Point;
typedef Point* Packet;

typedef uint32_t Memory;

struct Interval {
	Point low;
	Point high;
};

struct Rule
{
	int	priority;

	//int id;
	//int tag;
	bool markedDelete = 0;

	unsigned prefix_length[NumDims];

	Interval range[NumDims];

	bool inline MatchesPacket(const Packet p) const {
		for (int i = 0; i < NumDims; i++) {
			if (p[i] < range[i].low || p[i] > range[i].high) return false;
		}
		return true;
	}
	
	bool inline IntersectsRule(const Rule& r) const {
		for (int i = 0; i < NumDims; i++) {
			if (range[i].high < r.range[i].low || range[i].low > r.range[i].high) return false;
		}
		return true;
	}

	void Print() const {
		for (int i = 0; i < NumDims; i++) {
			printf("%u:%u ", range[i].low, range[i].high);
		}
		printf("\n");
	}
};

class Random {
public:
	// random number generator from Stroustrup: 
	// http://www.stroustrup.com/C++11FAQ.html#std-random
	// static: there is only one initialization (and therefore seed).
	static int random_int(int low, int high)
	{
		//static std::mt19937  generator;
		using Dist = std::uniform_int_distribution < int >;
		static Dist uid{};
		return uid(generator, Dist::param_type{ low, high });
	}

	// random number generator from Stroustrup: 
	// http://www.stroustrup.com/C++11FAQ.html#std-random
	// static: there is only one initialization (and therefore seed).
	static int random_unsigned_int()
	{
		//static std::mt19937  generator;
		using Dist = std::uniform_int_distribution < unsigned int >;
		static Dist uid{};
		return uid(generator, Dist::param_type{ 0, 4294967295 });
	}
	static double random_real_btw_0_1()
	{
		//static std::mt19937  generator;
		using Dist = std::uniform_real_distribution < double >;
		static Dist uid{};
		return uid(generator, Dist::param_type{ 0,1 });
	}

	template <class T>
	static std::vector<T> shuffle_vector(std::vector<T> vi) {
		//static std::mt19937  generator;
		std::shuffle(std::begin(vi), std::end(vi), generator);
		return vi;
	}
private:
	static std::mt19937 generator;
};

inline void SortRules(std::vector<Rule>& rules) {
	sort(rules.begin(), rules.end(), [](const Rule& rx, const Rule& ry) { return rx.priority > ry.priority; });
}

inline void SortRules(std::vector<Rule*>& rules) {
	sort(rules.begin(), rules.end(), [](const Rule* rx, const Rule* ry) { return rx->priority > ry->priority; });
}

inline void PrintRules(const std::vector<Rule>& rules) {
	for (const Rule& r : rules) {
		r.Print();
	}
}

#endif
