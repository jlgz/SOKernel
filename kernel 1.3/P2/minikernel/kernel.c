/*
 *  kernel/kernel.c
 *
 *  Minikernel. Versión 1.0
 *
 *  Fernando Pérez Costoya
 *
 */

/*
 *
 * Fichero que contiene la funcionalidad del sistema operativo
 *
 */

#include "kernel.h"	/* Contiene defs. usadas por este modulo */

/*
 *
 * Funciones relacionadas con la tabla de procesos:
 *	iniciar_tabla_proc buscar_BCP_libre
 *
 */

/*
 * Función que inicia la tabla de procesos
 */
static void iniciar_tabla_proc(){
	int i;

	for (i=0; i<MAX_PROC; i++)
		tabla_procs[i].estado=NO_USADA;
}
static void muestra_lista(lista_BCPs *lista){
                BCP *bcptmp = lista->primero;
                while(bcptmp != NULL){
		  printk("id: %d estado: %d rodaja: %d vuelta: %d idp: %d Nhijos %d ticks %d \n",bcptmp->id,bcptmp->estado,bcptmp->rodaja, bcptmp->vuelta,bcptmp->ppid,bcptmp->num_hijos,bcptmp->ticks);
                        bcptmp = bcptmp->siguiente;
                }
                return;
}


/*
 * Función que busca una entrada libre en la tabla de procesos
 */
static int buscar_BCP_libre(){
	int i;

	for (i=0; i<MAX_PROC; i++)
		if (tabla_procs[i].estado==NO_USADA)
			return i;
	return -1;
}

/*
 *
 * Funciones que facilitan el manejo de las listas de BCPs
 *	insertar_ultimo eliminar_primero eliminar_elem
 *
 * NOTA: PRIMERO SE DEBE LLAMAR A eliminar Y LUEGO A insertar
 */

/*
 * Inserta un BCP al final de la lista.
 */
static void insertar_segundo(lista_BCPs *lista, BCP *proc){
        if(lista->primero== NULL){
		lista->primero = proc;
		lista->ultimo = proc;
	        proc->siguiente = NULL;
	}
	else {
		proc->siguiente= lista->primero->siguiente;
		if(lista->primero->siguiente == NULL)
		  lista->ultimo = proc;
		lista->primero->siguiente = proc;
	}
		 
}
static void insertar_ultimo(lista_BCPs *lista, BCP * proc){
	if (lista->primero==NULL)
		lista->primero= proc;
	else
		lista->ultimo->siguiente=proc;
	lista->ultimo= proc;
	proc->siguiente=NULL;
}

/*
 * Elimina el primer BCP de la lista.
 */
static void eliminar_primero(lista_BCPs *lista){

	if (lista->ultimo==lista->primero)
		lista->ultimo=NULL;
	lista->primero=lista->primero->siguiente;
}

/*
 * Elimina un determinado BCP de la lista.
 */
static void eliminar_elem(lista_BCPs *lista, BCP * proc){
	BCP *paux=lista->primero;

	if (paux==proc)
		eliminar_primero(lista);
	else {
		for ( ; ((paux) && (paux->siguiente!=proc));
			paux=paux->siguiente);
		if (paux) {
			if (lista->ultimo==paux->siguiente)
				lista->ultimo=paux;
			paux->siguiente=paux->siguiente->siguiente;
		}
	}
}

/*
 *
 * Funciones relacionadas con la planificacion
 *	espera_int planificador
 */

/*
 * Espera a que se produzca una interrupcion
 */
static void espera_int(){
	int nivel;

	printk("-> NO HAY LISTOS. ESPERA INT\n");

	/* Baja al mínimo el nivel de interrupción mientras espera */
	nivel=fijar_nivel_int(NIVEL_1);
	halt();
	fijar_nivel_int(nivel);
}

/*
 * Función de planificacion que implementa un algoritmo FIFO.
 */
static BCP * planificador(){
	while (lista_listos.primero==NULL)
		espera_int();		/* No hay nada que hacer */
	return lista_listos.primero;
}
/*
 *
 * Funcion auxiliar que termina proceso actual liberando sus recursos.
 * Usada por llamada terminar_proceso y por rutinas que tratan excepciones
 *
 */

