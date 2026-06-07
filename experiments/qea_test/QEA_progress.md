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

## 2. Cosas probadas

Una por línea (qué se probó → resultado → conclusión). Métrica: media de AvgRE(%).
Se registran **todas** las ideas probadas, mejoren o no, para no repetirlas.

- Posicional, población 50, floor 0.02, sin lamarckismo → **18.32%**. Baseline inicial.
- Población 50 → 250 → **16.78%** (−1.5). Funcionó poco: el muestreo no era el cuello de botella.
- Esquema de **precedencia** `pref[a][b]` → **22.30%** (peor). NO funcionó: pierde el intercalado fino de operaciones. Descartado.
- floor 0.02 → 0.01 → **15.29%** (−1.5). Funcionó: el floor es el parámetro dominante (floor 0.05 es claramente el peor).
- Lamarckismo no → sí → **15.30%** (≈0). No funcionó: sin efecto apreciable.
- Aprendizaje por **élite** (rotar hacia top ~10% de cada generación en vez de hacia el único bestSoFar, estilo UMDA; mejor combo del barrido: elitefrac=0.1, rotation=0.10) → **15.43%**. NO funciona: 0.14 puntos peor que el baseline. Probada, sin mejora.
- **Mutación de amplitudes** (perturbación PBIL: cada generación, con prob mutprob por posición, mezclar la distribución con la uniforme con fuerza mutstr; mejor combo del barrido: mutprob=0.10, mutstr=0.10) → **15.37%**. NO funciona: 0.08 puntos peor que el baseline. Probada, sin mejora.
- **Mejor versión actual (baseline): posicional, población 250, floor 0.01, rotación 0.04 → AvgRE 15.29%.** Es el techo del motor puro; queda ~5.5 puntos por encima del GA puro.

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
