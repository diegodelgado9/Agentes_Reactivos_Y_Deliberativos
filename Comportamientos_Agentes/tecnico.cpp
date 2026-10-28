#include "tecnico.hpp"
#include "motorlib/util.h"
#include <iostream>
#include <queue>
#include <set>
#include <map>
#include <cstdlib>
#include <algorithm>

using namespace std;

// =========================================================================
// FUNCIONES DE APOYO PARA MIRAR EL ENTORNO
// =========================================================================

void CoordenadasLCRT(int rumbo, int f, int c, int &fL, int &cL, int &fC, int &cC, int &fR, int &cR)
{
  fC = f;
  cC = c;
  fL = f;
  cL = c;
  fR = f;
  cR = c;
  switch (rumbo)
  {
  case 0:
    fL--;
    cL--;
    fC--;
    fR--;
    cR++;
    break;
  case 1:
    fL--;
    fC--;
    cC++;
    cR++;
    break;
  case 2:
    fL--;
    cL++;
    cC++;
    fR++;
    cR++;
    break;
  case 3:
    cL++;
    fC++;
    cC++;
    fR++;
    break;
  case 4:
    fL++;
    cL++;
    fC++;
    fR++;
    cR--;
    break;
  case 5:
    fL++;
    fC++;
    cC--;
    cR--;
    break;
  case 6:
    fL++;
    cL--;
    cC--;
    fR--;
    cR--;
    break;
  case 7:
    cL--;
    fC--;
    cC--;
    fR--;
    break;
  }
  fL = max(0, min(199, fL));
  cL = max(0, min(199, cL));
  fC = max(0, min(199, fC));
  cC = max(0, min(199, cC));
  fR = max(0, min(199, fR));
  cR = max(0, min(199, cR));
}

// aquí no hay que tener en cuenta las zapatillas porque el técnico nunca se las pone
char ViablePorAlturaT(char casilla, int dif)
{
  if (abs(dif) <= 1)
    return casilla;
  return 'P';
}

// -------------------------------------------------------------------------
// VISIÓN NIVEL 0
// -------------------------------------------------------------------------

// tenemos en cuenta agentes desde el principio
int VeoCasillaInteresanteTN0(char i, char c, char d, bool zap, char aL, char aC, char aR, int vL, int vC, int vR)
{
  if (zap) {
  if (c == 'U' && aC == '_')
    return 2;
  if (d == 'U' && aR == '_')
    return 3;
  if (i == 'U' && aL == '_')
    return 1; }

  bool i_ok = (i == 'C' || i == 'S' || i == 'D') && (aL == '_');
  bool c_ok = (c == 'C' || c == 'S' || c == 'D') && (aC == '_');
  bool d_ok = (d == 'C' || d == 'S' || d == 'D') && (aR == '_');

  if (!i_ok && !c_ok && !d_ok)
    return 0;

  //priorizamos ir de frente y por eso restamos 1 al centro
  int scoreL = i_ok ? vL : 9999999;
  int scoreC = c_ok ? vC - 1 : 9999999;
  int scoreR = d_ok ? vR : 9999999;

  // la mejor opción será la casilla con menor puntuación actual 
  int best = min({scoreL, scoreC, scoreR});
  if (best == scoreC)
    return 2;
  if (best == scoreR)
    return 3;
  if (best == scoreL)
    return 1;

  return 0;
}

// -------------------------------------------------------------------------
// VISIÓN NIVEL 1
// -------------------------------------------------------------------------
int VeoCasillaInteresanteTN1(char i, char c, char d, char aL, char aC, char aR, int vL, int vC, int vR)
{
  if (c == 'U' && aC == '_')
    return 2;
  if (d == 'U' && aR == '_')
    return 3;
  if (i == 'U' && aL == '_')
    return 1;

 // prohibimos pasar por precipicios, agua, bosque, hierba y muros, para asegurar el paso por caminos y senderos
  bool i_ok = (i != 'M' && i != 'P' && i != 'A' && i != 'B' && i != 'H') && (aL == '_');
  bool c_ok = (c != 'M' && c != 'P' && c != 'A' && c != 'B' && c != 'H') && (aC == '_');
  bool d_ok = (d != 'M' && d != 'P' && d != 'A' && d != 'B' && d != 'H') && (aR == '_');

  // si no tenemos opción delante nos quedamos quietos
  if (!i_ok && !c_ok && !d_ok)
    return 0;

  // damos prioridad a los caminos y senderos
  auto calc_score = [](char obj, int visits)
  {
    int score = visits * 1000;
    if (obj == 'C' || obj == 'S')
      score -= 50;
    if (obj == 'T')
      score += 10;
    return score;
  };

  int randL = rand() % 20;
  int randC = rand() % 20;
  int randR = rand() % 20;

  // añadimos también aleatoridad para posibles bucles
  int scoreL = i_ok ? calc_score(i, vL) + randL : 9999999;
  //ir recto es ligeramente favorecido siempre
  int scoreC = c_ok ? calc_score(c, vC) + randC - 5 : 9999999;
  int scoreR = d_ok ? calc_score(d, vR) + randR : 9999999;

  int best = min({scoreL, scoreC, scoreR});
  if (best == scoreC)
    return 2;
  if (best == scoreR)
    return 3;
  if (best == scoreL)
    return 1;

  return 0;
}

// =========================================================================
// ÁREA DE IMPLEMENTACIÓN DEL ESTUDIANTE
// =========================================================================

