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
#include "ByteCuts.h"

#include "../Utilities/MapExtensions.h"

#include <limits>
#include <unordered_map>
#include <unordered_set>

using namespace std;

#define MaxDelta 16

ByteCutsClassifier::ByteCutsClassifier(const vector<Rule>& rules, const vector<ByteCutsNode*>& trees, const vector<int>& priorities, const vector<size_t>& sizes) : 
		rules(rules), trees(trees), priorities(priorities), sizes(sizes) {
}

ByteCutsClassifier::ByteCutsClassifier(const unordered_map<string, string>& args) 
	: dredgeFraction(GetDoubleOrElse(args, "BC.BadFraction", 0.02)),
	turningPoint(GetDoubleOrElse(args, "BC.TurningPoint", 0.01)),
	minFrac(GetDoubleOrElse(args, "BC.MinFraction", 0.75)) {
}

ByteCutsClassifier::~ByteCutsClassifier() { 
	for (ByteCutsNode* n : trees) {
		delete n;
	}
}

vector<Rule> ByteCutsClassifier::Separate(const vector<Rule>& rules, vector<Rule>& remain) {
	int bestDim = -1;
	uint8_t bestLen = 0;
	size_t bestCost = numeric_limits<size_t>::max();
	size_t bestPart = numeric_limits<size_t>::max();
	size_t bestRemain = numeric_limits<size_t>::max();
	
	
	vector<int> dims = {FieldSA, FieldDA};
	for (int d : dims) {
		for (uint8_t len = BitsPerNybble; len <= BitsPerField; len += BitsPerNybble) {
			unordered_map<Point, size_t> counts;
			size_t dropped = 0;
			
			for (const Rule& r : rules) {
				if (r.prefix_length[d] >= len) {
					Point x = r.range[d].low & (0xFFFFFFFF << (BitsPerField - len));
					counts[x]++;
				} else {
					dropped++;
				}
			}
			size_t maxPart = 0;
			for (auto pair : counts) {
				maxPart = max(maxPart, pair.second);
			}
			
			size_t cost = dropped + maxPart;;
			size_t kept = rules.size() - dropped;
			double ratioIn = maxPart * 1.0 / kept;
			double ratioOut = dropped * 1.0 / rules.size();
			bool betterPartition = maxPart < bestPart;
			bool betterRemain = dropped < bestRemain;
			bool goodPartition = maxPart <= bestPart;
			bool goodRemain = dropped <= bestRemain;
			
			bool better;
			if (goodPartition && goodRemain) {
				better = true;
			} else if (!goodPartition && !goodRemain) {
				better = false;
			} else {
				if (ratioIn < turningPoint) {
					better = betterRemain || (goodRemain && betterPartition);
				} else if (ratioOut < (1 - minFrac)) {
					better = betterPartition || (goodPartition && betterRemain);;
				} else {
					better = cost < bestCost;
				}
			}

			if (better) {
				bestDim = d;
				bestLen = len;
				bestCost = cost;
				bestPart = maxPart;
				bestRemain = dropped;
			}
		}
	}
	vector<Rule> results;
	for (const Rule& r : rules) {
		if (r.prefix_length[bestDim] >= bestLen) {
			results.push_back(r);
		} else {
			remain.push_back(r);
		}
	}
	
	return results;
}

void ByteCutsClassifier::ConstructClassifier(const std::vector<Rule>& rules) {
	this->rules = rules;
	SortRules(this->rules);
	
	vector<Rule> rl = this->rules;
	vector<vector<Rule>> parts;
	
	while (rl.size() > rules.size() * dredgeFraction) {
		vector<Rule> remain;
		parts.push_back(Separate(rl, remain));
		if (remain.size() == rl.size()) break;
		rl = remain;
	}
	
	for (vector<Rule>& part : parts) {
		BuildTree(part);
	}
	BuildBadTree(rl);
}

void ByteCutsClassifier::BuildTree(const vector<Rule>& rules) {
	vector<Rule> rl = rules;
	while (!rl.empty()) {
		goodTrees++;
		TreeBuilder bc(8);
		vector<Rule> remain;
		trees.push_back(bc.BuildPrimaryRoot(rl, remain));
		int priority = max_element(rl.begin(), rl.end(), [](auto rx, auto ry) { return rx.priority < ry.priority; })->priority;
		priorities.push_back(priority);
		sizes.push_back(rl.size());
		rl = remain;
		
	}
}

void ByteCutsClassifier::BuildBadTree(const vector<Rule>& rules) {
	vector<Rule> rl = rules;
	while (!rl.empty()) {
		badTrees++;
		TreeBuilder bc(8);
		vector<Rule> remain;
		trees.push_back(bc.BuildSecondaryRoot(rl, remain));
		int priority = max_element(rl.begin(), rl.end(), [](auto rx, auto ry) { return rx.priority < ry.priority; })->priority;
		priorities.push_back(priority);
		sizes.push_back(rl.size());
		rl = remain;
	}
}

int ByteCutsClassifier::ClassifyAPacket(const Packet& packet) {
	int result = -1;
	for (size_t i = 0; i < trees.size(); i++) {
		if (priorities[i] > result) {
			ByteCutsNode* tree = trees[i];
			result = max(result, tree->ClassifyAPacket(packet));
		}
	}
	return result;
}

bool ByteCutsClassifier::IsWideAddress(Interval s) const {
	return (s.low + 0xFFFF) < s.high;
}

