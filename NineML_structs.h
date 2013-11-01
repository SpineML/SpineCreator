#ifndef NINEML_STRUCTS_H
#define NINEML_STRUCTS_H

#include <vector>
#include <string>

using namespace std;

struct NineMLDimension {
   int m;
   int l;
   int t;
   int E;
   int c;
};

struct NineMLVariable {
    string name;
    NineMLDimension dims;
};

struct NineMLDifferentialEquation {
    string equation;
    int state_var_num;
};

struct equation {
    string equation;
    int param_num;
};

struct assign {
    string equation;
    int state_var_num;
};

struct condition {
    string equation;
    int param_num;
};

struct eventOut {
    int port;
    float data; // redo this - it isn't right
};

struct onEvent {
    // fill me
    int temp_val;
};

struct transition {
    int srcRegime;
    int targRegime;
    vector <assign > assignList;

};

struct regime {
    vector < NineMLDifferentialEquation > deList;
    vector < condition > condList;
    vector < onEvent > eventList;
    int state_var_num;
};

#endif // NINEML_STRUCTS_H