Action ComportamientoTecnico::think(Sensores sensores)
{
  Action accion = IDLE;
  switch (sensores.nivel)
  {
  case 0:
    accion = ComportamientoTecnicoNivel_0(sensores);
    break;
  case 1:
    accion = ComportamientoTecnicoNivel_1(sensores);
    break;
  case 2:
    accion = ComportamientoTecnicoNivel_2(sensores);
    break;
  case 3:
    accion = ComportamientoTecnicoNivel_3(sensores);
    break;
  case 4:
    accion = ComportamientoTecnicoNivel_4(sensores);
    break;
  case 5:
    accion = ComportamientoTecnicoNivel_5(sensores);
    break;
  case 6:
    accion = ComportamientoTecnicoNivel_6(sensores);
    break;
  }
  return accion;
}

// -------------------------------------------------------------------------
// NIVEL 0
// -------------------------------------------------------------------------

Action ComportamientoTecnico::ComportamientoTecnicoNivel_0(Sensores sensores)
{
  ActualizarMapa(sensores);
  // guardamos en memoria lo que el agente ve (terreno y alturas)

  //si estamos en la meta se para
  if (sensores.superficie[0] == 'U')
    return IDLE;
  if (sensores.superficie[0] == 'D')
    tiene_zapatillas = true;

  bool zap = tiene_zapatillas;
  
  mapa_visitas[sensores.posF][sensores.posC]++;

  char aC = sensores.agentes[2];
  // si tenemos a un agente delante giramos derecha
  if (aC != '_')
    return TURN_SR;

  char i = ViablePorAlturaT(sensores.superficie[1], sensores.cota[1] - sensores.cota[0]);
  char c = ViablePorAlturaT(sensores.superficie[2], sensores.cota[2] - sensores.cota[0]);
  char d = ViablePorAlturaT(sensores.superficie[3], sensores.cota[3] - sensores.cota[0]);
  char aL = sensores.agentes[1];
  char aR = sensores.agentes[3];

  int fL, cL, fC, cC, fR, cR;
  CoordenadasLCRT(sensores.rumbo, sensores.posF, sensores.posC, fL, cL, fC, cC, fR, cR);

  int pos = VeoCasillaInteresanteTN0(i, c, d, zap, aL, aC, aR, mapa_visitas[fL][cL], mapa_visitas[fC][cC], mapa_visitas[fR][cR]);

  if (pos == 2)
    return WALK;
  if (pos == 1)
    return TURN_SL;
  if (pos == 3)
    return TURN_SR;

  return TURN_SR;
}

// -------------------------------------------------------------------------
// NIVEL 1
// -------------------------------------------------------------------------

Action ComportamientoTecnico::ComportamientoTecnicoNivel_1(Sensores sensores)
{
  ActualizarMapa(sensores);
  // aumento asi en este nivel porque me ha dado error de segmentos en varias ocasiones
  // de esta manera aseguro que nos encontramos dentro del mapa
  // si me fio al 100% valdria la opcion de antes mapa_visitas[sensores.posF][sensores.posC]++;
  int f = max(0, min(199, sensores.posF));
  int c = max(0, min(199, sensores.posC));
  mapa_visitas[f][c]++;

  char aC = sensores.agentes[2];

  if (aC != '_')
  {
    // vamos aumentando la impaciencia poco a poco en cada iteracion por un posible 
    // bloqueo entre agentes
    giros_pendientes++;
    if (giros_pendientes > 3)
    {
      giros_pendientes = 0;
      return TURN_SR;
    }
    return IDLE;
  }
  else
  {
    giros_pendientes = 0;
  }

  // al igual que con el ingeniero sacamos el valor de las casillas que tiene el técnico de frente
  char i = ViablePorAlturaT(sensores.superficie[1], sensores.cota[1] - sensores.cota[0]);
  char c_front = ViablePorAlturaT(sensores.superficie[2], sensores.cota[2] - sensores.cota[0]);
  char d = ViablePorAlturaT(sensores.superficie[3], sensores.cota[3] - sensores.cota[0]);
  char aL = sensores.agentes[1];
  char aR = sensores.agentes[3];

  int fL, cL, fC, cC, fR, cR;
  CoordenadasLCRT(sensores.rumbo, f, c, fL, cL, fC, cC, fR, cR);

  int pos = VeoCasillaInteresanteTN1(i, c_front, d, aL, aC, aR, mapa_visitas[fL][cL], mapa_visitas[fC][cC], mapa_visitas[fR][cR]);

  if (pos == 2)
    return WALK;
  if (pos == 1)
    return TURN_SL;
  if (pos == 3)
    return TURN_SR;

  // Si estoy pillado sin salida clara, meto aleatoriedad para salir del bucle
  return (rand() % 2 == 0) ? TURN_SL : TURN_SR;
}

// -------------------------------------------------------------------------
// NIVEL 2 (El tecnico no hace absolutamente nada en el nivel 2)
// -------------------------------------------------------------------------

Action ComportamientoTecnico::ComportamientoTecnicoNivel_2(Sensores sensores) { return IDLE; }


// -------------------------------------------------------------------------
// NIVEL 3 (BÚSQUEDA A* DEL TÉCNICO)
// -------------------------------------------------------------------------

Action ComportamientoTecnico::ComportamientoTecnicoNivel_3(Sensores sensores)
{

  //activamos zapatillas si estamos en su casilla
  if (sensores.superficie[0] == 'D') tiene_zapatillas = true;

  // IMPORTANTE, Si nos chocamos, el paso falló. Lo volvemos a meter en la lista 
  // para reintentarlo. Así NUNCA perdemos el progreso ni recalculamos, haciendo que el A* siga siendo eficiente
  // y no tenga que recalcular caminos una y otra vez
  
  if (sensores.choque && hay_plan) {
      plan.push_front(WALK);
  }

  // Trazamos el plan UNA ÚNICA VEZ
  if (!hay_plan)
  {
    EstadoT inicio, fin;
    inicio.posicion.f = sensores.posF;
    inicio.posicion.c = sensores.posC;
    inicio.posicion.brujula = sensores.rumbo;
    inicio.zapatillas = tiene_zapatillas;
    fin.posicion.f = sensores.BelPosF; 
    fin.posicion.c = sensores.BelPosC;

    plan = AEstrellaTecnico(inicio, fin, mapaResultado, mapaCotas);
    hay_plan = (plan.size() > 0);
    VisualizaPlan(inicio.posicion, plan);
  }

  // ejecutamos el plan tal y como lo hicimos en el ingenierio
  if (hay_plan && plan.size() > 0)
  {
    Action accion_plan = plan.front();

    // Si toca caminar pero el Ingeniero está delante, solo ESPERAMOS.
    if (accion_plan == WALK && sensores.agentes[2] != '_') {
        return IDLE; // NO hacemos pop_front(), la acción sigue guardada.
    }

    plan.pop_front();
    if (plan.size() == 0) hay_plan = false;
    return accion_plan;
  }

  return IDLE;
}

