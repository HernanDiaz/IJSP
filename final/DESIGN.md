# Experimento final — Siembra de metaheurísticas IJSP con pools RL/heurísticos

Diseño cerrado el 2026-07-22 a partir de los hallazgos del piloto (ver
`pilot/` y memoria del proyecto). Objetivo: publicación Q1.

## 1. Claims a demostrar

1. **Velocidad (principal):** la siembra reduce el time-to-target de forma
   general, creciendo con el tamaño de instancia. En el SOTA (TSN2), dentro de
   presupuestos operativos la siembra domina la región anytime.
2. **Calidad final (condicional):** mejora significativa y persistente solo
   cuando la búsqueda propia del solver satura por debajo del nivel del seed
   (ABCE3); satura en solvers con LS fuerte (TSN2/fEABCLS); en un GA plano la
   mejora aparente se revierte a convergencia (artefacto de presupuesto).
3. **Aprendizaje:** la política RL (v2) compite con el mejor heurístico (GP)
   como generador de semillas.

## 2. Instancias — 61

- **60 Taillard**: TA1-TA70 **excluyendo TA11-TA20** (TA11-14 entrenamiento
  RL+GP; TA15-20 desarrollo/tuning RL+GP). Esta exclusión es irrenunciable:
  contamina a los GENERADORES de semillas.
- **+ ft10** (ft10_interval).
- Las 17 instancias del irace del TS + ft10 (tuning del SOLVER TSN2) se
  **incluyen** pero se **marcan**: la contaminación de tuning afecta por igual a
  todos los brazos del mismo solver (se cancela en la comparación pareada).
  **Análisis de sensibilidad**: replicar las conclusiones excluyéndolas
  (nota/apéndice). Lista (de `experiments/irace_hc_crisp/instances.txt` — misma
  selección que irace_hc): tai15_15_{03,08}, tai20_15_{02,07} (ya excluidas),
  tai20_20_{01,04,09}, tai30_15_{02,06}, tai30_20_{03,07,10}, tai50_15_{02,07},
  tai50_20_{03,07,10}, ft10.
- Clases: 15×15(10), 20×20(10), 30×15(10), 30×20(10), 50×15(10), 50×20(10),
  ft10. **No hay pools para 100×20** (los pools cubren TA1-70+ft10): el eje
  tamaño llega a 50×20 (1000 ops) — suficiente para el régimen "search-starved".

## 3. Algoritmos — 4 (setups en `repro/`)

| Algo | Rol en el eje | Setup |
|---|---|---|
| GA | evolutivo plano (revierte) | `setup_ga.txt` |
| ABCE3 | swarm débil (mejora persistente) | `setup_abce3.txt` |
| fEABCLS | memético HC (satura) | `setup_feabcls.txt` |
| TSN2 | SOTA, Tabú-N2-LEX2 (satura, titular anytime) | `setup_tsn2.txt` |

Todos con `postexecution.robustness = no`. ABCE3: gap de replicación ~2pp vs
paper 2022 documentado; irrelevante para comparaciones internas entre brazos.

## 4. Brazos — 7, misma k en la escalera

- **A0** control (aleatorio).
- Escalera al p* del sub-estudio (Fase 0): **MOR@p***, **GT@p***, **GP@p***,
  **v2@p***, **mix@p*** (mix = partes iguales v2/GT/GP hasta k, pool `mix3`
  regenerado con la k que toque).
- **v2@100** (extremo población entera sembrada).
- Pools = los corregidos `pools_test/corrected/int__<inst>_<gen>_repo_pool.csv`
  (generarlos para las 61 instancias con `seed_consistency_test --rewrite`).
  Asignación de bloques por run (circular), sin reordenar ni filtrar.

## 5. Fase 0 — Sub-estudio de fracción (fija p*)

- v2@{10, 25, 50, 75, 100}% (k = 25/62/125/187/250 de 250) + A0.
- 2 solvers representativos: **ABCE3** (calidad) y **TSN2** (velocidad).
- 8 instancias estratificadas (1-2 por clase), disjuntas del argumento
  principal en lo posible; 30 runs.
- **Regla de decisión pre-registrada**: p* = mejor rango medio combinando
  (i) avg E[Cmax] final en ABCE3 y (ii) time-to-target(A0-quality) en TSN2,
  promediado sobre las 8 instancias. Empates → p menor (menos coste de seeds).

## 5-bis. Criterio de parada: decisión y coherencia (2026-07-26)

**Regla única para los cuatro algoritmos** (no hay criterios distintos por
algoritmo; lo que varía es el valor calibrado de la celda):

> presupuesto(celda) = min(1.5 × t_conv(A0), 900 s) de tiempo de CPU,
> idéntico para todos los brazos de la celda.

