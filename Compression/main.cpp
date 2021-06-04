#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <bitset>
#include <sstream>

using std::pair; using std::vector; using std::uint32_t;
using std::map; using std::string; using std::cout; using std::endl;
using std::sort; using std::stable_sort; using std::ifstream; using std::bitset;
using std::stringstream; using std::ofstream; using std::cin;

// comp func to sort the binaries using index of entry
bool compSetNum(const pair<uint32_t, pair<unsigned int, unsigned int>>& a,const  pair<uint32_t, pair<unsigned int, unsigned int>>& b)
{
	return a.second.first < b.second.first;
}

//comp func to sort the binaries using the frequency of occurence
bool compVal(const pair<uint32_t, pair<unsigned int, unsigned int>>& a,const pair<uint32_t, pair<unsigned int, unsigned int>>& b)
{
	return a.second.second > b.second.second;
}

// comp func to sort the dictionary entries using the number of mismatches with a binary
bool cmpMismatch(const pair<uint32_t, vector<unsigned int>>& a,const pair<uint32_t, vector<unsigned int>>& b) {
	return a.second.size() < b.second.size();
}


stringstream compress(vector<uint32_t>& binaryVect, vector<uint32_t>& dictVect);
void findNumMismatch(uint32_t& curBinary, vector<uint32_t>& dictVect, stringstream& ss);
vector<uint32_t> generateDict(vector<uint32_t>& binaryVect);
vector<uint32_t> ReadOriginalFile(string fileName);
void WriteCompressedFile(vector<uint32_t>& dictVect, stringstream& ss);
uint32_t BinStringtoInt(string& s);
vector<uint32_t> ReadCompressedFile(string fileName, stringstream& ss);
void decompress(vector<uint32_t> dictVect, stringstream& readSS,ofstream& writeSS);


int main(int argc, char *argv[]) {

	if (argc != 2) {
		cout << "Invalid Number of Arguments" << endl;
		return 0;
	}

	

	unsigned problem = atoi(argv[1]);

	if (problem == 1) {
		vector<uint32_t> binaryVect = ReadOriginalFile("original.txt");
		vector<uint32_t> dictVect = generateDict(binaryVect);
		stringstream ss = compress(binaryVect, dictVect);
		WriteCompressedFile(dictVect, ss);
	}
	else if (problem == 2) {
		stringstream readSS;
		vector<uint32_t> dictVect = ReadCompressedFile("compressed.txt", readSS);
		ofstream writeSS("dout.txt");
		decompress(dictVect, readSS,writeSS);
	}
	else {
		cout << "Invalid Argument" << endl;
	}
	return 0;
}

/**
 * @brief Converts a binary string to an Integer
 * 
 * @param s - binary string which consists of '1's and '0's
 * @return uint32_t 
 */
uint32_t BinStringtoInt(string& s) {
	uint32_t val = 0;
	for (unsigned int i = 0; i < s.size(); i++) {
		val *= 2;
		val += (s[i] - '0');
	}
	return val;
}

/**
 * @brief This reads the file with the original binaries, converts the binaries to unsigned int
 * and returns the binaries using a vector
 * 
 * @param fileName - filename which consists the original binaries
 * @return vector<uint32_t> - vector of binaries 
 */
vector<uint32_t> ReadOriginalFile(string fileName) {
	ifstream ReadFStream(fileName);
	string tempS;
	uint32_t tempInt;
	vector<uint32_t> binaryVect;

	if (ReadFStream.fail()) {
		cout << "Failed to open the file" << endl;
	}

	while (getline(ReadFStream, tempS)) {
		tempInt = BinStringtoInt(tempS);
		binaryVect.push_back(tempInt);
	}
	return binaryVect;
}

/**
 * @brief Writes the compressed binaries and the dictionary entries in the required format to "cout.txt"
 * 
 * @param dictVect - vector of dictionary entries
 * @param ss - stringstream of compressed binaries
 */
void WriteCompressedFile(vector<uint32_t>& dictVect, stringstream& ss) {
	unsigned padsize = ss.str().length();
	padsize = padsize % 32;

	ofstream WriteFileStream("cout.txt");

	for (unsigned int i = 0; i < ss.str().length(); i++) {
		WriteFileStream << ss.str()[i];
		if (i % 32 == 31) WriteFileStream << endl;
	}

	WriteFileStream << string(32 - padsize, '0') << endl;

	WriteFileStream << "xxxx" << endl;
	for (auto& i : dictVect) {
		WriteFileStream << bitset<32>(i) << endl;
	}
}