// -------------------------------------------------------------------------
// NIVEL 4 (Nuevamente no hacemos nada con el tecnico en este nivel)
// -------------------------------------------------------------------------

Action ComportamientoTecnico::ComportamientoTecnicoNivel_4(Sensores sensores) { return IDLE; }


// -------------------------------------------------------------------------
// NIVEL 5: EL BAILE DEL MONTAJE (TÉCNICO)
// -------------------------------------------------------------------------

Action ComportamientoTecnico::ComportamientoTecnicoNivel_5(Sensores sensores)
{
  // variables estáticas que las vamos a usar para ir guardando las coordenadas a las 
  // que el ingeniero le ha ordenado ir, iniciamos en -1 por seguridad
  static int meta_f = -1;
  static int meta_c = -1;

  // sensores para declarar posibles situaciones iniciales o no iniciales
  if (sensores.superficie[0] == 'D')
    tiene_zapatillas = true;
  if (sensores.choque)
    hay_plan = false;

  // estos sensores se activan tras la orden come del ingeniero
  if (sensores.venpaca)
  {
    meta_f = sensores.GotoF;
    meta_c = sensores.GotoC;
    en_cita = false; // aún no hemos llegado
    hay_plan = false; // tenemos que calcular como ir hasta ahí
  }

  // seguridad
  if (meta_f == -1)
    return IDLE;


  if (!en_cita)
  { 
    // si sus coordenadas actuales coinciden con la meta significa que hemos llegado al nodo A que nos 
    // había dicho el ingeniero en su case 3
    if (sensores.posF == meta_f && sensores.posC == meta_c)
    {
      en_cita = true;
      hay_plan = false;
    }
    else
    { 
      if (!hay_plan)
      {
        // si no hay plan usamos A* como en el nivel 3 para calcular la ruta mas eficiente
        EstadoT inicio, fin;
        inicio.posicion.f = sensores.posF;
        inicio.posicion.c = sensores.posC;
        inicio.posicion.brujula = sensores.rumbo;
        inicio.zapatillas = tiene_zapatillas;
        fin.posicion.f = meta_f;
        fin.posicion.c = meta_c;

        plan = AEstrellaTecnico(inicio, fin, mapaResultado, mapaCotas);
        hay_plan = plan.size() > 0;
        VisualizaPlan(inicio.posicion, plan);
      }
      
      if (hay_plan && plan.size() > 0)
      {
        Action accion_plan = plan.front();

        // Si el plan dice caminar, pero el Ingeniero está delante nos oquedamos quietos que ya se quitará con sus sensores
        if (accion_plan == WALK && sensores.agentes[2] != '_')
        {
          
          return IDLE; 
        }

        plan.pop_front();
        if (plan.size() == 0) hay_plan = false;
        return accion_plan;
      }

      return IDLE;
    }
  }   

      
  if (en_cita)
  {
    // realizamos este if por pura seguridad de nuevo, si por lo que sea no estoy en el nodo A
    // recalculo la ruta
    if (sensores.posF != meta_f || sensores.posC != meta_c)
    {
      en_cita = false;
      return IDLE;
    }
    if (sensores.agentes[2] == 'i')
    {
      // El Técnico NO tiene acciones DIG ni RAISE según el guión.
      // La modificación del terreno la hace el Ingeniero en su máquina de estados (casos 2 y 6).
      // El Técnico simplemente espera cara a cara y ejecuta INSTALL.
      if (sensores.enfrente)
      {
        en_cita = false;
        meta_f = -1;
        meta_c = -1;
        return INSTALL;
      }
      return IDLE;
    }
    else
    {
      // giramos para ponernos de cara con el ingeniero
      return TURN_SR;
    }
  }
  return IDLE;
}

// -------------------------------------------------------------------------
// NIVEL 6: MODO COOPERATIVO
// -------------------------------------------------------------------------
Action ComportamientoTecnico::ComportamientoTecnicoNivel_6(Sensores sensores)
{
  // igual que en con el ingeniero, nos aseguramos antes de hacer nada sobre la situación actual
  ActualizarMapa(sensores);
  if (sensores.superficie[0] == 'D')
    tiene_zapatillas = true;

  if (sensores.venpaca)
    modo_construccion = true;
  if (!modo_construccion)
  {
    // Explorar el mapa mientras esperamos. Así el A* tendrá más terreno conocido
    // cuando el Ingeniero invoque COME y evitará rutas por casillas ?
    return ComportamientoTecnicoNivel_1(sensores);
  }

  
  Action accion_elegida = ComportamientoTecnicoNivel_5(sensores);

  if (accion_elegida == WALK)
  {
    char frente = sensores.superficie[2];
    int dif = sensores.cota[2] - sensores.cota[0];

    // Si tenemos zapatillas podemos subir desniveles de 2, si no, solo 1
    int max_dif = tiene_zapatillas ? 2 : 1;

    // Abortamos si hay un muro, un precipicio, un bosque sin zapatillas, un desnivel mortal o un obstáculo móvil
    if (frente == 'M' || frente == 'P' || (!tiene_zapatillas && frente == 'B') || abs(dif) > max_dif || sensores.agentes[2] != '_')
    {
      hay_plan = false;
      return IDLE;
    }
  }
  return accion_elegida;
}




