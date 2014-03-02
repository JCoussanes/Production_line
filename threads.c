/*
 * Jérôme COUSSANES
 * jerome.coussanes@utbm.fr
 *
 *
 */
#include "global.h"
#define NbTh 8      /*Number of robots (input, output, operatives)*/
#define N  16	/*Number of cases on the rotative line*/

/*Threads*/
pthread_t tid[NbTh+1];
pthread_t optid[6];

/*Mutex and monitor's conditions*/
pthread_mutex_t mutexLine, mutexNumber, mutexCase[8],mutexField[6];
pthread_cond_t  waitLineEnd,waitPrev[7],waitOp[6];

/*Initializations of the line and working fields of each robot*/
Prod line[16]={Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty};
Prod field[6]={Empty,Empty,Empty,Empty,Empty,Empty};

int Nb[4]={10,15,12,8},Nbts[4]={30,45,12,16},Nbs[4]={0,0,0,0},Nbf[4]={0,0,0,0};/*Number of products to do, number of basic components to insert, number of components already inserted, nuber of products finished*/
int NbInOp=0;    /*Number of objects in working area (line+in process+future products coming from the robot's stock)*/

#include "func.h"

/*This function print the content of the line*/
void printLine(){
    int i;
    printf("\r");
    for(i=0;i<16;i++){
        printProd(line[i]);
        printf("|");
    }
    fflush(stdout);
}

/*This function proceed to the rotation of the rotative line*/
void rotate(){
    Prod tmp;
    int i;

    tmp=line[15];
    for(i=15;i>0;i--){
        line[i]=line[i-1];
    }
    line[0]=tmp;
}

/*This function is the output robot*/
void out(Prod* sec){
    pthread_mutex_lock(&mutexNumber);
    switch(*sec){
        case P13:
            Nbf[0]++;
            *sec=Empty;
            NbInOp--;
            break;
        case P23:
            Nbf[1]++;
            *sec=Empty;
            NbInOp--;
            break;
        case P34:
            Nbf[2]++;
            *sec=Empty;
            NbInOp--;
            break;
        case P42:
            Nbf[3]++;
            *sec=Empty;
            NbInOp--;
            break;
    }
    pthread_mutex_unlock(&mutexNumber);
}

/*This funciton is the input robot*/
void in(Prod* sec){
    int l=0,i=0;
    srand(time(NULL));

    if(*sec==Empty && NbInOp < 16){
        l=rand()%4;

        if(Nbs[l]==Nbts[l]){
            for(i=1;i<4;i++){
                if(Nbs[(l+i)%4]<Nbts[(l+i)%4]){
                    l=(l+i)%4;
                    i=4;
                }
            }
        }
        if(Nbts[l]==Nbs[l]){}
        else{
            switch(l){
                case 0:
                    *sec=C1;
                    break;
                case 1:
                    *sec=C2;
                    break;
                case 2:
                    *sec=C3;
                    break;
                case 3:
                    *sec=C4;
                    break;
            }
            pthread_mutex_lock(&mutexNumber);
            NbInOp++;
            pthread_mutex_unlock(&mutexNumber);

            Nbs[l]++;
        }
    }
}

/*This function is the robot who rotate the line each one second*/
void LineBot(){
    while(1){
        sleep(1);
        //sleep(0.1);
        pthread_mutex_lock(&mutexLine);
        /*rotation*/
        rotate();

        printLine();
        printProd(field[0]);
        printf("|");
        printProd(field[1]);
        printf("|");
        printProd(field[2]);
        printf("|");
        printProd(field[3]);
        printf("|");
        printProd(field[4]);
        printf("|");
        printProd(field[5]);
        printf("|");
        printf("P1 : %d  P2 : %d  P3 : %d  P4 : %d",Nbf[0],Nbf[1],Nbf[2],Nbf[3]);
        fflush(stdout);
        pthread_mutex_unlock(&mutexLine);
        pthread_cond_signal(&waitLineEnd);
    }
}

