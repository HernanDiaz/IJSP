# Fase 0 — Notas de estudio: IJSP verde multiobjetivo

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

## Siguiente (Fase 1)

1. Esquema definitivo de potencias Pp para las 82 instancias (leer el detalle
   experimental de #12/#02; provisional U{2..8} seed 23).
2. `EvaluationIJSP_Energy` en C++ (NPE intervalo por huecos, componente a
   componente) + semántica de timing: ¿schedule semi-activo + right-shift
   post-hoc (à la #17), o timing libre en la representación?
3. Variante lexicográfica (Cmax→NPE) con `FitnessLexicographic` (casi gratis)
   como primer experimento; luego maquinaria Pareto (dominancia + archivo +
   NSGA-II-lite) — ver plan en la conversación 2026-07-04.
