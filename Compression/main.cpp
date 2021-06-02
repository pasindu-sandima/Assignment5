#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <bitset>
#include <sstream>

//using namespace std;
using std::pair; using std::vector; using std::uint32_t;
using std::map; using std::string; using std::cout; using std::endl;
using std::sort; using std::ifstream; using std::bitset;
using std::stringstream;

stringstream ss;

bool compSetNum(pair<uint32_t, pair<int,int>>& a, pair<uint32_t, pair<int, int>>& b)
{
	return a.second.first < b.second.first;
}

bool compVal(pair<uint32_t, pair<int, int>>& a, pair<uint32_t, pair<int, int>>& b)
{
	return a.second.second > b.second.second;
}

bool cmpMismatch(pair<uint32_t, vector<unsigned int>>& a, pair<uint32_t, vector<unsigned int>>& b) {
	return a.second.size() < b.second.size();
}

uint32_t BinStringtoInt(string& s) {
	uint32_t val = 0;
	for (unsigned int i = 0; i < s.size(); i++) {
		val *= 2;
		val += (s[i] - '0');
	}
	return val;
}

vector<uint32_t> ReadFile(string fileName) {
	ifstream MyReadFile(fileName);
	string tempS;
	uint32_t tempInt;
	vector<uint32_t> binaryVect;

	if (MyReadFile.fail()) {
		cout << "Failed to open the file" << endl;
	}

	while (getline(MyReadFile, tempS)) {
		tempInt = BinStringtoInt(tempS);
		binaryVect.push_back(tempInt);
	}
	return binaryVect;

}

vector<uint32_t> generateDict(vector<uint32_t>& binaryVect) {
	vector<uint32_t> dictVect;

	map<uint32_t, pair<int,int>> count;

	int setCounter = 0;
	for (uint32_t i : binaryVect) {
		if (count[i].second++ == 0) {
			count[i].first = setCounter++;
		}	
	}


	vector < pair<uint32_t, pair<int, int>>> sortVect;
	for (auto& m : count) {
		sortVect.push_back(m);
	}

	/*for (auto m : count) {
		cout << m.first << " :" << m.second.second << " " << m.second.first << endl << endl;
	}*/
	
	sort(sortVect.begin(), sortVect.end(), compSetNum);
	sort(sortVect.begin(), sortVect.end(), compVal);


	int i = 1;
	for (auto m : sortVect) {
		dictVect.push_back(m.first);
		//cout << m.first << " :" << m.second.second << " " << m.second.first << endl;
		if (i++ >= 16) break;
	}

	return dictVect;
}