/*This function put a product in the stock of a robot*/
void stocker(Prod* sec,Prod stock[]){
    int i,l=0;
    for(i=0;i<5;i++){
        if(stock[i]==*sec)
            l++;
    }

    for(i=0;i<5;i++){
        if(stock[i]==Empty){
            change(&stock[i],sec);
            break;
        }
    }
    pthread_mutex_lock(&mutexNumber);
    if(((stock[i]==C1||stock[i]==C2) && l!=0 && l!=3)||(stock[i]==C4 && l!=0 && l!=2 && l!=4)){
        NbInOp--;
    }

    pthread_mutex_unlock(&mutexNumber);
}

/*This function set the field of a robot to work from the stock*/
void from_stock(Prod* workSp,Prod stock[]){
    int i,j,c1=0,c2=0,c4=0;
    for(i=0;i<5;i++){
        if(stock[i]==C1)
            c1++;
        else if(stock[i]==C2)
            c2++;
        else if(stock[i]==C4)
            c4++;
    }

    if(c1>=3){
        for(i=0,j=0;j<3;i++){
            if(stock[i]==C1){
                stock[i]=Empty;
                j++;
            }
        }
        *workSp=C1;
    }
    else if(c2>=3){
         for(i=0,j=0;j<3;i++){
            if(stock[i]==C2){
                stock[i]=Empty;
                j++;
            }
        }
        *workSp=C2;
    }
    else if(c4>=2){
          for(i=0,j=0;j<2;i++){
            if(stock[i]==C4){
                stock[i]=Empty;
                j++;
            }
        }
        *workSp=C4;
    }
    else{
        for(i=0;i<5;i++){
            if(stock[i]!=Empty && stock[i]!=C1 && stock[i]!=C2 && stock[i]!=C4){
                change(workSp,&stock[i]);
                break;
            }
        }
    }

}

/*This function is the bot1. Each bot work same way, bot2 is comented more detailly*/
void Bot1(){
    sigset_t set,base;
    Prod stock[5]={Empty,Empty,Empty,Empty,Empty};
    int op=1;

    sigemptyset(&set);
    sigemptyset(&base);
    sigaddset(&base,SIGUSR1);
    sigprocmask(SIG_SETMASK,&base,NULL);
    while(1){

        pthread_cond_wait(&waitLineEnd,&mutexLine);
        pthread_cond_signal(&waitPrev[0]);

        if(line[3]==Empty){
            if(pthread_mutex_trylock(&mutexField[0])==0){
                if(op_check(field[0])!=op && field[0]!=Empty)
                    change(&line[3],&field[0]);
                pthread_mutex_unlock(&mutexField[0]);
            }
        }else if(op==op_check(line[3])){
            if(line[3]!=C1 && line[3]!=C2 && line[3]!=C4){
                if(pthread_mutex_trylock(&mutexField[0])==0){
                    if(op_check(field[0])!=op){
                        change(&field[0],&line[3]);
                    }else
                        stocker(&line[3],stock);
                    pthread_mutex_unlock(&mutexField[0]);
                }else
                    stocker(&line[3],stock);
            }else{
                stocker(&line[3],stock);
                if(pthread_mutex_trylock(&mutexField[0])==0){
                    if(line[3]==Empty  && op_check(field[0])!=op && field[0]!=Empty)
                        change(&line[3],&field[0]);
                    pthread_mutex_unlock(&mutexField[0]);
                }
            }
        }
        if(pthread_mutex_trylock(&mutexField[0])==0){
            if(op_check(field[0])==op)
                pthread_cond_signal(&waitOp[0]);
            else if(field[0]==Empty && line[3]!=Empty){
                from_stock(&field[0],stock);
                pthread_cond_signal(&waitOp[0]);
            }
            pthread_mutex_unlock(&mutexField[0]);
        }
        sigsuspend(&set);
    }
}

/*This function is to change the action of SIGUSR1*/
void nothing(){
}

