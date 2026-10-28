#include "ingeniero.hpp"
#include "motorlib/util.h"
#include <iostream>
#include <queue>
#include <set>
#include <cstdlib>
#include <algorithm>

using namespace std;

// =========================================================================
// FUNCIONES DE APOYO PARA MIRAR EL ENTORNO
// =========================================================================

// Calcula las coordenadas exactas de las casillas Izquierda, Centro y Derecha según el rumbo
// Nos sirve para poder consultar el mapa de calor de las casillas que tenemos delante
void CoordenadasLCRI(int rumbo, int f, int c, int &fL, int &cL, int &fC, int &cC, int &fR, int &cR)
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
    break; // Norte
  case 1:
    fL--;
    fC--;
    cC++;
    cR++;
    break; // Noreste
  case 2:
    fL--;
    cL++;
    cC++;
    fR++;
    cR++;
    break; // Este
  case 3:
    cL++;
    fC++;
    cC++;
    fR++;
    break; // Sureste
  case 4:
    fL++;
    cL++;
    fC++;
    fR++;
    cR--;
    break; // Sur
  case 5:
    fL++;
    fC++;
    cC--;
    cR--;
    break; // Suroeste
  case 6:
    fL++;
    cL--;
    cC--;
    fR--;
    cR--;
    break; // Oeste
  case 7:
    cL--;
    fC--;
    cC--;
    fR--;
    break; // Noroeste
  }
  // Protegemos para no salirnos de la matriz y dar un core dump
  fL = max(0, min(199, fL));
  cL = max(0, min(199, cL));
  fC = max(0, min(199, fC));
  cC = max(0, min(199, cC));
  fR = max(0, min(199, fR));
  cR = max(0, min(199, cR));
}

// Miramos si nos matamos al dar el paso por culpa del desnivel
char ViablePorAlturaI(char casilla, int dif, bool zap)
{
  // Salto 1 normal y 2 si tengo las zapatillas
  if (abs(dif) <= 1 || (zap && abs(dif) <= 2))
    return casilla;
  return 'P'; // si no llego lo trato como un precipicio para no caerme
}

// VISIÓN NIVEL 0: MAPA DE CALOR
int VeoCasillaInteresanteIN0(char i, char c, char d, bool zap, int vL, int vC, int vR)
{
  // Meta (Prioridad 1 absoluta)
  if (c == 'U')
    return 2;
  if (i == 'U')
    return 1;
  if (d == 'U')
    return 3;

  // En Nivel 0 nos ceñimos a caminar por piedra o arena estrictamente
  bool i_ok = (i == 'C' || i == 'S');
  bool c_ok = (c == 'C' || c == 'S');
  bool d_ok = (d == 'C' || d == 'S');

  // Si no hay camino, devolvemos 0 (callejón sin salida)
  if (!i_ok && !c_ok && !d_ok)
    return 0;

  // Elegimos la casilla que MENOS veces hayamos visitado (Mapa de Calor)
  int min_v = 999999;
  int best_pos = 0;

  // Priorizamos ir de frente si hay empate de visitas
  if (c_ok && vC <= min_v)
  {
    min_v = vC;
    best_pos = 2;
  }
  if (i_ok && vL < min_v)
  {
    min_v = vL;
    best_pos = 1;
  }
  if (d_ok && vR < min_v)
  {
    min_v = vR;
    best_pos = 3;
  }

  return best_pos;
}

// VISIÓN NIVEL 1
int VeoCasillaInteresanteIN1(char i, char c, char d, bool zap, int vL, int vC, int vR)
{
  // Meta (damos prioridad a la meta porque suele estar ubicada en senderos o caminos)
  if (c == 'U')
    return 2;
  if (i == 'U')
    return 1;
  if (d == 'U')
    return 3;

  // A por las zapatillas si no las tengo (Prioridad 2)
  if (!zap)
  {
    if (c == 'D')
      return 2;
    if (i == 'D')
      return 1;
    if (d == 'D')
      return 3;
  }

  bool i_ok = false, c_ok = false, d_ok = false;

  // Si tengo zapatillas me atrevo con el bosque
  if (zap)
  {
    if (i == 'C' || i == 'S' || i == 'B')
      i_ok = true;
    if (c == 'C' || c == 'S' || c == 'B')
      c_ok = true;
    if (d == 'C' || d == 'S' || d == 'B')
      d_ok = true;
  }
  else
  {
    // Si no tengo zapatillas no hago caso al bosque
    if (i == 'C' || i == 'S')
      i_ok = true;
    if (c == 'C' || c == 'S')
      c_ok = true;
    if (d == 'C' || d == 'S')
      d_ok = true;
  }

  if (!i_ok && !c_ok && !d_ok)
    return 0;

  // Usamos el mapa de calor para explorar de forma inteligente
  int min_v = 999999;
  int best_pos = 0;

  if (c_ok && vC <= min_v)
  {
    min_v = vC;
    best_pos = 2;
  }
  if (i_ok && vL < min_v)
  {
    min_v = vL;
    best_pos = 1;
  }
  if (d_ok && vR < min_v)
  {
    min_v = vR;
    best_pos = 3;
  }

  return best_pos;
}

// =========================================================================
// ÁREA DE IMPLEMENTACIÓN DEL ESTUDIANTE
// =========================================================================

