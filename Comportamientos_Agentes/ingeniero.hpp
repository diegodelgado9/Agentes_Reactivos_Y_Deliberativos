#ifndef COMPORTAMIENTOINGENIERO_H
#define COMPORTAMIENTOINGENIERO_H

#include <chrono>
#include <list>
#include <map>
#include <set>
#include <thread>
#include <time.h>
#include "comportamientos/comportamiento.hpp"

using namespace std;

// =========================================================================
// ESTRUCTURAS PARA LA BÚSQUEDA DELIBERATIVA (NIVEL 2)
// =========================================================================

// creamos este struct como "foto" de la situación del agente en un momento puntual de 
// la planificación del nivel 2
struct EstadoI
{
  ubicacion posicion; // Guarda dónde estoy en mi cabeza (fila, columna y orientación), nos servira para saber a donde mirar en la siguiente iteración
  bool zapatillas;    // para saber si podemos atravesar bosques o surbir desniveles de valor 2 de altura

  // sobrecargamos == para comparar dos estados para saber si el buscador ha vuelta a una situación
  // que ya conocemos y así evitamos entrar en bucles sin intención alguna
  bool operator==(const EstadoI &st) const
  {
    return posicion.f == st.posicion.f && posicion.c == st.posicion.c && posicion.brujula == st.posicion.brujula && zapatillas == st.zapatillas;
  }
};

struct NodoI // el nodo es una mezcla del estado actual del agente mas una idea de como has llegado hasta ahí
{
  EstadoI estado; // la "foto" de la ubicación de esa situacion actual
  list<Action> secuencia; // Los pasos que he dado para llegar hasta aquí

  bool operator==(const NodoI &nodo) const
  {
    return estado == nodo.estado;
  }

  // Operador < necesario para usar 'set' y buscar rapidísimo sin quedarnos colgados en bucles
  bool operator<(const NodoI &nodo) const
  {
    if (estado.posicion.f < nodo.estado.posicion.f)
      return true;
    if (estado.posicion.f == nodo.estado.posicion.f && estado.posicion.c < nodo.estado.posicion.c)
      return true;
    if (estado.posicion.f == nodo.estado.posicion.f && estado.posicion.c == nodo.estado.posicion.c && estado.posicion.brujula < nodo.estado.posicion.brujula)
      return true;
    if (estado.posicion.f == nodo.estado.posicion.f && estado.posicion.c == nodo.estado.posicion.c && estado.posicion.brujula == nodo.estado.posicion.brujula && !estado.zapatillas && nodo.estado.zapatillas)
      return true;
    return false;
  }
};

// =========================================================================
// ESTRUCTURAS PARA LA RED DE TUBERÍAS (NIVEL 4)
// =========================================================================

// guardamos las coordenadas de la casilla actual y la operacion que hay que hacer en ella
struct EstadoTuberias
{
  int f; // fila
  int c; // columna
  int op; // operaciones -1 (DIG), 0 (Nada), 1 (RAISE)

  // sobrecargamos estos operadores para mas adelante
  bool operator==(const EstadoTuberias &st) const
  {
    return f == st.f && c == st.c && op == st.op;
  }

  bool operator<(const EstadoTuberias &st) const
  {
    if (f != st.f)
      return f < st.f;
    if (c != st.c)
      return c < st.c;
    return op < st.op;
  }
};


// guardamos el estado actual, las "obras" realizadas hasta aqui y la cuenta de tuberias y contaminación causada
struct NodoTuberias
{
  EstadoTuberias estado;
  list<Paso> secuencia; // Ruta de pasos constructivos
  int g_dist;           // Para priorizar rutas cortas (menos tubos = mejor)
  int g_eco;            // Para desempatar buscando el menor impacto ecológico

  bool operator==(const NodoTuberias &nodo) const
  {
    return estado == nodo.estado;
  }
};

// Comparador para la cola de prioridad (A* de tuberías)
struct ComparaNodosTuberias {
  bool operator()(const NodoTuberias &a, const NodoTuberias &b) const {
    if (a.g_dist != b.g_dist) return a.g_dist > b.g_dist; // 1º PRIORIDAD: Menos tuberías (distancia)
    return a.g_eco > b.g_eco;                             // 2º DESEMPATE: Ecología
  }
};

class ComportamientoIngeniero : public Comportamiento
{
public:
  // =========================================================================
  // CONSTRUCTORES
  // =========================================================================

