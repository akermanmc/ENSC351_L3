//word count with mapreduce


#include <iostream>
#include <fstream>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

using namespace std;

int main(){
	// read in file
	ifstream inFile;
	inFile.open("words.txt");
	if(!inFile.is_open()) 
		return 1;

	string word;
	int count = 0;
	while (inFile >> word){
		count ++;
	}

	inFile.close();


	// map

	// reduce

	// output
	ofstream outFile;
	outFile.open("word_count.txt");

	outFile << count;

	outFile.close();

	return 0;

}