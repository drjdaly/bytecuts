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

#include "ByteCuts/ByteCuts.h"
#include "IO/InputReader.h"
#include "IO/OutputWriter.h"
#include "Utilities/MapExtensions.h"
#include "Utilities/VectorExtensions.h"

#include <map>
#include <string>

using namespace std;
using namespace std::chrono;

int main(int argc, char* argv[]) {
	printf("Hello, world.\n");
	
	unordered_map<string, string> args = ParseArgs(argc, argv);
	
	string infile = args["Rules"];
	string packetFile = args["Packets"];
	string resultsFile = GetOrElse(args, "Results", "");
	string statsFile = args["Stats"];
	
	time_point<steady_clock> start, end;
	duration<double> elapsedSeconds;
	duration<double,std::milli> elapsedMilliseconds;
	
	map<string, string> data;
	data["Name"] = "ByteCuts";
	
	vector<Rule> rules = InputReader::ReadFilterFile(infile);
	printf("%lu rules\n", rules.size());
	vector<Packet> packets = InputReader::ReadPackets(packetFile);
	printf("%lu packets\n", packets.size());
	
	printf("Constructing!\n");
	start = steady_clock::now();
	//ByteCutsClassifier bc(args);
	//SpanCutsClassifier bc(args);
	ByteCutsClassifier bc(args);
	bc.ConstructClassifier(rules);
	//StepCuts sc(8);
	//ByteCutsClassifier bc = sc.ConstructClassifier(rules);
	
	end = steady_clock::now();
	elapsedMilliseconds = end - start;
	elapsedSeconds = end - start;
	printf("\tConstruction time: %f ms\n", elapsedMilliseconds.count());
	data["Build"] = to_string(elapsedSeconds.count());
	
	printf("Testing!\n");
	int* results = new int[packets.size()];
	int i = 0;
	start = steady_clock::now();
	for (Packet p : packets) {
		results[i++] = bc.ClassifyAPacket(p);
	}
	end = steady_clock::now();
	elapsedMilliseconds = end - start;
	elapsedSeconds = end - start;
	printf("\tClassification time: %f ms\n", elapsedMilliseconds.count());
	data["Classify"] = to_string(elapsedSeconds.count());
	
	Memory memBytes = bc.MemSizeBytes();
	printf("\tMemory: %d B\n", memBytes);
	printf("\tMemory: %.2f MiB\n", memBytes / (1024 * 1024.0));
	data["Memory"] = to_string(memBytes);
	
	printf("\tTrees: %lu\n", bc.NumTables());
	data["Trees"] = to_string(bc.NumTables());
	int height = 0;
	int maxHeight = 0;
	int cost = 0;
	int maxCost = 0;
	vector<int> heights;
	vector<int> costs;
	vector<int> priors;
	for (size_t i = 0; i < bc.NumTables(); i++) {
		int h = bc.HeightOfTree(i);
		int c = bc.CostOfTree(i);
		printf("Height:  %d / %d\n", h, c);
		height += h;
		maxHeight = max(h, maxHeight);
		cost += c;
		maxCost = max(c, maxCost);
		heights.push_back(h);
		costs.push_back(c);
		priors.push_back(bc.PriorityOfTable(i));
	}
	data["MaxHeight"] = to_string(maxHeight);
	data["SumHeight"] = to_string(height);
	data["Heights"] = Join("-", heights);
	data["MaxCost"] = to_string(maxCost);
	data["SumCost"] = to_string(cost);
	data["Costs"] = Join("-", costs);
	data["Priors"] = Join("-", priors);
	
	size_t firstSize = bc.RulesInTable(0);
	printf("\tRules In First Tree: %lu (%.2f%%)\n", firstSize, 100.0 * firstSize / rules.size());
	data["FirstSize"] = to_string(1.0 * firstSize / rules.size());
	
	size_t rulesFound = 0;
	size_t numTables = 0;
	size_t rules90 = 0.9 * rules.size();
	size_t rules95 = 0.95 * rules.size();
	size_t rules99 = 0.99 * rules.size();
	
	while (rulesFound < rules90) {
		rulesFound += bc.RulesInTable(numTables++);
	}
	data["Table90"] = to_string(numTables);
	while (rulesFound < rules95) {
		rulesFound += bc.RulesInTable(numTables++);
	}
	data["Table95"] = to_string(numTables);
	while (rulesFound < rules99) {
		rulesFound += bc.RulesInTable(numTables++);
	}
	data["Table99"] = to_string(numTables);
	
	data["GoodTrees"] = to_string(bc.NumGoodTrees());
	data["BadTrees"] = to_string(bc.NumBadTrees());
	
	printf("Done testing: %d.\n", i);
	
	if (!resultsFile.empty()) {
		OutputWriter::WriteResults(resultsFile, results, packets.size());
	}
	
	delete [] results;
	for (Packet p : packets) {
		delete [] p;
	}
	
	
	
	printf("Writing statistics\n");
	vector<string> header = {"Name", "Build", "Classify", "Memory", "MaxHeight", "SumHeight", "MaxCost", "SumCost", "Trees", "FirstSize", "Table90", "Table95", "Table99", "Heights", "Costs", "Priors", "BadTrees", "GoodTrees"};
	vector<map<string, string>> multidata = {data};
	OutputWriter::WriteCsvFile(statsFile, header, multidata);
	
	printf("Goodbye, world.\n");
	return 0;
}