//cambios de contexto
static void cambio_proceso(lista_BCPs *lista_destino){
	int nivel=fijar_nivel_int(NIVEL_3);
	BCP *p_proc_anterior = p_proc_actual;
	replanificacion_pendiente = 0;
	eliminar_primero(&lista_listos);
	p_proc_actual=planificador();
        p_proc_actual->estado = EJECUCION;
	printk("CAMBIO DE CONTEXTO de %d a %d \n ",p_proc_anterior->id,p_proc_actual->id);
	if (lista_destino == NULL){ 
		cambio_contexto(NULL, &(p_proc_actual->contexto_regs));
	}
	else {
		insertar_ultimo(lista_destino, p_proc_anterior);
		cambio_contexto(&(p_proc_anterior->contexto_regs), &(p_proc_actual->contexto_regs));}
	printk("lista dormidos : \n");
	muestra_lista(&lista_dormidos);
	printk("lista listos : \n");
	muestra_lista(&lista_listos);
	printk("lista espera : \n");
	muestra_lista(&lista_espera);
	fijar_nivel_int(nivel);
	return;
}
static void actualizar_padre(lista_BCPs *lista_destino){
        BCP *bcptmp = lista_destino->primero;
        while(bcptmp != NULL){
	  if(bcptmp->ppid == p_proc_actual->id){
	                bcptmp->ppid =0;
	                lista_espera.primero->num_hijos += 1;
	  }
	  if(bcptmp->id == p_proc_actual->ppid){bcptmp->num_hijos -=1;}
		        bcptmp = bcptmp->siguiente;
        }
	return;
}
static void liberar_proceso(){
	liberar_imagen(p_proc_actual->info_mem); /* liberar mapa */
	p_proc_actual->estado=TERMINADO;
	liberar_pila(p_proc_actual->pila);
	p_proc_actual->estado = TERMINADO;
	actualizar_padre(&lista_listos);
	actualizar_padre(&lista_dormidos);
	actualizar_padre(&lista_espera);
	cambio_proceso(NULL);
        return; /* no debería llegar aqui */
}

/*
 *
 * Funciones relacionadas con el tratamiento de interrupciones
 *	excepciones: exc_arit exc_mem
 *	interrupciones de reloj: int_reloj
 *	interrupciones del terminal: int_terminal
 *	llamadas al sistemas: llam_sis
 *	interrupciones SW: int_sw
 *
 */

/*
 * Tratamiento de excepciones aritmeticas
 */
static void exc_arit(){

	if (!viene_de_modo_usuario())
		panico("excepcion aritmetica cuando estaba dentro del kernel");


	printk("-> EXCEPCION ARITMETICA EN PROC %d\n", p_proc_actual->id);
	liberar_proceso();

        return; /* no debería llegar aqui */
}

/*
 * Tratamiento de excepciones en el acceso a memoria
 */
static void exc_mem(){

	if (!viene_de_modo_usuario())
		panico("excepcion de memoria cuando estaba dentro del kernel");


	printk("-> EXCEPCION DE MEMORIA EN PROC %d\n", p_proc_actual->id);
	liberar_proceso();

        return; /* no debería llegar aqui */
}

/*
 * Tratamiento de interrupciones de terminal
 */
static void int_terminal(){

	printk("-> TRATANDO INT. DE TERMINAL %c\n", leer_puerto(DIR_TERMINAL));

        return;
}


static void ajustar_rodaja(){
	p_proc_actual->rodaja -= 1;
	if(p_proc_actual->rodaja <=0){
		printk("rodaja  acabada \n");
		replanificacion_pendiente =1;
		activar_int_SW();}
		 
}
static void desbloquear(BCP *proc,lista_BCPs *lista){
	int nivel=fijar_nivel_int(NIVEL_3);
	eliminar_elem(lista, proc);
	proc->estado = LISTO;
	if(proc->rodaja == 0) {
		proc->rodaja = TICKS_POR_RODAJA;
		insertar_ultimo(&lista_listos, proc);
	}
	else{
	        insertar_segundo(&lista_listos, proc);
	}
	fijar_nivel_int(nivel);
	return;
}
static void ajustar_dormidos(){
		BCP *bcptmp = lista_dormidos.primero;
		BCP *bcptmp2; 
		while(bcptmp != NULL){
			bcptmp->ticks-=1;
		if (bcptmp->ticks <= 0) {
				bcptmp2 = bcptmp->siguiente;
				desbloquear(bcptmp, &lista_dormidos);
				bcptmp = bcptmp2;
			}
	
		else{	bcptmp = bcptmp->siguiente;}
		}
		return;
}
static void tratar_padre(){
  	BCP *bcptmp = lista_espera.primero;
	BCP *bcptmp2;
        while(bcptmp != NULL){
	  if(bcptmp->num_hijos == 0){
		bcptmp2 = bcptmp->siguiente;
		desbloquear(bcptmp,&lista_espera);
		bcptmp = bcptmp2;
	  }
	  else{
		bcptmp = bcptmp->siguiente;
          }
	}
	return;
}
/*
 * Tratamiento de interrupciones de reloj
 */
static void int_reloj(){
	printk("tick \n ");
	ajustar_dormidos();
	tratar_padre();
	if(p_proc_actual->estado == EJECUCION)
		ajustar_rodaja();
        return;
}

/*
 * Tratamiento de llamadas al sistema
 */
static void tratar_llamsis(){
	int nserv, res;

	nserv=leer_registro(0);
	if (nserv<NSERVICIOS)
		res=(tabla_servicios[nserv].fservicio)();
	else
		res=-1;		/* servicio no existente */
	escribir_registro(0,res);
	return;
}

/*
 * Tratamiento de interrupciuones software
 */
