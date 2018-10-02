#include "servicios.h"
#include <stdlib.h>
int  main(){
	long  i = 0;
        int segundos = rand()%6 +1;
	printf(" identificador de proceso idproceso: %d y padre %d \n" ,get_pid(),get_ppid());
	printf(" me voy a dormir %d segundos \n", segundos);
	dormir(segundos);
	while(i<200000){
	  printf(" identificador de proceso idproceso: %d y padre %d \n" ,get_pid(),get_ppid());
	  i++;}
	return 1;
}


