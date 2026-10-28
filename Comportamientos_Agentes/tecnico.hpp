#ifndef COMPORTAMIENTOTECNICO_H
#define COMPORTAMIENTOTECNICO_H

#include <chrono>
#include <time.h>
#include <thread>
#include <list>
#include <queue>
#include <set>
#include <map> // Necesario para el filtro anti-timeout
#include "comportamientos/comportamiento.hpp"

// =========================================================================
// ESTRUCTURAS PARA LA BÚSQUEDA A* (NIVEL 3)
// =========================================================================
struct EstadoT
{
  // IGUAL QUE EN EL INGENIERO
  ubicacion posicion; // Guarda dónde estoy en mi cabeza (fila, columna y orientación), nos servira para saber a donde mirar en la siguiente iteración
  bool zapatillas;    // para saber si podemos atravesar bosques o surbir desniveles de valor 2 de altura

  // sirve para saber si dos casillas son exactamente iguales
  bool operator==(const EstadoT &st) const
  {
    return posicion.f == st.posicion.f && posicion.c == st.posicion.c &&
           posicion.brujula == st.posicion.brujula && zapatillas == st.zapatillas;
  }

  // esta es la primera gran diferencia con el ingeniero, el algoritmo A* no usa un simple set para los visitados, usa un map.
  // Necesitamos guardar si hemos estado en una casilla o no y su costo respecto a la batería. Tenemos que ordenarlas de menor a mayor
  // para que la busqueda sea lo más rapida posible
  bool operator<(const EstadoT &st) const
  {
    if (posicion.f != st.posicion.f)
      return posicion.f < st.posicion.f;
    if (posicion.c != st.posicion.c)
      return posicion.c < st.posicion.c;
    if (posicion.brujula != st.posicion.brujula)
      return posicion.brujula < st.posicion.brujula;
    return zapatillas < st.zapatillas;
  }
};

// en este nivel el nodo no solo guarda los pasos sino que también vamos a usarlo como "calculadora" de costes
struct NodoT
{
  // iniciamos igual que el ingeniero en el nivel 2, guardamos la "foto" actual y los pasos o "botones" pulsados hasta
  // aquí
  EstadoT estado;
  std::list<Action> secuencia;

  // usamos coste_g como la batería usada por el técnico hasta un esado en concreto
  int coste_g;
  // usamos heuristica_h como  un calculo a ojo de cúanto falta hasta la meta más o menos
  int heuristica_h;

  bool operator==(const NodoT &nodo) const
  {
    return estado == nodo.estado;
  }
};

// Comparador para A* (Priorizamos f(n) = g(n) + h(n))
struct ComparaNodosT
{
  bool operator()(const NodoT &a, const NodoT &b) const
  {
    int f_a = a.coste_g + a.heuristica_h;
    int f_b = b.coste_g + b.heuristica_h;

    // desempate: rompe los bucles infinitos  (Evita CPU TIME EXCEEDED)
    if (f_a == f_b)
    {
      return a.heuristica_h > b.heuristica_h; // En caso de empate, elige la casilla más cercana a la meta
    }
    return f_a > f_b;
  }
};

//============================================================================
//============================================================================
//============================================================================

class ComportamientoTecnico : public Comportamiento
{
public:
  // =========================================================================
  // CONSTRUCTORES
  // =========================================================================

  ComportamientoTecnico(unsigned int size = 0) : Comportamiento(size)
  {
    tiene_zapatillas = false;
    modo_esquivar = false;
    giros_pendientes = 0;

    for (int i = 0; i < 200; i++)
    {
      for (int j = 0; j < 200; j++)
      {
        mapa_visitas[i][j] = 0;
      }
    }

    hay_plan = false;
    en_cita = false;
    modo_construccion = false;
  }

  ComportamientoTecnico(std::vector<std::vector<unsigned char>> mapaR,
                        std::vector<std::vector<unsigned char>> mapaC) : Comportamiento(mapaR, mapaC)
  {
    tiene_zapatillas = false;
    modo_esquivar = false;
    giros_pendientes = 0;

    for (int i = 0; i < 200; i++)
    {
      for (int j = 0; j < 200; j++)
      {
        mapa_visitas[i][j] = 0;
      }
    }

    hay_plan = false;
    en_cita = false;
    modo_construccion = false;
  }

  ComportamientoTecnico(const ComportamientoTecnico &comport)
      : Comportamiento(comport) {}
  ~ComportamientoTecnico() {}

  Comportamiento *clone() { return new ComportamientoTecnico(*this); }

  Action think(Sensores sensores);

  Action ComportamientoTecnicoNivel_0(Sensores sensores);
  Action ComportamientoTecnicoNivel_1(Sensores sensores);
  Action ComportamientoTecnicoNivel_2(Sensores sensores);
  Action ComportamientoTecnicoNivel_3(Sensores sensores);
  Action ComportamientoTecnicoNivel_4(Sensores sensores);
  Action ComportamientoTecnicoNivel_5(Sensores sensores);
  Action ComportamientoTecnicoNivel_6(Sensores sensores);

  // =========================================================================
  // MÉTODOS AUXILIARES
  // =========================================================================
  void ActualizarMapa(Sensores sensores);
  bool EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas);
  bool EsAccesiblePorAltura(const ubicacion &actual);
  ubicacion Delante(const ubicacion &actual) const;
  bool es_camino(unsigned char c) const;

  void PintaPlan(const std::list<Action> &plan);
  void PintaPlan(const std::list<Paso> &plan);
  void VisualizaPlan(const ubicacion &st, const std::list<Action> &plan);
  int interact(Action accion, int valor);

private:
  // =========================================================================
  // VARIABLES DE ESTADO
  // =========================================================================
  bool tiene_zapatillas;
  bool modo_construccion;
  bool en_cita;

  bool modo_esquivar;
  int giros_pendientes;
  int mapa_visitas[200][200];

  // =========================================================================
  // VARIABLES Y MÉTODOS PARA BÚSQUEDA A* (NIVEL 3)
  // =========================================================================
  bool hay_plan;
  std::list<Action> plan;

  std::list<Action> AEstrellaTecnico(const EstadoT &inicio, const EstadoT &final, const std::vector<std::vector<unsigned char>> &terreno, const std::vector<std::vector<unsigned char>> &altura);
  bool CasillaAccesibleTecnico(const EstadoT &st, Action accion, const std::vector<std::vector<unsigned char>> &terreno, const std::vector<std::vector<unsigned char>> &altura);
  EstadoT SiguienteCasillaTecnico(const EstadoT &st, Action accion);
  EstadoT AplicarAccionTecnico(Action accion, const EstadoT &st, const std::vector<std::vector<unsigned char>> &terreno, const std::vector<std::vector<unsigned char>> &altura);

  int CosteEnergiaT(Action accion, char terreno_origen, int dif_altura, bool zap);
  int HeuristicaT(int f_actual, int c_actual, int f_meta, int c_meta);
};

#endif
