# MO green IJSP — makespan + idle energy (línea multiobjetivo)

Diario canónico de la línea. Detalle fino de la Fase 0 en
[PHASE0_NOTES.md](PHASE0_NOTES.md); teoría del vecindario en
[N2ME_theory.tex](N2ME_theory.tex); borrador del paper en
`Papers/GreenIJSP/main.tex`. Rama: `experiment/multiobjective`.

## Idea y hueco

Multiobjetivo verde para el **interval JSP** (sin flexibilidad): minimizar
makespan intervalo + energía de paradas (NPE). Hueco en la literatura del
grupo: fuzzy JSP tiene Pareto-MO (Afşar 2022 SwEvo); interval FJSP tiene
lexicográfico (Afşar 2024/25); **interval JSP no lo tiene nadie**.

Dos propiedades estructurales guían todo:
1. **AE invariante** en JSP no-flexible → el objetivo verde es solo la
   energía pasiva/idle (NPE = Σ Pₖ·huecos, por componente del intervalo).
2. **NPE no es regular** → el espacio de decisión es (orden, timing); con
   timing semi-activo Cmax y NPE parecen casi colineales (corr +0.80..+0.96)
   pero es un artefacto.

## Cronología / estado

| Fase | Qué | Estado | Commit |
|---|---|---|---|
| 0 | Biblio + modelo + **gate de conflicto** (ε-constraint CP-SAT, timing libre: −56% NPE a +10% Cmax → conflicto GENUINO) | ✅ 2026-07-04 | `ee393ff` |
| 1 | C++: `ijsp.energy`, `ijsp.makespan-energy` (lexicográfico LEX2 4 niveles), sección POTENCIA PASIVA, frontera LS en ABCPSO; smoke verificado MATCH vs Python | ✅ 2026-07-04 | `1d07a99` |
| 2a | Runner LexME + baseline gratis (N2 **publicado**, _Sols del exp4 en el clon viejo); brazo con N2 tras decisión de usar solo lo publicado | ✅ | `97b9a93`, `0214151` |
| Teoría | N2ME (vecindario bi-crítico): factibilidad probada; conectividad Casos A+B probados, **Caso C abierto** | ✅ nota | `0214151` |
| 2b | Right-shift: exacto (LP/CP-SAT ms) + heurístico (86-95% del exacto); validación PASS | ✅ | `4e262e4` |
| **Exp. 1** | **LexME vs N2 publicado, 82/82 (30 runs, 900 s)** | ✅ 2026-07-05 | `1b51795` |
| Paper | Borrador completo elsarticle con Exp. 1 dentro | ✅ | `3abed4c`, `1b51795` |
| Exp. 2 | Frentes exactos (6 instancias × 11 ε, proxy midpoint): conflicto confirmado en TODAS (−23%..−56% NPE a +10% Cmax). Distancia al frente (media): N2-pub ~50%, LexME ~38%, LexME+RS ~23% → hueco restante = objetivo N2ME. `.dat` pgfplots en `results/fronts_dat/`; la29 sin envolvente en caps tensos (CP-SAT no cierra) | ✅ 2026-07-05 | `07da3c7` |
| Exp. 3 runs | LexME-N2ME full 82 (30 runs/900s, 14 paralelo; binario ORIGINAL homogéneo — rebuilds posteriores al último launch) | 🟡 72/82 a 2026-07-07 00:40 | — |
| Exp. 3 preview | **N2ME original NO escala**: bien en ≤20×20 (NPE −1..−3%) pero colapso en grandes (hasta +10% Cmax, +92% NPE vs LexME-N2). Causas: E(σ) explota + computeNPE O(nm) por vecino evaluado | ⚠ diagnóstico | — |
| N2ME v2 | **E restringido** (≤2 arcos/máquina/comp; param `neighbourhood.energy-restricted`) + **NPE perezoso** (solo si makespan empata-o-mejora; centinela recomputado en accept). Smokes tai50 300s: completo 2960/22343 → restringido 2861/17775 → **v2 2838/15957 — sigue perdiendo vs desempate puro 2760/8298**. Diagnóstico final: no es (solo) coste — es DINÁMICA: los movimientos de energía makespan-neutrales desplazan a los de empeoramiento que la TS necesita para diversificar → ciclado en mesetas mediocres. Explica pequeñas-bien/grandes-mal. **HIPÓTESIS v3 anotada (no implementada): E "plateau-gated"** (ofrecer arcos de energía solo cuando el makespan actual iguala al mejor-hasta-ahora). DECISIÓN: parar de iterar; N2ME queda como resultado con alcance (≤20×20) salvo que Hernán quiera probar v3 | ⚠ v2 insuficiente | `5be99f2` |
| Exp. 3 | N2ME en C++ (`ijsp.makespan-energy.n2me`): TS lexicográfica-consciente; smoke ft10 VERIFICADO (makespan 940, NPE (4881,5899) vs (7528,8241) del desempate solo ≈ **−32%**; MATCH vs Python); regresión 5/5 | 🟡 implementado, runs pendientes (`run_n2me.sh`) | `efd0616` |
| Exp. 4 | Maquinaria Pareto (dominancia sobre producto LEX2, archivo, NDS+crowding) + frentes | ⬜ | — |

