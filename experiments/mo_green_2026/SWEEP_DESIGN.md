# Diseño: barrido ε memético ("SweepPareto")

Generador de frentes makespan/energía por descomposición en problemas
mono-objetivo. Cada elemento del diseño responde a un principio medido
(P1-P4, ver README y análisis 2026-07-07).

## Arquitectura

```
Fase A (ancla)                    Fase B (barrido)
ABCPSO + TS-N2 lexicográfico  →   para ε ∈ {ε1 < ε2 < ... < εK}:
(= brazo LexME del Exp. 1)          cap ← (1+ε)·C*A
obtiene C*A y cosecha meseta        TS con fitness "clamped-LEX2" (abajo)
                                    seed = punto del ε anterior (warm start)
                                    archivar mejor punto
después: RS post-hoc sobre TODOS los puntos del archivo → _Front.csv
```

- **Fase B sin población**: trayectoria de TS repetida sobre el incumbente
  (barata, sin coste de diversidad — P4). Opcional v2: mini-pool por ε.
- El punto de ε_k siembra ε_{k+1}: la búsqueda *nace* al otro lado del
  valle de makespan en vez de tener que cruzarlo (P3).

## La pieza nueva: fitness "clamped-LEX2" (`ijsp.energy-goal`)

Evaluación con parámetro `energy.goal-cmax = <cap>` (lo fija el driver por
ε-run) que produce un FitnessLexicographic de 2 componentes:

1. **Cmax "clampeado"**: `max(Cmax, cap)` por componente del intervalo →
   todas las soluciones bajo el cap son IGUALES en el primario y el
   desempate pasa al NPE; sobre el cap, manda la violación (LEX2 normal).
2. **NPE** (el objetivo real de la fase).

Ventajas decisivas:
- **Reutiliza la TS N2ME sin tocarla**: el vecindario lexicográfico-
  consciente ya existe; con el primario clampeado, sus arcos de energía
  son ciudadanos de primera (no desplazan nada: el makespan no se
  optimiza, solo se respeta). Segunda vida de N2ME y del Caso C.
- **El prune heredado se convierte en el prune correcto**: la poda por
  cota inferior de makespan del evaluateNeighbour compara contra el
  primario actual (= cap cuando se está al límite) → descarta justo los
  vecinos infactibles respecto al cap.
- Cero clases de Fitness nuevas; una Evaluation nueva (~80 líneas, patrón
  MakespanEnergy).

## Driver (`SweepPareto`, subclase de EvolutiveAlgorithm o utilidad)

```
run():
  A: delegar en ABCPSO (budget_A) con ijsp.makespan-energy → ancla σ*, C*A
  archive.offer(σ*)
  σ ← σ*
  B: for ε in escalera:
       fijar cap en la evaluación clamped
       repetir TS(N2ME, fitness clamped) sobre σ hasta estancar o budget_ε
       archive.offer(mejor σ_ε);  σ ← σ_ε
  RS heurístico sobre cada entrada del archivo (re-timing por punto)
  dump _Front.csv
```

- **Escalera**: la del Exp. 2 (permiles 0,5,10,15,20,30,40,50,60,80,100)
  — comparabilidad directa con los frentes exactos.
- **Presupuestos** (P4, adaptativo por tamaño): budget_A = 60-70% del
  total; budget_ε = resto/K. En grandes K menor.

## Protocolo de validación (piloto, antes de comprometer nada)

Mismas 8 instancias y presupuesto TOTAL que los pilotos v1/v2 (900 s):
- vs frentes exactos (HV ratio, ε+) en ft10/tai15 — objetivo: HV > 0.66
  (mejor marca actual, ABC+N2).
- vs pilotos v1/v2 en todas: nº puntos, spans, dominancia punto a punto.
- Chequeo del extremo: el punto ε=0 debe igualar al LexME de referencia
  (si no, la fase A está mal presupuestada).

## Teoría pendiente (para el paper)

Re-enunciar la conectividad de la fase B: "bajo cap de Cmax, los arcos
gap-críticos + delay moves (Caso C) conectan con el óptimo de energía del
nivel". El clamping convierte el Caso C de hueco teórico en operador
natural — posible resolución elegante de la Open Question de la nota.

## Esfuerzo estimado

| Pieza | Tamaño |
|---|---|
| EvaluationIJSP_EnergyGoal (clamped-LEX2) | ~80 líneas, patrón conocido |
| Driver SweepPareto + registro | ~150 líneas |
| Piloto + análisis | scripts existentes (pilot_report, front_metrics) |
| Total | 1-2 sesiones |

## Decisiones abiertas (Hernán)

1. ¿Fase B con trayectoria pura (propuesto) o mini-pool por ε?
2. ¿N2ME completo, restringido, o N2+arcos-gap en la fase B? (bajo cap el
   coste del completo podría ser asumible: propongo empezar restringido)
3. Escalera fija vs adaptativa (más puntos donde el frente curva).
4. Si el piloto bate a v1/v2: ¿sustituye al brazo Pareto del Exp. 4 o se
   compara contra ABC-Pareto como tercer brazo?