/**
 * @brief generates the dictionary entries for a set of binaries
 * 
 * @param binaryVect - set of binaries to generate the dictionary
 * @return vector<uint32_t> - return the dictionary as a vector
 */
vector<uint32_t> generateDict(vector<uint32_t>& binaryVect) {
	vector<uint32_t> dictVect;

	map<uint32_t, pair<unsigned int, unsigned int>> count;

	unsigned int setCounter = 0;
	for (uint32_t i : binaryVect) {
		if (count[i].second++ == 0) {
			count[i].first = setCounter++;
		}
	}

	vector < pair<uint32_t, pair<unsigned int, unsigned int>>> sortVect;
	for (auto& m : count) {
		sortVect.push_back(m);
	}

	//sorts in the order they appear in the original binary
	sort(sortVect.begin(), sortVect.end(), compSetNum);
	//sorts in the order of the frequency 
	stable_sort(sortVect.begin(), sortVect.end(), compVal);


	int i = 1;
	for (auto m : sortVect) {
		dictVect.push_back(m.first);
		if (i++ >= 16) break;
	}

	return dictVect;
}

/**
 * @brief Finds the number of mismatches for a given binary with the set of dictionary entires and compresses
 * binary accordingly. 
 * 
 * @param curBinary - binary to be compressed
 * @param dictVect - set of dictionary entries
 * @param ss - stream to add the compressed binary
 */