/*This function is bot2*/
void Bot2(){
    /*Creation of internal variables in the thread and initialization of SIGUSR1 to make it doing nothing when it's send*/
    signal(SIGUSR1,nothing);
    Prod stock[5]={Empty,Empty,Empty,Empty,Empty};
    int op=2;

    /*Initialization of two signal masks, base which block SIGUSR1 all the time and set which will momentarily free this signal from pending state*/
    sigset_t set,base;
    sigemptyset(&set);
    sigemptyset(&base);
    sigaddset(&base,SIGUSR1);
    sigprocmask(SIG_SETMASK,&base,NULL);

    /*Begining of the execution loop*/
    while(1){
        /*Waiting of previous robot to get the mutex of his case*/
        pthread_cond_wait(&waitPrev[0],&mutexCase[1]);

        /*I have the mutex so I prevent the next robot*/
        pthread_cond_signal(&waitPrev[1]);

        /*Work on the case*/

        if(line[5]==Empty){

            /*The section of the line is empty so I try to lock the mutex of my field (am I already working?)*/
            if(pthread_mutex_trylock(&mutexField[1])==0){

                /*The operative thread is not working on the field
                 * So we check that this robot can operate on this product*/
                if(op_check(field[1])!=op && field[1]!=Empty){

                    /*There is a product that I can not work on in the field so I put it on the line*/
                    change(&line[5],&field[1]);
                }

                /*I finished my work on field so I unlock the mutex of field*/
                pthread_mutex_unlock(&mutexField[1]);
            }

        }else if(op==op_check(line[5])){

            /*The section of the line is not empty and this robot can work on this product*/
            if(line[5]!=C1 && line[5]!=C2 && line[5]!=C4){

                /*The product on th section isn't a basic components which need to be several to be operate*/
                if(pthread_mutex_trylock(&mutexField[1])==0){

                    /*There is no operation on the field*/
                    if(op_check(field[1])!=op){

                        /*The product on the working area can't be operated bu this robot (or the area is empty)
                         * so we exchange the product of the area with the product on the line */
                        change(&field[1],&line[5]);
                    }else{

                        /*The product of the area can be operated so we put the product on the line in the stock of this robot*/
                        stocker(&line[5],stock);
                    }

                    /*The work on the field is done so we unlock the field mutex*/
                    pthread_mutex_unlock(&mutexField[1]);
                }else{

                    /*The field is in operation so we just stock the product from the line*/
                    stocker(&line[5],stock);
                }
            }else{

                /*The product on the line is a basic component which need several of it to be operated so we stock it*/
                stocker(&line[5],stock);

                if(pthread_mutex_trylock(&mutexField[1])==0){

                    /*There is no operation on the field*/
                    if(line[5]==Empty  && op_check(field[1])!=op && field[1]!=Empty){
                        /*There is no product on the line and the product in the field canot be operate by this robot so we put the product in the field on the line*/
                        change(&line[5],&field[1]);
                    }

                    /*We finished to work on the field so we unlock the mutex field*/
                    pthread_mutex_unlock(&mutexField[1]);
                }
            }
        }

        /*End of the work on the line section*/

        if(pthread_mutex_trylock(&mutexField[1])==0){
            /*There is no operation on the field*/
            if(op_check(field[1])==op){
                /*If the product on the field can be operate we wake up the operative thread of this robot*/
                pthread_cond_signal(&waitOp[1]);
            }
            else if(field[1]==Empty && line[5]!=Empty){
                /*If there is nothing on the field we work with the stock and we wake up the operator's thread*/
                from_stock(&field[1],stock);
                pthread_cond_signal(&waitOp[1]);
            }
            /*We finished to work on the field so we unlock the field mutex*/
            pthread_mutex_unlock(&mutexField[1]);
        }

        /*We finished our work for this turn of line so we wait the next robot sent us a signal to prevent us that it has finished*/
        sigsuspend(&set);

        /*The next robot has finished its wirk and me too so I prevent the previous robot that I have finish too*/
        pthread_kill(tid[0],SIGUSR1);

    }
}

