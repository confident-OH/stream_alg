#ifndef _PARAMS_H
#define _PARAMS_H

#define N 1000000  // maximum flow
#define M 1000000  // maximum size of stream-summary or CSS
#define MAX_MEM 1000000 // maximum memory size
#define MAX_DATA_SIZE 8

struct heavykeeper_node {
    int *C;
    char FP[MAX_DATA_SIZE];
};

#endif //_PARAMS_H