// =========================================================================
// =========================================================================
//                       METODOS PARA LOS NIVELES
// =========================================================================
// =========================================================================


// =========================================================================
// MÉTODOS DEL NIVEL 3: BÚSQUEDA A*
// =========================================================================

int ComportamientoTecnico::CosteEnergiaT(Action accion, char terreno_origen, int dif_altura, bool zap)
{
  // Penalización alta para terrain desconocido: el A* prefiere rutas conocidas.
  // Si no existe otra ruta, usará el terreno desconocido, pero solo como último recurso.
  if (terreno_origen == '?')
    return 500;

  int coste = 1;
  if (accion == WALK)
  {
    // El coste base depende de la Casilla Inicial (Origen)
    if (terreno_origen == 'A')
      coste = 60;
    else if (terreno_origen == 'H')
      coste = 6;
    else if (terreno_origen == 'S')
      coste = 3;
    else
      coste = 1; // Resto de casillas (Tierra 'T', Bosque 'B' con zapatillas, Piedra 'C', etc.)

    // El desnivel solo se aplica si salimos de Agua ('A'), Hierba ('H') o Arena ('S')
    if (terreno_origen == 'A' || terreno_origen == 'H' || terreno_origen == 'S')
    {
      if (dif_altura > 0)
        coste += 5;
      else if (dif_altura < 0)
        coste -= 2;
    }
  }
  else if (accion == TURN_SL || accion == TURN_SR)
  {
    // Girar depende de la Casilla Inicial (Origen)
    if (terreno_origen == 'A')
      coste = 5;
    else if (terreno_origen == 'H')
      coste = 2;
    else if (terreno_origen == 'S')
      coste = 1;
    else
      coste = 1; // Resto de casillas
  }

  // seguro por si el valor es menor que 1 (he tenido problemas en algunos test del bot de telegram con esto)
  return (coste > 0) ? coste : 1;
}


int ComportamientoTecnico::HeuristicaT(int f_actual, int c_actual, int f_meta, int c_meta)
{
  // calculamos la distancia entre mi casilla actual y la meta usando la "Distancia De Chebyshev"
  // como se puede caminar en diagonal la distancia real no tiene porque ser una L, tenemos que tener en cuenta
  // 8 direcciones y no 4

  return std::max(std::abs(f_actual - f_meta), std::abs(c_actual - c_meta));
}


EstadoT ComportamientoTecnico::SiguienteCasillaTecnico(const EstadoT &st, Action accion)
{
  //copiamos estado actual para no modificar el original
  EstadoT siguiente = st;

  // solo alteramos las coordernadas si la acción es caminar y luego miramos la brujula interna
  if (accion == WALK)
  {
    switch (st.posicion.brujula)
    {
    case norte:
      siguiente.posicion.f--;
      break;
    case noreste:
      siguiente.posicion.f--;
      siguiente.posicion.c++;
      break;
    case este:
      siguiente.posicion.c++;
      break;
    case sureste:
      siguiente.posicion.f++;
      siguiente.posicion.c++;
      break;
    case sur:
      siguiente.posicion.f++;
      break;
    case suroeste:
      siguiente.posicion.f++;
      siguiente.posicion.c--;
      break;
    case oeste:
      siguiente.posicion.c--;
      break;
    case noroeste:
      siguiente.posicion.f--;
      siguiente.posicion.c--;
      break;
    }
  }
  return siguiente;
}


bool ComportamientoTecnico::CasillaAccesibleTecnico(const EstadoT &st, Action accion, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura)
{
  // vemos a que casilla vamos a ir
  EstadoT siguiente = SiguienteCasillaTecnico(st, accion);

  // seguro para no salir del mapa y dejarnos en bucle colgados
  if (siguiente.posicion.f < 0 || siguiente.posicion.f >= terreno.size() ||
      siguiente.posicion.c < 0 || siguiente.posicion.c >= terreno[0].size())
    return false;


  if (accion == WALK)
  {
    char destino = terreno[siguiente.posicion.f][siguiente.posicion.c];

    // Muros y precipicios son intransitables siempre
    if (destino == 'M' || destino == 'P')
      return false;

    // El bosque solo se puede pisar si tienes zapatillas
    if (destino == 'B' && !st.zapatillas)
      return false;
    
    // permitimos pasar por lo desconocido en este caso
    if (destino == '?')
      return true;

    // El Técnico NUNCA puede escalar desniveles mayores a 1, tenga zapatillas o no.
    int dif = altura[siguiente.posicion.f][siguiente.posicion.c] - altura[st.posicion.f][st.posicion.c];
    return (std::abs(dif) <= 1);
  }
  return true;
}


EstadoT ComportamientoTecnico::AplicarAccionTecnico(Action accion, const EstadoT &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura)
{
  EstadoT siguiente = st;

  if (accion == WALK)
  {
    siguiente = SiguienteCasillaTecnico(st, accion);
    if (terreno[siguiente.posicion.f][siguiente.posicion.c] == 'D')
      siguiente.zapatillas = true;
  }
  else if (accion == TURN_SR)
  {
    // el %8 asegura que si estás en la dirección 7 y giras, pases a la 0 (círculo perfecto). 
    // Igual para la izquierda
    siguiente.posicion.brujula = (Orientacion)((siguiente.posicion.brujula + 1) % 8);
  }
  else if (accion == TURN_SL)
  {
    siguiente.posicion.brujula = (Orientacion)((siguiente.posicion.brujula + 7) % 8);
  }
  return siguiente;
}


