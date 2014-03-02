/*
 * Jérôme COUSSANES
 * jerome.coussanes@utbm.fr
 *
 *
 */
#include "global.h"

/*This function print the product state on the console*/
void printProd(Prod p){
    switch(p){
        case C1:
            printf("C1 ");
            break;
        case P10:
            printf("P10");
            break;
        case P11:
            printf("P11");
            break;
        case P12:
            printf("P12");
            break;
        case P13:
            printf("P13");
            break;
        case C2:
            printf("C2 ");
            break;
        case P20:
            printf("P20");
            break;
        case P21:
            printf("P21");
            break;
        case P22:
            printf("P22");
            break;
        case P23:
            printf("P23");
            break;
        case C3:
            printf("C3 ");
            break;
        case P30:
            printf("P30");
            break;
        case P31:
            printf("P31");
            break;
        case P32:
            printf("P32");
            break;
        case P33:
            printf("P33");
            break;
        case P34:
            printf("P34");
            break;
        case C4:
            printf("C4 ");
            break;
        case P40:
            printf("P40");
            break;
        case P41:
            printf("P41");
            break;
        case P42:
            printf("P42");
            break;
        case Empty:
            printf("Emp");
            break;
    }

}

/*This function perform the operation on the product according with its state*/
void op(Prod* p){

    if(*p==C1)
        *p=P10;
    else if(*p==P10)
        *p=P11;
    else if(*p==P11)
        *p=P12;
    else if(*p==P12)
        *p=P13;

    else if(*p==C2)
        *p=P20;
    else if(*p==P20)
        *p=P21;
    else if(*p==P21)
        *p=P22;
    else if(*p==P22)
        *p=P23;

    else if(*p==C3)
        *p=P30;
    else if(*p==P30)
        *p=P31;
    else if(*p==P31)
        *p=P32;
    else if(*p==P32)
        *p=P33;
    else if(*p==P33)
        *p=P34;

    else if(*p==C4)
        *p=P40;
    else if(*p==P40)
        *p=P41;
    else if(*p==P41)
        *p=P42;
}

/*This function exchange the content of case1 with the content of case2 (two products)*/
void change(Prod* case1, Prod* case2){
    Prod tmp;
    tmp=*case1;
    *case1=*case2;
    *case2=tmp;
}

/*This function check which operation have to be performed on p*/
int op_check(Prod p){
    if(p==C1 || p==C3 || p==P21 || p==P32 || p==P41)
        return 1;
    else if(p==P10 || p==C2)
        return 2;
    else if(p==P11 || p==P30 || p==P33)
        return 3;
    else if(p==P20 || p==C4)
        return 4;
    else if(p==P31 || p==P12)
        return 5;
    else if(p==P22 || p==P40)
        return 6;
    else
        return 0;
}
