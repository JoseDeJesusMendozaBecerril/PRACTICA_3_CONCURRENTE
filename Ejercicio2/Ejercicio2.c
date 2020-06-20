#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
const int ARRAY_TAM = 10000000;

long pqtEnvio=0;
long pqtRecibo=0;

int main(int argc, char *argv[])
{
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

      // Crear el arreglo de tamaño ARRAY_TAM
      arreglo = calloc(ARRAY_TAM,sizeof(int));

      // Llenarlo con los número 1, 2,...,ARRAY_TAM
      for (int i = 0; i < ARRAY_TAM; i++) arreglo[i] = i + 1;
      
      // Para cada proceso esclavo (q=1,...,comm_size) enviarle solamente los
      // elementos de la parte del arreglo que le corresponde.

      // Deben calcular el índice inicial del subarreglo para cada proceso y
      // luego enviar.
      for(int q = 1; q <= comm_size - 1; q++){ /* 1 2 3 4 ... numprocesos */
          if(q <= (ARRAY_TAM % (comm_size))) tamSubarreglo++;
          if (q == 1) inicio = tamSubarreglo;
          MPI_Send(&arreglo[inicio], tamSubarreglo, MPI_INT, q, 0, MPI_COMM_WORLD); //pos inicio arr , numElementos , tipo dato , proceso dest , 0 , comunicador
          inicio = inicio + tamSubarreglo; 
          tamSubarreglo = ARRAY_TAM / (comm_size);

      }

      // El maestro si hará cálculo.
      inicio=0;
      if(my_rank < (ARRAY_TAM % (comm_size))){ tamSubarreglo++;}
      else{tamSubarreglo = ARRAY_TAM / (comm_size);}
      long sumaM=0;
      for(int i=0; i <tamSubarreglo; i++ ){
        for(int j=0;j<1000;j++);
        sumaM+=(arreglo[i]*arreglo[i])/arreglo[i];
        
      }



      //RECEPCIONES

      int residuo=1;

      int L = 2;
      for(int i = 0; i < numPasos; i++){
          if(my_rank % L == 0 || my_rank == 0 ){ //RECEPCIONES
               MPI_Recv(&pqtRecibo,tamSubarreglo, MPI_LONG,my_rank + residuo, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
               sumaM =sumaM + pqtRecibo;
          }
          residuo = residuo * 2;
          L = L * 2;
      }

      printf("\nResultado final %ld\n",sumaM);

      free(arreglo);
   }
   else { // Esto lo harán los esclavos.
      //Este hilo esclavo debe reservar memoria para la parte que le toca.
      //Podrá usar también 'arreglo' porque cada proceso es independiente.
      // ¿De qué tamaño debe ser?

      if(my_rank <= (ARRAY_TAM % (comm_size))) tamSubarreglo++;

      arreglo = calloc(tamSubarreglo,sizeof(int)); //subarreglo

      // Debe Recibir en arreglo los elementos que deberá sumar.
      MPI_Recv(arreglo,tamSubarreglo, MPI_INT, MAESTRO, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      // Debe sumar los elementos de arreglo
      // Debe enviar un mensaje al MAESTRO con el resultado.
      long suma=0;
      for(int i=0; i <tamSubarreglo; i++ ){  
        for(int j=0;j<1000;j++);
        suma+=(arreglo[i]*arreglo[i])/arreglo[i];
      }
      int M = 2;
      int L=2;
      int residuo=1;
      //ENVIOS
      for (int i = 0; i < numPasos; i++) {
          if(my_rank % M == residuo ){ //ENVIOS
               pqtRecibo=suma;
               MPI_Send(&pqtRecibo,1, MPI_LONG, my_rank-residuo, 0, MPI_COMM_WORLD);
          }
          if(my_rank % L == 0 ){ //RECEPCIONES
              MPI_Recv(&pqtRecibo,tamSubarreglo, MPI_LONG,my_rank + residuo, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
              suma = suma + pqtRecibo;
          }
          residuo = residuo * 2;
          M = M*2;
          L=L*2;
      }
      free(arreglo);
   }
   MPI_Finalize();

   return 0;
} /* fin del main */