Action ComportamientoIngeniero::think(Sensores sensores)
{
  Action accion = IDLE;
  switch (sensores.nivel)
  {
  case 0:
    accion = ComportamientoIngenieroNivel_0(sensores);
    break;
  case 1:
    accion = ComportamientoIngenieroNivel_1(sensores);
    break;
  case 2:
    accion = ComportamientoIngenieroNivel_2(sensores);
    break;
  case 3:
    accion = ComportamientoIngenieroNivel_3(sensores);
    break;
  case 4:
    accion = ComportamientoIngenieroNivel_4(sensores);
    break;
  case 5:
    accion = ComportamientoIngenieroNivel_5(sensores);
    break;
  case 6:
    accion = ComportamientoIngenieroNivel_6(sensores);
    break;
  }
  return accion;
}

// -------------------------------------------------------------------------
// NIVEL 0
// -------------------------------------------------------------------------

Action ComportamientoIngeniero::ComportamientoIngenieroNivel_0(Sensores sensores)
{
  // Guardamos en memoria lo que el agente ve (terrenos + alturas)
  ActualizarMapa(sensores);

  // comprobaciones de baldosas
  if (sensores.superficie[0] == 'D')
    tiene_zapatillas = true;
  if (sensores.superficie[0] == 'U')
    return IDLE; // Llegamos a la meta

  // Marcamos la casilla actual como visitada para alimentar el mapa de calor
  mapa_visitas[sensores.posF][sensores.posC]++;

  // Si estamos en medio de un giro evasivo, lo continuamos
  if (modo_esquivar)
  {
    if (giros_pendientes > 0)
    {
      giros_pendientes--;
      return TURN_SL;
    }
    else
    {
      modo_esquivar = false;
    }
  }

  // Comprobamos desniveles
  char i = ViablePorAlturaI(sensores.superficie[1], sensores.cota[1] - sensores.cota[0], tiene_zapatillas);
  char c = ViablePorAlturaI(sensores.superficie[2], sensores.cota[2] - sensores.cota[0], tiene_zapatillas);
  char d = ViablePorAlturaI(sensores.superficie[3], sensores.cota[3] - sensores.cota[0], tiene_zapatillas);

  // Consultamos el mapa de calor
  int fL, cL, fC, cC, fR, cR;
  CoordenadasLCRI(sensores.rumbo, sensores.posF, sensores.posC, fL, cL, fC, cC, fR, cR);
  int vL = mapa_visitas[fL][cL];
  int vC = mapa_visitas[fC][cC];
  int vR = mapa_visitas[fR][cR];

  int pos = VeoCasillaInteresanteIN0(i, c, d, tiene_zapatillas, vL, vC, vR);

  if (pos == 2)
  {
    // Si nos chocamos con alguien, iniciamos maniobra evasiva (giros pendientes)
    if (sensores.agentes[2] != '_')
    {
      modo_esquivar = true;
      giros_pendientes = 3;
      return TURN_SL;
    }
    if (c != 'P' && c != 'M' && c != 'B')
      return WALK;
  }
  if (pos == 1)
    return TURN_SL;
  if (pos == 3)
    return TURN_SR;

  // Ante un callejón sin salida, giramos para darnos la vuelta
  return TURN_SL;
}

// -------------------------------------------------------------------------
// NIVEL 1
// -------------------------------------------------------------------------