/*This funciton is bot3*/
void Bot3(){
    signal(SIGUSR1,nothing);
    Prod stock[5]={Empty,Empty,Empty,Empty,Empty};
    int op=3;

    sigset_t set,base;
    sigemptyset(&set);
    sigemptyset(&base);
    sigaddset(&base,SIGUSR1);
    sigprocmask(SIG_SETMASK,&base,NULL);
    while(1){
        pthread_cond_wait(&waitPrev[1],&mutexCase[2]);
        pthread_cond_signal(&waitPrev[2]);
        if(line[7]==Empty){
            if(pthread_mutex_trylock(&mutexField[2])==0){
                if(op_check(field[2])!=op && field[2]!=Empty)
                    change(&line[7],&field[2]);
                pthread_mutex_unlock(&mutexField[2]);
            }
        }else if(op==op_check(line[7])){
            if(line[7]!=C1 && line[7]!=C2 && line[7]!=C4){
                if(pthread_mutex_trylock(&mutexField[2])==0){
                    if(op_check(field[2])!=op){
                        change(&field[2],&line[7]);
                    }else
                        stocker(&line[7],stock);
                    pthread_mutex_unlock(&mutexField[2]);
                }else
                    stocker(&line[7],stock);
            }else{
                stocker(&line[7],stock);
                if(pthread_mutex_trylock(&mutexField[2])==0){
                    if(line[7]==Empty  && op_check(field[2])!=op && field[2]!=Empty)
                        change(&line[7],&field[2]);
                    pthread_mutex_unlock(&mutexField[2]);
                }
            }
        }
        if(pthread_mutex_trylock(&mutexField[2])==0){
            if(op_check(field[2])==op)
                pthread_cond_signal(&waitOp[2]);
            else if(field[2]==Empty && line[7]!=Empty){
                from_stock(&field[2],stock);
                pthread_cond_signal(&waitOp[2]);
            }
            pthread_mutex_unlock(&mutexField[2]);
        }
        sigsuspend(&set);

        pthread_kill(tid[1],SIGUSR1);

    }
}

/*This function is the bot4*/
void Bot4(){
    signal(SIGUSR1,nothing);
    Prod stock[5]={Empty,Empty,Empty,Empty,Empty};
    int op=4;

    sigset_t set,base;
    sigemptyset(&set);
    sigemptyset(&base);
    sigaddset(&base,SIGUSR1);
    sigprocmask(SIG_SETMASK,&base,NULL);
    while(1){
        pthread_cond_wait(&waitPrev[2],&mutexCase[3]);
        pthread_cond_signal(&waitPrev[3]);

        if(line[9]==Empty){
            if(pthread_mutex_trylock(&mutexField[3])==0){
                if(op_check(field[3])!=op && field[3]!=Empty)
                    change(&line[9],&field[3]);
                pthread_mutex_unlock(&mutexField[3]);
            }
        }else if(op==op_check(line[9])){
            if(line[9]!=C1 && line[9]!=C2 && line[9]!=C4){
                if(pthread_mutex_trylock(&mutexField[3])==0){
                    if(op_check(field[3])!=op){
                        change(&field[3],&line[9]);
                    }else
                        stocker(&line[9],stock);
                    pthread_mutex_unlock(&mutexField[3]);
                }else
                    stocker(&line[9],stock);
            }else{
                stocker(&line[9],stock);
                if(pthread_mutex_trylock(&mutexField[3])==0){
                    if(line[9]==Empty  && op_check(field[3])!=op && field[3]!=Empty)
                        change(&line[9],&field[3]);
                    pthread_mutex_unlock(&mutexField[3]);
                }
            }
        }
        if(pthread_mutex_trylock(&mutexField[3])==0){
            if(op_check(field[3])==op)
                pthread_cond_signal(&waitOp[3]);
            else if(field[3]==Empty && line[9]!=Empty){
                from_stock(&field[3],stock);
                pthread_cond_signal(&waitOp[3]);
            }
            pthread_mutex_unlock(&mutexField[3]);
        }

        sigsuspend(&set);

        pthread_kill(tid[2],SIGUSR1);

    }
}