list<Action> ComportamientoTecnico::AEstrellaTecnico(const EstadoT &inicio, const EstadoT &final, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura)
{
  // la cola de espera ordenada por la mejor nota final f
  priority_queue<NodoT, vector<NodoT>, ComparaNodosT> frontera;
  // set para saber que casillas hemos visitado
  set<EstadoT> explorados;
  // mapa para guardar el récord de batéria mas barato para cada casilla
  map<EstadoT, int> distancias; // Filtro Anti-CPU Time Exceeded (he tenido varios errores con los test debido a este problema)

  NodoT nodo_inicial;
  nodo_inicial.estado = inicio; // Guardamos la "foto" de nuestra posición inicial
  nodo_inicial.coste_g = 0;     // dejamos la batería inicial en 0
  nodo_inicial.heuristica_h = HeuristicaT(inicio.posicion.f, inicio.posicion.c, final.posicion.f, final.posicion.c); // Intuición de lo que falta hasta la meta

  frontera.push(nodo_inicial); // Metemos el inicio a la priority_queue
  distancias[inicio] = 0;      // Anotamos en el map que llegar al inicio cuesta 0

  Action acciones[] = {WALK, TURN_SL, TURN_SR}; 

  // A* funciona mientras queden caminos por revisar en la cola)
  while (!frontera.empty())
  {
    NodoT actual = frontera.top(); // Sacamos el camino más prometedor
    frontera.pop();                // Lo borramos de la lista de espera

    // si las coordenadas del nodo que acabamos de sacar coinciden con la meta
    if (actual.estado.posicion.f == final.posicion.f && actual.estado.posicion.c == final.posicion.c)
    {
      return actual.secuencia; // Entregamos el plan con las instrucciones ganadoras 
    }

    // Si esta "foto" exacta ya la habíamos cerrado antes, la ignoramos y pasamos al siguiente.
    // (Como sacamos lo más barato primero, si repetimos casilla seguro que este camino es peor).
    if (explorados.find(actual.estado) != explorados.end())
      continue;
    explorados.insert(actual.estado); // Si es nueva, la "cerramos" metiéndola en la lista negra de visitados

    // Generamos a los hijos probando las 3 acciones posibles
    for (Action acc : acciones)
    {
      if (CasillaAccesibleTecnico(actual.estado, acc, terreno, altura))
      {
        NodoT hijo;
        // Actualizamos la "foto" mental del hijo (le cambiamos la coordenada o le giramos la brújula)
        hijo.estado = AplicarAccionTecnico(acc, actual.estado, terreno, altura);
        
        // El hijo hereda la libreta de instrucciones del padre y apunta su nueva acción al final
        hijo.secuencia = actual.secuencia;
        hijo.secuencia.push_back(acc);

        // Preparamos las variables necesarias para calcular la batería
        char char_origen = terreno[actual.estado.posicion.f][actual.estado.posicion.c];
        int dif_alt = altura[hijo.estado.posicion.f][hijo.estado.posicion.c] - altura[actual.estado.posicion.f][actual.estado.posicion.c];

        // actualizamos la g
        hijo.coste_g = actual.coste_g + CosteEnergiaT(acc, char_origen, dif_alt, actual.estado.zapatillas);
        
        // actualizamos la h
        hijo.heuristica_h = HeuristicaT(hijo.estado.posicion.f, hijo.estado.posicion.c, final.posicion.f, final.posicion.c);

        // (ANTI TIME-OUT) Buscamos si ya existía un récord de batería para esta casilla
        auto it = distancias.find(hijo.estado);
        
        // Solo metemos al hijo en la cola si:
        // 1. Es la primera vez que pisamos esta casilla en toda la historia (it == end)
        // 2. O si hemos encontrado una ruta que gasta MENOS batería que el récord anterior (< it->second)
        if (it == distancias.end() || hijo.coste_g < it->second)
        {
          distancias[hijo.estado] = hijo.coste_g; // Actualizamos el récord de precio
          frontera.push(hijo);                    // Metemos al hijo en la cola VIP para que el while lo analice luego
        }
      }
    }
  }
  
  // Si la cola se vacía entera y el while termina sin hacer el return de arriba,
  // significa que estamos encerrados entre muros y no hay camino posible.
  return list<Action>();
}



// ------------------------------------------------------------------------
// MÉTODOS OBLIGATORIOS Y HERRAMIENTAS DE DIBUJO (PROFESORES)
// ------------------------------------------------------------------------

bool ComportamientoTecnico::es_camino(unsigned char c) const
{
  return (c == 'C' || c == 'D' || c == 'U' || c == 'S');
}

