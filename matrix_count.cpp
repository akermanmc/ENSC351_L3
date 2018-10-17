//matrix count without mapreduce


#include <iostream>
#include <fstream>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <utility>
#include <vector>
#include <algorithm>
#define IntIntPair pair<long long int, int>
using namespace std;

#define N 10

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

// map function
IntIntPair mapFunc(const my2DArray &arg){
    IntIntPair key_val_pair;
    key_val_pair.first = GetDeterminant(arg);
    key_val_pair.second = 1;
    return key_val_pair;
}

//reduce function
IntIntPair reduceFunc(vector<IntIntPair> dup_vector){
    IntIntPair combined_pair;
    combined_pair.first = dup_vector[0].first;
    combined_pair.second = dup_vector.size();
    return combined_pair;
}

bool compFuncDescending(const IntIntPair a, const IntIntPair b)
{
    return (a.second > b.second);
}

int main(){
    vector<my2DArray> word_vector; // holds input item: vector of words parsed from input file
    vector<IntIntPair> pair_vector;	// holds initial mapped pairs
    vector<IntIntPair> dup_vector;	// holds duplicate pairs to be reduced
    vector<IntIntPair> used_vector;	// keeps track of which pairs have been tested for duplicity
    vector<IntIntPair> reduced_vector;	// holds the final reduced pairs for word_count output

    // STEP 1: read in file
    ifstream inFile;
    inFile.open("matrices.txt");
    if(!inFile.is_open()){
        cout << "open failed" << endl;
        return 1;
    }

    while (!inFile.eof()){
        my2DArray word;
        for(int i = 0; i < N; i++)
            for(int j = 0; j < N; j++)
                inFile >> word.Matrix[i][j];
        word_vector.push_back(word);
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
        reduced_vector.push_back(reduceFunc(dup_vector));
        dup_vector.clear();
    }

    // STEP 3.5: sort the reduced vector in descending
    std::sort(reduced_vector.begin(), reduced_vector.end(), compFuncDescending);

    // STEP 4: output
    for (int i = 0; i<reduced_vector.size(); i++){
        cout << reduced_vector[i].first << "," << reduced_vector[i].second << endl;
    }

    return 0;

}