void findNumMismatch(uint32_t curBinary, vector<uint32_t>& dictVect ) {
	unsigned int lowMis = 32;
	
	const uint32_t k = 1 << 31U;
	vector<pair<uint32_t, vector<unsigned int>>> mismatches;  // dictionary item, vector<mismatch index> 

	for (unsigned int i = 0; i < dictVect.size(); i++) {
		unsigned int numMis = 0;
		uint32_t  XORed = dictVect[i] ^ curBinary;
		vector<unsigned int> misPos;
		for (unsigned int j = 0; j < 32; j++) {
			if ((XORed << j) & k) {
				numMis++;
				misPos.push_back(j);
			}
			if (numMis > 4) break;
		} 
		if (numMis < lowMis) lowMis = numMis;
		
		if (numMis < 5) {
			mismatches.push_back(make_pair(i, misPos));
		}

		if (numMis == 1) break;
	}
	if (lowMis > 4) {
		ss << bitset<3>(0) << bitset<32>(curBinary);
	}
	else {
		unsigned cost = 10;
		stringstream costss;
		unsigned costIndex = 15;
		sort(mismatches.begin(), mismatches.end(), cmpMismatch);

		if (lowMis == 1) {
			//Compress for 1 bit mismatch
			cost = 2;
			costss << bitset<3>(3) << bitset<5>(mismatches[0].second[0]) << bitset<4>(mismatches[0].first);
		}
		else {
			for (auto& i : mismatches) {
				if (i.second.size() == 2) {
					if ((i.second[1] - i.second[0]) == 1) {
						if (i.first < costIndex) {
							cost = 2;
							costIndex = i.first;
							//compress for 2 bit consecutive
							costss.str(string());
							costss << bitset<3>(4) << bitset<5>(i.second[0]) << bitset<4>(i.first);
						}
						
					}
					else if ((i.second[1] - i.second[0]) < 4) {
						if (cost >= 5 && i.first < costIndex) {
							cost = 5;
							costIndex = i.first;
							unsigned bitmask = (1 << (3 - (i.second[1]-i.second[0]))) | (1 << 3);
							//compress for 4 bit bitmask with 2 bit mismatches
							costss.str(string());
							costss << bitset<3>(2) << bitset<5>(i.second[0]) << bitset<4>(bitmask) << bitset<4>(i.first);
							
						}
					}
					else {
						if (cost >= 6 && i.first < costIndex) {
							cost = 6;
							costIndex = i.first;
							//compress for 2 bit anywhere
							costss.str(string());
							costss << bitset<3>(6) << bitset<5>(i.second[0]) << bitset<5>(i.second[1]) << bitset<4>(i.first);
						}
					}
				}
				else if (i.second.size() == 3) {
					if ((i.second[2] - i.second[0]) < 4) {
						if (cost >= 5 && i.first < costIndex) {
							unsigned bitmask = (1 << (3 - (i.second[1] - i.second[0]))) | (1 << (3 - (i.second[2] - i.second[0]))) | (1 << 3);
							cost = 5;
							costIndex = i.first;
							//compress for 4 bit bitmask with 3 bit mismatches
							costss.str(string());
							costss << bitset<3>(2) << bitset<5>(i.second[0]) << bitset<4>(bitmask) << bitset<4>(i.first);
						}
					}
				}
				else {
					if ((i.second[3] - i.second[0]) == 3) {
						if (cost >= 4  && i.first < costIndex) {
							cost = 4;
							costIndex = i.first;
							//compress for 4 bit consecutive
							costss.str(string());
							costss << bitset<3>(5) << bitset<5>(i.second[0]) << bitset<4>(i.first);
						}
					}

				}
			}

			if (cost > 6) {
				costss << bitset<3>(0) << bitset<32>(curBinary);
			}
		}
		ss << costss.str();

	}

}

void compress(vector<uint32_t>& binaryVect, vector<uint32_t>& dictVect) {

	unsigned int RLEcount = 0;
	unsigned int vectSize = binaryVect.size();
	uint32_t curBinary;

	for (unsigned i= 0; i < vectSize ; i++) {
		curBinary = binaryVect[i];
		if (RLEcount == 0) {
			// TODO ----- implement compression except RLE
			// 1. Check for Direct Mapping
			auto findIt = find(dictVect.begin(), dictVect.end(), curBinary);
			if (findIt != dictVect.end()) {
				//cout << "Direct Mapping at index " << findIt - dictVect.begin() << endl;
				// Compress for direct mapping
				ss << bitset<3>(7) << bitset<4>(findIt - dictVect.begin());
			}
			else
			{
				//Find number of mismatches with dict entries
				findNumMismatch(curBinary, dictVect);

			}

		}
		if ((i+1) < vectSize && curBinary == binaryVect[i + 1]) {
			if (RLEcount == 8) {
				//Implement RLC encoding
				ss << bitset<3>(1) << bitset<3>(7);
				RLEcount = 0;

			}
			else
			{
				RLEcount++;
			}
		}
		else if(RLEcount != 0)
		{	
			// Implement RLE encoding for RLEcount
			ss << bitset<3>(1) << bitset<3>(RLEcount-1);
			RLEcount = 0;

		}
	}
}



int main() {

	vector<uint32_t> binaryVect = ReadFile("original.txt");

	vector<uint32_t> dictVect = generateDict(binaryVect);

	compress(binaryVect,dictVect);

	unsigned padsize = ss.str().length();
	padsize = padsize % 32; 

	for (unsigned int i = 0; i < ss.str().length(); i++) {
		cout << ss.str()[i];
		if (i % 32 == 31) cout << endl;
	}

	cout << string(32 - padsize, '0') << endl;
	



	cout << "xxxx" << endl;
	for (auto& i : dictVect) {
		cout << bitset<32>(i) << endl;
	}
	cout << endl;

	return 0;
}