void ComportamientoTecnico::ActualizarMapa(Sensores sensores)
{
  mapaResultado[sensores.posF][sensores.posC] = sensores.superficie[0];
  mapaCotas[sensores.posF][sensores.posC] = sensores.cota[0];
  int pos = 1;
  switch (sensores.rumbo)
  {
  case norte:
    for (int j = 1; j < 4; j++)
      for (int i = -j; i <= j; i++)
      {
        mapaResultado[sensores.posF - j][sensores.posC + i] = sensores.superficie[pos];
        mapaCotas[sensores.posF - j][sensores.posC + i] = sensores.cota[pos++];
      }
    break;
  case noreste:
    mapaResultado[sensores.posF - 1][sensores.posC] = sensores.superficie[1];
    mapaCotas[sensores.posF - 1][sensores.posC] = sensores.cota[1];
    mapaResultado[sensores.posF - 1][sensores.posC + 1] = sensores.superficie[2];
    mapaCotas[sensores.posF - 1][sensores.posC + 1] = sensores.cota[2];
    mapaResultado[sensores.posF][sensores.posC + 1] = sensores.superficie[3];
    mapaCotas[sensores.posF][sensores.posC + 1] = sensores.cota[3];
    mapaResultado[sensores.posF - 2][sensores.posC] = sensores.superficie[4];
    mapaCotas[sensores.posF - 2][sensores.posC] = sensores.cota[4];
    mapaResultado[sensores.posF - 2][sensores.posC + 1] = sensores.superficie[5];
    mapaCotas[sensores.posF - 2][sensores.posC + 1] = sensores.cota[5];
    mapaResultado[sensores.posF - 2][sensores.posC + 2] = sensores.superficie[6];
    mapaCotas[sensores.posF - 2][sensores.posC + 2] = sensores.cota[6];
    mapaResultado[sensores.posF - 1][sensores.posC + 2] = sensores.superficie[7];
    mapaCotas[sensores.posF - 1][sensores.posC + 2] = sensores.cota[7];
    mapaResultado[sensores.posF][sensores.posC + 2] = sensores.superficie[8];
    mapaCotas[sensores.posF][sensores.posC + 2] = sensores.cota[8];
    mapaResultado[sensores.posF - 3][sensores.posC] = sensores.superficie[9];
    mapaCotas[sensores.posF - 3][sensores.posC] = sensores.cota[9];
    mapaResultado[sensores.posF - 3][sensores.posC + 1] = sensores.superficie[10];
    mapaCotas[sensores.posF - 3][sensores.posC + 1] = sensores.cota[10];
    mapaResultado[sensores.posF - 3][sensores.posC + 2] = sensores.superficie[11];
    mapaCotas[sensores.posF - 3][sensores.posC + 2] = sensores.cota[11];
    mapaResultado[sensores.posF - 3][sensores.posC + 3] = sensores.superficie[12];
    mapaCotas[sensores.posF - 3][sensores.posC + 3] = sensores.cota[12];
    mapaResultado[sensores.posF - 2][sensores.posC + 3] = sensores.superficie[13];
    mapaCotas[sensores.posF - 2][sensores.posC + 3] = sensores.cota[13];
    mapaResultado[sensores.posF - 1][sensores.posC + 3] = sensores.superficie[14];
    mapaCotas[sensores.posF - 1][sensores.posC + 3] = sensores.cota[14];
    mapaResultado[sensores.posF][sensores.posC + 3] = sensores.superficie[15];
    mapaCotas[sensores.posF][sensores.posC + 3] = sensores.cota[15];
    break;
  case este:
    for (int j = 1; j < 4; j++)
      for (int i = -j; i <= j; i++)
      {
        mapaResultado[sensores.posF + i][sensores.posC + j] = sensores.superficie[pos];
        mapaCotas[sensores.posF + i][sensores.posC + j] = sensores.cota[pos++];
      }
    break;
  case sureste:
    mapaResultado[sensores.posF][sensores.posC + 1] = sensores.superficie[1];
    mapaCotas[sensores.posF][sensores.posC + 1] = sensores.cota[1];
    mapaResultado[sensores.posF + 1][sensores.posC + 1] = sensores.superficie[2];
    mapaCotas[sensores.posF + 1][sensores.posC + 1] = sensores.cota[2];
    mapaResultado[sensores.posF + 1][sensores.posC] = sensores.superficie[3];
    mapaCotas[sensores.posF + 1][sensores.posC] = sensores.cota[3];
    mapaResultado[sensores.posF][sensores.posC + 2] = sensores.superficie[4];
    mapaCotas[sensores.posF][sensores.posC + 2] = sensores.cota[4];
    mapaResultado[sensores.posF + 1][sensores.posC + 2] = sensores.superficie[5];
    mapaCotas[sensores.posF + 1][sensores.posC + 2] = sensores.cota[5];
    mapaResultado[sensores.posF + 2][sensores.posC + 2] = sensores.superficie[6];
    mapaCotas[sensores.posF + 2][sensores.posC + 2] = sensores.cota[6];
    mapaResultado[sensores.posF + 2][sensores.posC + 1] = sensores.superficie[7];
    mapaCotas[sensores.posF + 2][sensores.posC + 1] = sensores.cota[7];
    mapaResultado[sensores.posF + 2][sensores.posC] = sensores.superficie[8];
    mapaCotas[sensores.posF + 2][sensores.posC] = sensores.cota[8];
    mapaResultado[sensores.posF][sensores.posC + 3] = sensores.superficie[9];
    mapaCotas[sensores.posF][sensores.posC + 3] = sensores.cota[9];
    mapaResultado[sensores.posF + 1][sensores.posC + 3] = sensores.superficie[10];
    mapaCotas[sensores.posF + 1][sensores.posC + 3] = sensores.cota[10];
    mapaResultado[sensores.posF + 2][sensores.posC + 3] = sensores.superficie[11];
    mapaCotas[sensores.posF + 2][sensores.posC + 3] = sensores.cota[11];
    mapaResultado[sensores.posF + 3][sensores.posC + 3] = sensores.superficie[12];
    mapaCotas[sensores.posF + 3][sensores.posC + 3] = sensores.cota[12];
    mapaResultado[sensores.posF + 3][sensores.posC + 2] = sensores.superficie[13];
    mapaCotas[sensores.posF + 3][sensores.posC + 2] = sensores.cota[13];
    mapaResultado[sensores.posF + 3][sensores.posC + 1] = sensores.superficie[14];
    mapaCotas[sensores.posF + 3][sensores.posC + 1] = sensores.cota[14];
    mapaResultado[sensores.posF + 3][sensores.posC] = sensores.superficie[15];
    mapaCotas[sensores.posF + 3][sensores.posC] = sensores.cota[15];
    break;
  case sur:
    for (int j = 1; j < 4; j++)
      for (int i = -j; i <= j; i++)
      {
        mapaResultado[sensores.posF + j][sensores.posC - i] = sensores.superficie[pos];
        mapaCotas[sensores.posF + j][sensores.posC - i] = sensores.cota[pos++];
      }
    break;
  case suroeste:
    mapaResultado[sensores.posF + 1][sensores.posC] = sensores.superficie[1];
    mapaCotas[sensores.posF + 1][sensores.posC] = sensores.cota[1];
    mapaResultado[sensores.posF + 1][sensores.posC - 1] = sensores.superficie[2];
    mapaCotas[sensores.posF + 1][sensores.posC - 1] = sensores.cota[2];
    mapaResultado[sensores.posF][sensores.posC - 1] = sensores.superficie[3];
    mapaCotas[sensores.posF][sensores.posC - 1] = sensores.cota[3];
    mapaResultado[sensores.posF + 2][sensores.posC] = sensores.superficie[4];
    mapaCotas[sensores.posF + 2][sensores.posC] = sensores.cota[4];
    mapaResultado[sensores.posF + 2][sensores.posC - 1] = sensores.superficie[5];
    mapaCotas[sensores.posF + 2][sensores.posC - 1] = sensores.cota[5];
    mapaResultado[sensores.posF + 2][sensores.posC - 2] = sensores.superficie[6];
    mapaCotas[sensores.posF + 2][sensores.posC - 2] = sensores.cota[6];
    mapaResultado[sensores.posF + 1][sensores.posC - 2] = sensores.superficie[7];
    mapaCotas[sensores.posF + 1][sensores.posC - 2] = sensores.cota[7];
    mapaResultado[sensores.posF][sensores.posC - 2] = sensores.superficie[8];
    mapaCotas[sensores.posF][sensores.posC - 2] = sensores.cota[8];
    mapaResultado[sensores.posF + 3][sensores.posC] = sensores.superficie[9];
    mapaCotas[sensores.posF + 3][sensores.posC] = sensores.cota[9];
    mapaResultado[sensores.posF + 3][sensores.posC - 1] = sensores.superficie[10];
    mapaCotas[sensores.posF + 3][sensores.posC - 1] = sensores.cota[10];
    mapaResultado[sensores.posF + 3][sensores.posC - 2] = sensores.superficie[11];
    mapaCotas[sensores.posF + 3][sensores.posC - 2] = sensores.cota[11];
    mapaResultado[sensores.posF + 3][sensores.posC - 3] = sensores.superficie[12];
    mapaCotas[sensores.posF + 3][sensores.posC - 3] = sensores.cota[12];
    mapaResultado[sensores.posF + 2][sensores.posC - 3] = sensores.superficie[13];
    mapaCotas[sensores.posF + 2][sensores.posC - 3] = sensores.cota[13];
    mapaResultado[sensores.posF + 1][sensores.posC - 3] = sensores.superficie[14];
    mapaCotas[sensores.posF + 1][sensores.posC - 3] = sensores.cota[14];
    mapaResultado[sensores.posF][sensores.posC - 3] = sensores.superficie[15];
    mapaCotas[sensores.posF][sensores.posC - 3] = sensores.cota[15];
    break;
  case oeste:
    for (int j = 1; j < 4; j++)
      for (int i = -j; i <= j; i++)
      {
        mapaResultado[sensores.posF - i][sensores.posC - j] = sensores.superficie[pos];
        mapaCotas[sensores.posF - i][sensores.posC - j] = sensores.cota[pos++];
      }
    break;
  case noroeste:
    mapaResultado[sensores.posF][sensores.posC - 1] = sensores.superficie[1];
    mapaCotas[sensores.posF][sensores.posC - 1] = sensores.cota[1];
    mapaResultado[sensores.posF - 1][sensores.posC - 1] = sensores.superficie[2];
    mapaCotas[sensores.posF - 1][sensores.posC - 1] = sensores.cota[2];
    mapaResultado[sensores.posF - 1][sensores.posC] = sensores.superficie[3];
    mapaCotas[sensores.posF - 1][sensores.posC] = sensores.cota[3];
    mapaResultado[sensores.posF][sensores.posC - 2] = sensores.superficie[4];
    mapaCotas[sensores.posF][sensores.posC - 2] = sensores.cota[4];
    mapaResultado[sensores.posF - 1][sensores.posC - 2] = sensores.superficie[5];
    mapaCotas[sensores.posF - 1][sensores.posC - 2] = sensores.cota[5];
    mapaResultado[sensores.posF - 2][sensores.posC - 2] = sensores.superficie[6];
    mapaCotas[sensores.posF - 2][sensores.posC - 2] = sensores.cota[6];
    mapaResultado[sensores.posF - 2][sensores.posC - 1] = sensores.superficie[7];
    mapaCotas[sensores.posF - 2][sensores.posC - 1] = sensores.cota[7];
    mapaResultado[sensores.posF - 2][sensores.posC] = sensores.superficie[8];
    mapaCotas[sensores.posF - 2][sensores.posC] = sensores.cota[8];
    mapaResultado[sensores.posF][sensores.posC - 3] = sensores.superficie[9];
    mapaCotas[sensores.posF][sensores.posC - 3] = sensores.cota[9];
    mapaResultado[sensores.posF - 1][sensores.posC - 3] = sensores.superficie[10];
    mapaCotas[sensores.posF - 1][sensores.posC - 3] = sensores.cota[10];
    mapaResultado[sensores.posF - 2][sensores.posC - 3] = sensores.superficie[11];
    mapaCotas[sensores.posF - 2][sensores.posC - 3] = sensores.cota[11];
    mapaResultado[sensores.posF - 3][sensores.posC - 3] = sensores.superficie[12];
    mapaCotas[sensores.posF - 3][sensores.posC - 3] = sensores.cota[12];
    mapaResultado[sensores.posF - 3][sensores.posC - 2] = sensores.superficie[13];
    mapaCotas[sensores.posF - 3][sensores.posC - 2] = sensores.cota[13];
    mapaResultado[sensores.posF - 3][sensores.posC - 1] = sensores.superficie[14];
    mapaCotas[sensores.posF - 3][sensores.posC - 1] = sensores.cota[14];
    mapaResultado[sensores.posF - 3][sensores.posC] = sensores.superficie[15];
    mapaCotas[sensores.posF - 3][sensores.posC] = sensores.cota[15];
    break;
  }
}

