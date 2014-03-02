/*
 * Jérôme COUSSANES
 * jerome.coussanes@utbm.fr
 *
 *
 */


/*This function print the product state on the console*/
void printProd(Prod p);

/*This function perform the operation on the product according with its state*/
void op(Prod* p);

/*This function exchange the content of case1 with the content of case2 (two products)*/
void change(Prod* case1, Prod* case2);

/*This function check which operation have to be performed on p*/
int op_check(Prod p);

