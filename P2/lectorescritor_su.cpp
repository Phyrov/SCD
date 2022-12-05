
#include <iostream>
#include <iomanip>
#include <cassert>
#include <random>
#include <thread>
#include "scd.h"

using namespace std ;
using namespace scd ;

const int 
   n_lectores = 1,
   n_escritores = 2;


// *****************************************************************************
// clase para monitor estanco

class Lec_Esc : public HoareMonitor
{
 private:
                  // variables permanentes
 bool escrib;     // true si un escritor está escribiendo
                  // false si no hay escritores escribiendo

 int n_lec;       // No negativa
                  // Número de lectores que estan leyendo en un momento dado

 CondVar // colas condicion:
   lectura,
   escritura;

 public:                    // constructor y métodos públicos
   Lec_Esc() ;             // constructor
   void ini_lectura();
   void ini_escritura();
   void fin_lectura();
   void fin_escritura();
   
} ;


// -----------------------------------------------------------------------------
// Constructor
Lec_Esc::Lec_Esc(  )
{
     escrib = false;
     n_lec = 0;
     lectura = newCondVar();
     escritura = newCondVar();
}

// Usado por los lectores
void Lec_Esc::ini_lectura()
{
   if(escrib==true)
      lectura.wait();

   // Aumentan los lectores
   n_lec++;
   lectura.signal();
}

void Lec_Esc::fin_lectura() 
{
   n_lec--;
   if(n_lec==0)
      escritura.signal();
}

// Usado por los escritores
void Lec_Esc::ini_escritura()
{
   if(escrib==true || n_lec>0)
      escritura.wait();

   escrib = true;
}

void Lec_Esc::fin_escritura()
{
   escrib = false;
   // Seminario: Devuelve el número de hebras esperando en la cola
   if(lectura.get_nwt() != 0)
      lectura.signal();
   else
      escritura.signal();
}

void leer(int i)
{
   // calcular milisegundos aleatorios de duración de la acción de leer)
   chrono::milliseconds duracion_produ( aleatorio<50,100>() );

   // informa de que comienza a leer
   cout << "Lector " << i << " comienza a leer:" << duracion_produ.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produ );

   // informa de que ha terminado de leer
   cout << "Lector " << i << " : termina de leer" << endl;
}

void escribir(int i)
{
   // calcular milisegundos aleatorios de duración de la acción de leer)
   chrono::milliseconds duracion_produ( aleatorio<10,100>() );

   // informa de que comienza a leer
   cout << "Escritor " << i << " comienza a escribir:" << duracion_produ.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produ );

   // informa de que ha terminado de leer
   cout << "Escritor " << i << " : termina de escribir" << endl;
}

void funcion_hebra_lector(MRef<Lec_Esc> monitor, int i)
{
   while(true){
      monitor->ini_lectura();
      leer(i);
      monitor->fin_lectura();
   }
}

void funcion_hebra_escritor(MRef<Lec_Esc> monitor, int i)
{
   while(true){
      monitor->ini_escritura();
      escribir(i);
      monitor->fin_escritura();
   }
}
int main()
{
   cout << "--------------------------------------------------------------------" << endl
        << "Problema de lectores-escritores (SU). " << endl
        << "--------------------------------------------------------------------" << endl
        << flush ;

   // crear monitor  ('monitor' es una referencia al mismo, de tipo MRef<...>)
   MRef<Lec_Esc> monitor = Create<Lec_Esc>() ;

   // crear y lanzar las hebras
   thread
      hebra_lector[n_lectores],
      hebra_escritor[n_escritores];

   for(int i=0; i<n_lectores; i++){
      hebra_lector[i] = thread(funcion_hebra_lector, monitor, i);
   }

   for(int i=0; i<n_escritores; i++){
      hebra_escritor[i] = thread(funcion_hebra_escritor, monitor, i);
   }

   // esperar a que terminen las hebras
   
   for(int i=0; i<n_lectores; i++){
      hebra_lector[i].join();
   }

   for(int i=0; i<n_escritores; i++){
      hebra_escritor[i].join();
   }
}