## Resultado Exp. 1 (titular del paper)

LexME (desempate lexicográfico Cmax→NPE) vs TS-N2 makespan-only publicado;
82 instancias × 30 runs; todo Wilcoxon-significativo
([RESULTS_LEXME_FULL.txt](RESULTS_LEXME_FULL.txt)):

| Grupo | ΔCmax% | ΔNPE% | ΔNPE% tras RS |
|---|---|---|---|
| Classical | −0.19 | −7.1 | −6.1 |
| 15×15 | −0.34 | −3.2 | −2.1 |
| 20×15 | −0.26 | −6.5 | −5.8 |
| 20×20 | −0.25 | −3.4 | −3.2 |
| 30×15 | −0.19 | −10.3 | −9.8 |
| 30×20 | −0.31 | −5.7 | −5.2 |
| 50×15 | −0.11 | **−26.5** | **−27.2** |
| 50×20 | −0.55 | −12.6 | −12.5 |
| **GRAND** | **−0.29** | **−7.9** | **−7.4** |

- Descomposición: orden −7.9% + timing −8.2% = **−15.0% combinado** a coste
  cero de makespan. La ventaja del desempate **sobrevive** al right-shift.
- Patrón: crece con el tamaño y el ratio jobs/máquinas — mesetas de makespan
  anchas (p.ej. 50×15, RE≈0.3% en el paper ASOC) = más energía abandonada
  por la búsqueda ciega.
- Cmax −0.11..−0.55% a favor de LexME: en el paper se enuncia como "no
  cuesta nada" (caveat de condiciones de carga; sin reclamar mejora).

## Mapa de ficheros

| Fichero | Qué hace |
|---|---|
| `conflict_check.py` | Check de conflicto con schedules semi-activos (Fase 0); parser dual de instancias (formato A con cabeceras / B sin) |
| `epsilon_probe.py` | Frentes exactos ε-constraint (CP-SAT, midpoints ×2, inicios libres); env `PROBE_TIME`, `PROBE_CSV` → `results/fronts.csv` |
| `exp2_fronts.sh` | Lanzador del Exp. 2 (6 instancias × 11 ε, 120 s/punto) |
| `extend_instances.py` | Genera `SelectosYTaillardIntervalosEnergia/` (Pp~U{2..8}, seed 23 POR instancia — mismo esquema en todo el pipeline) |
| `setup/setup_LexME_N2_tuned.txt` | Config del brazo del paper (N2 publicado) |
| `setup/setup_LexME_tuned.txt` | Variante N2Plus (solo uso interno, no publicado) |
| `run_lexme.sh` | Runner escalonado/reanudable (smoke/classical/tai_small/full) |
| `rightshift.py` | Timing post-hoc para orden fijo: exacto (LP) + heurístico (pasada atrás) |
| `validate_npe.py`, `validate_rightshift.py` | Validadores cruzados (C++ vs Python; invariantes RS) |
| `analyze_lexme.py` | Análisis 3 capas vs baseline; env `RS=off|heur|exact`, `BASE_DIR`, `LEX_DIR` |
| `regression_smokes.sh` | Gate de regresión ligero (sustituye al desaparecido verify_refactor.sh) |
| `results/LexME/` | Brazo del paper (N2), 82/82 — gitignored |
| `results/LexME_N2Plus_arm/` | Brazo interno N2Plus (clásicas) — gitignored |
| `N2ME_theory.tex` | Nota formal del vecindario bi-crítico |

