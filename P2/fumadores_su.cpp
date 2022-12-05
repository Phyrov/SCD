
#include <iostream>
#include <iomanip>
#include <cassert>
#include <random>
#include <thread>
#include "scd.h"

using namespace std ;
using namespace scd ;

const int n_fumadores = 3;

// *****************************************************************************
// clase para monitor estanco

class Estanco : public HoareMonitor
{
 private:
 int                        // variables permanentes
   ingrediente_mostrador;   // Indica que ingrediente hay en el mostrador
                           // El valor "vacío" será el -1 ya que hay 3 ingredientes

 CondVar                    // colas condicion:
   mostrador,                // cola donde espera el estanquero en caso de que el mostrador este ocupado  
   ingredientes[n_fumadores];

 public:                    // constructor y métodos públicos
   Estanco() ;             // constructor
   void obtenerIngrediente(int i);
   void ponerIngrediente(int i);
   void esperarRecogidaIngrediente();
} ;


// -----------------------------------------------------------------------------

Estanco::Estanco(  )
{
   ingrediente_mostrador = -1;
   mostrador      = newCondVar();
   for(int i = 0; i<n_fumadores; i++){
      ingredientes[i] = newCondVar();
   }
   
   
}

// *****************************************************************************
// función obtenerIngrediente
// Se recoge un ingrediente por lo que el mostrador pasa a estar vacio.
void Estanco::obtenerIngrediente(int i)
{
   if (ingrediente_mostrador != i)
      ingredientes[i].wait();

   assert( -1 < ingrediente_mostrador && 3 > ingrediente_mostrador);
   // Se indica que el mostrador está vacio
   ingrediente_mostrador = -1;
   mostrador.signal();

}

// *****************************************************************************
// función ponerIngrediente
// Se coloca el ingrediente i
void Estanco::ponerIngrediente(int i)
{
   // Se coloca en el mostrador el ingrediente pasado como parámetro
   ingrediente_mostrador = i;
   ingredientes[i].signal();
}

// *****************************************************************************
// función esperarRecogidaIngrediente
// Espera cuando no hay ingrediente en el mostrador, es decir, -1
void Estanco::esperarRecogidaIngrediente()
{
   if(ingrediente_mostrador!=-1)
      mostrador.wait();
}

//-------------------------------------------------------------------------
// Función que simula la acción de producir un ingrediente, como un retardo
// aleatorio de la hebra (devuelve número de ingrediente producido)

int producir_ingrediente()
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_produ( aleatorio<10,100>() );

   // informa de que comienza a producir
   cout << "Estanquero : empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produ );

   const int num_ingrediente = aleatorio<0,n_fumadores-1>() ;

   // informa de que ha terminado de producir
   cout << "Estanquero : termina de producir ingrediente " << num_ingrediente << endl;

   return num_ingrediente ;
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}

void funcion_hebra_fumador(MRef<Estanco> monitor, int i)
{
   while(true){
      monitor->obtenerIngrediente(i);
      cout << "Fumador " << i << "  : recoge el ingrediente" << endl;
      fumar(i);
   }
}

void funcion_hebra_estanquero(MRef<Estanco> monitor)
{
   int ingre;
   while(true){
      ingre = producir_ingrediente();
      cout << "Estanquero : coloca el ingrediente " << ingre << endl;
      monitor->ponerIngrediente(ingre);
       
      monitor->esperarRecogidaIngrediente();
   }
}

int main()
{
   cout << "--------------------------------------------------------------------" << endl
        << "Problema de los fumadores (Monitor SU). " << endl
        << "--------------------------------------------------------------------" << endl
        << flush ;

   // crear monitor  ('monitor' es una referencia al mismo, de tipo MRef<...>)
   MRef<Estanco> monitor = Create<Estanco>() ;

   // crear y lanzar las hebras
   thread
      hebra_estanquero(funcion_hebra_estanquero, monitor),
      hebra_fumador[n_fumadores];

   for(int i=0; i<n_fumadores; i++){
      hebra_fumador[i] = thread(funcion_hebra_fumador, monitor, i);
   }

   // esperar a que terminen las hebras
   hebra_estanquero.join();
   
   for(int i=0; i<n_fumadores; i++){
      hebra_fumador[i].join();
   }
}