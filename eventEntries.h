#ifndef PROJECT_EVENTENTRIES_H
#define PROJECT_EVENTENTRIES_H
#include <string>
using namespace std;

const int maxEntries = 10000;


class traceEntry{
public:
    traceEntry() : name(), cat(), phase('\0'), ts(0), pid(0), tid(0), arguments(), obj(nullptr), ckey(), cvalue() {}
    traceEntry(const char* _name, const char* _cat, const char _phase, const long long int _ts,const int _pid,
               const int _tid, const  char* _arguments, const void* _obj, const char* _key, const char* val)
            : name(_name), cat(_cat), phase(_phase), ts(_ts), pid(_pid), tid(_tid), arguments(_arguments), obj((void*)_obj), ckey(_key), cvalue(val) {}

    //setters
    void setName(const char* _name) {name = string(_name);}
    void setCat(const char* _cat) {cat = string(_cat);}
    void setPhase(const char _phase) {phase = _phase;}
    void setTS(const long long int _ts) {ts = _ts;}
    void setPID(const int _pid = 1) {pid = _pid;}
    void setTID(const int _tid = 1) {tid = _tid;}
    void setArgs(const char* args) {arguments = string(args);}
    void setObjRef(const void* _obj) {obj = (void*)_obj;}
    void setKey(const char* _key) {ckey = string(_key);}
    void setValue(const char* val) {cvalue = string(val);}

    void printPhase();
    void printName();
    void WriteToFile();

private:
    string name;		// event name
    string cat;		    // event category
    string arguments;   // event arguments
    char phase;			// event phase (beginning, instant, end)
    long long int ts;	// trace event time stamp
    int pid;			// process ID
    int tid;			// thread ID
    void* obj;          // object event obj ref
    string ckey;		// counter event key
    string cvalue;		// counter event value
};

#endif //PROJECT_EVENTENTRIES_H