/**
 * @brief Determina si una casilla es transitable para el técnico.
 * En esta práctica, si el técnico tiene zapatillas, el bosque ('B') es transitable.
 * @param f Fila de la casilla.
 * @param c Columna de la casilla.
 * @param tieneZapatillas Indica si el agente posee las zapatillas.
 * @return true si la casilla es transitable.
 */
bool ComportamientoTecnico::EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas)
{
  if (f < 0 || f >= mapaResultado.size() || c < 0 || c >= mapaResultado[0].size())
    return false;
  return es_camino(mapaResultado[f][c]); // Solo 'C', 'S', 'D', 'U' son transitables en Nivel 0
}

/**
 * @brief Comprueba si la casilla de delante es accesible por diferencia de altura.
 * Para el técnico: desnivel máximo siempre 1.
 * @param actual Estado actual del agente (fila, columna, orientacion).
 * @return true si el desnivel con la casilla de delante es admisible.
 */
bool ComportamientoTecnico::EsAccesiblePorAltura(const ubicacion &actual)
{
  ubicacion del = Delante(actual);
  if (del.f < 0 || del.f >= mapaCotas.size() || del.c < 0 || del.c >= mapaCotas[0].size())
    return false;
  int desnivel = abs(mapaCotas[del.f][del.c] - mapaCotas[actual.f][actual.c]);
  if (desnivel > 1)
    return false;
  return true;
}

