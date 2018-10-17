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
#define IntIntPair pair<long long int, int>
#define N 10
using namespace std;

class my2DArray{
public:
    long long int** Matrix;
    int size;
    my2DArray() {
        size = N;
        Matrix = new long long int*[size];
        for(int i =0; i < size; i++)
            Matrix[i] = new long long int[size];
    }
    my2DArray(int n) {
        size = n;
        Matrix = new long long int*[size];
        for(int i =0; i < size; i++)
            Matrix[i] = new long long int[size];
    }
    my2DArray(const my2DArray &arg){
        size = arg.size;
        Matrix = new long long int*[size];
        for(int i =0; i < size; i++) {
            Matrix[i] = new long long int[size];
            for(int j = 0; j < size; j++)
                Matrix[i][j] = arg.Matrix[i][j];
        }
    }
    ~my2DArray(){
        for (int i = 0; i < size; i++)
            delete[] Matrix[i];
        delete[] Matrix;
    }
};

ifstream inFile;
unsigned long int numThreads;
vector<my2DArray> word_vector; // holds input item: vector of words parsed from input file
vector<IntIntPair> pair_vector;	// holds initial mapped pairs
vector<vector<IntIntPair>> dup_vector;	// vector of vector!! wohoo! holds duplicate pairs to be reduced as distinct vectors
vector<IntIntPair> reduced_vector;	// holds the final reduced pairs for word_count output
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

long long int GetDeterminant(const my2DArray &arg){
    long long int result = 0;
    if(arg.size == 2)
        result = (arg.Matrix[0][0]*arg.Matrix[1][1]) - (arg.Matrix[1][0]*arg.Matrix[0][1]);
    else
    {
        for(int j = 0; j < arg.size; j++)
        {
            int j3 =0;
            my2DArray subMatrix(arg.size-1);
            for(int j2 = 0; j2 < arg.size; j2++)
            {
                if(j2 != j)
                {
                    for (int i = 1; i < arg.size; i++)
                        subMatrix.Matrix[i - 1][j3] = arg.Matrix[i][j2];
                    j3++;
                }
            }
            if(j%2 == 0)
                result += arg.Matrix[0][j]*GetDeterminant(subMatrix);
            else
                result -= arg.Matrix[0][j]*GetDeterminant(subMatrix);
        }
    }
    return result;
}

bool compFuncDescending(const IntIntPair a, const IntIntPair b)
{
    return (a.second > b.second);
}

// input function
vector<my2DArray> inputReadFunc(vector<my2DArray> &word_vector){
    my2DArray word;
    while (!inFile.eof()){
        for(int i = 0; i < N; i++)
            for(int j = 0; j < N; j++)
                inFile >> word.Matrix[i][j];
        word_vector.push_back(word);
    }
    return word_vector;
}

// map function
IntIntPair mapFunc(const my2DArray &arg){
    IntIntPair key_val_pair;
    key_val_pair.first = GetDeterminant(arg);
    key_val_pair.second = 1;
    return key_val_pair;
}

//reduce function
IntIntPair reduceFunc(const vector<IntIntPair> &dup_vector){
    IntIntPair combined_pair;
    combined_pair.first = dup_vector[0].first;
    combined_pair.second = dup_vector.size();
    return combined_pair;
}

//output function
void outputFunc(const vector<IntIntPair> &reduced_vector){
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
        IntIntPair temp = mapFunc(word_vector[pos]);
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
        IntIntPair temp = reduceFunc(dup_vector[pos]);
        pthread_mutex_lock(&lock);
        reduced_vector.push_back(temp);
        pthread_mutex_unlock(&lock);
        pos += numThreads;
    }
}

int main(){
    // STEP 1: read in file
    inFile.open("matrices.txt");
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
            vector<IntIntPair> temp;
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