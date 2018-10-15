//word count without mapreduce


#include <iostream>
#include <fstream>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <utility>
#include <vector>	
#include <algorithm>

using namespace std;

int count = 0;

// map function
pair<string, int> mapFunc(string word){
	int value = 1;
	pair<string, int> key_val_pair;
	key_val_pair.first = word;
	key_val_pair.second = value;
	return key_val_pair;
}

//reduce function
pair<string,int> reduceFunc(vector<pair<string,int> > dup_vector){
	pair<string, int> combined_pair;
	combined_pair.first = dup_vector[0].first;
	combined_pair.second = dup_vector.size();
	return combined_pair;
}

int main(){
	vector<string> word_vector; // holds input item: vector of words parsed from input file
	vector<pair<string, int> > pair_vector;	// holds initial mapped pairs
	vector<pair<string, int> > dup_vector;	// holds duplicate pairs to be reduced
	vector<pair<string, int> > used_vector;	// keeps track of which pairs have been tested for duplicity
	vector<pair<string, int> > reduced_vector;	// holds the final reduced pairs for word_count output
	string word;

	// STEP 1: read in file
	ifstream inFile;
	inFile.open("words.txt");
	if(!inFile.is_open()){
		cout << "open failed" << endl; 
		return 1;
	}

	while (inFile >> word){
		word_vector.push_back(word);
		//count ++;
	}

	inFile.close();


	// STEP 2: map
	for (int i = 0; i<word_vector.size(); i++){
		pair_vector.push_back(mapFunc(word_vector[i]));
	}

	// STEP 3: reduce: send vectors of duplicates to reduce function
	for (int i= 0; i<pair_vector.size(); i++){
		if (find(used_vector.begin(), used_vector.end(), pair_vector[i]) != used_vector.end()){
			continue;
		}
		dup_vector.push_back(pair_vector[i]);
		used_vector.push_back(pair_vector[i]);
		for (int j = i+1; j<pair_vector.size(); j++){
			if (find(dup_vector.begin(), dup_vector.end(), pair_vector[j]) != dup_vector.end()){
				dup_vector.push_back(pair_vector[j]);
			}
		}
		/*for (int i=0; i<dup_vector.size();i++)
			cout << dup_vector[i].first << dup_vector[i].second << endl;
		cout << endl;*/
		reduced_vector.push_back(reduceFunc(dup_vector));
		dup_vector.clear();
	}


	// STEP 4: output
	for (int i = 0; i<reduced_vector.size(); i++){
		cout << reduced_vector[i].first << "," << reduced_vector[i].second << endl;
	}

	return 0;

}