/**
 * @brief Devuelve la posición (fila, columna) de la casilla que hay delante del agente.
 * Calcula la casilla frontal según la orientación actual (8 direcciones).
 * @param actual Estado actual del agente (fila, columna, orientacion).
 * @return Estado con la fila y columna de la casilla de enfrente.
 */
ubicacion ComportamientoTecnico::Delante(const ubicacion &actual) const
{
  ubicacion delante = actual;
  switch (actual.brujula)
  {
  case 0:
    delante.f--;
    break; // norte
  case 1:
    delante.f--;
    delante.c++;
    break; // noreste
  case 2:
    delante.c++;
    break; // este
  case 3:
    delante.f++;
    delante.c++;
    break; // sureste
  case 4:
    delante.f++;
    break; // sur
  case 5:
    delante.f++;
    delante.c--;
    break; // suroeste
  case 6:
    delante.c--;
    break; // oeste
  case 7:
    delante.f--;
    delante.c--;
    break; // noroeste
  }
  return delante;
}

/**
 * @brief Imprime por consola la secuencia de acciones de un plan.
 *
 * @param plan  Lista de acciones del plan.
 */
void ComportamientoTecnico::PintaPlan(const list<Action> &plan)
{
  auto it = plan.begin();
  while (it != plan.end())
  {
    if (*it == WALK)
    {
      cout << "W ";
    }
    else if (*it == JUMP)
    {
      cout << "J ";
    }
    else if (*it == TURN_SR)
    {
      cout << "r ";
    }
    else if (*it == TURN_SL)
    {
      cout << "l ";
    }
    else if (*it == COME)
    {
      cout << "C ";
    }
    else if (*it == IDLE)
    {
      cout << "I ";
    }
    else
    {
      cout << "-_ ";
    }
    it++;
  }
  cout << "( longitud " << plan.size() << ")" << endl;
}

/**
 * @brief Convierte un plan de acciones en una lista de casillas para
 *        su visualización en el mapa 2D.
 *
 * @param st    Estado de partida.
 * @param plan  Lista de acciones del plan.
 */
void ComportamientoTecnico::VisualizaPlan(const ubicacion &st,
                                          const list<Action> &plan)
{
  listaPlanCasillas.clear();
  ubicacion cst = st;

  listaPlanCasillas.push_back({cst.f, cst.c, WALK});
  auto it = plan.begin();
  while (it != plan.end())
  {

    switch (*it)
    {
    case JUMP:
      switch (cst.brujula)
      {
      case 0:
        cst.f--;
        break;
      case 1:
        cst.f--;
        cst.c++;
        break;
      case 2:
        cst.c++;
        break;
      case 3:
        cst.f++;
        cst.c++;
        break;
      case 4:
        cst.f++;
        break;
      case 5:
        cst.f++;
        cst.c--;
        break;
      case 6:
        cst.c--;
        break;
      case 7:
        cst.f--;
        cst.c--;
        break;
      }
      if (cst.f >= 0 && cst.f < mapaResultado.size() &&
          cst.c >= 0 && cst.c < mapaResultado[0].size())
        listaPlanCasillas.push_back({cst.f, cst.c, JUMP});
    case WALK:
      switch (cst.brujula)
      {
      case 0:
        cst.f--;
        break;
      case 1:
        cst.f--;
        cst.c++;
        break;
      case 2:
        cst.c++;
        break;
      case 3:
        cst.f++;
        cst.c++;
        break;
      case 4:
        cst.f++;
        break;
      case 5:
        cst.f++;
        cst.c--;
        break;
      case 6:
        cst.c--;
        break;
      case 7:
        cst.f--;
        cst.c--;
        break;
      }
      if (cst.f >= 0 && cst.f < mapaResultado.size() &&
          cst.c >= 0 && cst.c < mapaResultado[0].size())
        listaPlanCasillas.push_back({cst.f, cst.c, WALK});
      break;
    case TURN_SR:
      cst.brujula = (Orientacion)(((int)cst.brujula + 1) % 8);
      break;
    case TURN_SL:
      cst.brujula = (Orientacion)(((int)cst.brujula + 7) % 8);
      break;
    }
    it++;
  }
}

int ComportamientoTecnico::interact(Action accion, int valor) { return 0; }