/*This functino is bot5*/
void Bot5(){
    signal(SIGUSR1,nothing);
    Prod stock[5]={Empty,Empty,Empty,Empty,Empty};
    int op=5;

    sigset_t set,base;
    sigemptyset(&set);
    sigemptyset(&base);
    sigaddset(&base,SIGUSR1);
    sigprocmask(SIG_SETMASK,&base,NULL);

    while(1){
        pthread_cond_wait(&waitPrev[3],&mutexCase[4]);
        pthread_cond_signal(&waitPrev[4]);

        if(line[11]==Empty){
            if(pthread_mutex_trylock(&mutexField[4])==0){
                if(op_check(field[4])!=op && field[4]!=Empty)
                    change(&line[11],&field[4]);
                pthread_mutex_unlock(&mutexField[4]);
            }
        }else if(op==op_check(line[11])){
            if(line[11]!=C1 && line[11]!=C2 && line[11]!=C4){
                if(pthread_mutex_trylock(&mutexField[4])==0){
                    if(op_check(field[4])!=op){
                        change(&field[4],&line[11]);
                    }else
                        stocker(&line[11],stock);
                    pthread_mutex_unlock(&mutexField[4]);
                }else
                    stocker(&line[11],stock);
            }else{
                stocker(&line[11],stock);
                if(pthread_mutex_trylock(&mutexField[4])==0){
                    if(line[11]==Empty  && op_check(field[4])!=op && field[4]!=Empty)
                        change(&line[11],&field[4]);
                    pthread_mutex_unlock(&mutexField[4]);
                }
            }
        }
        if(pthread_mutex_trylock(&mutexField[4])==0){
            if(op_check(field[4])==op)
                pthread_cond_signal(&waitOp[4]);
            else if(field[4]==Empty && line[11]!=Empty){
                from_stock(&field[4],stock);
                pthread_cond_signal(&waitOp[4]);
            }
            pthread_mutex_unlock(&mutexField[4]);
        }
        sigsuspend(&set);

        pthread_kill(tid[3],SIGUSR1);

    }
}

/*This function is bot6*/
void Bot6(){
    signal(SIGUSR1,nothing);
    Prod stock[5]={Empty,Empty,Empty,Empty,Empty};
    int op=6;

    sigset_t set,base;
    sigemptyset(&set);
    sigemptyset(&base);
    sigaddset(&base,SIGUSR1);
    sigprocmask(SIG_SETMASK,&base,NULL);

    while(1){
        pthread_cond_wait(&waitPrev[4],&mutexCase[5]);
        pthread_cond_signal(&waitPrev[5]);

        if(line[13]==Empty){
            if(pthread_mutex_trylock(&mutexField[5])==0){
                if(op_check(field[5])!=op && field[5]!=Empty)
                    change(&line[13],&field[5]);
                pthread_mutex_unlock(&mutexField[5]);
            }
        }else if(op==op_check(line[13])){
            if(line[13]!=C1 && line[13]!=C2 && line[13]!=C4){
                if(pthread_mutex_trylock(&mutexField[5])==0){
                    if(op_check(field[5])!=op){
                        change(&field[5],&line[13]);
                    }else
                        stocker(&line[13],stock);
                    pthread_mutex_unlock(&mutexField[5]);
                }else
                    stocker(&line[13],stock);
            }else{
                stocker(&line[13],stock);
                if(pthread_mutex_trylock(&mutexField[5])==0){
                    if(line[13]==Empty  && op_check(field[5])!=op && field[5]!=Empty)
                        change(&line[13],&field[5]);
                    pthread_mutex_unlock(&mutexField[5]);
                }
            }
        }
        if(pthread_mutex_trylock(&mutexField[5])==0){
            if(op_check(field[5])==op)
                pthread_cond_signal(&waitOp[5]);
            else if(field[5]==Empty){
                from_stock(&field[5],stock);
                pthread_cond_signal(&waitOp[5]);
            }
            pthread_mutex_unlock(&mutexField[5]);
        }

        sigsuspend(&set);


        pthread_kill(tid[4],SIGUSR1);
    }
}