Action ComportamientoIngeniero::ComportamientoIngenieroNivel_1(Sensores sensores)
{
  // guardamos en memoria lo que ve el agente
  ActualizarMapa(sensores);

  // activamos variable tiene_zapatillas si estamos en casilla zapatillas
  if (sensores.superficie[0] == 'D')
    tiene_zapatillas = true;

  // Marcamos visita
  mapa_visitas[sensores.posF][sensores.posC]++;

  // Esquivar obstáculos
  if (modo_esquivar)
  {
    if (giros_pendientes > 0)
    {
      giros_pendientes--;
      return TURN_SR;
    }
    else
    {
      modo_esquivar = false;
    }
  }

  char i = ViablePorAlturaI(sensores.superficie[1], sensores.cota[1] - sensores.cota[0], tiene_zapatillas);
  char c = ViablePorAlturaI(sensores.superficie[2], sensores.cota[2] - sensores.cota[0], tiene_zapatillas);
  char d = ViablePorAlturaI(sensores.superficie[3], sensores.cota[3] - sensores.cota[0], tiene_zapatillas);

  // Si vemos un muro de frente, nos anticipamos y giramos para no perder tiempo
  if (sensores.superficie[2] == 'M' || (!tiene_zapatillas && sensores.superficie[2] == 'B') || sensores.agentes[2] != '_')
  {
    modo_esquivar = true;
    giros_pendientes = (rand() % 3) + 1;
    return TURN_SR;
  }

  int fL, cL, fC, cC, fR, cR;
  CoordenadasLCRI(sensores.rumbo, sensores.posF, sensores.posC, fL, cL, fC, cC, fR, cR);
  int vL = mapa_visitas[fL][cL];
  int vC = mapa_visitas[fC][cC];
  int vR = mapa_visitas[fR][cR];

  int pos = VeoCasillaInteresanteIN1(i, c, d, tiene_zapatillas, vL, vC, vR);

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
// NIVEL 2 (BUSQUEDA EN ANCHURA)
// -------------------------------------------------------------------------

Action ComportamientoIngeniero::ComportamientoIngenieroNivel_2(Sensores sensores)
{
  // en cada turno el simulador comprueba la variable hay_plan, si es falsa significa que nos hemos chocado
  // contra un obstaculo en el instante anterior y no avanzamos
  if (sensores.choque)
  {
    hay_plan = false;
  }

  if (!hay_plan)
  {
    // creamos estas dos variables porque el buscador necesita saber desde dónde arrancar la simulación y donde terminar
    EstadoI inicio, fin;

    inicio.posicion.f = sensores.posF;        // fila
    inicio.posicion.c = sensores.posC;        // columna
    inicio.posicion.brujula = sensores.rumbo; // mirando hacia este rumbo
    inicio.zapatillas = tiene_zapatillas;     // tengo zapatillas o no

    fin.posicion.f = sensores.BelPosF; // fila de la belkanita
    fin.posicion.c = sensores.BelPosC; // columna de la belkanita

    // llamamos a la funcion y le pasamos el inicio el final, y los mapas de terreno y alturas
    plan = AnchuraIngeniero(inicio, fin, mapaResultado, mapaCotas);

    // cogemos esta lista de acciones creada y le decimos al motor gráfico del juego pinte la linea para ver la ruta
    VisualizaPlan(inicio.posicion, plan);

    // esto funciona como seguro, si el plan tiene mas de 0 elementos devolvemos true sino lo dejamos en false todavia
    hay_plan = (plan.size() > 0);
  }

  // acción por defecto si algo falla
  Action accion = IDLE;

  if (hay_plan && plan.size() > 0)
  {

    accion = plan.front();

    // Si el plan dice caminar, pero el Técnico está justo delante, abortamos
    if (accion == WALK && sensores.agentes[2] != '_')
    {
      hay_plan = false;
      return IDLE; // En el siguiente intento calculará una ruta esquivándolo
    }

    // como ya tenemos guardad la orden en accion la tachamos para no repetirla en el turno
    plan.pop_front();

    // si ya no quedan acciones en el plan, significa que la siguiente orden es el último paso antes de la belkanita,
    // de esta forma nos aseguramos de que en el próximo turno el ingenierio no intente seguir caminando hacia la nada
    if (plan.size() == 0)
    {
      hay_plan = false;
    }
  }

  // realizamos la acción requerida
  return accion;
}

// -------------------------------------------------------------------------
// NIVEL 3: Vacío (El Ingeniero no hace A*)
// -------------------------------------------------------------------------

//originalmente aquí no hacia nada con el ingeniero, pero tras ver un test fallido del bot de telegram
// caí en la cuenta de que si el ingeniero esta en el camino del tecnico y al quedarse quieto rompe el algoritmo A* por completo
// asique para evitar fallos realizo este mini-ajuste
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_3(Sensores sensores)
{

  // Si el Técnico está justo delante (agentes[2]), NO podemos dar un paso porque
  // chocaríamos contra él y el simulador nos bloquea. Giramos obligatoriamente.
  if (sensores.agentes[2] != '_')
  {
    return TURN_SR;
  }

  // Si vemos al Técnico de reojo (agentes[1] o agentes[3]), significa que ya
  // no lo miramos de frente. Si nuestro camino frontal está libre "huimos" de él
  if (sensores.agentes[1] != '_' || sensores.agentes[3] != '_')
  {
    if (sensores.superficie[2] != 'M' && sensores.superficie[2] != 'P')
    {
      return WALK; // Nos apartamos dando un paso hacia el hueco libre
    }
    else
    {
      return TURN_SR; // Si hay muro, seguimos girando para buscar salida
    }
  }

  // Rotamos en el sitio vigilando. Como no hacemos WALK, no bloqueamos
  // otros pasillos por error, pero garantizamos que nadie nos pille por la espalda.
  return TURN_SR;
}

// -------------------------------------------------------------------------
// NIVEL 4: PLANIFICACIÓN DE TUBERÍAS
// -------------------------------------------------------------------------
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_4(Sensores sensores)
{
  if (!hay_plan)
  {
    // Buscamos la ruta óptima de tuberías desde la posición de la filtración
    list<Paso> planTuberias = PlanificarTuberias(sensores.BelPosF, sensores.BelPosC, mapaResultado, mapaCotas);

    // Le pasamos el plan al simulador para que verifique si la red construida cumple las reglas
    VisualizaRedTuberias(planTuberias);

    hay_plan = true;
  }

  // En el nivel 4 el Ingeniero se limita a planificar mentalmente, no se mueve en el mundo real
  return IDLE;
}

// -------------------------------------------------------------------------
// NIVEL 5: MÁQUINA DE ESTADOS COOPERATIVA
// -------------------------------------------------------------------------
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_5(Sensores sensores)
{
  // filtro para ver si tenemos zapatillas por si hicieran falta
  if (sensores.superficie[0] == 'D')
    tiene_zapatillas = true;

  // Si no hay plan, lo generamos y lo pasamos al vector pasos_red
  if (!hay_plan_tuberia)
  {
    list<Paso> plan_list = PlanificarTuberias(sensores.BelPosF, sensores.BelPosC, mapaResultado, mapaCotas);
    VisualizaRedTuberias(plan_list);
    pasos_red.clear();
    for (auto p : plan_list)
      pasos_red.push_back(p);

    hay_plan_tuberia = true;
    estado_ing = 0;
    paso_actual = 0;

    // TRUCO: Consumimos el primer instante para que se vea la red (Nivel 4)
    // antes de que se superponga con la línea de movimiento del Nivel 5.
    return IDLE;
  }

  // Prevención de errores si el plan está vacío o ya terminamos
  if (pasos_red.empty() || paso_actual >= (int)pasos_red.size() - 1)
    return IDLE;

  // Si ocurre un choque inesperado (ej. se nos cruza el técnico), replanificamos el movimiento
  if (sensores.choque)
  {
    if (estado_ing == 1)
      estado_ing = 0;
    if (estado_ing == 5)
      estado_ing = 4;
  }

  // MÁQUINA DE ESTADOS DEL INGENIERO
  switch (estado_ing)
  {
  case 0:
  { // Calcular ruta hacia el Nodo A (donde se pondrá el técnico)
    EstadoI inicio, fin;
    inicio.posicion.f = sensores.posF;
    inicio.posicion.c = sensores.posC;
    inicio.posicion.brujula = sensores.rumbo;
    inicio.zapatillas = tiene_zapatillas;
    fin.posicion.f = pasos_red[paso_actual].fil;
    fin.posicion.c = pasos_red[paso_actual].col;

    plan_mov = AnchuraIngeniero(inicio, fin, mapaResultado, mapaCotas);

    // LA LÍNEA SE PINTA UNA SOLA VEZ AQUÍ Y SE QUEDA ESTÁTICA
    VisualizaPlan(inicio.posicion, plan_mov);

    estado_ing = 1;
  }
  case 1:
  { // Ejecutar movimiento hacia el Nodo A
    if (!plan_mov.empty())
    {
      Action a = plan_mov.front();

      // RADAR ANTICHOQUES
      if (a == WALK && sensores.agentes[2] != '_')
      {
        estado_ing = 0; // Abortamos plan y volvemos a recalcular
        return IDLE;    // No damos el paso para no generar el mensaje de error
      }

      plan_mov.pop_front();
      return a;
    }
    estado_ing = 2;
    // una vez que la lista ya esta vacia es que esta pisando el nodo objetivo
  } 
  case 2:
  { // Modificar el terreno del Nodo A si es necesario (El Ingeniero hace todo)
    if (pasos_red[paso_actual].op == 1)
    {
      pasos_red[paso_actual].op = 0;
      return RAISE;
    }
    else if (pasos_red[paso_actual].op == -1)
    {
      pasos_red[paso_actual].op = 0;
      return DIG;
    }
    estado_ing = 3;
    //la obra que teniamos que hacer en ese nodo objetivo ya esta acabada
  } 
  case 3:
  { // Llamar al técnico al Nodo A (donde estamos parados ahora mismo)
    estado_ing = 4;
    return COME;
  }
  case 4:
  { // Calcular ruta hacia el Nodo B (nuestra posición de instalación)
    EstadoI inicio, fin;
    inicio.posicion.f = sensores.posF;
    inicio.posicion.c = sensores.posC;
    inicio.posicion.brujula = sensores.rumbo;
    inicio.zapatillas = tiene_zapatillas;
    fin.posicion.f = pasos_red[paso_actual + 1].fil;
    fin.posicion.c = pasos_red[paso_actual + 1].col;

    plan_mov = AnchuraIngeniero(inicio, fin, mapaResultado, mapaCotas);

    // LA LÍNEA SE PINTA UNA SOLA VEZ AQUÍ Y SE QUEDA ESTÁTICA
    VisualizaPlan(inicio.posicion, plan_mov);

    estado_ing = 5;
  } 
  case 5:
  { // Ejecutar movimiento hacia el Nodo B
    // Todo exacatmente igual que en el case 1
    if (!plan_mov.empty())
    {
      Action a = plan_mov.front();

      // RADAR ANTICHOQUES
      if (a == WALK && sensores.agentes[2] != '_')
      {
        estado_ing = 0; // Abortamos plan y volvemos a recalcular
        return IDLE;    // No damos el paso para no generar el mensaje de error
      }

      plan_mov.pop_front();
      return a;
    }
    estado_ing = 4;
  } 
  case 6:
  { // Modificar el terreno del Nodo B si es necesario
    if (pasos_red[paso_actual + 1].op == 1)
    {
      pasos_red[paso_actual + 1].op = 0;
      return RAISE;
    }
    else if (pasos_red[paso_actual + 1].op == -1)
    {
      pasos_red[paso_actual + 1].op = 0;
      return DIG;
    }
    estado_ing = 7;
  } 
  case 7:
  { // Girar para mirar de frente al Nodo A (donde llegará el Técnico)
    int f1 = sensores.posF;
    int c1 = sensores.posC;
    int f2 = pasos_red[paso_actual].fil;
    int c2 = pasos_red[paso_actual].col;
    // variable para guardar donde tenemos que mirar y que ponemos por defecto a norte
    Orientacion target = norte;

    if (f1 > f2)
      target = norte;
    else if (f1 < f2)
      target = sur;
    else if (c1 > c2)
      target = oeste;
    else if (c1 < c2)
      target = este;

    if (sensores.rumbo != target)
    {
      int diff = (target - sensores.rumbo + 8) % 8;
      return (diff <= 4) ? TURN_SR : TURN_SL;
    }
    estado_ing = 8;
  } 
  case 8:
  { // Esperar la sincronización del técnico e instalar
    if (sensores.enfrente)
    {
      estado_ing = 9;
      return INSTALL;
    }
    return IDLE; // Esperar a que el técnico llegue y nos mire
  }
  case 9:
  { // Avanzar al siguiente tramo
    paso_actual++;
    if (paso_actual < (int)pasos_red.size() - 1)
    {
      // Al avanzar, el Ingeniero ya está sobre el nuevo Nodo A (antiguo Nodo B)
      // Saltamos directamente a la llamada del técnico
      estado_ing = 3;
      return IDLE;
    }
    return IDLE; // Construcción finalizada
  }
  }
  return IDLE;
}

// -------------------------------------------------------------------------
// NIVEL 6: MODO COOPERATIVO
// -------------------------------------------------------------------------
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_6(Sensores sensores)
{
  // antes de pensar en algo más cogemos la informacion de lo que nos rodea y vemos si pisamos zapatillas por si las moscas
  ActualizarMapa(sensores);
  if (sensores.superficie[0] == 'D')
    tiene_zapatillas = true;

  // Usamos la variable de clase en lugar de static local para que se reinicie
  // correctamente al comenzar una nueva partida en la misma sesión del simulador.
  int &fase_mision = fase_mision_6;


  // FASE 0 ir a la Belkanita usando Anchura
  if (fase_mision == 0)
  {
    // si hemos llegado ya está
    if (sensores.posF == sensores.BelPosF && sensores.posC == sensores.BelPosC)
    {
      fase_mision = 1;
      hay_plan = false;
      return IDLE;
    }
    Action accion_elegida = ComportamientoIngenieroNivel_2(sensores);

    // Radar de seguridad para la Fase 0 (Si descubrimos en vivo que no se puede pasar)
    if (accion_elegida == WALK)
    {
      char frente = sensores.superficie[2];
      int dif = sensores.cota[2] - sensores.cota[0];
      bool muerte = tiene_zapatillas ? (abs(dif) > 2) : (abs(dif) > 1);
      if (frente == 'M' || frente == 'P' || (!tiene_zapatillas && frente == 'B') || muerte || sensores.agentes[2] != '_')
      {
        hay_plan = false;
        return IDLE;
      }
    }

    if (sensores.choque)
      hay_plan = false;

    return accion_elegida;
  }

  // FASE 1 Explorar hasta ver la Planta 'U'
  if (fase_mision == 1)
  {
    // SI HEMOS VISTO LA VARIABLE U en algun punto lo marcamos como vista
    bool planta_vista = false;
    for (int i = 0; i < (int)mapaResultado.size() && !planta_vista; i++)
      for (int j = 0; j < (int)mapaResultado[0].size() && !planta_vista; j++)
        if (mapaResultado[i][j] == 'U')
          planta_vista = true;

    if (planta_vista)
    {
      list<Paso> plan_list = PlanificarTuberias(sensores.BelPosF, sensores.BelPosC, mapaResultado, mapaCotas);
      if (!plan_list.empty())
      {
        VisualizaRedTuberias(plan_list);
        pasos_red.clear();
        for (auto p : plan_list)
          pasos_red.push_back(p);
        hay_plan_tuberia = true;
        estado_ing = 0;
        paso_actual = 0;
        fase_mision = 2;
      }
    }

    // Si no hemos encontrado aún la 'U' (o el plan de tuberías aún no es válido),
    // exploramos con el comportamiento del Nivel 1 (mapa de calor) que es mucho
    // más eficiente que hacerlo de manera aleatoria y consume menos instantes de simulación.
    if (fase_mision == 1)
    {
      return ComportamientoIngenieroNivel_1(sensores);
    }
  }

  // FASE 2 Construcción (similar al nivel 5)
  if (fase_mision == 2)
  {
    Action accion_elegida = ComportamientoIngenieroNivel_5(sensores);
    if (accion_elegida == WALK)
    {
      // nuevamente ponemos este filtro por seguridad, si encontramos algo inesperado en nuestro camino tenemos que recalcular
      char frente = sensores.superficie[2];
      int dif = sensores.cota[2] - sensores.cota[0];
      bool muerte = tiene_zapatillas ? (abs(dif) > 2) : (abs(dif) > 1);
      if (frente == 'M' || frente == 'P' || (!tiene_zapatillas && frente == 'B') || muerte || sensores.agentes[2] != '_')
      {
        estado_ing = 0;
        return IDLE;
      }
    }
    return accion_elegida;
  }
  return IDLE;
}


// =========================================================================
// =========================================================================
//                       METODOS PARA LOS NIVELES
// =========================================================================
// =========================================================================


// =========================================================================
// MÉTODOS DEL NIVEL 2: BÚSQUEDA EN ANCHURA
// =========================================================================

// Miro mentalmente en qué casilla caería si hago este movimiento
EstadoI ComportamientoIngeniero::SiguienteCasillaIngeniero(const EstadoI &st, Action accion)
{
  EstadoI siguiente = st;
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
  else if (accion == JUMP)
  {
    switch (st.posicion.brujula)
    {
    case norte:
      siguiente.posicion.f -= 2;
      break;
    case noreste:
      siguiente.posicion.f -= 2;
      siguiente.posicion.c += 2;
      break;
    case este:
      siguiente.posicion.c += 2;
      break;
    case sureste:
      siguiente.posicion.f += 2;
      siguiente.posicion.c += 2;
      break;
    case sur:
      siguiente.posicion.f += 2;
      break;
    case suroeste:
      siguiente.posicion.f += 2;
      siguiente.posicion.c -= 2;
      break;
    case oeste:
      siguiente.posicion.c -= 2;
      break;
    case noroeste:
      siguiente.posicion.f -= 2;
      siguiente.posicion.c -= 2;
      break;
    }
  }
  return siguiente;
}

// Compruebo si el salto o el paso me mata por culpa del desnivel o si choco
bool ComportamientoIngeniero::CasillaAccesibleIngeniero(const EstadoI &st, Action accion, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura)
{
  //calculamos el destino
  EstadoI siguiente = SiguienteCasillaIngeniero(st, accion);

  //revisamos limites
  if (siguiente.posicion.f < 0 || siguiente.posicion.f >= terreno.size() || siguiente.posicion.c < 0 || siguiente.posicion.c >= terreno[0].size())
    return false;

  // nos aseguramos de evitar muros, precipios y bosque si no tenemos zapatillas para este último
  if (accion == WALK)
  {
    bool transitable = (terreno[siguiente.posicion.f][siguiente.posicion.c] != 'M' &&
                        terreno[siguiente.posicion.f][siguiente.posicion.c] != 'P' &&
                        terreno[siguiente.posicion.f][siguiente.posicion.c] != 'B');

    int dif = altura[siguiente.posicion.f][siguiente.posicion.c] - altura[st.posicion.f][st.posicion.c];
    bool alt_ok = st.zapatillas ? (abs(dif) <= 2) : (abs(dif) <= 1);

    // si el terreno es desconocido el algoritmo confia en él
    if (terreno[siguiente.posicion.f][siguiente.posicion.c] == '?')
    {
      transitable = true;
      alt_ok = true;
    }

    return transitable && alt_ok;
  }
  else if (accion == JUMP)
  {
    EstadoI intermedia = SiguienteCasillaIngeniero(st, WALK);
    if (intermedia.posicion.f < 0 || intermedia.posicion.f >= terreno.size() || intermedia.posicion.c < 0 || intermedia.posicion.c >= terreno[0].size())
      return false;

    char inter = terreno[intermedia.posicion.f][intermedia.posicion.c];
    char dest = terreno[siguiente.posicion.f][siguiente.posicion.c];

    // PROHIBIDO SALTAR A CIEGAS: Si no sabemos qué hay, obligamos al algoritmo a ir andando (WALK)
    if (inter == '?' || dest == '?')
      return false;

    bool transitable_inter = (inter != 'M' && inter != 'P' && inter != 'B');
    bool transitable_dest = (dest != 'M' && dest != 'P' && dest != 'B');

    int dif = altura[siguiente.posicion.f][siguiente.posicion.c] - altura[st.posicion.f][st.posicion.c];
    bool alt_ok = st.zapatillas ? (abs(dif) <= 2) : (abs(dif) <= 1);

    return transitable_inter && transitable_dest && alt_ok;
  }
  return false;
}

// Simulo en mi cabeza qué pasaría si hago esta acción y me actualizo el estado
EstadoI ComportamientoIngeniero::AplicarAccionIngeniero(Action accion, const EstadoI &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura)
{
  EstadoI siguiente = st;
  if (accion == WALK || accion == JUMP)
  {
    if (CasillaAccesibleIngeniero(st, accion, terreno, altura))
    {
      siguiente = SiguienteCasillaIngeniero(st, accion);
      // Si resulta que caigo encima de unas zapatillas, me las pongo del tirón para los siguientes cálculos
      if (terreno[siguiente.posicion.f][siguiente.posicion.c] == 'D')
      {
        siguiente.zapatillas = true;
      }
    }
  }
  else if (accion == TURN_SR)
  {
    siguiente.posicion.brujula = (Orientacion)((siguiente.posicion.brujula + 1) % 8);
  }
  else if (accion == TURN_SL)
  {
    siguiente.posicion.brujula = (Orientacion)((siguiente.posicion.brujula + 7) % 8);
  }
  return siguiente;
}

// Mi buscador en anchura (para ir a lo seguro y por el camino con menos movimientos)
list<Action> ComportamientoIngeniero::AnchuraIngeniero(const EstadoI &inicio, const EstadoI &final, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura)
{
  NodoI nodo_actual;
  list<NodoI> frontera; // nodos por explorar
  set<NodoI> explorados; // estados ya visitados
  list<Action> camino;

  nodo_actual.estado = inicio;
  frontera.push_back(nodo_actual);

  bool solucion_encontrada = (nodo_actual.estado.posicion.f == final.posicion.f && nodo_actual.estado.posicion.c == final.posicion.c);

  // Las acciones que me permiten llegar a mi destino
  Action acciones[] = {WALK, JUMP, TURN_SL, TURN_SR};

  while (!solucion_encontrada && !frontera.empty())
  {
    frontera.pop_front();
    explorados.insert(nodo_actual);

    // Si resulta que empezamos ya encima de las zapatillas
    if (terreno[nodo_actual.estado.posicion.f][nodo_actual.estado.posicion.c] == 'D')
    {
      nodo_actual.estado.zapatillas = true;
    }

    for (Action acc : acciones)
    {
      NodoI hijo = nodo_actual;
      hijo.estado = AplicarAccionIngeniero(acc, nodo_actual.estado, terreno, altura);

      // Si el movimiento me deja en el mismo sitio dándome cabezazos, paso de él
      if (hijo.estado == nodo_actual.estado && (acc == WALK || acc == JUMP))
        continue;

      if (hijo.estado.posicion.f == final.posicion.f && hijo.estado.posicion.c == final.posicion.c)
      {
        //Hemos encontrado la meta
        hijo.secuencia.push_back(acc);
        nodo_actual = hijo;
        solucion_encontrada = true;
        break;
      }

      if (explorados.find(hijo) == explorados.end())
      {
        hijo.secuencia.push_back(acc);
        frontera.push_back(hijo);
      }
    }

    // eliminamos los nodos que son inutiles para nuestra busqueda en anchura
    if (!solucion_encontrada && !frontera.empty())
    {
      nodo_actual = frontera.front();
      while (explorados.find(nodo_actual) != explorados.end() && !frontera.empty())
      {
        frontera.pop_front();
        if (!frontera.empty())
          nodo_actual = frontera.front();
      }
    }
  }

  if (solucion_encontrada)
  {
    camino = nodo_actual.secuencia;
  }

  return camino;
}

// =========================================================================
//  MÉTODOS DEL NIVEL 4: BÚSQUEDA DE TUBERÍAS 
// =========================================================================

// Calcula el destrozo ecológico en función del tipo de terreno y operación
int ComportamientoIngeniero::CosteEcoTuberia(char terreno, int op)
{
  int eco = 0;

  // Impacto base por Instalar tubería (INSTALL)
  if (terreno == 'A')
    eco += 50;
  else if (terreno == 'H')
    eco += 45;
  else if (terreno == 'S')
    eco += 25;
  else if (terreno == 'C' || terreno == 'U')
    eco += 15;
  else
    eco += 30; // 'B', 'D', etc.

  // Impacto añadido por elevar terreno (RAISE)
  if (op == 1)
  {
    if (terreno == 'H')
      eco += 55;
    else if (terreno == 'S')
      eco += 30;
    else if (terreno == 'C' || terreno == 'U')
      eco += 10;
    else
      eco += 40;
  }
  // Impacto añadido por rebajar terreno (DIG)
  else if (op == -1)
  {
    if (terreno == 'H')
      eco += 65;
    else if (terreno == 'S')
      eco += 40;
    else if (terreno == 'C' || terreno == 'U')
      eco += 25;
    else
      eco += 50;
  }
  return eco;
}

// Filtra si la casilla puede soportar nuestra tubería
bool ComportamientoIngeniero::EsValidaParaTuberia(int f, int c, int op, const vector<vector<unsigned char>> &mapa, const vector<vector<unsigned char>> &cotas)
{
  // comprobamos que la coordenada (f, c) no se salga de los bordes del mapa y nos de error
  if (f < 0 || f >= mapa.size() || c < 0 || c >= mapa[0].size())
    return false;

  char terr = mapa[f][c];

  // prohibido en muros, precipicios y bosques
  if (terr == 'M' || terr == 'P' || terr == 'B')
    return false;

  if (op == 1)
  {
    // prohibido RAISE en el agua
    if (terr == 'A')
      return false;
    // prohibido subir terreno si ya estamos en la altura maxima
    if (cotas[f][c] >= '9')
      return false;
  }
  
  else if (op == -1)
  {
    // prohibido DIG en el agua
    if (terr == 'A')
      return false;
    // prohibido excavar mas si ya estamos en el nivel 1 
    if (cotas[f][c] <= '1')
      return false;
  }
  return true;
}


// El buscador propiamente dicho (A* para la red de tuberías)
list<Paso> ComportamientoIngeniero::PlanificarTuberias(int bel_f, int bel_c, const vector<vector<unsigned char>> &mapa, const vector<vector<unsigned char>> &cotas)
{

  // al igual que en el nivel 3 esta cola será la que ordene los caminos
  priority_queue<NodoTuberias, vector<NodoTuberias>, ComparaNodosTuberias> frontera;

  // creo una matriz 200x200 multiplicada por 3 capas que van a ser las 3 operaciones, DIG, RAISE y NADA
  static bool cerrados[200][200][3];

  // Limpiamos la matriz para esta búsqueda la cual vamos iniciando de nuevo en cada iteracion 
  for (int i = 0; i < 200; i++)
    for (int j = 0; j < 200; j++)
      for (int k = 0; k < 3; k++)
        cerrados[i][j][k] = false;

  // Insertamos el estado inicial (la Belkanita)
  if (EsValidaParaTuberia(bel_f, bel_c, 0, mapa, cotas))
  {
    NodoTuberias n; // creamos nuestro primer nodo
    n.estado = {bel_f, bel_c, 0};
    n.g_dist = 1;
    n.g_eco = CosteEcoTuberia(mapa[bel_f][bel_c], 0); // calculamos su impacto suponiendo que no realizamos ninguna operacion y a la cola 
    n.secuencia.push_back({bel_f, bel_c, 0});
    frontera.push(n);
  }

  // Direcciones ortogonales obligatorias y asi evitamos hacer un switch
  int df[] = {-1, 0, 1, 0};
  int dc[] = {0, 1, 0, -1};

  while (!frontera.empty())
  {
    NodoTuberias actual = frontera.top();
    frontera.pop();

    // si hemos caido en una 'U' hemos acabado
    if (mapa[actual.estado.f][actual.estado.c] == 'U')
    {
      return actual.secuencia;
    }

    // Dijkstra: Si esta casilla con esta operación ya fue procesada, se ignora.
    if (cerrados[actual.estado.f][actual.estado.c][actual.estado.op + 1])
    {
      continue;
    }

    // La cerramos para siempre
    cerrados[actual.estado.f][actual.estado.c][actual.estado.op + 1] = true;

    // le restamos 48 siguiendo el codigo ASCII y le sumamos op para saber la altura real de la tuberia actual 
    int alt_curr = (int)cotas[actual.estado.f][actual.estado.c] - 48 + actual.estado.op;

    // expandimos las tuberias
    // lo que hago aqui es mirar las 4 direccines para cada paso, y para cada direccion simulo las 3 operaciones distentas
    // por lo que seran 12 posibles casos a evaluar
    for (int i = 0; i < 4; ++i)
    {
      int nf = actual.estado.f + df[i];
      int nc = actual.estado.c + dc[i];

      if (mapa[nf][nc] != 'H' && mapa[nf][nc] != 'U'){
        continue;
      }

      for (int nop = -1; nop <= 1; ++nop)
      {
        if (EsValidaParaTuberia(nf, nc, nop, mapa, cotas))
        {
          int alt_next = (int)cotas[nf][nc] - 48 + nop;

          // La gravedad exige que la tubería actual esté al mismo nivel o 1 unidad por encima
          if (alt_curr == alt_next || alt_curr == alt_next + 1)
          {
            // Solo analizamos el paso si NO está en cerrados
            if (!cerrados[nf][nc][nop + 1])
            {
              int coste_paso = CosteEcoTuberia(mapa[nf][nc], nop);
              int nuevo_g_eco = actual.g_eco + coste_paso;

              // Penalización extrema si el terreno es desconocido
              if (mapa[nf][nc] == '?')
                nuevo_g_eco += 5000;

              NodoTuberias hijo;
              hijo.estado = {nf, nc, nop};
              hijo.g_dist = actual.g_dist + 1; // Prioridad 1
              hijo.g_eco = nuevo_g_eco;        // Prioridad 2

              hijo.secuencia = actual.secuencia;
              hijo.secuencia.push_back({nf, nc, nop});

              frontera.push(hijo);
            }
          }
        }
      }
    }
  }

  // Si no hay camino posible
  return list<Paso>();
}

// ------------------------------------------------------------------------
// MÉTODOS OBLIGATORIOS Y HERRAMIENTAS DE DIBUJO (PROFESORES)
// ------------------------------------------------------------------------

bool ComportamientoIngeniero::es_camino(unsigned char c) const
{
  return (c == 'C' || c == 'D' || c == 'U' || c == 'S');
}

void ComportamientoIngeniero::ActualizarMapa(Sensores sensores)
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
 * @brief Determina si una casilla es transitable para el ingeniero.
 * @param f Fila de la casilla.
 * @param c Columna de la casilla.
 * @param tieneZapatillas Indica si el agente posee las zapatillas.
 * @return true si la casilla es transitable (no es muro ni precipicio).
 */
bool ComportamientoIngeniero::EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas)
{
  if (f < 0 || f >= mapaResultado.size() || c < 0 || c >= mapaResultado[0].size())
    return false;
  return es_camino(mapaResultado[f][c]); // Solo 'C', 'D', 'U' son transitables en Nivel 0
}