**Por qué NO se usa `noimprovement=25`** (el criterio de los cuatro papers
publicados): el piloto demostró que penaliza estructuralmente a la siembra —
los brazos sembrados disparan la regla en el suelo de 25 iteraciones porque el
algoritmo no logra mejorar la población inyectada, mientras el control corre
100-150 generaciones. Además, con generaciones fijas insuficientes la ventaja
aparente de la siembra se INVIERTE al ampliar el presupuesto (medido:
-34 -> +67 en GA/TA44). Conclusión: para comparar calidad final el control debe
converger, y para comparar velocidad hace falta un eje temporal común.

**DECISIÓN (usuario, 2026-07-26):** NO se ejecuta el régimen nativo
`noimprovement=25` a escala (61 instancias). Correr un criterio que sabemos
distorsionado solo produciría números indefendibles. La evidencia del piloto
(3 instancias × 30 runs, ya medida) es suficiente para justificar la desviación
en una subsección metodológica. El estudio principal tiene **un solo régimen**.

**Nota de implementación (no afecta al diseño):** el framework acumula en RAM
una entrada de traza por generación de los 30 runs. Con presupuesto por tiempo
el GA hace ~3000 generaciones/run (vs ~105 con noimprovement=25), lo que hacía
crecer cada proceso de 437 MB a ~1 GB y agotaba los 16 GB de la máquina con 14
workers (dos cuelgues). Solución: `evolution.unit = time` + `evolution.span = 5`
(traza muestreada cada 5 s, que además es la granularidad correcta para las
curvas anytime) y, si hiciera falta, partir los 30 runs en trozos con
desplazamiento de semilla (mismas semillas 1..30, procesos independientes).
Ninguna de las dos cosas altera el experimento.

## 6. Fase 1 — Calibración de presupuestos

- Pre-run de **A0, 5 runs** por (algo, clase); mediana del tiempo de
  convergencia con regla de meseta pre-definida: mejora < 0.1% en el último 10%
  de la traza.
- **Presupuesto de la celda = min(1.5 × t_conv(A0), 900 s)**, idéntico para
  todos los brazos de la celda. `timelimit` (tiempo de CPU, monohilo) +
  `generations` alto como respaldo.
- Celdas donde A0 NO converge dentro del cap (esperado: GA y TSN2 en clases
  grandes) se marcan **anytime-only**: en ellas no se afirma calidad final.

## 7. Fase 2 — Ejecución principal

- 61 instancias × 4 algos × 7 brazos × **30 runs** (semillas 1..30 pareadas).
- ≤14 procesos concurrentes, MISMO nivel de concurrencia todo el experimento
  (el timelimit es CPU-time, pero se mantiene por homogeneidad).
- Runners idempotentes (patrón `run_remaining_fixed.sh`), reanudables; sin
  ficheros de robustez/escenarios.
- Estimación: TSN2 ~12.8k runs es el cuello (~2 días); total ~4-6 días de
  reloj por fases (rápidos primero).

## 8. Métricas y análisis

- **Anytime**: curvas mejor-E[Cmax] vs tiempo de CPU (media 30 runs) por celda;
  curvas de **success-rate vs tiempo** (fracción de runs que alcanzan objetivo).
- **Time-to-target, 2 métricas**: (A) a meseta propia (≤1% del final propio);
  (B) a la calidad final de A0 (objetivo común, tol +0.5%) — la justa.
- **RE a presupuestos fijos** {60s, 300s, fin} y a fracciones {10%, 50%} del
  presupuesto de celda.
- **Calidad final** solo en celdas convergidas (flag por celda).
- **Estadística**: Wilcoxon pareado por run + **corrección de Holm** por familia
  de comparaciones + **tamaño de efecto** (Vargha-Delaney Â12). Friedman +
  post-hoc para ranking de brazos por clase.
- **Coste de las semillas**: medir tiempo de generación de k semillas por
  generador e incluirlo como fracción del presupuesto (esperado: despreciable;
  demostrarlo). Coste de entrenamiento del RL declarado y amortizado.

## 9. Reproducibilidad

- Rama dedicada con commit fijado (el tag ASOC + siembra `ijsp.seeded` + gate
  de robustez). Publicar en Zenodo: pools corregidos, setups, scripts de
  análisis, CSVs agregados.
- Documentar: test de consistencia del decodificador (lex-por-upper vs
  componentwise) y corrección de pools; replicación de GA/fEABC validada;
  gap ABCE3 divulgado.

## 10. Orden de ejecución

1. Fase 0 (fracción) → fija p*.
2. Generar pools corregidos + mix para las 61 instancias.
3. Fase 1 (calibración A0) → tabla de presupuestos por celda.
4. Fase 2 por bloques: GA+ABCE3 (baratos) → fEABCLS → TSN2.
5. Análisis + sensibilidad (sin las 17+ft10) + figuras.