Baseline del paper: `CLionProjects/IJSP/experiments/results/exp4/N2_tuned/`
(_Sols crudos del N2 publicado, clon viejo — NO mover/borrar).

## Cómo reproducir

```bash
# (WSL) generar instancias con energía
python3 experiments/mo_green_2026/extend_instances.py
# brazo LexME (Hernán lanza; reanudable)
bash experiments/mo_green_2026/run_lexme.sh full 14
# análisis 3 capas vs N2 publicado
RS=heur python3 experiments/mo_green_2026/analyze_lexme.py
# frentes exactos (Exp. 2)
bash experiments/mo_green_2026/exp2_fronts.sh
```

## Cadena de diagnóstico fase B (2026-07-07) — mecanismo identificado

1. Sweep v0 (TS por niveles): plano (6302→6243). 2. +E completo, +Caso C,
+presupuesto: plano igual. 3. **`neighbourhood_probe.py` (decisivo)**:
paisaje exhaustivo de 1 movimiento desde el ancla — de 89 inversiones y
561 inserciones factibles, solo 1-2 mejoran NPE incluso con cap +10%
(techo 6243 vs exacto 2400-4400) → **ninguna búsqueda de trayectoria
puede llegar; el mecanismo de cruce es población+recombinación**
(pilotos evolutivos llegaron a 3862-4038; JOX está en TODOS los motores —
la diferencia entre ABC y MA es la RETENCIÓN, no el cruce). 4. **Test del
clamp poblacional (variante b): VALIDADO** — ABC-Pareto con
`energy.goal-cmax-lo/hi` (evaluación + vecindario clampeados) en ft10 a
+5%: NPE ~4087 en 900s (~4485 en 30s!) con makespan ≤ cap garantizado.
5. Fix: los offers al archivo re-evalúan con raw=true (Cmax real, no
clampeado). 6. `pilot_ladder.sh`: escalera de 6 niveles × 90s × 8
instancias corriendo; `merge_ladder.py` listo.

## ★ ESCALERA v2 (warm-seeded) — VALIDADA EN TODO EL RANGO (2026-07-07)

`RESULTS_LADDER2_PILOT.txt`. Niveles clampeados de 90 s sembrados
(L1 ← 30 anclas LexME; Lk ← frente de Lk−1 + anclas, maxmin):

| Instancia | Frente | vs todo lo anterior |
|---|---|---|
| ft10 | 11 p., NPE→3138, HV 0.788 | ≈récord v1 (0.796); libre era 0.66 |
| tai20_20 | 8 p., NPE→47545 | mejor que todos (48450) |
| tai30_15 | 6 p., 1781/9850 | **domina todo** (mejor Cmax Y NPE) |
| tai30_20 | 5 p., 2094/36946 | **domina todo** |
| **tai50_15** | 2760-2762, **NPE 5767** | LexME best 6630, libre 7711 — **aplasta** |
| **tai50_20** | 2882-2890, NPE→19528 | mejor que el best-of-30 de LexME |

**Por primera vez hay frentes reales en las instancias grandes, con puntos
que además baten el mejor NPE conocido a makespan de ancla.** Único flojo:
tai15 (HV 0.35 — cobertura estrecha de Cmax; nota de tuning: los niveles
altos no añaden — investigar si el archivo satura o los caps altos
necesitan menos siembra de ancla). Presupuesto: 6×90 s + anclas
amortizadas del Exp. 1.

**→ Diseño del brazo Pareto del Exp. 4: escalera clampeada warm-seeded
(variante b + FromFile).** Decisiones para Hernán: niveles/presupuestos
por tamaño, irace-sobre-HV sí/no, y si ABC/MA libres quedan como brazos de
comparación.

## Pendiente

- **Exp. 2 (corriendo):** al acabar → análisis distancia-al-frente (N2 /
  LexME / LexME+RS vs frente exacto en ε=0) + figura del frente con el punto
  lexicográfico → §6.4 y Fig. del paper. Cuantifica el margen de N2ME.