/*This bot is the output bot*/
void BotO(){
    signal(SIGUSR1,nothing);
    sigset_t set,base;
    sigemptyset(&set);
    sigemptyset(&base);
    sigaddset(&base,SIGUSR1);
    sigprocmask(SIG_SETMASK,&base,NULL);

    while(1){
        pthread_cond_wait(&waitPrev[5],&mutexCase[6]);
        pthread_cond_signal(&waitPrev[6]);
        out(&line[15]);

        sigsuspend(&set);

        pthread_kill(tid[5],SIGUSR1);
    }
}

/*This bot is the output bot*/
void BotI(){
    signal(SIGUSR1,nothing);
    sigset_t set;
    sigemptyset(&set);
    while(1){
        pthread_cond_wait(&waitPrev[6],&mutexCase[7]);

        in(&line[0]);


        pthread_kill(tid[6],SIGUSR1);

    }
}

/*This pat is the function sent to threads*/
void * func_botLine(){
    LineBot();
}

void func_bot1(){
    pthread_mutex_lock(&mutexLine);
    Bot1();
    pthread_mutex_unlock(&mutexLine);
}

void func_bot2(){
    pthread_mutex_lock(&mutexCase[1]);
    Bot2();
    pthread_mutex_unlock(&mutexCase[1]);
}

void func_bot3(){
    pthread_mutex_lock(&mutexCase[2]);
    Bot3();
    pthread_mutex_unlock(&mutexCase[2]);
}

void func_bot4(){
    pthread_mutex_lock(&mutexCase[3]);
    Bot4();
    pthread_mutex_unlock(&mutexCase[3]);
}

void func_bot5(){
    pthread_mutex_lock(&mutexCase[4]);
    Bot5();
    pthread_mutex_unlock(&mutexCase[4]);
}

void func_bot6(){
    pthread_mutex_lock(&mutexCase[5]);
    Bot6();
    pthread_mutex_unlock(&mutexCase[5]);
}

void func_botO(){
    pthread_mutex_lock(&mutexCase[6]);
    BotO();
    pthread_mutex_unlock(&mutexCase[6]);
}

void func_botI(){
    pthread_mutex_lock(&mutexCase[7]);
    BotI();
    pthread_mutex_unlock(&mutexCase[7]);
}

void func_op1(){
    pthread_mutex_lock(&mutexField[0]);
    while(1){
        pthread_cond_wait(&waitOp[0],&mutexField[0]);
        sleep(2);

        op(&field[0]);
    }
    pthread_mutex_unlock(&mutexField[0]);
}
void func_op2(){
    pthread_mutex_lock(&mutexField[1]);
    while(1){
        pthread_cond_wait(&waitOp[1],&mutexField[1]);
        sleep(2);

        op(&field[1]);
    }
    pthread_mutex_unlock(&mutexField[1]);
}
void func_op3(){
    pthread_mutex_lock(&mutexField[2]);
    while(1){
        pthread_cond_wait(&waitOp[2],&mutexField[2]);
        sleep(2);

        op(&field[2]);
    }
    pthread_mutex_unlock(&mutexField[2]);
}
void func_op4(){
    pthread_mutex_lock(&mutexField[3]);
    while(1){
        pthread_cond_wait(&waitOp[3],&mutexField[3]);
        sleep(2);

        op(&field[3]);
    }
    pthread_mutex_unlock(&mutexField[3]);
}
void func_op5(){
    pthread_mutex_lock(&mutexField[4]);
    while(1){
        pthread_cond_wait(&waitOp[4],&mutexField[4]);
        sleep(2);

        op(&field[4]);
    }
    pthread_mutex_unlock(&mutexField[4]);
}
void func_op6(){
    pthread_mutex_lock(&mutexField[5]);
    while(1){
        pthread_cond_wait(&waitOp[5],&mutexField[5]);
        sleep(2);

        op(&field[5]);
    }
    pthread_mutex_unlock(&mutexField[5]);
}


