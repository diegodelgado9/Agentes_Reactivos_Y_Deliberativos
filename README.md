# Práctica 2: Operación Contención Belkanita - Inteligencia Artificial

Este repositorio contiene la solución a la Práctica 2 de la asignatura Inteligencia Artificial de la Universidad de Granada. El objetivo de la "Operación Contención Belkanita" es dotar de comportamientos inteligentes (reactivos y deliberativos) a dos agentes de rescate para evitar un desastre ecológico en un mundo cuadriculado 3D con diferentes cotas de altura.

Los dos agentes que conforman el equipo son:
* **Ingeniero (Triángulo Rojo):** Líder topógrafo, rápido y ágil. Puede realizar saltos (`JUMP`), elevar el terreno (`RAISE`) y es el encargado de planificar el trazado de la red de tuberías.
* **Técnico (Triángulo Naranja):** Especialista en infraestructura. Puede rebajar el terreno (`DIG`) y calcular rutas óptimas de bajo coste energético. Si consigue unas Zapatillas, podrá atravesar los bosques libremente.

---

## 🛠️ Requisitos e Instalación

El entorno de simulación requiere diversas librerías gráficas y de sistema (OpenGL, MPI, Boost, etc.). Para facilitar la instalación en distribuciones basadas en Ubuntu/Debian, se proporciona un script automatizado.

Ejecuta los siguientes comandos en la raíz del proyecto para instalar todas las dependencias necesarias:

```bash
chmod +x install.sh
./install.sh
```

---

## ⚙️ Compilación

Para mantener el código fuente limpio y evitar subir ejecutables temporales al repositorio, se debe compilar el proyecto utilizando un directorio aislado llamado `build`. 

Ejecuta los siguientes pasos desde la raíz del proyecto:

```bash
# 1. Crear el directorio de compilación y entrar en él
mkdir build
cd build

# 2. Configurar el proyecto con CMake apuntando al directorio raíz
cmake ..

# 3. Compilar los ejecutables
make
```

Esto generará dos binarios principales dentro de la carpeta `build`:
* `practica2`: Simulador interactivo con interfaz gráfica 2D/3D.
* `practica2SG`: Modo consola/batch (Sin Gráficos) ideal para pruebas masivas y depuración.

---

## ▶️ Ejecución y Niveles

Para ejecutar el simulador, asegúrate de estar dentro de la carpeta `build/`. Como los mapas se encuentran en la raíz del proyecto, las rutas a los mismos se referencian como `../mapas/`.

A continuación, se describen los 7 niveles del juego y un ejemplo de ejecución para cada uno:

### Nivel 0: Rastreadores de Alcantarillas (Reactivo)
**Objetivo:** Los agentes deben llegar a casillas de Planta de Tratamiento ('U') moviéndose exclusivamente por casillas de Camino ('C') en un mapa desconocido mediante un comportamiento puramente reactivo.
```bash
./practica2 -m ../mapas/mapa30.map -n 0 -i 17 5 0 -t 17 17 0
```

### Nivel 1: De Reconocimiento por la Naturaleza (Exploración)
**Objetivo:** Exploración reactiva en un mapa desconocido. El objetivo es descubrir e incorporar a la memoria interna la mayor cantidad posible de casillas transitables (Caminos y Senderos).
```bash
./practica2 -m ../mapas/mapa30.map -seed 1 -n 1 -i 6 3 0 -t 21 15 2
```

### Nivel 2: ¡Corre, Ingeniero, Corre! (Búsqueda Ingeniero)
**Objetivo:** Primer nivel deliberativo con mapa conocido. El Ingeniero debe calcular y ejecutar la ruta más rápida (menor número de instantes de simulación) hasta la filtración de Belkanita.
```bash
./practica2 -m ../mapas/mapa30.map -seed 1 -n 2 -i 8 13 0 -t 11 18 0 -O 18 12
```

### Nivel 3: El Técnico mide sus Esfuerzos (Búsqueda A* Técnico)
**Objetivo:** Con el mapa conocido, el Técnico debe encontrar y recorrer la ruta que minimice el gasto de energía hasta la Belkanita, implementando el algoritmo de búsqueda óptima A* con una heurística admisible.
```bash
./practica2 -m ../mapas/mapa75.map -n 3 -i 5 5 2 -t 8 5 2 -O 31 54
```

### Nivel 4: ¡Planifica, Ingeniero, Planifica! (Planificación)
**Objetivo:** El Ingeniero debe diseñar (sin moverse) una red de tuberías válida desde la Belkanita hasta una planta 'U'. Debe minimizar los tramos, respetar el impacto ecológico y asegurar que la tubería fluya por gravedad.
```bash
./practica2 -m ../mapas/mapa30.map -seed 1 -n 4 -i 8 13 0 -t 11 18 0 -O 18 12
```

### Nivel 5: ¡A Poner Tuberías! (Construcción Cooperativa)
**Objetivo:** Nivel mixto con mapa conocido. El equipo debe desplazarse, coordinarse cara a cara y usar acciones concurrentes (`RAISE`, `DIG`, `INSTALL`) para construir físicamente la red planificada en el Nivel 4.
```bash
./practica2 -m ../mapas/mapa30.map -seed 1 -n 5 -i 8 13 0 -t 11 18 0 -O 18 12
```

### Nivel 6: ¿Que alguien ponga luz? (El Desafío Final)
**Objetivo:** El reto definitivo. El mapa es completamente desconocido al inicio. Requiere combinar la exploración reactiva (Niveles 0/1), la búsqueda y planificación con información incierta (Niveles 2/3/4) y la construcción cooperativa (Nivel 5) antes de que se agoten los recursos.
```bash
./practica2 -m ../mapas/mapa30.map -seed 1 -n 6 -i 8 13 0 -t 11 18 0 -O 18 12
```
*(Nota: Puedes cambiar `./practica2` por `./practica2SG` en cualquiera de los comandos anteriores si prefieres ejecutar la simulación por consola sin cargar el entorno gráfico).*

---

## ✒️ Autores / Créditos

* **Autor del código de los agentes:** [Diego Delgado Pérez / diegodelgado9]
* **Asignatura:** Inteligencia Artificial (IA)
* **Institución:** Universidad de Granada (UGR) - ETSIIT, Departamento de Ciencias de la Computación e Inteligencia Artificial (DECSAI). Base del simulador y entorno proporcionados por el equipo docente.
