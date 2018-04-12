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
#include "Common.h"
#include "IO/InputReader.h"
#include "IO/OutputWriter.h"
#include "Utilities/MapExtensions.h"

#define DisagreeLimit 5

using namespace std;

int TrueResult(const vector<Rule>& rules, const Packet & packet) {
	for (size_t i = 0; i < rules.size(); i++) {
		if (rules[i].MatchesPacket(packet)) {
			return rules[i].priority; //.size() - i;
		}
	}
	return -1;
}

int main(int argc, char* argv[]) {
	
	unordered_map<string, string> args = ParseArgs(argc, argv);
	
	string infile = args["Rules"];
	string packetFile = args["Packets"];
	
	vector<Rule> rules = InputReader::ReadFilterFile(infile);
	printf("%lu rules\n", rules.size());
	vector<Packet> packets = InputReader::ReadPackets(packetFile);
	printf("%lu packets\n", packets.size());
	
	unordered_map<string, vector<int>> algs;
	
	for (auto pair : args) {
		string alg = pair.first;
		if (alg != "Rules" && alg != "Packets") {
			algs[alg] = InputReader::ReadResults(pair.second);
		}
	}
	
	int numDisagree = 0;
	for (size_t i = 0; i < packets.size(); i++) {
		unordered_map<string, int> results;
		int result = -1;
		
		for (auto &pair : algs) {
			results[pair.first] = pair.second[i];
			result = pair.second[i];
		}
		
		if (!all_of(results.begin(), results.end(), [=](const auto& pair) { return pair.second == result; })) {
			printf("Disagreement! i = %lu\n", i);
			for (size_t d = 0; d < NumDims; d++) {
				printf("%u ", packets[i][d]);
			}
			printf("\n");
			numDisagree++;
			for (auto pair : results) {
				printf("%s : %d\n", pair.first.c_str(), pair.second);
			}
			printf("Truth : %d\n", TrueResult(rules, packets[i]));
			if (numDisagree > DisagreeLimit) {
				exit(1);
			}
		}
	}
	
	if (numDisagree == 0) {
		printf("**All classifiers are in accord**\n");
	}
	
	return 0;
}
