#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "scd.h"
#include <mutex>

using namespace std ;
using namespace scd ;

//**********************************************************************
// Variables globales

const unsigned 
   num_items = 40 ,   // número de items
	tam_vec   = 10 ,   // tamaño del buffer
   num_productoras = 2,
   num_consumidoras = 2;
unsigned  
   cont_prod[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha producido.
   cont_cons[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha consumido.
   siguiente_dato       = 0 ;  // siguiente dato a producir en 'producir_dato' (solo se usa ahí)

Semaphore
   ocupadas(0),      // Inicialmente 0, el valor será: Insertados - extraidos
   libres(tam_vec);  // Inicialmente tam_vec, el valor será: 
                     // tam_vector + extraidos - insertados

int 
   primera_libre = 0,
   primera_ocupada = 0, 
   intermedio[tam_vec]; // Vector intermedio

mutex
   mtx1,
   mtx2;

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

unsigned producir_dato()
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   const unsigned dato_producido = siguiente_dato ;
   siguiente_dato++ ;
   cont_prod[dato_producido] ++ ;
   mtx1.lock();
   cout << "producido: " << dato_producido << endl << flush ;
   mtx1.unlock();
   return dato_producido ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   assert( dato < num_items );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   mtx1.lock();
   cout << "                  consumido: " << dato << endl ;
   mtx1.unlock();
}


//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." ;
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  if ( cont_prod[i] != 1 )
      {  cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {  cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

//----------------------------------------------------------------------

void  funcion_hebra_productora( int num_hebras )
{
   int items_hebra = num_items/num_productoras;

   for( unsigned i = 0 ; i < items_hebra ; i++ )
   {
      int dato = producir_dato() ;
      // LIFO
      sem_wait(libres);
      mtx2.lock();
      intermedio[primera_libre] = dato;
      primera_libre++;
      mtx2.unlock();
      sem_signal(ocupadas);
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora( int num_hebras )
{
   int items_hebra = num_items/num_consumidoras;

   for( unsigned i = 0 ; i < items_hebra ; i++ )
   {
      int dato ;
      // LIFO
      sem_wait(ocupadas);
      mtx2.lock();
      dato = intermedio[primera_libre-1];
      primera_libre--;
      mtx2.unlock();
      sem_signal(libres);
      consumir_dato( dato ) ;
    }
}
//----------------------------------------------------------------------

int main()
{
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución LIFO MULTIPLES)." << endl
        << "------------------------------------------------------------------" << endl
        << flush ;

   
   thread hebra_productora[num_productoras];
   thread hebra_consumidora[num_consumidoras];

   for (int i=0; i<num_productoras; i++){
      hebra_productora[i] = thread(funcion_hebra_productora,i);
   }

   for (int i=0; i<num_consumidoras; i++){
      hebra_consumidora[i] = thread(funcion_hebra_consumidora,i);
   }

   for (int i=0; i<num_productoras; i++){
      hebra_productora[i].join();
   }

   for (int i=0; i<num_consumidoras; i++){
      hebra_consumidora[i].join();
   }


   test_contadores();
}
