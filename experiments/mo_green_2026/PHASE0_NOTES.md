# Fase 0 — Notas de estudio: IJSP verde multiobjetivo

> **NOTA:** este fichero es el registro detallado de las Fases 0-1
> (2026-07-04). El diario vivo de la línea, con la cronología completa,
> resultados y estado, es [README.md](README.md).

## Modelo de energía (de Afşar 2024 IWINAC #05 / 2025 NatComp #02, FJSP-intervalo)

- Máquina k encendida desde el inicio de su primera tarea hasta el fin de la última:
  **PEₖ = PPₖ · (ct_last_k − st_first_k)** (energía pasiva; PPₖ = potencia pasiva).
- **AEₖ = Σ APᵢⱼₖ · dᵢⱼₖ** (energía activa, intervalo — escala con las duraciones).
- **TE = Σₖ (PEₖ + AEₖ)**. La "idle energy" es la parte de PE sin tarea en ejecución.
- Alternativa (#17 cita [6]): máquinas encendidas de 0 a fin — modelo distinto.
- García-Gómez 2023 (#12, fuzzy FJSP): vecindario para energía **con prueba de
  conectividad** → plantilla para nuestra futura LS de energía. Modelos "stack vs gaps".
- Datos de potencia: sus instancias heredan Pa/Pp de García-Gómez et al. (fuzzy),
  a su vez de Dauzère-Pérès&Paulli; para intervalos descartan el componente modal
  del TFN. Para nuestras 82 IJSP habría que GENERAR PPₖ (esquema exacto pendiente
  de leer en detalle; provisional aquí: PPₖ ~ U{2..8}, semilla 23).

## Observación estructural clave (JSP no-flexible)

**AE es invariante al orden** (cada tarea siempre en la misma máquina con la misma
duración) → el único término que depende del schedule es PE. El objetivo verde
efectivo en JSP es la energía pasiva/idle ponderada por máquina.

## Formatos de instancia (gotcha)

`SelectosYTaillardIntervalos/` mezcla DOS formatos: (A) con cabeceras
(`NUMERO DE TRABAJOS`…, tuplas `(a,b)`) p.ej. ft10, tai*; (B) sin cabeceras
(línea en blanco, njobs, nmach, matrices con tabs, tuplas `(a, b)` con espacio)
p.ej. la29. `conflict_check.py::parse_instance` soporta ambos.

## Sanity check de conflicto (conflict_check.py, 2026-07-04) — ⚠ RIESGO ALTO

Reutilizando los 60 schedules near-óptimos por instancia de n2_worstcase (_Sols,
decodificados semi-activos componente a componente) + 200 aleatorios:

| Instancia | corr global (aleatorios) | corr near-opt | frente near-opt | spread PE near-opt | spread PE entre empates Cmax |
|---|---|---|---|---|---|
| ft10 (10×10) | **+0.80** | +0.71 | 1 punto | 8.6% | 1.3% |
| la29 (20×10) | **+0.94** | +0.52 | 1 punto | 5.5% | 2.9% |
| tai30_20_01 (30×20) | **+0.96** | +0.85 | 4 puntos | 4.6% | — |

**Lectura:** con el modelo de ventana de encendido en JSP NO-flexible, Cmax y PE
están FUERTEMENTE alineados (comprimir makespan comprime ventanas). El frente
Pareto aparente es casi degenerado. Nota técnica: pasar a contabilidad de "gaps"
(PE − trabajo constante) NO cambia la correlación ni la dominancia (desplazamiento
constante), solo los porcentajes relativos (el 1.3–2.9% sobre ventana puede ser
mucho sobre idle puro).

**Sesgo del check:** los 60 schedules provienen de búsquedas que solo optimizan
makespan — no exploran la dirección de energía. #17 (fuzzy JSP no-flexible)
SÍ demostró conflicto con un frente exacto por ε-constraint (su Fig. 1); su
modelo NPE exacto hay que replicarlo con cuidado.

## GATE SUPERADO — sonda exacta ε-constraint (epsilon_probe.py, 2026-07-04)

CP-SAT (OR-Tools), duraciones = midpoints (escala ×2), tiempos de inicio
LIBRES (right-shifts permitidos, como exige la no-regularidad del NPE del
#17), NPE = Σ Pp_k·idle_k (huecos), 60 s/punto:

| Instancia | C* | NPE(C*) | NPE(C*+10%) | caída |
|---|---|---|---|---|
| ft10 (10×10) | 930 (OPT) | 5431 | 2400 | **−55.8%** |
| la29 (20×10) | 1160 (feas) | 5553 (en +4%) | 2480 | **−55.3%** |

**CONFLICTO GENUINO: relajar el makespan un 10% permite reducir la energía
idle a la mitad.** El check semi-activo anterior engañaba porque el timing
semi-activo esclaviza el NPE al makespan; el trade-off vive en (orden +
timing). Consecuencias de diseño:
- La componente de *timing* (right-shifts / inserción de retrasos) es parte
  del espacio de decisión → el memético necesita la heurística de right-shift
  del #17 (o similar) además de la LS de orden.
- la29: CP-SAT ni siquiera encuentra factible en 60 s con Cmax ≤ C*+3% en
  20×10 → la sonda exacta no escala; el frente de referencia en instancias
  medianas/grandes tendrá que ser aproximado (metaheurística), como en #17.

## Fase 1 — infraestructura HECHA (2026-07-04)

1. **Potencias**: #02/#05/#12 no publican distribución (heredan de DPdata-energy)
   → esquema propio documentado: Pp ~ U{2..8}, `random.Random(23)` POR instancia
   (idéntico en conflict_check/epsilon_probe/extend_instances). `extend_instances.py`
   genera `SelectosYTaillardIntervalosEnergia/` (92 copias + sección POTENCIA
   PASIVA; regenerables, gitignored). Pa omitida: AE invariante en JSP.
2. **C++** (compila limpio, smoke OK):
   - `ProblemIJSP`: sección opcional POTENCIA PASIVA (`loadPowers`, tolerante;
     default Pp=1 → NPE = tiempo idle), copiada en copy-ctor y limpiada en clear.
   - `EvaluationIJSP_Energy` (`ijsp.energy`): NPE intervalo por huecos entre
     tareas consecutivas por máquina, componente a componente, sobre el schedule
     semi-activo del SGS. Timing: semi-activo en Fase 1; right-shift → Fase 2 (LS).
   - `EvaluationIJSP_MakespanEnergy` (`ijsp.makespan-energy`): FitnessLexicographic
     [Cmax, NPE], ambos FitnessInterval con el ranking global (LEX2).
   - Frontera LS en `ArtificialBeeColonyPSO::applyLocalSearch`: si el fitness es
     lexicográfico, la TS de makespan recibe la componente 0 y el fitness completo
     se reconstruye del schedule optimizado (evaluateSchedule).
   - Post-ejecución: `castFitness` desenvuelve lexicográfico (componente primaria);
     registro `ijsp.makespan-energy` → analizador de makespan.
3. **Smoke VERIFICADO** (ft10, runs=1, tl=10, N2Plus): makespan 940 (= referencia
   N2Plus), NPE=(7528,8241); `validate_npe.py` recomputa en Python y da **MATCH
   exacto**. (Coherencia con sonda: NPE semi-activo 7885 mid > óptimo free-timing
   4467 en Cmax≈940, como cabe esperar.)

## Siguiente (Fase 2)

- Experimento lexicográfico a escala (Cmax→NPE vs makespan-only en las 82;
  medir cuánta energía recupera "gratis" el desempate lexicográfico).
- Right-shift / optimización de timing como operador (la sonda demuestra que
  ahí vive la mitad del NPE) — plantilla: heurística HER del #17 + LP post.
- Maquinaria Pareto (dominancia intervalo-consciente + archivo + NSGA-II-lite).
