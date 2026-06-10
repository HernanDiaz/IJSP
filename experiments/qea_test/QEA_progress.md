# QEA para IJSP — Registro de desarrollo

## 1. Descripción

Algoritmo evolutivo de inspiración cuántica (*Quantum-inspired EA*, **QEA**) para el
Interval Job Shop Problem (IJSP). A diferencia de un GA, no mantiene una población de
soluciones sino una **distribución de probabilidad** sobre el genotipo *job-order*.
Cada generación:

1. **Observa** K individuos muestreando la distribución (con reparación para que cada
   job aparezca el número de veces que le corresponde).
2. Los **evalúa** reutilizando el decoder y el evaluador del framework, de modo que el
   ranking de intervalos (EV/LEX2) sale sin coste adicional.
3. **Rota** la distribución hacia la mejor solución encontrada (puerta de rotación tipo
   q-bit).

Esquema actual: **posicional**, `amplitude[posición][job]`. El QEA se integra como un
`EvolutiveAlgorithm` más, **sin modificar la lógica de ningún otro algoritmo**.

**Archivos creados:**

- `QuantumInspiredEA.h` / `QuantumInspiredEA.cpp` — la clase del algoritmo (nuevos).
- `AlgorithmClassRegister.h` — registro como `QEA`/`QIEA` (1 `#include` + 2 líneas; aditivo).
- `Makefile` — `QuantumInspiredEA.cpp` añadido a `SOURCES` (aditivo).
- `experiments/qea_test/` — setups (`setup_qea*.txt`) y scripts auxiliares
  (`run_experiment.sh`, `parse_qea.sh`, `sweep.sh`, `sweep_parse.sh`, `timings.sh`).

**Métrica de evaluación:** RE(%) = (midpoint − LB) / LB × 100, sobre las 12 instancias
clásicas (ABZ7-9, FT10, FT20, la21/24/25/27/29/38/40), 30 runs por instancia.
Referencias del paper COR_Tabu (puros y meméticos): **GA puro 9.78%**, **TS-N₂ 2.40%**.

---

## 1.5 Restricción de tiempo (requisito de publicación)

**Para una publicación, el QEA no puede tardar mucho más que el GA de
referencia** o los revisores lo rechazarían. Se impone por tanto un
**presupuesto de tiempo: el tiempo de un run del QEA debe ser ≤ 4× la mediana
de tiempo del GA**, por instancia. El GA es el de \cite{DiazIPMU2020} (columna
*t* de la Tabla `tab:lit_classical` en `Papers/COR_Tabu/main.tex`); se toma el
valor de la tabla tal cual (**4× literal, sin ajuste de hardware**).
Además se debe utilizar una población de 250 individuos

| Inst | GA t (s) | 4×GA (s) | | Inst | GA t (s) | 4×GA (s) |
|------|----------|----------|-|------|----------|----------|
| ABZ7 | 1.8 | 7.2 | | la24 | 0.8 | 3.2 |
| ABZ8 | 1.8 | 7.2 | | la25 | 1.0 | 4.0 |
| ABZ9 | 2.2 | 8.8 | | la27 | 1.3 | 5.2 |
| FT10 | 0.5 | 2.0 | | la29 | 1.1 | 4.4 |
| FT20 | 0.7 | 2.8 | | la38 | 1.4 | 5.6 |
| la21 | 1.1 | 4.4 | | la40 | 1.2 | 4.8 |

---

## 2. Cosas probadas

Una por línea (qué se probó → resultado → conclusión). Métrica: media de AvgRE(%).
Se registran **todas** las ideas probadas, mejoren o no, para no repetirlas.

**Puntos de referencia bajo el presupuesto de tiempo (12 inst × 30 runs, G
calibrado ≤ 4× GA, single-process):**
- Baseline posicional puro (samples 250, rotación 0.04, floor 0.01, sin
  schedules; G 155–255 de `run_base.sh`) → **16.54%**. Punto de partida del loop.
- *(meta: GA puro 9.78%, TS-N₂ 2.40%)*

- **Iter 1 — Schedule lineal de rotación** (la rotación se interpola linealmente
  de `qea.rot_start` (gen 0) a `qea.rot_end` (última gen) en vez de ser
  constante; coste/gen ≈ nulo → mismo G que el baseline → comparación limpia).
  Sweep 3 inst (ft10/ft20/la25, runs=10): mejor schedule 0.06→0.10 = 9.68 vs
  constante baseline 0.04 = 10.27. Full 12×30: el mejor schedule (0.06→0.10) da
  **16.10%** (−0.44 vs 16.54). **PERO** la prueba de aislamiento (constantes vs
  schedules a igual magnitud media) muestra que una **rotación CONSTANTE 0.10 da
  15.94%**, MEJOR que el schedule (16.10%): el gradiente no aporta nada, la
  ganancia se debe a la **magnitud** de rotación, no a programarla. →
  **PROBADO, SIN mejora** (un schedule no supera a una constante de igual media).
  Código descartado (vuelta a d8f1d66). Lección: bajo presupuesto corto
  (~200 gen) conviene rotar más fuerte, pero una constante alta basta; hay que
  aislar siempre magnitud-vs-forma (cf. deuda cosine/floor del régimen 500 gen).

- **Iter 1 (hallazgo lateral) — Rotación constante 0.04 → 0.10** (la rotación
  0.04 del baseline estaba infra-tuneada; emergió de la prueba de aislamiento. Es
  un re-tuneo de parámetro, no un cambio de motor, pero avanza el baseline —
  análogo a "floor 0.02→0.01" del régimen histórico). Full 12×30 con el binario
  d8f1d66 limpio (`run_base_r10.sh`) → **15.94%** (−0.60 vs 16.54). Gana en 11/12
  (solo la40 +0.06); mayores: ABZ8 −1.09, FT20 −0.95, ABZ9 −0.85, la29 −0.69,
  la25 −0.65. ✅ **FUNCIONA.** Pendiente: barrer constantes más altas
  (0.12–0.20) en una iteración futura para hallar el óptimo.

- **Mejor versión actual (baseline del loop): posicional, samples 250, rotación
  constante 0.10, floor 0.01, G calibrado ≤ 4× GA → AvgRE 15.94%.** Brecha con
  GA puro: 6.16 puntos. Evidencia: `experiments/qea_test/run30_base_r10/`.

---
## 3. Cómo proceder

Protocolo de trabajo (iterativo):

1. **Proponer una idea** de mejora que **no se haya probado ya** (revisar la sección 2
   para no repetir ninguna). Debe ser un cambio del propio motor del QEA.
2. **Implementarla** respetando la arquitectura del framework: los cambios viven solo en
   `QuantumInspiredEA.h/.cpp`; no se debe afectar al funcionamiento de ningún otro
   algoritmo ni operador.
3. **Probarla** con un barrido en rejilla para fijar sus parámetros y, a continuación,
   con una corrida completa comparada contra el baseline actual.
4. **Si mejora** el baseline → `git commit` (sin push) y actualizar este documento:
   añadir la idea a la sección 2 con su resultado y fijar el nuevo baseline.
5. **Si no mejora** → descartar los cambios (`git restore` de los dos archivos) para
   volver al baseline, y registrar igualmente la idea en la sección 2 como
   *probada, sin mejora*.
