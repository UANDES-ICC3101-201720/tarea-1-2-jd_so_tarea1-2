#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "types.h"
#include "const.h"
#include "util.h"

typedef struct data{
	UINT* dat;
	int lo;
	int hi;
	int ciclos;
}data;

int partition(UINT* A, int lo, int hi,int pivote)
{
    	int pivot;
    	pivot = A[pivote];
	UINT aux;
	aux= A[pivote];
	A[pivote]=A[hi];
	A[hi]=aux;
	int guardar= lo;
	for(int i=lo; i<hi;i++){
		if(A[i]<= pivot){
			UINT aux1;
			aux1= A[i];
			A[i]=A[guardar];
			A[guardar]=aux1;
			guardar++;}
	}
	UINT aux2;
	aux2= A[guardar];
	A[guardar]=A[hi];
	A[hi]=aux2;
	return guardar;
    
}

//implement->Listo!!!!!!!!!!!
int quicksort(UINT* A, int lo, int hi) {
	if(lo<hi){
		int x = A[lo];
		int s = lo;
		for(int i= lo+1;i<hi;i++){
			if(A[i]<=x){
				s = s+1;
				UINT aux;
				aux= A[s];
				A[s]=A[i];
				A[i]=aux;}
			}
		UINT aux1;
		aux1=A[lo];
		A[lo] = A[s];
		A[s]=aux1;
		quicksort(A,lo,s);
		quicksort(A,s+1,hi);
		}
    return 0;	
}

int proce_disp;

void parallel_quicksort(UINT* A, int lo, int hi,int ciclos);

void *conect(void * dat1){
	struct data *aux = dat1;
	parallel_quicksort(aux->dat, aux->lo,aux->hi,aux->ciclos);
	return NULL;
	}

// TODO: implement				
void parallel_quicksort(UINT* A, int lo, int hi,int ciclos) {
   
 	int j;
	
    	if( lo < hi ) 
    	{
		int pivote= lo +(hi-lo)/2;
    	    j = partition(A, lo, hi,pivote);
	
    	    if(ciclos > 0){
		struct data entrada = {A,lo,j-1,ciclos};
		pthread_t trencito;
		pthread_create(&trencito,NULL,conect, &entrada);
		parallel_quicksort(A,j+1,hi,ciclos);
		pthread_join(trencito,NULL);
		pivote--;}
    	    else
    	    {
    	        quicksort(A, lo, j - 1);
    	        quicksort(A, j + 1, hi);
    	    }
    	}
}


int main(int argc, char** argv) {
    printf("[quicksort] Starting up...\n");
    printf("[quicksort] Number of cores available: '%ld'\n",
           sysconf(_SC_NPROCESSORS_ONLN));

    /*parse arguments with getopt *///LISTOOOOO!!!
	int flag2;//dato tnumero de repeticiones->potencia de 10;
	int flag4;//dato eejecuciones secuenciales->se tienen que tener datos nuevos para cada;
	int c;
	opterr=0;
	
	while((c=getopt(argc,argv,"T:E:")) !=-1 )
	{
		switch(c){
			case 'T':
				flag2= atoi(optarg);
				break;
			case 'E':
				flag4= atoi(optarg);
				break;
			case '?':
				if(optopt=='T'){
					fprintf(stderr, "OPCION -%c requiere un argumento.\n",optopt);}
				if(optopt=='E'){
					fprintf(stderr, "OPCION -%c requiere un argumento.\n",optopt);}
				else
					fprintf(stderr,"Caracter desconocido");
				return 1;
			default:
				abort();
			 }
	}
	printf("Numeros dentro : %d , %d \n",flag2,flag4);
	
	if(flag4<1 || flag2<3 || 8<flag2){
	    	printf("Program terminated, value(s) out of range");
	    	exit(0);
	}
	
	//int size = pow(10,flag2);
    /* start datagen here as a child process. *///->Listo!!
	int pid;
	pid= fork();
	if(pid==0)
	{
		if(execv("./datagen",argv)<0){
			fprintf(stderr, "Fallo en execv :(");
			exit(0);}
	}
	if (pid==-1)
	{
		fprintf(stderr,"No se creo el proceso hijo\n");
		exit(0);
	}

	printf("[datagen] Iniciado correctamente \n");
	

    /* Create the domain socket to talk to datagen.*/ //LIsto!!!!!
    	struct sockaddr_un addr;
    	int fd;

    	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        	perror("[quicksort] Socket error.\n");
        	exit(-1);
    		}

    	memset(&addr, 0, sizeof(addr));
    	addr.sun_family = AF_UNIX;
    	strncpy(addr.sun_path, DSOCKET_PATH, sizeof(addr.sun_path) - 1);

    	if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        	perror("[quicksort] connect error.\n");
        	close(fd);
        	exit(-1);
    	}

	int ciclos = 2 * (sysconf(_SC_NPROCESSORS_ONLN));
	for(int j=0; j<flag4;j++){


        char begin[100];
    	char msg[80];
	int rc;
    	sprintf(msg, "%d", flag2);
    	strcpy(begin,"BEGIN U ");
	strcat(begin, msg);
        
/* Request the random number stream to datagen */
        if ((rc=write(fd, begin, sizeof(begin))) == -1) {
            if (rc > 0) fprintf(stderr, "[quicksort] partial write.\n");
            else {
                perror("[quicksort] write error.\n");
                exit(-1);
            }
        }

        /* validate the response */
        char respbuf[10];
        read(fd, respbuf, strlen(DATAGEN_OK_RESPONSE));
        respbuf[strlen(DATAGEN_OK_RESPONSE)] = '\0';

        if (strcmp(respbuf, DATAGEN_OK_RESPONSE)) {
            perror("[quicksort] Response from datagen failed.\n");
            close(fd);
            exit(-1);
        }

        UINT readvalues = 0;
        size_t numvalues = pow(10,flag2);
        size_t readbytes = 0;

        UINT *readbuf = malloc(sizeof(UINT) * numvalues); //-> readbuf nuestra data de datagen!!

        while (readvalues < numvalues) {
            /* read the bytestream */
            readbytes = read(fd, readbuf + readvalues, sizeof(UINT) * numvalues);
            readvalues += readbytes / 4;
        }
	printf("E%d: ",j+1);
	for (UINT *pv = readbuf; pv < readbuf + numvalues; pv++){
		printf("%u, ",*pv);
	}
	
	parallel_quicksort(readbuf,0,numvalues-1,ciclos);
	
	printf("\n\n S%d: ",j+1);
	
	for(UINT *pv=readbuf; pv<readbuf + numvalues; pv++){
		printf("%u, ",*pv);
	}
	//juntar readbuf con ambos quicksorts!!
	free(readbuf);
	int rd = strlen(DATAGEN_END_CMD);
	if (write(fd, DATAGEN_END_CMD, strlen(DATAGEN_END_CMD)) != rd) {
    	    if (rd > 0) fprintf(stderr, "[quicksort] partial write.\n");
    	    else {
    	        perror("[quicksort] write error.\n");
    	        close(fd);
    	    }
    	}
	}
		close(fd);		
	}	