void findNumMismatch(uint32_t& curBinary, vector<uint32_t>& dictVect, stringstream& ss) {
	unsigned int lowMis = 32;

	const uint32_t k = 1 << 31U;
	vector<pair<uint32_t, vector<unsigned int>>> mismatches;  // dictionary item, vector<mismatch indices> 

	//count the number of mismatches between current binary and dictionary entries
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
		unsigned costIndex = 15;  //Index of the dictionary item cost is assigned
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
					else if ((i.second[1] - i.second[0]) < 4  && i.second[0] <29 ) {
						if (cost >= 5 && i.first < costIndex) {
							cost = 5;
							costIndex = i.first;
							unsigned bitmask = (1 << (3 - (i.second[1] - i.second[0]))) | (1 << 3);
							//compress for bitmask based compression with 2 bit mismatches
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
					if ((i.second[2] - i.second[0]) < 4 && i.second[0] <29) {
						if (cost >= 5 && i.first < costIndex) {
							unsigned bitmask = (1 << (3 - (i.second[1] - i.second[0]))) | (1 << (3 - (i.second[2] - i.second[0]))) | (1 << 3);
							cost = 5;
							costIndex = i.first;
							//compress for bitmask based compression with 3 bit mismatches
							costss.str(string());
							costss << bitset<3>(2) << bitset<5>(i.second[0]) << bitset<4>(bitmask) << bitset<4>(i.first);
						}
					}
				}
				else {
					if ((i.second[3] - i.second[0]) == 3) {
						if (cost >= 4 && i.first < costIndex) {
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

/**
 * @brief Main compression algorithm. Searches for a direct match, and running length encodings. 
 * If direct match is not found, findNumMismatch is called to compress the binary using other formats. 
 * 
 * @param binaryVect - set of binaries
 * @param dictVect  - set of dictionary entries
 * @return stringstream - return a stream of compressed binaries to be written to a file
 */
stringstream compress(vector<uint32_t>& binaryVect, vector<uint32_t>& dictVect) {

	unsigned int RLEcount = 0;
	unsigned int vectSize = binaryVect.size();
	uint32_t curBinary;
	stringstream ss;

	for (unsigned i = 0; i < vectSize; i++) {
		curBinary = binaryVect[i];
		if (RLEcount == 0) {
			// 1. Check for Direct Mapping
			auto findIt = find(dictVect.begin(), dictVect.end(), curBinary);
			if (findIt != dictVect.end()) {
				// Compress for direct mapping
				ss << bitset<3>(7) << bitset<4>(findIt - dictVect.begin());
			}
			else
			{
				//Find number of mismatches with dict entries and compress accordingly
				findNumMismatch(curBinary, dictVect,ss);
			}

		}
		if ((i + 1) < vectSize && curBinary == binaryVect[i + 1]) {
			if (RLEcount == 8) {
				//Implement RLE encoding when the limit each reached
				ss << bitset<3>(1) << bitset<3>(7);
				RLEcount = 0;

			}
			else
			{
				RLEcount++;
			}
		}
		else if (RLEcount != 0)
		{
			// Implement RLE encoding for RLEcount when the sequence is ended
			ss << bitset<3>(1) << bitset<3>(RLEcount - 1);
			RLEcount = 0;

		}
	}
	return ss;
}


/**
 * @brief Reads the compressed file and returns the set of dictionary entries.
 * The compressed binaries are saved to a stringstream
 * 
 * @param fileName - Filename of the compressed binary
 * @param ss - stringstream to save the compressed binaries
 * @return vector<uint32_t> 
 */
vector<uint32_t> ReadCompressedFile(string fileName, stringstream& ss) {
	ifstream ReadFStream(fileName);
	string tempS;
	unsigned tempInt;
	vector<uint32_t> dictVect;

	if (ReadFStream.fail()) {
		cout << "Failed to open the file" << endl;
	}

	while (getline(ReadFStream, tempS)) {
		if (tempS == "xxxx") break;
		ss << tempS;
	}
	
	while (getline(ReadFStream, tempS)) {
		tempInt = BinStringtoInt(tempS);
		dictVect.push_back(tempInt);
	}
	return dictVect;

}

/**
 * @brief Performs decompression for a given compressed binary and a set of dictionary entries. 
 * 
 * @param dictVect - set of dictionary enties
 * @param readSS - stream of compressed binaries
 * @param writeSS - stream to write the original binaries
 */
void decompress(vector<uint32_t> dictVect, stringstream& readSS,ofstream& writeSS) {

	bitset<3> prefixB;
	bitset<32> binaryB;
	bitset<5> locationB,locationB2;
	bitset<4> bitmaskB;
	bitset<4> dictInB;
	bitset<3> RLEB;

	unsigned sslen = readSS.str().length();
	unsigned curlen = 0;
	//decompress the stream of binaries by going through the prefix and decoding accordingly
	while (curlen + 3 < sslen) {
		readSS >> prefixB;
		curlen += 3;

		switch (prefixB.to_ulong())
		{
		case 0: //When no compression has happened
			if (curlen + 32 > sslen) break;
			curlen += 32;
			readSS >> binaryB;
			writeSS << binaryB << endl;
			continue;
		case 1://When RLE encoding 
			if (curlen + 3 > sslen) break;
			curlen += 3;
			readSS >> RLEB;
			for (unsigned i = 0; i <= RLEB.to_ulong(); i++) {
				writeSS << binaryB << endl;
			}
			continue;
		case 2://When bitmask based compression
			if (curlen + 13 > sslen) break;
			curlen += 13;
			readSS >> locationB >> bitmaskB >> dictInB;
			binaryB = bitset<32>(dictVect[dictInB.to_ulong()] ^ (bitmaskB.to_ulong() << (28U - locationB.to_ulong())));
			writeSS << binaryB << endl;
			continue;
		case 3://When 1 bit mismatch compression
			if (curlen + 9 > sslen) break;
			curlen += 9;
			readSS >> locationB  >> dictInB;
			binaryB = bitset<32>(dictVect[dictInB.to_ulong()] ^ (1U << (31U - locationB.to_ulong())));
			writeSS << binaryB << endl;
			continue;
		case 4://When 2 bit consecutive mismatch compression
			if (curlen + 9 > sslen) break;
			curlen += 9;
			readSS >> locationB >> dictInB;
			binaryB = bitset<32>(dictVect[dictInB.to_ulong()] ^ (3U << (30U - locationB.to_ulong())));
			writeSS << binaryB << endl;
			continue;
		case 5://When 4 bit consecutive mismatch compression
			if (curlen + 9 > sslen) break;
			curlen += 9;
			readSS >> locationB >> dictInB;
			binaryB = bitset<32>(dictVect[dictInB.to_ulong()] ^ (15U << (28U - locationB.to_ulong())));
			writeSS << binaryB << endl;
			continue;
		case 6://When 2 bit anywhere compression
			if (curlen + 14 > sslen) break;
			curlen += 14;
			readSS >> locationB >> locationB2 >> dictInB;
			binaryB = bitset<32>(dictVect[dictInB.to_ulong()] ^ (  (1U << (31U - locationB.to_ulong())) | (1U << (31U - locationB2.to_ulong())))   );
			writeSS << binaryB << endl;
			continue;
		case 7://When direct mapping compression
			if (curlen + 4 > sslen) break;
			curlen += 4;
			readSS >> dictInB;
			binaryB = bitset<32>(dictVect[dictInB.to_ulong()]);
			writeSS << binaryB << endl;
			continue;

		default:
			break;
		}
		break;

	}
}