- **Exp. 3:** N2ME C++ (con teorema débil basta; def. en N2ME_theory.tex).
- **Exp. 4 — MAQUINARIA COMPLETA (commits `1611698`+`1557d60`+`3749d61`):**
  ParetoArchive + ReplacementNSGA2 ('nsga2') + ABCPSO-Pareto (dump
  `_Front.csv` por run). HALLAZGO: el motor ABCPSO no llama al replacement
  (supervivencia por trials) → archivo alimentado desde applyLocalSearch;
  nsga2 queda listo para motores GA/MA. Smoke ft10: frente de 5 puntos.
  `front_metrics.py` (HV ratio + eps+ vs frente exacto): smoke 10s = HV
  0.31, eps+ 0.74 — baseline pobre esperable (motor lexicográfico no visita
  Cmax relajado). DECISIÓN PENDIENTE con Hernán: motor para el experimento
  a escala (ABC archive-based vs GA/MA con supervivencia NSGA-II real).
  Nota paper: NPE⁻ puede superar NPE⁺ (par de escenarios, no intervalo
  ordenado; footnote). PENDIENTE runs a escala (tras Exp. 3; no lanzar en
  paralelo con él — 14 cores ocupados).
  **DISEÑO Exp. 4 ACORDADO 2026-07-06 (decisión final tras Exp. 3):**
  - Ablación **Pareto±N2ME** (2 brazos): en modo Pareto el caso de N2ME es
    más fuerte — sus arcos de energía son el único operador que empuja A LO
    LARGO del frente (N2 solo hacia min-makespan); métrica = cobertura
    (HV/ε+), no punto extremo.
  - **Tuning irace por brazo escalarizado con HIPERVOLUMEN**: evaluación =
    run → `_Front.csv` → HV contra punto de referencia FIJO por instancia
    (1.1× nadir empírico de los runs de Exps. 1+3, ya disponibles) con
    normalización por instancia → coste −HV. Target-runner = patrón del
    paper de tabu + `front_metrics.py`. Entrenamiento ~20/82 estratificado
    (protocolo Fase B), held-out para evaluar. Presupuesto reducido por
    evaluación (150-300 s), validación final con el completo. Descartado:
    ε-indicator (exige frente de referencia por instancia de entrenamiento).
- **(histórico) Exp. 4 fundaciones:** `ParetoArchive` HECHO
  (dominancia sobre producto LEX2 con bounds explícitos, cap+crowding en
  plano de midpoints, dump CSV `cmax_lo;cmax_hi;npe_lo;npe_hi;solution`;
  compila). **Plan de wiring pendiente** (diseñado, no escrito):
  1. `Replacement_NSGA2`: en `apply(old,new,svars)` — NDS+crowding sobre
     old∪new escribiendo supervivientes en `new` (¡OJO ownership de
     Population/replaceIndividual: los punteros se intercambian entre
     poblaciones, revisar ReplacementParents::applyRepeat como modelo!);
     método const → archivo `mutable` o en el algoritmo. Registrar "nsga2".
  2. `ABCPSOPareto : ArtificialBeeColonyPSO`: override `run(...)` — llama
     al padre, vuelca el archivo a `<logFolder>/<signature>_Front.csv`
     (firma en EvoLauncher.cpp:94), resetea archivo. Registrar en
     AlgorithmClassRegister ("ABCPSO-Pareto").
  3. Hook de inserciones al archivo: en el Replacement (ve padres+hijos
     cada generación, choke point único) o en evaluatePopulation.
  4. Smoke ft10 contra frente exacto (fronts_dat) + métricas HV/ε Python.
  Individual fitness sigue FitnessLexicographic (N2ME LS y estadísticas
  intactas): híbrido "explotación lexicográfica + supervivencia Pareto".
- **Sonda bi-componente (pendiente, anotado 2026-07-05):** los frentes del
  Exp. 2 usan duraciones midpoint (proxy crisp; válido para el análisis de
  conflicto, declararlo en el paper). Para la validación fina de §6.4:
  actualizar `epsilon_probe.py` al modelo intervalo EXACTO — booleanos de
  secuenciación compartidos + dos copias de timing (p⁻/p⁺) + objetivos LEX2
  sobre (C⁺,C⁻)/(NPE⁺,NPE⁻), à la MILP multi-componente del #17. Solo para
  las 2-3 instancias de la validación (el modelo dobla tamaño).
- **Teoría:** Caso C de conectividad (arranques de máquina) — abierto.
- **Paper:** TODOs marcados en main.tex (coautores, refs TODO-VERIFY,
  agradecimientos); decidir irace por brazo para el experimento final.
