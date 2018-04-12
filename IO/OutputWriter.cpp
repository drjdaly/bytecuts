/*
 * MIT License
 *
 * Copyright (c) 2016, 2017 by S. Yingchareonthawornchai and J. Daly at Michigan State University
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
#include "OutputWriter.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>

using namespace std;

string Join(const string& separator, const vector<string>& vec) {
	stringstream ss;
	for (const string& s : vec) {
		ss << s << separator;
	}
	string s = ss.str();
	s.erase(s.length() - 1);
	return s;
}

bool OutputWriter::WriteCsvFile(const string& filename, const vector<string>& header, const vector<map<string, string>>& data) {
	ofstream out(filename);
	if (out.good()) {
		printf("Writing to file %s\n", filename.c_str());
	} else {
		printf("Failed to open %s\n", filename.c_str());
		return false;
	}

	out << Join(",", header) << endl;

	for (auto& m : data) {
		vector<string> line;
		for (auto& f : header) {
			line.push_back(m.at(f));
		}
		out << Join(",", line) << endl;
	}
	out.close();

	if (out.good()) {
		//printf("Done writing\n");
	} else {
		printf("Problem writing\n");
	}
	
	return out.good();
}

bool OutputWriter::WritePackets(const string& filename, const vector<vector<Point>>& packets) {
	ofstream out(filename);
	if (!out.good()) {
		printf("Failed to open %s\n", filename.c_str());
		return false;
	}
	unsigned int i = 0;
	for (auto& p : packets) {
		vector<string> line;
		for (Point x : p) {
			line.push_back(to_string(x));
		}
		line.push_back(to_string(i));
		line.push_back(to_string(i));
		out << Join("\t", line) << endl;
		i++;
	}
	out.close();
	if (!out.good()) {
		printf("Problem writing\n");
	}
	return out.good();
}

bool OutputWriter::WriteResults(const string& filename, const int* results, int numResults) {
	ofstream out(filename);
	
	if (!out.good()) {
		printf("Failed to open %s\n", filename.c_str());
		return false;
	}
	
	for (int i = 0; i < numResults; i++) {
		out << results[i] << endl;
	}
	out.close();
	if (!out.good()) {
		printf("Problem writing\n");
	}
	return out.good();
}