/*This is the main of the program*/
int main(int argc, char** argv){

    int num,v;
    /*Initialization*/
    pthread_mutex_init(&mutexLine,NULL);
    pthread_mutex_init(&mutexNumber,NULL);
    for(v=0;v<NbTh;v++){
        pthread_mutex_init(&mutexCase[v],NULL);
    }

    for(v=0;v<6;v++){
        pthread_mutex_init(&mutexField[v],NULL);
    }

    pthread_cond_init(&waitLineEnd,0);

    for(v=0;v<7;v++){
        pthread_cond_init(&waitPrev[v],0);
    }

    for(v=0;v<6;v++){
        pthread_cond_init(&waitOp[v],0);
    }
    /*Display*/
    printf(" C1| C2| C3| C4| C5| C6| C7| C8| C9|C10|C11|C12|C13|C14|C15|C16| R1| R2| R3| R4| R5| R6|  Finished products.|\n");
    printf("_I_|___|___|_1_|___|_2_|___|_3_|___|_4_|___|_5_|___|_6_|___|_O_|___|___|___|___|___|___|____________________|\n\n");
    printLine();
    printProd(field[0]);
    printf("|");
    printProd(field[1]);
    printf("|");
    printProd(field[2]);
    printf("|");
    printProd(field[3]);
    printf("|");
    printProd(field[4]);
    printf("|");
    printProd(field[5]);
    printf("|");
    printf("P1 : %d  P2 : %d  P3 : %d  P4 : %d",Nbf[0],Nbf[1],Nbf[2],Nbf[3]);
    fflush(stdout);
    /* creation of the thread line*/
    pthread_create(tid+NbTh,0,(void *(*)())func_botLine,(void *)NbTh);

    /*creation of bot threads*/
    pthread_create(tid,0,(void *(*)())func_bot1,(void *)0);
    pthread_create(tid+1,0,(void *(*)())func_bot2,(void *)1);
    pthread_create(tid+2,0,(void *(*)())func_bot3,(void *)2);
    pthread_create(tid+3,0,(void *(*)())func_bot4,(void *)3);
    pthread_create(tid+4,0,(void *(*)())func_bot5,(void *)4);
    pthread_create(tid+5,0,(void *(*)())func_bot6,(void *)5);
    pthread_create(tid+6,0,(void *(*)())func_botO,(void *)6);
    pthread_create(tid+7,0,(void *(*)())func_botI,(void *)7);
    /*creation of operative threads*/
    pthread_create(optid,0,(void *(*)())func_op1,(void *)0);
    pthread_create(optid+1,0,(void *(*)())func_op2,(void *)1);
    pthread_create(optid+2,0,(void *(*)())func_op3,(void *)2);
    pthread_create(optid+3,0,(void *(*)())func_op4,(void *)3);
    pthread_create(optid+4,0,(void *(*)())func_op5,(void *)4);
    pthread_create(optid+5,0,(void *(*)())func_op6,(void *)5);


    /*Waiting the end of all threads*/
    pthread_join(tid[0],NULL);
    pthread_join(tid[1],NULL);
    pthread_join(tid[2],NULL);
    pthread_join(tid[3],NULL);
    pthread_join(tid[4],NULL);
    pthread_join(tid[5],NULL);
    pthread_join(tid[6],NULL);
    pthread_join(tid[7],NULL);
    pthread_join(tid[NbTh],NULL);
    pthread_join(optid[0],NULL);
    pthread_join(optid[1],NULL);
    pthread_join(optid[2],NULL);
    pthread_join(optid[3],NULL);
    pthread_join(optid[4],NULL);
    pthread_join(optid[5],NULL);

    return 0;
}
