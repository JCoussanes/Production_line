/*
 * Jérôme COUSSANES
 * jerome.coussanes@utbm.fr
 *
 *
 */

#ifndef DEF_GLOBAL
#define DEF_GLOBAL

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

/*This enum represent all states of products*/
typedef enum Prod Prod;
enum Prod{
    C1,P10,P11,P12,P13,C2,P20,P21,P22,P23,C3,P30,P31,P32,P33,P34,C4,P40,P41,P42,Empty
};




#endif