  /**
   * @brief Constructor para niveles 0, 1 y 6 (sin mapa completo)
   * @param size Tamaño del mapa (si es 0, se inicializa más tarde)
   */
  ComportamientoIngeniero(unsigned int size = 0) : Comportamiento(size)
  {
    // Inicializamos las variables de estado que usaremos en Nivel 0 y 1
    tiene_zapatillas = false;
    modo_esquivar = false;
    giros_pendientes = 0;

    // Inicializamos el mapa de calor a 0 visitas
    for (int i = 0; i < 200; i++)
    {
      for (int j = 0; j < 200; j++)
      {
        mapa_visitas[i][j] = 0;
      }
    }

    // Variables para las máquinas de estado de niveles superiores
    hay_plan = false;
    hay_plan_tuberia = false;
    paso_actual = 0;
    estado_ing = 1;
    hay_plan_mov = false;
    fase_mision_6 = 0;
  }

  /**
   * @brief Constructor para niveles 2, 3, 4 y 5 (con mapa completo conocido)
   * @param mapaR Mapa de terreno conocido
   * @param mapaC Mapa de cotas conocido
   */
  ComportamientoIngeniero(std::vector<std::vector<unsigned char>> mapaR,
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
    hay_plan_tuberia = false;
    paso_actual = 0;
    estado_ing = 1;
    hay_plan_mov = false;
    fase_mision_6 = 0;
  }

  ComportamientoIngeniero(const ComportamientoIngeniero &comport)
      : Comportamiento(comport) {}
  ~ComportamientoIngeniero() {}

  Comportamiento *clone() { return new ComportamientoIngeniero(*this); }

  /**
   * @brief Bucle principal de decisión del agente.
   */
  Action think(Sensores sensores);

  // Niveles de la práctica
  Action ComportamientoIngenieroNivel_0(Sensores sensores);
  Action ComportamientoIngenieroNivel_1(Sensores sensores);
  Action ComportamientoIngenieroNivel_2(Sensores sensores);
  Action ComportamientoIngenieroNivel_3(Sensores sensores);
  Action ComportamientoIngenieroNivel_4(Sensores sensores);
  Action ComportamientoIngenieroNivel_5(Sensores sensores);
  Action ComportamientoIngenieroNivel_6(Sensores sensores);

  // =========================================================================
  // METODOS AUXILIARES
  // =========================================================================
  void ActualizarMapa(Sensores sensores);
  bool EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas);
  bool EsAccesiblePorAltura(const ubicacion &actual, bool zap);
  ubicacion Delante(const ubicacion &actual) const;
  bool es_camino(unsigned char c) const;

  void PintaPlan(const list<Action> &plan);
  void PintaPlan(const list<Paso> &plan);
  void VisualizaPlan(const ubicacion &st, const list<Action> &plan);
  void VisualizaRedTuberias(const list<Paso> &plan);
  int interact(Action accion, int valor);

private:
  // =========================================================================
  // VARIABLES DE ESTADO (Niveles Reactivos)
  // =========================================================================
  bool tiene_zapatillas;

  // Memoria a corto plazo para chocar y girar (Nivel 1)
  bool modo_esquivar;
  int giros_pendientes;

  // MAPA DE CALOR PRIVADO (Memoria de visitadas)
  int mapa_visitas[200][200];

  // =========================================================================
  // VARIABLES Y MÉTODOS PARA BÚSQUEDA DELIBERATIVA (NIVEL 2)
  // =========================================================================
  bool hay_plan;
  list<Action> plan;

  list<Action> AnchuraIngeniero(const EstadoI &inicio, const EstadoI &final, const std::vector<std::vector<unsigned char>> &terreno, const std::vector<std::vector<unsigned char>> &altura);
  bool CasillaAccesibleIngeniero(const EstadoI &st, Action accion, const std::vector<std::vector<unsigned char>> &terreno, const std::vector<std::vector<unsigned char>> &altura);
  EstadoI SiguienteCasillaIngeniero(const EstadoI &st, Action accion);
  EstadoI AplicarAccionIngeniero(Action accion, const EstadoI &st, const std::vector<std::vector<unsigned char>> &terreno, const std::vector<std::vector<unsigned char>> &altura);

  // =========================================================================
  // MÉTODOS PARA PLANIFICAR TUBERÍAS (NIVEL 4)
  // =========================================================================
  list<Paso> PlanificarTuberias(int bel_f, int bel_c, const std::vector<std::vector<unsigned char>> &mapa, const std::vector<std::vector<unsigned char>> &cotas);
  bool EsValidaParaTuberia(int f, int c, int op, const std::vector<std::vector<unsigned char>> &mapa, const std::vector<std::vector<unsigned char>> &cotas);
  int CosteEcoTuberia(char terreno, int op);

  // =========================================================================
  // VARIABLES PARA EL MONTAJE DE TUBERÍAS (NIVEL 5)
  // =========================================================================
  bool hay_plan_tuberia;
  std::vector<Paso> pasos_red;
  int paso_actual;
  int estado_ing;
  bool hay_plan_mov;
  std::list<Action> plan_mov;

  // Variable de fase para el Nivel 6 como miembro de clase (no static local)
  // para que se reinicie correctamente al comenzar una nueva partida.
  int fase_mision_6;
};

#endif
