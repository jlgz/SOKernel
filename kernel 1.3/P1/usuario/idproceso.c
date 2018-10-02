#include "servicios.h"
#include <stdlib.h>
int  main(){
        int segundos = rand()%6 +1;
	printf(" identificador de proceso idproceso: %d y padre %d \n" ,get_pid(),get_ppid());
	printf(" me voy a dormir %d segundos \n", segundos);
	dormir(segundos);
	while(1)
	  printf(" identificador de proceso idproceso: %d y padre %d \n" ,get_pid(),get_ppid());
return 1;
}


