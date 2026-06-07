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
- **Reinicio selectivo de posiciones colapsadas** (cada reset_every generaciones, reiniciar a uniforme las posiciones con max(α²) > reset_thr; mejor combo: thr=0.95, every=50) → **15.29%**. EMPATE exacto con el baseline (el umbral 0.95 nunca se dispara en este presupuesto). Probada, sin mejora.
- **Schedule lineal de rotación** (rot variable a lo largo de las generaciones entre rot_start y rot_end; mejor combo: 0.02 → 0.08, exploración→explotación) → **15.03%**. FUNCIONA: 0.26 puntos mejor que el baseline anterior. Gana en 9 de 12 instancias (mayor beneficio en tamaño medio: la24 −0.88, la38 −0.78, la25 −0.80). Lección: barridos en instancias pequeñas no extrapolan perfecto; las medianas/grandes son las que más se benefician de schedules temporales.
- **Schedule lineal de floor** (floor variable entre floor_start y floor_end; mejor combo: 0.02 → 0.005, alto al inicio → bajo al final) → **14.69%**. FUNCIONA: 0.34 puntos mejor. Gana en 8 de 12 instancias, ahora también las ABZ grandes (ABZ7 −0.71, ABZ8 −0.64) que el rot schedule no movía. Confirma la hipótesis: los **schedules temporales** son una familia productiva (no casualidad del rot schedule); rot + floor son ortogonales, sus mejoras se acumulan (−0.60 desde el baseline pre-iter4 15.29 → 14.69).
- **Cosine annealing en la rotación + floor constante bajado a 0.005** (vs el lineal anterior + floor schedule 0.02→0.005) → **13.70%**. FUNCIONA: 0.99 puntos mejor, la mayor mejora del loop. Gana en 11 de 12 instancias (solo la24 +0.52); mejoras grandes en ABZ (~−1.2) y la´s (~−1.0 a −1.6). **Deuda metodológica:** este combo mezcla 2 cambios (cosine shape + floor constante 0.005); el comparable limpio del barrido (cosine con los MISMOS endpoints del baseline = `cos_baseline`) salió peor (9.09 vs 8.77 lineal en 3 instancias). Por tanto la mejora puede deberse al floor bajado a 0.005 constante más que a la forma cosine. Iteración futura debería **aislar** el efecto probando "floor constante 0.005 con rotación lineal" para descomponer la contribución de cada cambio.
- **Schedule de samples** (tamaño de muestreo K variable por generación, manteniendo media ≈ 250 para presupuesto comparable; mejor combo: 100 → 400, ramp up con cosine) → **13.54%**. FUNCIONA: 0.16 puntos mejor (la mejora más pequeña del loop). Gana en 7 de 12 instancias, beneficio principal en medianas (la24 −0.87, la29 −0.69, la38 −0.44). Confirma parcialmente la hipótesis "todas las dimensiones temporales se benefician de schedules", pero con **magnitud decreciente** — rendimientos decrecientes en la familia. Nuevos parámetros `qea.samples_start` / `qea.samples_end` (opt-in, defaults a constante).
- **Cosine warm restarts** (período de reinicio del schedule; SGDR-style: cada `restart_period` generaciones el progreso del schedule vuelve a 0, permitiendo múltiples ciclos exploración→explotación; mejor combo del barrido: period=50, 10 ciclos en 500 generaciones) → **13.72%**. NO funciona: 0.18 puntos peor, gana solo en 4 de 12. El barrido en 3 instancias pequeñas cantaba −0.59 puntos (period=50 vs no restart), pero las grandes ABZ y la mayoría de la´s salen peor con restarts — los reinicios rompen el progreso de búsqueda útil en instancias grandes. **Refina la teoría**: la familia "schedules temporales" del QEA en IJSP funciona mejor **monótona** que cíclica; los kicks periódicos pueden ayudar a instancias que convergen rápido pero perjudican a las que necesitan progreso sostenido. Probada, sin mejora.
- **Rotation dropout** (cada generación, cada posición se actualiza con probabilidad `rot_frac` < 1; análogo a dropout en redes neuronales — regularización del aprendizaje. Mejor combo del barrido: rot_frac=0.6, −0.35 puntos en 3 instancias) → **13.78%**. NO funciona: 0.24 puntos peor que el baseline, gana solo en 5 de 12. Mismo patrón que warm restarts (iter 8): el sweep en 3 pequeñas no extrapola a las 12. **Refina aún más la teoría**: el QEA en IJSP es **sensible** al patrón actualización-completa-por-generación; ralentizar el aprendizaje (dropout) o reiniciarlo (restarts) ambos perjudican. La señal estable y completa hacia el best parece esencial. Probada, sin mejora.
- **Schedule de temperatura en el muestreo** (al observar K individuos, P[j] ∝ amplitude^(2τ), con τ variable por generación; ortogonal al floor: floor modifica la *distribución almacenada*, τ modifica *cómo se muestrea*. Mejor combo del barrido: τ 0.7 → 1.3, explorar → explotar, misma dirección que rot schedule) → **11.98%**. ⭐ **MAYOR MEJORA DEL LOOP: 1.56 puntos**. Gana en **TODAS las 12 instancias**, mejoras espectaculares en las históricamente más difíciles (ABZ7 −2.45, ABZ8 −3.31, ABZ9 −3.36, la27 −3.00, la29 −1.93). El sweep cantó −0.61 (en 3 instancias) y la realidad dio −1.56 — esta vez el sweep **subestimó** la mejora (opuesto a iters 8/9). Confirma que la familia "schedules monótonos" sigue viva y que **la dimensión "muestreo" era una palanca completamente nueva**, ortogonal al floor en términos teóricos y experimentales. Nuevos parámetros `qea.tau_start` / `qea.tau_end` (opt-in, default 1.0).
- **Schedule con plateau inicial (warmup_frac)** (durante una fracción inicial del presupuesto todos los schedules se mantienen en sus valores `_start`; después progresan en el tiempo restante. Mejor combo del barrido: warmup_frac=0.3, −0.20 puntos en 3 instancias) → **12.46%**. NO funciona: 0.48 puntos peor, gana solo en 1 de 12. El plateau retrasa la transición τ 0.7 → 1.3, que era lo que más ayudaba; mantener τ=0.7 (exploración) durante 150 generaciones extra impide la fase de explotación crítica. **Refina la teoría aún más**: ALTERAR EL TIMING de los schedules existentes (no sus valores) es contraproducente; los schedules monótonos funcionan precisamente porque cubren todo el budget linealmente. Probada, sin mejora.
- **Ceiling schedule** (techo sobre la amplitud máxima por posición: dual del floor; previene colapso total. Mejor combo del barrido: ceil 1.0 → 0.90, cap gradual hacia 0.9 al final, −0.31 puntos en 3 instancias) → **11.96%**. BORDERLINE: 0.02 puntos por debajo del baseline (11.98 → 11.96), gana en 8 de 12 pero por márgenes ínfimos (±0.30). Magnitud DENTRO DEL RUIDO ESTADÍSTICO de 30 runs. Aplicando consistencia con iter 3 (empate exacto → rollback), se descarta. Lección: una mejora real requiere magnitud > ~0.1 puntos en 12 instancias × 30 runs. Probada, sin mejora estadística clara.
- **Top-K truncation en muestreo** (al observar la posición p, considerar solo los topK jobs con mayor amplitud², renormalizar entre ellos. Schedule monótono topk_start → topk_end. Mejor combo del barrido: 20 → 5, todos los combos empataban a 7.42 en 3 instancias) → **11.98%**. NO funciona: EMPATE EXACTO al cuarto decimal con baseline (cada instancia idéntica). Confirmación experimental: tau=1.3 al final ya concentra suficientemente la distribución que el truncamiento extra acaba seleccionando los mismos jobs que la roulette completa. **Tau es el "techo" de la dimensión muestreo** — más concentración no aporta información nueva. Probada, sin mejora (redundante con tau).
- **Schedule de target de rotación: bestSoFar vs genBest** (cada gen, con prob `w(t)` rotar hacia bestSoFar global, con prob `1-w(t)` rotar hacia el mejor de la generación actual; schedule monótono w_start → w_end. Mejor combo del barrido: `w 0.0 → 1.0`, puro genBest al inicio → puro bestSoFar al final, −0.76 puntos en 3 instancias) → **11.08%**. FUNCIONA: 0.90 puntos mejor, gana en 11 de 12 instancias (solo la25 empate). Mejoras grandes en medianas (FT20 −1.71, la38 −1.52, la24 −1.42), también ABZ (−0.55 a −0.76). **Patrón crítico del barrido: SOLO los extremos funcionan** (w_start ∈ {0.0}), las mezclas suaves (0.3, 0.5, 0.7) o reverse (1.0→0.5) empeoran. Coherente con el principio "la señal de aprendizaje debe ser estable y completa en cada fase": al inicio TODO genBest (novedad pura), al final TODO bestSoFar (estabilidad pura), nunca un híbrido que no se compromete. Nueva dimensión escalar ("qué tan novedosa es la señal de aprendizaje"), ortogonal a todas las anteriores. Nuevos parámetros `qea.target_w_start` / `qea.target_w_end` (opt-in, default 1.0).
- **Step factor diferenciado para genBest target** (cuando target=genBest en iter 14, multiplicar el step de rotación por un factor < 1 para amortiguar la señal ruidosa; sin schedule, sólo escalar fijo. Mejor del barrido: factor=0.7, −0.33 puntos en 3 instancias, pero patrón no-monotónico: 0.85 empeora, 0.5 empeora más) → **11.30%**. NO funciona: 0.22 puntos peor, gana solo en 5 de 12. Las medianas (la21, la24, la29) salen peor (~+0.85 cada una), confirmando que el step REDUCIDO impide a la rotación capitalizar la "novedad" de genBest. Refina la teoría iter 14: **rotar hacia genBest requiere step COMPLETO** — la dirección de exploración solo aporta cuando se aplica con magnitud completa; amortiguarla la convierte en ruido inútil. Probada, sin mejora.
- **Mejor versión actual (baseline): posicional, samples schedule 100→400 cosine (avg 250), rotación cosine 0.02→0.08, floor constante 0.005, sampling τ 0.7→1.3, target_w 0.0→1.0 → AvgRE 11.08%.** Brecha con GA puro: ~1.30 puntos. El QEA puro está prácticamente a la par del GA puro.

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
