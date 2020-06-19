#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h> /* Para la funciones de MPI, etc. */
#include <math.h>
/*
    En esta version el usuario puede seleccionar cualquier numero de procesos no importa si divide o no al array
    y el proceso maestro tambien ayuda a la suma de los elementos
*/
double my_log(double x, int base) {
    return log(x) / log(base);
}

#define MAESTRO 0

// Tamaño del arreglo
const int ARRAY_TAM = 1000000;


int sumaM=0;
int suma=0;
int M=2;
int L=2;
int residuo=1;
int sumaNuevo;
int pqtEnvio=0;
int pqtRecibo=0;

int main(int argc, char *argv[]){
   int comm_size; /* Número de procesos.*/
   int my_rank;   /* Mi identificador de proceso.*/
   int* arreglo; // El proceso 0 deberá crearlo dinámicamente.
   int inicio=0;   // indice donde inicia el subarreglo que enviará el MAESTRO.
   int tamSubarreglo; // Número de elementos de cada subarreglo.

   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

   int numPasos= my_log(comm_size, 2);
   tamSubarreglo = ARRAY_TAM / (comm_size); // Cómo saber cuántos tocan para cada proceso?

   if (my_rank == MAESTRO) { // Soy el procesador maestro.
                   printf("Comm size %d\n",comm_size);
                   printf("Tam subarreglo %d\n",tamSubarreglo);
                   printf("Num Pasos %d\n",numPasos );

                  arreglo = calloc(ARRAY_TAM,sizeof(int));
                  for (int i = 0; i < ARRAY_TAM; i++) arreglo[i] = i + 1;
                  //for (int i = 0; i < ARRAY_TAM; i++) printf("%d ",arreglo[i]);

                  for(int q = 1; q <= comm_size - 1; q++){ /* 1 2 3 4 ... numprocesos */
                      if(q <= (ARRAY_TAM % (comm_size))) tamSubarreglo++;
                      if (q == 1) inicio = tamSubarreglo;
                      //printf("\nLe mando a process PID %d desde %d va a sumar un total de %d elementos ",q,inicio,tamSubarreglo);
                      MPI_Send(&arreglo[inicio], tamSubarreglo, MPI_INT, q, 0, MPI_COMM_WORLD); //pos inicio arr , numElementos , tipo dato , proceso dest , 0 , comunicador
                      inicio = inicio + tamSubarreglo; //** check
                      tamSubarreglo = ARRAY_TAM / (comm_size);
                  }
                  // El maestro si hará cálculo.
                  inicio=0;
                  if(my_rank < (ARRAY_TAM % (comm_size))){ tamSubarreglo++;}
                  else{tamSubarreglo = ARRAY_TAM / (comm_size);}

                  for(int i=0; i <tamSubarreglo; i++ ){
                     suma+=arreglo[i];
                  }
                  int parcial=0;
                  int final=0;
                  for(int q = 1; q <= comm_size - 1; q++){ /* 1 2 3 4 ... numprocesos */
                      MPI_Recv(&parcial, 1, MPI_INT, q, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                      final = final + parcial;
                  }
                  //free(arreglo);
   }
   else { // Esto lo harán los esclavos.

                  if(my_rank <= (ARRAY_TAM % (comm_size))) tamSubarreglo++;

                  arreglo = calloc(tamSubarreglo,sizeof(int)); //subarreglo

                  // Debe Recibir en arreglo los elementos que deberá sumar.
                  MPI_Recv(arreglo,tamSubarreglo, MPI_INT, MAESTRO, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                  // Debe sumar los elementos de arreglo
                  // Debe enviar un mensaje al MAESTRO con el resultado.
                  for(int i=0; i <tamSubarreglo; i++ ){
                     suma+=arreglo[i];
                  }
                  //**printf("Soy proceso con PID %d sume total %d\n",my_rank,suma);

                  for(int i=0; i < comm_size - 1 ; i++ ){
                      MPI_Send(&suma, 1, MPI_INT, MAESTRO, 0, MPI_COMM_WORLD);
                  }
    }


    /*Verificando que valor tiene cada Proceso*/
    printf("Soy proceso %d y tengo valor %d\n",my_rank,suma);


    int res2,res3;

    printf("Soy PROCESO %d y tengo valor %d\n",my_rank,sumaNuevo);
    int res = MPI_Barrier(MPI_COMM_WORLD);

    sumaNuevo=suma;
    //ENVIOS
    for (int i = 0; i < numPasos; i++) {
        if(my_rank % L == 0 ){
            MPI_Recv(&pqtRecibo,1, MPI_INT,my_rank + residuo, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            pqtEnvio=pqtRecibo+sumaNuevo;
            printf("Soy %d y voy a recibir de %d en el Paso %d valor %d ahora tengo %d \n",my_rank,my_rank+residuo,i,pqtRecibo,pqtEnvio);

        }

        if(my_rank % M == residuo ){
             pqtEnvio=sumaNuevo+pqtRecibo;
             MPI_Send(&pqtEnvio,1, MPI_INT, my_rank - residuo,0,MPI_COMM_WORLD);
             printf("Soy %d y voy a enviar a %d en el paso %d el valor de %d  \n",my_rank,my_rank - residuo,i,pqtEnvio);
             //pqtEnvio=sumaNuevo;
        }
        residuo = residuo * 2;
        M = M*2;
        L = L * 2;

    }


    MPI_Finalize();
   return 0;
}