/**
 * @brief Comprueba si la casilla de delante es accesible por diferencia de altura.
 * Para el ingeniero: desnivel máximo 1 sin zapatillas, 2 con zapatillas.
 * @param actual Estado actual del agente (fila, columna, orientacion, zap).
 * @return true si el desnivel con la casilla de delante es admisible.
 */
bool ComportamientoIngeniero::EsAccesiblePorAltura(const ubicacion &actual, bool zap)
{
  ubicacion del = Delante(actual);
  if (del.f < 0 || del.f >= mapaCotas.size() || del.c < 0 || del.c >= mapaCotas[0].size())
    return false;
  int desnivel = abs(mapaCotas[del.f][del.c] - mapaCotas[actual.f][actual.c]);
  if (zap && desnivel > 2)
    return false;
  if (!zap && desnivel > 1)
    return false;
  return true;
}

/**
 * @brief Devuelve la posición (fila, columna) de la casilla que hay delante del agente.
 * Calcula la casilla frontal según la orientación actual (8 direcciones).
 * @param actual Estado actual del agente (fila, columna, orientacion).
 * @return Estado con la fila y columna de la casilla de enfrente.
 */
ubicacion ComportamientoIngeniero::Delante(const ubicacion &actual) const
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
void ComportamientoIngeniero::PintaPlan(const list<Action> &plan)
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
 * @brief Imprime las coordenadas y operaciones de un plan de tubería.
 *
 * @param plan  Lista de pasos (fila, columna, operación),
 *              donde operacion = -1 (DIG), operación = 1 (RAISE).
 */
