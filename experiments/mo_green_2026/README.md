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
| Exp. 2 | Frentes exactos ampliados (6 instancias × 11 ε) + distancia-al-frente | 🟡 lanzado | `7134358` |
| Exp. 3 | N2ME en C++; LexME-N2ME vs LexME-N2 | ⬜ implementando | — |
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

## Pendiente

- **Exp. 2 (corriendo):** al acabar → análisis distancia-al-frente (N2 /
  LexME / LexME+RS vs frente exacto en ε=0) + figura del frente con el punto
  lexicográfico → §6.4 y Fig. del paper. Cuantifica el margen de N2ME.
- **Exp. 3:** N2ME C++ (con teorema débil basta; def. en N2ME_theory.tex).
- **Exp. 4:** Pareto (dominancia sobre producto de órdenes LEX2 + archivo +
  NDS/crowding + intensificador por objetivo).
- **Teoría:** Caso C de conectividad (arranques de máquina) — abierto.
- **Paper:** TODOs marcados en main.tex (coautores, refs TODO-VERIFY,
  agradecimientos); decidir irace por brazo para el experimento final.