static void int_sw(){

	printk("-> TRATANDO INT. SW\n");
	if(replanificacion_pendiente){
		if(p_proc_actual->siguiente == NULL)
			p_proc_actual->rodaja = TICKS_POR_RODAJA;
		else{
			if(p_proc_actual->vuelta < 2){
				p_proc_actual->vuelta++;
				p_proc_actual->rodaja = TICKS_POR_RODAJA/2;
				p_proc_actual->estado = LISTO;
				cambio_proceso(&(lista_listos));
			}
			else {
				p_proc_actual->vuelta = 0;
				p_proc_actual->ticks = (TICKS_POR_RODAJA/4)*3;
				p_proc_actual->estado = BLOQUEADO;
				cambio_proceso(&(lista_dormidos));
			}
		
		}
		
	}
	return;
}

/*
 *
 * Funcion auxiliar que crea un proceso reservando sus recursos.
 * Usada por llamada crear_proceso.
 *
 */
static int crear_tarea(char *prog,int ppid){
	void * imagen, *pc_inicial;
	int error=0;
	int proc;
	BCP *p_proc;

	proc=buscar_BCP_libre();
	if (proc==-1)
		return -1;	/* no hay entrada libre */

	/* A rellenar el BCP ... */
	p_proc=&(tabla_procs[proc]);

	/* crea la imagen de memoria leyendo ejecutable */
	imagen=crear_imagen(prog, &pc_inicial);
	if (imagen)
	{
		p_proc->info_mem=imagen;
		p_proc->pila=crear_pila(TAM_PILA);
		fijar_contexto_ini(p_proc->info_mem, p_proc->pila, TAM_PILA,
			pc_inicial,
			&(p_proc->contexto_regs));
		p_proc->id=proc;
		p_proc->estado=LISTO;
		p_proc->rodaja=TICKS_POR_RODAJA;
		p_proc->vuelta=0;
		p_proc->num_hijos = 0;
		p_proc->ppid = ppid;
		p_proc->ticks = 0;
		/* lo inserta al final de cola de listos */
		insertar_ultimo(&lista_listos, p_proc);
		error= 0;
	}
	else
		error= -1; /* fallo al crear imagen */

	return error;
}


/*
 *
 * Rutinas que llevan a cabo las llamadas al sistema
 *	sis_crear_proceso sis_escribir
 *
 */
int sis_espera(){
  if(p_proc_actual->num_hijos == 0){return -1;}
  else{
        p_proc_actual->estado = BLOQUEADO;
	cambio_proceso(&(lista_espera));
	return 0;
  }
}
/*
 * Tratamiento de llamada al sistema crear_proceso. Llama a la
 * funcion auxiliar crear_tarea sis_terminar_proceso
 */
int sis_crear_proceso(){
	char *prog;
	int res;
	printk("-> PROC %d: CREAR PROCESO\n", p_proc_actual->id);
	prog=(char *)leer_registro(1);
	res=crear_tarea(prog,p_proc_actual->id);
	if(res != -1)
		p_proc_actual->num_hijos +=1;
	return res;
}

/*
 * Tratamiento de llamada al sistema escribir. Llama simplemente a la
 * funcion de apoyo escribir_ker
 */
int sis_escribir()
{
	char *texto;
	unsigned int longi;

	texto=(char *)leer_registro(1);
	longi=(unsigned int)leer_registro(2);

	escribir_ker(texto, longi);
	return 0;
}

/*
 * Tratamiento de llamada al sistema terminar_proceso. Llama a la
 * funcion auxiliar liberar_proceso
 */
int sis_terminar_proceso(){

	printk("-> FIN PROCESO %d\n", p_proc_actual->id);

	liberar_proceso();

        return 0; /* no debería llegar aqui */
}
/*
retorna id del  proceso
*/
int sis_get_pid(){
	return p_proc_actual->id;
}
int sis_get_ppid(){
        return p_proc_actual->ppid;
}
int sis_dormir() {
	unsigned int segundos = (leer_registro(1));
	unsigned int tickss = TICK * segundos;
	p_proc_actual->ticks = tickss;
	p_proc_actual->estado = BLOQUEADO;
	cambio_proceso(&(lista_dormidos));
	return 0;
}
/*
 *
 * Rutina de inicialización invocada en arranque
 *
 */

int main(){
	/* se llega con las interrupciones prohibidas */
	iniciar_tabla_proc();

	instal_man_int(EXC_ARITM, exc_arit); 
	instal_man_int(EXC_MEM, exc_mem); 
	instal_man_int(INT_RELOJ, int_reloj); 
	instal_man_int(INT_TERMINAL, int_terminal); 
	instal_man_int(LLAM_SIS, tratar_llamsis); 
	instal_man_int(INT_SW, int_sw); 

	iniciar_cont_int();		/* inicia cont. interr. */
	iniciar_cont_reloj(TICK);	/* fija frecuencia del reloj */
	iniciar_cont_teclado();		/* inici cont. teclado */

	/* crea proceso inicial */
	if (crear_tarea((void *)"init",-1)<0)
		panico("no encontrado el proceso inicial");
	/* activa proceso inicial */
	p_proc_actual=planificador();
	p_proc_actual->estado = EJECUCION;
	cambio_contexto(NULL, &(p_proc_actual->contexto_regs));
	panico("S.O. reactivado inesperadamente");
	return 0;
}