void ComportamientoIngeniero::PintaPlan(const list<Paso> &plan)
{
  auto it = plan.begin();
  while (it != plan.end())
  {
    cout << it->fil << ", " << it->col << " (" << it->op << ")\n";
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
void ComportamientoIngeniero::VisualizaPlan(const ubicacion &st,
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

/**
 * @brief Convierte un plan de tubería en la lista de casillas usada
 *        por el sistema de visualización.
 *
 * @param st    Estado de partida (no utilizado directamente).
 * @param plan  Lista de pasos del plan de tubería.
 */
void ComportamientoIngeniero::VisualizaRedTuberias(const list<Paso> &plan)
{
  listaCanalizacionTuberias.clear();
  auto it = plan.begin();
  while (it != plan.end())
  {
    listaCanalizacionTuberias.push_back({it->fil, it->col, it->op});
    it++;
  }
}

// =========================================================================
// MÉTODOS OBLIGATORIOS POR HERENCIA
// =========================================================================
// Implementamos esta función virtual pura heredada de la clase Comportamiento
// para evitar el error de compilación de clase abstracta (vtable).
// Como en los niveles 0 y 1 nuestros agentes actúan de forma individual y no
// necesitan comunicarse para pasarse objetos (eso es propio de los niveles 5 y 6),
// la dejamos vacía devolviendo 0.
int ComportamientoIngeniero::interact(Action accion, int valor) { return 0; }
