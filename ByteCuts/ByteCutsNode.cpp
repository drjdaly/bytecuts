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
#include "ByteCutsNode.h"

#include "../Utilities/MapExtensions.h"

#include <limits>
#include <unordered_map>
#include <unordered_set>

using namespace std;

#define MaxDelta 16

//************
// ByteCutsNode
//************

void ByteCutsNode::LeafNode(ByteCutsNode& self, const vector<Rule>& rules) {
	self.mode = Leaf;
	self.numRules = rules.size();
	self.rules = new Rule[rules.size()];
	for (size_t i = 0; i < rules.size(); i++) {
		self.rules[i] = rules[i];
	}
	
}
void ByteCutsNode::SplitNode(ByteCutsNode& self, uint8_t dim, uint16_t point, ByteCutsNode* left, ByteCutsNode* right) {
	self.mode = Split;
	self.dim = dim;
	self.splitPoint = point;
	self.children = new ByteCutsNode*[2];
	self.children[0] = left;
	self.children[1] = right;
}

void ByteCutsNode::CutNode(ByteCutsNode& self, uint8_t dim, uint8_t left, uint8_t right, ByteCutsNode** children) {
	self.mode = Cut;
	self.dim = dim;
	self.cutInfo.cutLow = right;
	self.cutInfo.cutTotal = left + right;
	self.children = children;
}

ByteCutsNode::ByteCutsNode() {
	// TODO
}

ByteCutsNode::~ByteCutsNode() {
	if (mode == Leaf) {
		delete [] rules;
	} else if (mode == Cut) {
		size_t numChildren = NumChildren();
		
		unordered_set<ByteCutsNode*> uniqueChildren;
		for (size_t i = 0; i < numChildren; i++) {
			uniqueChildren.insert(children[i]);
		}
		for (auto c : uniqueChildren) {
			delete c;
		}
		delete [] children;
	} else {
		delete children[0];
		delete children[1];
		delete [] children;
	}
}

int ByteCutsNode::ClassifyAPacket(const Packet& p) const {
	switch (mode) {
		case Cut:
			return children[IndexPacket(p)]->ClassifyAPacket(p);
		case Split:
			if (p[dim] <= splitPoint) {
				return children[0]->ClassifyAPacket(p);
			} else {
				return children[1]->ClassifyAPacket(p);
			}
		case Leaf:
			for (uint16_t i = 0; i < numRules; i++) {
				if (rules[i].MatchesPacket(p)) {
					return rules[i].priority;
				}
			}
			return -1;
			break;
	}
}

int ByteCutsNode::Size(int ruleSize) const {
	int result = NodeSize;
	switch (mode) {
		case Cut:
			{
				size_t numChildren = NumChildren();
				unordered_set<ByteCutsNode*> uchildren(children, children + numChildren);
				for (ByteCutsNode* child : uchildren) {
					result += child->Size(ruleSize);
				}
			}
			break;
		case Split:
			result += children[0]->Size(ruleSize);
			result += children[1]->Size(ruleSize);
			break;
		case Leaf:
			result += numRules * ruleSize;
			break;
	}
	return result;
	
}

int ByteCutsNode::Height() const {
	switch (mode) {
		case Cut:
			{
				size_t numChildren = NumChildren();
				int maxHeight = 0;
				unordered_set<ByteCutsNode*> uchildren(children, children + numChildren);
				for (ByteCutsNode* child : uchildren) {
					maxHeight = max(maxHeight, child->Height());
				}
				return maxHeight + 1;
			}
			break;
		case Split:
			return max(children[0]->Height(), children[1]->Height()) + 1;
			break;
		case Leaf:
			return 1;
			break;
	}
}

int ByteCutsNode::Cost() const {
	switch (mode) {
		case Cut:
			{
				size_t numChildren = NumChildren();
				int maxHeight = 0;
				unordered_set<ByteCutsNode*> uchildren(children, children + numChildren);
				for (ByteCutsNode* child : uchildren) {
					maxHeight = max(maxHeight, child->Cost());
				}
				return maxHeight + 1;
			}
			break;
		case Split:
			return max(children[0]->Height(), children[1]->Cost()) + 1;
			break;
		case Leaf:
			return numRules;
			break;
	}
}

size_t ByteCutsNode::IndexPacket(const Packet& p) const {
	Point pt = p[dim];
	Point mask = 0xFFFFFFFFu >> (cutInfo.cutTotal);
	return (pt >> cutInfo.cutLow) & mask;
}

