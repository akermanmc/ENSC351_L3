//word count with mapreduce


#include <iostream>
#include <fstream>
#include <cstdlib>
#include <thread>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <utility>
#include <vector>	
#include <algorithm>
#define StringIntPair pair<string, int>
using namespace std;

ifstream inFile;
int numThreads;
vector<string> word_vector; // holds input item: vector of words parsed from input file
vector<StringIntPair> pair_vector;	// holds initial mapped pairs
vector<vector<StringIntPair>> dup_vector;	// vector of vector!! wohoo! holds duplicate pairs to be reduced as distinct vectors
vector<StringIntPair> reduced_vector;	// holds the final reduced pairs for word_count output
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

bool compFuncDescending(const StringIntPair a, const StringIntPair b)
{
    return (a.second > b.second);
}

// input function
vector<string> inputReadFunc(vector<string> &word_vector){
	string word;
	while (inFile >> word){
		word_vector.push_back(word);
	}
	return word_vector;
}

// map function
StringIntPair mapFunc(const string &word){
	int value = 1;
    StringIntPair key_val_pair;
	key_val_pair.first = word;
	key_val_pair.second = value;
	return key_val_pair;
}

//reduce function
StringIntPair reduceFunc(const vector<StringIntPair> &dup_vector){
    StringIntPair combined_pair;
	combined_pair.first = dup_vector[0].first;
	combined_pair.second = dup_vector.size();
	return combined_pair;
}

//output function
void outputFunc(const vector<StringIntPair> &reduced_vector){
	for (int i = 0; i<reduced_vector.size(); i++){
		cout << reduced_vector[i].first << "," << reduced_vector[i].second << endl;
	}
}

//thread proc for Mapping
void* mapProc(void* arg){
    int pos = *(int*)arg;
    int size = word_vector.size();
    while(pos < size)
    {
        StringIntPair temp = mapFunc(word_vector[pos]);
        pthread_mutex_lock(&lock);
        pair_vector.push_back(temp);
        pthread_mutex_unlock(&lock);
        pos += numThreads;
    }
}

//thread proc for Reduce
void* reduceProc(void* arg){
    int pos = *(int*)arg;
    int size = dup_vector.size();
    while(pos < size){
        StringIntPair temp = reduceFunc(dup_vector[pos]);
        pthread_mutex_lock(&lock);
        reduced_vector.push_back(temp);
        pthread_mutex_unlock(&lock);
        pos += numThreads;
    }
}

int main(){
	// STEP 1: read in file
	inFile.open("words.txt");
	if(!inFile.is_open()){
		cout << "open failed" << endl; 
		return 1;
	}
	word_vector = inputReadFunc(word_vector);

	inFile.close();
    //get number of threads
	numThreads = thread::hardware_concurrency();
    vector<pthread_t> threadVector(numThreads);
    vector<int> threadPosVector(numThreads);

    // STEP 2: map
	for (int i = 0; i<numThreads; i++){
	    threadPosVector[i] = i;
		pthread_create(&threadVector[i], NULL, mapProc, &threadPosVector[i]);
	}

	for (int i = 0; i<numThreads; i++){
	    pthread_join(threadVector[i],NULL);
	}

	//form dup_vector structure with separate vectors for all unique words
	for (int i = 0; i < pair_vector.size(); i++)
    {
	    int j =0;
	    for (j = 0; j < dup_vector.size(); j++)
	    {
            if (pair_vector[i].first == dup_vector[j][0].first)
            {
                dup_vector[j].push_back(pair_vector[i]);
                break;
            }
        }
        if(j >= dup_vector.size())
        {
            vector<StringIntPair> temp;
            temp.push_back(pair_vector[i]);
            dup_vector.push_back(temp);
        }
    }
    //STEP3: reduce
    for (int i = 0; i<numThreads; i++){
        threadPosVector[i] = i;
        pthread_create(&threadVector[i], NULL, reduceProc, &threadPosVector[i]);
    }

    for (int i = 0; i<numThreads; i++){
        pthread_join(threadVector[i],NULL);
    }

    // STEP 3.5: sort the reduced vector in descending
    std::sort(reduced_vector.begin(), reduced_vector.end(), compFuncDescending);

    // STEP 4: output
    outputFunc(reduced_vector);
}