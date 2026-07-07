# Experimento 4 — Frentes Pareto para el IJSP verde (protocolo definitivo)

Acordado con Hernán 2026-07-07. Principio rector: **todos los brazos
arrancan aleatorios, 900 s/run, mismas semillas RNG** — sin datos externos,
sin contabilidades de presupuesto. La siembra de la escalera es interna al
run (eslabón a eslabón), parte del algoritmo.

## Brazos

| Brazo | Algoritmo | Inicialización |
|---|---|---|
| **LADDER** | Pipeline autocontenido: ancla lexicográfica (aleatoria) → niveles clampeados encadenados (siembra interna del run) | Aleatoria |
| **ABC-P** | ABCPSO-Pareto libre (archivo; N2ME LS) | Aleatoria |
| **MA-P** | MEMETIC-PARETO (NSGA-II; N2ME LS) | Aleatoria |
| *LexME*, *N2-pub* | Ya computados (Exps. 1/baseline) | Referencias externas contextuales |

Nota: los tres brazos usan N2ME como LS (goal-mode solo existe ahí; en los
libres actúa como lexicográfico-consciente). Se declara.

## Presupuesto adaptativo del LADDER (regla por tamaño, dentro de 900 s)

| Grupo | Ancla | Niveles (‰ sobre C* del ancla del run) |
|---|---|---|
| Classical, 15×15, 20×15, 20×20 | 400 s | 8 × 60 s: {2,5,10,15,20,30,50,70} |
| 30×15, 30×20 | 500 s | 6 × 65 s: {5,10,20,30,50,100} |
| 50×15, 50×20 | 600 s | 5 × 60 s: {5,10,20,50,100} |

- El ancla es un run ABCPSO-Pareto lexicográfico normal (sin cap): produce
  el C* del run, su `_Sols` y su `_Front` → semillas del nivel 1.
- Nivel k siembra de: frente del nivel k−1 + `_Sols`+frente del ancla
  (`creation = ijsp.solutions-file`, `seed-selection = maxmin`).
- Salida del run: frente fusionado (no-dominados de ancla + todos los
  niveles, valores SIN clampear).

## Protocolo

- 82 instancias × 30 runs × 3 brazos; seeds 1-30; RS heurístico post-hoc
  sobre todos los puntos archivados de todos los brazos (simétrico).
- Ejecución por tandas (classical → tai_small → full), 14-paralelo,
  reanudable. Hernán lanza. CPU estimada ≈ 30-45 h por brazo.

## Métricas y estadística

1. **Frente de referencia por instancia** = no-dominados de la unión de
   todos los puntos de todos los brazos y runs (+ envolventes exactas
   donde existen). Normalización por instancia (rangos de la unión).
2. **HV ratio y ε+ por run** contra la referencia → media por instancia →
   Wilcoxon pareado LADDER-vs-ABC-P y LADDER-vs-MA-P por grupo y global.
3. **Validación exacta**: distancia al frente CP-SAT en las 6 sondeadas.
4. **Extremo**: mejor Cmax del frente LADDER vs los brazos libres (el
   ancla interna a 400-600 s vs LexME-900s se comenta, no se compara).
5. Tamaño de frente y cobertura (spans) como descriptivos.

## Parámetros

Heredados del paper ASOC (población 250, JOX 0.9, insertion 0.1, TS
tuneada) en los tres brazos; knobs de escalera = tabla de arriba (fijados
por pilotos, no tuneados). **Sin irace por brazo** — declarado; queda como
extensión si los revisores lo piden.

## Teoría que lo respalda (N2ME_theory.tex §ladder)

Prop goal-form + Prop ε-constraint (solidez en Σ_κ + completitud) +
Thm conectividad bajo cap + análisis de paisaje (sonda 1-move) para la
necesidad de niveles evolutivos. Remark de solidez global vía el filtro
de dominancia del archivo (pendiente revisión Hernán).

## Pendiente de implementación

- `run_exp4_ladder.sh` (pipeline por (instancia, run), reanudable) ✔ junto
  a este doc; `run_exp4_baselines.sh` ✔; `exp4_metrics.py` (referencia
  unión + HV/ε+ por run + Wilcoxon) — siguiente sesión.
