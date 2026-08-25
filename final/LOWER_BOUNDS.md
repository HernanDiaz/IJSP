# Referencia para el RPD: cotas inferiores publicadas

## Problema que resuelve

Hasta ahora el RPD se calculaba contra `final/refs.csv` (`best_known`), extraído de
`Papers/COR_Tabu/data/phaseA/all_results_full.csv`. Eso es **el mejor resultado del propio
grupo**, no una cota inferior: fEABCLS lo bate en varias instancias, así que los RPD salían
artificialmente pequeños y un revisor de Q1 lo detectaría. Sustituido por cotas inferiores
publicadas e independientes.

## Fuente

Coupvent des Graviers, M.-E.; Kobrosly, L.; Guettier, C.; Cazenave, T.
*Updating Lower and Upper Bounds for the Job-Shop Scheduling Problem Test Instances*.
arXiv:2504.16106 [cs.DS], 17 abril 2025 — Tabla 14 (bounds consolidados del conjunto de
Taillard, ta01–ta80; LB y UB, `*` = óptimo probado).

Valores volcados en `final/ta_lb.csv` (columnas `inst,ta,lb,ub`). `ft10` (Fisher & Thompson
10×10) usa su óptimo probado clásico, 930.

Nota: existen LB más fuertes para instancias sueltas (p. ej. la página de JSPLib/OptalCP da
ta22 = 1600 cerrada y ta44 LB = 1961, frente a 1581 y 1952 del artículo). Usamos las del
artículo por ser una fuente única, completa y citable. **Es además la dirección conservadora**:
una LB menor produce un RPD *mayor*, luego no infla nuestros resultados.

## Validación 1 — el mapeo `tai{n}_{m}_{k}` → `ta##` no se supone, se comprueba

`final/check_ta_mapping.sh` calcula desde las instancias *crisp* del repo la cota trivial
`LBtriv = max(max_j Σ_k p_jk , max_m Σ p en m)` y la contrasta con la LB publicada:

- 60/60 coherentes (`LBtriv ≤ LB_pub`; lo contrario haría el mapeo imposible),
- **19 con identificación exacta** (`LBtriv == LB_pub` al dígito), concentradas en ta31–ta40
  y ta51–ta70, donde la LB publicada *es* la cota de máquina,
- 0 incompatibilidades.

Una permutación dentro de una clase de tamaño rompería las coincidencias exactas, así que el
orden dentro de cada clase también queda fijado. `ft10`: cota de trabajo 655, la conocida de
Fisher & Thompson 10×10.

## Validación 2 — por qué una LB *crisp* acota el makespan esperado con intervalos

`final/check_midpoint.sh` comprueba sobre **57.450 operaciones (82 instancias)**: 0 casos con
punto medio distinto de la duración crisp, 0 intervalos asimétricos, 0 rutas discrepantes. Es
decir `p_ij = [c_ij − δ_ij , c_ij + δ_ij]`, con `mid(p_ij) = c_ij` exacto.

De ahí, con el máximo *componentwise* que usa el repo:

- `mid(A + B) = mid(A) + mid(B)`
- `mid(max(A,B)) = (max(a⁻,b⁻) + max(a⁺,b⁺))/2 ≥ max(mid A, mid B)`

El makespan se evalúa componiendo solo sumas y máximos sobre el DAG de la planificación, luego
por inducción `mid(Cmax_int(x)) ≥ Cmax_crisp(x)` **para toda planificación x**. Tomando mínimos,

    E[Cmax]*_intervalos  ≥  Cmax*_crisp  ≥  LB_publicada

La LB crisp es por tanto una cota inferior válida (no ajustada, pero válida) de `E[Cmax]`.
Esto es demostrable en el paper en tres líneas y no depende de resultados propios.

## Efecto sobre los números

La referencia antigua estaba, en mediana, **+3,05 %** por encima de la cota inferior (rango
0 % – 12,52 %). Ninguna quedaba por debajo de la LB, así que no había inconsistencia, pero sí
una holgura que rebajaba los RPD.

RPD medio (%) sobre la cota inferior publicada, 61 instancias, 30 ejecuciones
(`final/rpd_vs_lb.R`):

| algoritmo | A0 (control) | V2H | V2 | MOR | GT | GP | MIX |
|---|---|---|---|---|---|---|---|
| GA      | **7,52**  | 7,68 | 8,07 | 9,69  | 8,47  | 7,99 | 7,58 |
| ABCE3   | 10,52 | 9,52 | 9,40 | 10,73 | 10,22 | 9,69 | **9,39** |
| fEABCLS | 3,61  | **3,56** | 3,60 | 5,08  | 4,43  | 3,96 | 3,59 |
| TSN2    | 3,51  | 3,48 | 3,57 | 4,18  | 3,95  | 3,62 | **3,35** |

Experimento completo: 61 instancias × 7 brazos × 30 ejecuciones × 4 algoritmos, presupuesto
fijo por tamaño. Los RPD absolutos de todo el paper se calculan con `final/ta_lb.csv`;
`final/refs.csv` queda solo como contexto histórico.

## Incidencia de procedencia (resuelta, 2026-08-09)

Las corridas MONOLÍTICAS abortadas del 26-jul dejaron `<arm>/*_Sols.csv`, y
`extract_phase2c.sh` los prefería sobre los `c*/`. Alcance auditado
(`final/audit_provenance.sh`): ABCE3 tiene 427 monolíticos **completos de 30 runs y sin
chunks** (la corrida sí terminó, no hay conflicto); fEABCLS y TSN2 son puramente chunked;
del GA, 315 brazos monolíticos completos y **solo 14 celdas contaminadas** — las 7 ramas de
`tai50_15_05` y `tai50_15_06`, servidas con 21 runs viejos teniendo los 30 buenos al lado.

Corregido dando prioridad a los chunks en el extractor. Efecto real: GA A0 7,53 → 7,52 y
cambios equivalentes en el resto de brazos; en Wilcoxon solo se movió V2 (8/25/28 → 8/26/27).
Ninguna conclusión cambia, pero los números publicados son los de esta tabla.
