#include "servicios.h"
int main(){

	printf("padre de idProceso: comienza\n");

	
        
	if (crear_proceso("idproceso")<0)
                printf("Error creando idproceso\n");
	if (crear_proceso("idproceso")<0)
                printf("Error creando idproceso\n");
	printf("padre de idProceso: a dormir \n");
	dormir(4);
	int i=get_pid();
	if(i % 2 == 0) {
		printf("Yo padre id proceso: %d espero a que acaben mis hijos\n ", i); 
		espera();
	}
	return 0; 
}
