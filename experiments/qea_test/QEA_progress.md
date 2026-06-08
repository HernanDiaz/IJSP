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

| Inst | GA t (s) | 4×GA (s) | | Inst | GA t (s) | 4×GA (s) |
|------|----------|----------|-|------|----------|----------|
| ABZ7 | 1.8 | 7.2 | | la24 | 0.8 | 3.2 |
| ABZ8 | 1.8 | 7.2 | | la25 | 1.0 | 4.0 |
| ABZ9 | 2.2 | 8.8 | | la27 | 1.3 | 5.2 |
| FT10 | 0.5 | 2.0 | | la29 | 1.1 | 4.4 |
| FT20 | 0.7 | 2.8 | | la38 | 1.4 | 5.6 |
| la21 | 1.1 | 4.4 | | la40 | 1.2 | 4.8 |

**Mecanismo:** el QEA sigue parando **por generaciones** (reproducible — parar
por tiempo dependería de la carga de la máquina y no sería reportable), pero el
número de generaciones *G* se **calibra por instancia** para que un run quede
≤ 4× GA. La calibración mide cuántas generaciones completa la versión actual
dentro de 4× GA (`calibrate.sh`). *G* depende de la versión: una operación más
cara por generación deja menos generaciones, así que **se recalibra cada vez que
cambia el coste por generación**. El tiempo a comparar es el **single-process**
(sin contención de runs en paralelo), que es el comparable con la tabla del GA.

**Guardrail del loop:** una idea se descarta si, además de no mejorar AvgRE,
empuja el tiempo por encima de 4× GA a igualdad de presupuesto.

**Implicación metodológica (importante):** todas las mediciones de la sección 2
se hicieron con **500 generaciones**, sin restricción de tiempo. Bajo el
presupuesto de tiempo, una mejora cara por generación (p.ej. el sampling con
`pow(tau)`) deja menos generaciones y puede dejar de compensar. Comprobado: el
baseline iter 17 (todos los schedules) da AvgRE **10.96%** con 500 gen, pero
**14.62%** al recortarlo a ≤ 4× GA (G ≈ 78–127). Por eso el desarrollo se
**reinicia desde el baseline posicional** (commit `d8f1d66`) y cada idea se
re-valida por su efecto en AvgRE *a igualdad de tiempo* (≤ 4× GA), no de
generaciones.

---

## 2. Cosas probadas

