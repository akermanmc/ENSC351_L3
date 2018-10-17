#include "tracelib.h"
#include "eventEntries.h"
#include <unistd.h>
#include <pthread.h>
#include <fstream>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <cstring>

using namespace std;

namespace {
    ofstream *fileStream;
    traceEntry *entryArray; // array of trace entries
    int entryCounter;
    chrono::time_point<chrono::system_clock> start_time;
    chrono::time_point<chrono::system_clock> current_time;
    char *buff;
}

// begins trace generation, opens write file, starts timer
void trace_start(const char *filename) {
    //allocate file stream and open it
    fileStream = new ofstream();
    fileStream->open(filename, ofstream::out);

    //allocate array of 10,000 event entries
    entryArray = new traceEntry[maxEntries];
    entryCounter = 0;

    //allocate char buff
    buff = new char[200];

    //write first square bracket open to file
    *fileStream<<"["<<endl;

    //initialize start time for trace
    start_time = chrono::system_clock::now();
}



// saves event start trace to entries array
void trace_event_start(const char *name, const char *categories, const char *arguments) {
    if(entryCounter >= maxEntries)
        trace_flush();
    current_time = chrono::system_clock::now();
    entryArray[entryCounter].setTS(chrono::duration_cast<chrono::microseconds>(
            current_time - start_time).count());
    entryArray[entryCounter].setName(name);
    entryArray[entryCounter].setCat(categories);
    entryArray[entryCounter].setPID(getpid());
    entryArray[entryCounter].setTID(pthread_self());
    entryArray[entryCounter].setPhase('B');
    if(arguments != nullptr)
        entryArray[entryCounter].setArgs(arguments);
    entryCounter++;
}

// saves event end trace to entries array
void trace_event_end(const char *arguments) {
    if(entryCounter >= maxEntries)
        trace_flush();
    current_time = chrono::system_clock::now();
    entryArray[entryCounter].setTS(chrono::duration_cast<chrono::microseconds>(
            current_time - start_time).count());
    entryArray[entryCounter].setPID(getpid());
    entryArray[entryCounter].setTID(pthread_self());
    entryArray[entryCounter].setPhase('E');
    if(arguments != nullptr)
        entryArray[entryCounter].setArgs(arguments);
    entryCounter++;
}

// saves instant type events to entries array
void trace_instant_global(const char *name) {
    if(entryCounter >= maxEntries)
        trace_flush();
    current_time = chrono::system_clock::now();
    entryArray[entryCounter].setTS(chrono::duration_cast<chrono::microseconds>(
            current_time - start_time).count());
    entryArray[entryCounter].setName(name);
    entryArray[entryCounter].setPID(getpid());
    entryArray[entryCounter].setTID(pthread_self());
    entryArray[entryCounter].setPhase('i');
    entryCounter++;
}

// writes object event creation trace to file
void trace_object_new(const char *name, const void *obj_pointer) {
    if(entryCounter >= maxEntries)
        trace_flush();
    current_time = chrono::system_clock::now();
    entryArray[entryCounter].setTS(chrono::duration_cast<chrono::microseconds>(
            current_time - start_time).count());
    entryArray[entryCounter].setName(name);
    entryArray[entryCounter].setPID(getpid());
    entryArray[entryCounter].setTID(pthread_self());
    entryArray[entryCounter].setPhase('N');
    entryArray[entryCounter].setObjRef(obj_pointer);
    entryCounter++;
}

// writes counter event to file
// single series only***
void trace_counter(const char *name, const char *key, const char *value) {
    if(entryCounter >= maxEntries)
        trace_flush();
    current_time = chrono::system_clock::now();
    entryArray[entryCounter].setTS(chrono::duration_cast<chrono::microseconds>(
            current_time - start_time).count());
    entryArray[entryCounter].setName(name);
    entryArray[entryCounter].setPhase('C');
    entryArray[entryCounter].setKey(key);
    entryArray[entryCounter].setValue(value);
    entryArray[entryCounter].setPID(getpid());
    entryArray[entryCounter].setTID(pthread_self());
    entryCounter++;
}


// will be used to flush all the entries we have in our 10,000 size entry array to file
void trace_flush() {
    for (int i = 0; i < entryCounter; i++)
        entryArray[i].WriteToFile();
    entryCounter = 0;
}

// signals the end of trace writing in the program, closes the write file
void trace_end() {
    trace_flush();
    sprintf(buff,"{\"name\": \"TRACE_END\", \"ph\": \"i\", \"ts\": 0, \"pid\": 1, \"tid\": 1, \"s\": \"g\"}");
    *fileStream<<buff<<endl<<"]"<<endl;
    fileStream->close();
    delete fileStream;
    delete[] entryArray;
    delete[] buff;
    fileStream = nullptr;
}

void trace_object_gone(const char *name, const void *obj_pointer) {
    if(entryCounter >= maxEntries)
        trace_flush();
    current_time = chrono::system_clock::now();
    entryArray[entryCounter].setTS(chrono::duration_cast<chrono::microseconds>(
            current_time - start_time).count());
    entryArray[entryCounter].setName(name);
    entryArray[entryCounter].setPID(1);
    entryArray[entryCounter].setTID(1);
    entryArray[entryCounter].setPhase('D');
    entryArray[entryCounter].setObjRef(obj_pointer);
    entryCounter++;
}

void traceEntry::printPhase(){
    cout << phase << endl;
}
void traceEntry::printName(){
    cout << name << endl;
}

// writes appropriate trace entry to file depending on event description variable values
void traceEntry::WriteToFile() {
    //printPhase();
    switch(phase)
    {
        case 'E':
            *fileStream << "{\"ph\": \""<<phase<<"\", \"ts\": "<< ts
                        <<", \"pid\": "<< pid<< ", \"tid\": "
                        << tid <<"}," << endl;
            break;
        case 'i':
            *fileStream << "{\"name\": \"" << name << "\", \"ph\": \"" << phase
                        <<"\", \"ts\": "<< ts <<", \"pid\": "<< pid<< ", \"tid\": "
                        << tid <<", \"s\": \"t\"}," << endl;
            break;
        case 'B':
            *fileStream << "{\"name\": \"" << name << "\", \"cat\": \"" << cat
                        << "\", \"ph\": \"" << phase <<"\", \"ts\": "<< ts
                        <<", \"pid\": "<< pid<< ", \"tid\": " << tid <<"}," << endl;
            break;
        case 'N':
            *fileStream << "{\"name\": \"" << name << "\", \"id\": \"" << obj
                        << "\", \"ph\": \"" << phase <<"\", \"ts\": "<< ts
                        <<", \"pid\": "<< pid<< ", \"tid\": " << tid <<"}," << endl;
            break;
        case 'D':
            *fileStream << "{\"name\": \"" << name << "\", \"id\": \"" << obj
                        << "\", \"ph\": \"" << phase <<"\", \"ts\": "<< ts
                        <<", \"pid\": "<< pid<< ", \"tid\": " << tid <<"}," << endl;
            break;
        case 'C':
            *fileStream << "{\"name\": \"" << name << "\", \"ph\": \"" << phase <<"\", \"ts\": "<< ts
                        <<", \"pid\": "<< pid<< ", \"tid\": " << tid << ", \"args\": {\"" << ckey
                        << "\": " << cvalue <<"}}," << endl;
            break;
        default: assert(0);
    }
}