> ⚠️ **Cambio de régimen (ver §1.5).** Las entradas siguientes se midieron con
> **500 generaciones sin límite de tiempo** y se conservan como conocimiento
> histórico (qué se probó y por qué funcionó/no). A partir del reinicio, el loop
> opera bajo el **presupuesto de tiempo (≤ 4× GA)** y cada idea se **re-valida**
> en ese régimen — su veredicto histórico no se da por bueno automáticamente.

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
- **Step factor amplificado para bestSoFar target** (la simétrica conceptual de iter 15: cuando target=bestSoFar, escalar el step por un factor > 1 para reforzar la señal estable. Mejor del barrido: factor=1.3, −0.57 puntos en 3 instancias) → **11.08%**. EMPATE EXACTO al segundo decimal con baseline (11.08 = 11.08), gana en 6 de 12 (ABZ7 −0.58, ABZ8 −0.94) pero pierde en 6 (la21 +0.70, la24 +0.62) — las mejoras y empeoramientos se cancelan exactamente. Hipótesis simétrica refutada: amplificar la señal estable NO ayuda. Refina aún más la teoría: el step `rot_end=0.08` ya está bien tuneado para AMBOS targets; cualquier asimetría en el step (ni amplificar bestSoFar ni amortiguar genBest) mejora la búsqueda. **La rotación es agnóstica al target**: lo que importa es la dirección, no la magnitud. Probada, sin mejora.
- **Anti-rotación hacia genWorst** (tras la rotación principal, aplicar un paso negativo (`-antiStep`) hacia el peor individuo de la generación actual para alejar la distribución de genotipos malos; schedule monótono `anti_step_start → anti_step_end`. Mejor combo del barrido en 3 instancias: 0.005 → 0.02, −0.60 puntos; 0.02→0 taper off: −0.31; constante 0.005: −0.05; 0→0.01 ramp: +0.72; constante 0.01: +0.55; 0→0.02 ramp: −0.07) → **10.96%**. FUNCIONA: 0.12 puntos mejor, gana globalmente aunque con mejoras pequeñas por instancia. Primera vez que se introduce **señal negativa** (aprendizaje contrastivo); el efecto es modesto pero consistente. Se añade clamp `cNew < 0.0` en `rotatePositional` para pasos negativos. Nuevos parámetros `qea.anti_step_start` / `qea.anti_step_end` (opt-in, default 0 = desactivado).
- **Stagnation-triggered K boost** (cuando `iterationsNI >= stagnation_threshold`, multiplicar K por `boost_k` para explorar más ampliamente; la señal se resetea cuando bestSoFar mejora. Mejor combo del barrido en 3 instancias: thresh=10, boost_k=3.0 → 5.43% vs baseline 6.06% (−0.63 pts)) → **DESCARTADO POR TRAMPA METODOLÓGICA**. Con thresh=10 y boost_k=3.0, el algoritmo estanca frecuentemente desde gen 10 → K queda triplicado la mayor parte del run (K efectivo ~750 vs K nominal 250), lo que supone ~3× más evaluaciones que el baseline. La mejora observada en el sweep es un artefacto de usar más evaluaciones, no del mecanismo adaptativo. Los boosts justos (boost_k=1.5, solo 50% más evaluaciones) no dan mejora apreciable en el sweep (−0.06 pts, ruido). Además, boost_k=3.0 hace que cada run tarde ~3× más, haciendo impracticable el experimento de 30 runs (abortado). **Conclusión: cualquier mejora por "muestrear más cuando estancado" equivale a simplemente aumentar el presupuesto de evaluaciones, no a una mejora algorítmica real.**
- **Consensus reinforcement** (tras la rotación principal, aplicar un paso extra SOLO en las posiciones donde el target de rotación y genSecondBest colocan el mismo job; el doble acuerdo entre buenos individuos debería reforzar posiciones con evidencia sólida. Mejor combo del barrido en 3 instancias: no hay — TODOS son peores que baseline. Baseline: 6.06%; mejor consensus: 0→0.03 da 6.13% (+0.07), pero la mayoría empeoran significativamente (0→0.01: +0.87, 0→0.02: +0.87).) → NO funciona. **Diagnóstico**: genBest y genSecondBest tienen alta correlación (ambos se muestrean de la misma distribución ya parcialmente convergida), por lo que el consensus se activa en casi todas las posiciones y equivale a duplicar el paso de rotación → convergencia prematura. Probada, sin mejora.
- **Amplitude decay (olvido explícito hacia uniforme)** (cada generación, mezclar amplitudes ligeramente hacia la distribución uniforme: `a = normalize((1-d)*a + d*(1/√n))`; factor `d` scheduled de alto (inicio, más olvido) a bajo (final, menos olvido). Análogo al dropout en redes neuronales pero continuo. Barrido en 3 instancias: baseline 6.06%; mejor decay: constant 0.005 → 6.35% (+0.29); peor: 0.001→0.0001 → 7.18% (+1.12).) → **NO funciona.** Todos los variants con decay empeoran. El olvido acumulado en 500 generaciones es destructivo: incluso d=0.001 da `(1-d)^500 ≈ 0.61` → 40% del aprendizaje borrado. El floor ya maneja la diversidad sin interferir con la señal de aprendizaje. Probada, sin mejora.
- **Anti-rotation conflict filter** (skip anti-rotation at positions where genWorst places the SAME job as bestSoFar; rationale: at those positions anti-rotating reduces the amplitude of bestSoFar's job, opposing the main learning signal. Parámetro: `qea.anti_filter = true`. Barrido en 3 instancias: baseline 6.06%; filter+0.002→0.01: 6.51%, filter+0.01→0.04: 6.60%, filter+0.005→0.02: 6.68%, filter+0.005→0.04: 7.13%.) → **NO funciona.** Todos los variants con filtro empeoran. El filtro REDUCE la anti-rotación total (elimina posiciones que sí se actualizaban), y el efecto "conflictivo" de esas posiciones es menor que el costo de perder su presupuesto de anti-rotación. La anti-rotación necesita cubrir todas las posiciones consistentemente para ser efectiva.
- **Anti-rotación multi-objetivo (bottom-M)** (en lugar de anti-rotar solo hacia genWorst (rango K-1), anti-rotar hacia los M peores con paso antiStep/M cada uno (mismo presupuesto total, señal más robusta). Parámetro: `qea.anti_num`. Barrido en 3 instancias: baseline N=1: 6.06%; N=2 mismo total: 6.51%; N=3 mismo total: 6.36%; N=5 mismo total: 7.22%; N=2 con 2× presupuesto: 6.11%.) → **NO funciona.** Todos peores que baseline. El paso reducido por target (antiStep/M) es demasiado pequeño para ser efectivo. La anti-rotación necesita la MAGNITUD COMPLETA en un solo target para dar señal significativa. El ruido en un único individuo extremo no es el problema — la robustez de múltiples targets no compensa la pérdida de magnitud. El caso N=2 con 2× presupuesto (6.11%) es casi igual al baseline pero esto equivale simplemente a doblar el anti-step, ya probado como peor en iter 17.
- **Mejor versión actual (baseline): posicional, samples schedule 100→400 cosine (avg 250), rotación cosine 0.02→0.08, floor constante 0.005, sampling τ 0.7→1.3, target_w 0.0→1.0, anti-rotación 0.005→0.02 → AvgRE 10.96%.** Brecha con GA puro: ~1.18 puntos. El QEA puro está prácticamente a la par del GA puro.

---

## 2.T Re-validación bajo presupuesto de tiempo (≤ 4× GA)

Desde el reinicio (§1.5) cada idea se mide bajo el presupuesto de tiempo
(generaciones calibradas a ≤ 4× GA por instancia, calibración single-process).
Los AvgRE de aquí **no** son comparables con los de §2 (presupuesto distinto).

**Puntos de referencia bajo tiempo (12 inst × 30 runs):**
- Baseline posicional simple (sin schedules, G ≈ 155–255): **16.54%**.
- iter 17 completo (todos los schedules, G ≈ 78–127): **14.62%**.
- *(meta: GA puro 9.78%)*

- **T1 — Quitar el schedule de temperatura de muestreo (tau)** (sweep 3 inst,
  calibrado a 4× GA: full 8.71%, **notau 6.91%** (−1.80); nosamp 9.50, noanti
  9.00, nosamp_noanti 9.99 → samples-schedule y anti-rotación SÍ ayudan bajo
  tiempo, solo tau estorba). Full 12×30: **13.08%** (−1.54 vs iter17 14.62%).
  ✅ **FUNCIONA.** Sin el `pow()` del muestreo el QEA completa 3–4× más
  generaciones (G ≈ 249–403 vs 78–127); la mayor cantidad de iteraciones supera
  a la mejor explotación por iteración. **Contra-intuitivo:** tau fue la MAYOR
  mejora con 500 gen (−1.56) y es la PEOR bajo tiempo — el régimen de tiempo
  reordena las prioridades. Lección: bajo presupuesto corto, **el coste por
  generación pesa tanto como la calidad por generación**; las operaciones caras
  (pow por muestra) hay que evitarlas salvo que su ganancia por gen sea enorme.

- **T2 — Re-tunear el samples schedule sobre notau** (variantes (start,end):
  100→400 base, 100→300, 100→250, 100→200, 250 const, 50→250; sweep 3 inst con
  **recalibración de G por variante**). Resultados 7.33–8.19%, pero **sin mejora
  detectable**: la calibración de G por tiempo es **ruidosa** (~0.5 pts — la
  misma config notau dio 6.91% en T1 y 7.41% en T2 por G distinto), y todas las
  diferencias caen dentro de ese ruido. Se mantiene samples 100→400. **Lección
  metodológica:** para variantes que NO cambian el coste/gen (rotation,
  target_w, floor, anti-step), usar el **G FIJO** de notau (sin recalibrar)
  elimina este ruido; solo recalibrar cuando la variante cambia el coste/gen
  (samples, o reintroducir tau).

- **T3 — Re-tunear el rotation schedule sobre notau** (G **FIJO** de notau, sin
  recalibrar → comparación limpia; sweep 3 inst, runs=10: r02_08 base 6.96,
  **r04_08 6.72**, r04c 6.75, r02_12 6.78, r02_10 7.08, r02_06 7.32). Mejor:
  **rot_start 0.04** (vs 0.02), rot_end 0.08. Full 12×30: **12.73%** (−0.35 vs
  notau 13.08%), gana en 9/12 (mayores en abz −0.5/−0.86 y la38 −1.01). ✅
  **FUNCIONA** (modesta). Con ~300 gen (menos que las 500 del régimen viejo)
  conviene rotación inicial más fuerte para converger antes. La metodología de
  G fijo (vs recalibrar) elimina el ruido que hizo inconcluso a T2.

**Mejor versión bajo tiempo (baseline del loop-T):** posicional, samples
100→400 cosine, rotación cosine **0.04→0.08**, floor 0.005, target_w 0→1,
anti-rotación 0.005→0.02, **SIN tau**, parada por generaciones (G calibrado a
≤ 4× GA) → AvgRE **12.73%**. Brecha con GA puro: 2.95 puntos.

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
