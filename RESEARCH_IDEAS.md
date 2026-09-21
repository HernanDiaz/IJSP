# Bucle de investigación

Protocolo, referencia vigente, historial y backlog de este proyecto. Lo
ejecuta el skill `research-iteration`, cuyas reglas generales quedan
subordinadas a lo que diga este fichero.

Objetivo de la línea: mejorar una mejor solución conocida (BKS) del
benchmark de Taillard, JSP crisp, con el ABCPSO + búsqueda tabú sobre N2 de
este repositorio. Todo lo que hay detrás está en
`experiments/classic_jsp_2026/JOURNAL.md`; este fichero solo fija el
protocolo y lleva la cuenta.

## Protocolo

**Evaluación completa**

```
experiments/classic_jsp_2026/scripts/queue_jobs.sh <jobs.tsv>
```

sobre las **21 instancias abiertas con banco de semillas** (las 22 de
`taillard_bounds.csv` menos `ta18`, 20x15, para la que no hay pools):
`ta22 ta23 ta25 ta26 ta27 ta29 ta30 ta32 ta33 ta34 ta40 ta41-ta50`.
Tarda unas 6-7 horas en 14 núcleos para 5 celdas x 30 runs.

**Filtro barato**

Mismo comando sobre las instancias del filtro, **fijadas de antemano y para
siempre**: `ta29 ta30 ta23` (20x20, las tres más cerca del récord) y `ta45`
(30x20, para no ajustarse a un solo tamaño). Unos 50 minutos en 14 núcleos
para 5 celdas x 30 runs. **El filtro solo descarta, nunca acepta.**

**Semillas**: 30 runs por celda e instancia, `seed = 1`, `runs = 30`, las
mismas en todas las comparaciones. Con el algoritmo estocástico, una tirada
no es una medida.

**Presupuesto por run, igualado en tiempo de reloj**, y **por clase de
tamaño**, porque la búsqueda muere a tiempos distintos (JOURNAL,
2026-09-21, "última mejora"): 20x20 -> 40 s, 30x15 -> 100 s, 30x20 -> 150 s.
Este es el régimen de **tiradas cortas**; el de 300 s por tirada queda como
ancla de deriva (abajo). Un run = un proceso del solver con `runs = 30`.

**Métrica**: makespan del mejor horario de cada run, **recomputado desde el
propio horario** (`verify_certificate.py` contra los datos de OR-Library),
nunca el número que reporta el solver. Menor es mejor. Se agregan las 30
runs por instancia (media) y se compara **pareado por instancia**.

**Referencia contra la que se mide**: el **BKS** de cada instancia
(`taillard_bounds.csv`). Un valor por debajo es un récord: se anota como tal
en el historial y la prueba es el propio horario. Para aceptar o descartar
una idea, la referencia operativa es la **celda de control** de la misma
tanda (abajo).

**Criterio de aceptación**: contraste de Wilcoxon de rangos con signo sobre
la media por instancia de la celda propuesta contra la de control, pareado
por las 21 instancias, dos colas, **p <= 0.05 y la celda propuesta con menor
rango medio**. Siempre sobre la evaluación completa y nunca sobre el filtro.
Un p significativo dice que difieren, no en qué dirección: la dirección la
dan los rangos. Los endpoints secundarios se reportan y no deciden.

**Intocable desde el bucle**: `scripts/verify_certificate.py`,
`taillard_bounds.csv`, `reference/taillard_orlib.txt`, las instancias
`TaillardJSP/`, el banco de semillas de origen (solo lectura, ver abajo), la
definición de la métrica y los presupuestos por clase.

**Parámetros del algoritmo**: **congelados** en la configuración 136 que
irace afinó para el ABC (`setup/prereg2_abc_300s.txt`; JOURNAL,
2026-09-20). Decisión del PI (2026-09-21): reajustar en cada iteración
haría cada bucle demasiado largo. Las ideas que introduzcan un parámetro
nuevo o cambien el papel de uno existente se marcan `[REAJUSTA]`; se
prueban igualmente con los parámetros congelados, y la etiqueta obliga a
reajustar antes de darlas por confirmadas.

**Ejecución**: un solo experimento en la máquina a la vez, nunca dos
apilados (regla del PI). `queue_jobs.sh` corre todas las celdas de una tanda
por una única cola, alternadas, como máximo un solver por núcleo, y se niega
a arrancar si detecta otro solver o un irace vivo. Las celdas de una misma
tanda quedan repartidas por toda la ventana temporal, así que la deriva de
la máquina (~9 % entre lotes) les afecta por igual.

**Control interno de cada tanda**: toda tanda lleva una celda `control`
(`creation = jsp.random`, el resto igual) corrida junto a las demás. Es la
referencia contra la que se aplica el criterio de aceptación, porque el
presupuesto corto es nuevo y una referencia de otro presupuesto no mide
nada.

## Referencia actual

- Configuración vigente: commit `6544637` + el porte de `jsp.seeded`
  (`CreationJSP_Seeded.*`), ABC configuración 136, `creation = jsp.random`.
- Registro de la evaluación completa **a 300 s** (ancla de deriva):
  `experiments/classic_jsp_2026/results/prereg2_abc/`, 22 instancias x 10
  runs. Gap medio del mejor al LB: **4.361 %**; mejor verificado por
  instancia en `results/prereg2_comparison.txt`.
- Referencia **original** del bucle, para medir la deriva: la misma tanda
  `prereg2_abc` (2026-09-20 20:01 - 2026-09-21 00:36). Cada 3 aceptaciones,
  volver a correr la configuración vigente contra ella a 300 s.
- Referencia en el **régimen de tiradas cortas**: la celda `control` de la
  primera tanda completa (I-001); se rellena al cerrar I-001.
- Registro del filtro: pendiente (I-001).

## Banco de semillas (solo lectura)

`C:\Users\diazhernan\CLionProjects\RL Seeds\IJSP\pools_test\corrected\`
(`/mnt/c/...` desde WSL). Pools de 1024 permutaciones de trabajos (1-based,
`j1 ... jN;[lo, up]`) por instancia con intervalos `tai{n}_{m}_{k}`, mapeo a
`ta` en `final/ta_lb.csv` de la rama `seeding-study`. Enrutado verificado
idéntico al crisp (`tai20_20_09.F.15_01` = `ta29`, 2026-09-21); el punto
medio de cada intervalo es la duración crisp. Generadores: `v2` (política RL
profunda), `gp` (regla GP), `gtmwkr` (Giffler-Thompson + eps), `graspmor`
(MOR + eps), `mix` (mezcla). La política `v2` se entrenó en TA11-14 y se
desarrolló en TA15-20: ninguna de las 21 instancias objetivo. Los pools
derivados que usa cada iteración se generan con `scripts/prepare_pools.py`
en `experiments/classic_jsp_2026/seeds/` (fuera de git; md5 en el
historial).

## Historial

Una entrada por idea, aceptada o no. Las tres primeras son anteriores al
bucle y están aquí para que no se repitan; sus cifras están en el JOURNAL.

| id | idea | hipótesis | filtro | confirmación | decisión |
|---|---|---|---|---|---|
| H-1 | back-jump (Nowicki-Smutnicki) en el tabú | vuelve mejor a los buenos puntos | -- | 22 abiertas x 5 x 300 s: 10 de 22, p = 1.000; en las 12 no vistas, tabú simple mejor, p = 0.022 | **descartada** (2026-09-19) |
| H-2 | pool de élite + path relinking (IPRTS, rama `path-relinking`, IJSP) | recombinar casi-óptimos sale de la meseta | -- | x20: 0 mejoras en ~500 llamadas a PR; meseta neutra en calidad | **descartada** (2026-06-21) |
| H-3 | memético con su configuración afinada vs ABC con la suya | el memético alcanza mejores makespans | -- | 22 x 10 x 300 s: ABC mejor en 18 de 22, W = 30, p = 0.002 | **descartada** (2026-09-21) |
| I-001 | sembrar la población inicial en tiradas cortas desde el banco; composición vs calidad | ver abajo | pendiente | pendiente | **en curso** (2026-09-21) |

### I-001 — siembra en tiradas cortas: composición contra calidad

**Hipótesis** (una frase): en el régimen de tiradas cortas, sembrar la
población inicial del ABC con `k = 25` de 247 individuos tomados del banco
mejora el makespan final respecto al arranque aleatorio, y **lo que importa
es la composición del pool, no la calidad de las semillas**.

De dónde sale: (i) el artículo de `seeding-study` (IJSP, 58.560 runs) mide
que sembrar vale 0.1-1.0 % a presupuesto completo y **1.3-5.2 % a un décimo
del presupuesto**, y que el pool *mezclado* arranca peor y acaba mejor que
sus componentes; (ii) el JOURNAL de 2026-09-21 mide que en 20x20 la búsqueda
termina a los 36 s de 300 y que reiniciar gana en 21 de 22 instancias. Los
dos resultados apuntan al mismo régimen. Idea del PI: optimizar las
semillas contra el resultado final, no contra su propio makespan; esta
iteración es la medición de si hay señal y de qué propiedad la lleva.

**Qué se toca**: solo la creación de la población inicial
(`creation = jsp.seeded`, `creation.seed.pool`, `creation.seed.count = 25`),
portada de `seeding-study`. Parámetros del ABC congelados. Presupuesto por
clase, `runs = 30`.

**Celdas** (misma k = 25 en todas las sembradas):

| celda | pool | qué aísla |
|---|---|---|
| `control` | ninguno (`jsp.random`) | referencia del régimen corto |
| `v2rand` | `v2` tal cual, 1024 líneas en su orden | un generador, sin selección |
| `v2top` | las 250 mejores de `v2` por E[Cmax] | **calidad** (mismo generador) |
| `v2maxmin` | 250 de `v2` por máx-mín Hamming | **dispersión** (mismo generador) |
| `mix` | `mix` tal cual, 1024 | **heterogeneidad** (varios generadores) |

**Endpoint primario, fijado antes de correr**: `mix` contra `control`,
media por instancia de las 30 runs, Wilcoxon pareado por las 21 instancias,
p <= 0.05 y `mix` con menor rango. Es la celda que el artículo señala.
**Secundarios, reportados y no decisorios**: `v2top`, `v2rand`, `v2maxmin`
contra `control` y entre sí (calidad vs dispersión), el mejor-de-7 por
bloques de runs consecutivos (lo que ve un intento de récord), y el
makespan del paso 0 por celda (para comprobar que la siembra entró).

**Regla del filtro**: se descarta si en las 4 instancias del filtro la media
de `mix` menos la de `control`, promediada, es mayor que +2 unidades de
makespan (peor). Si no, va a la confirmación completa, que **repite** las 4
instancias del filtro en vez de reutilizarlas.

**Si se acepta**: `creation = jsp.seeded` con el pool ganador pasa a la
configuración vigente del régimen corto, y la siguiente idea del backlog es
optimizar la receta de composición contra el resultado final (B-2).

## Backlog

- **B-1** Confirmar el reinicio con tiradas reales: `k x L` por clase contra
  1 x 300 s, mismas instancias. La celda `control` de I-001 da la mitad; la
  otra mitad ya existe (`prereg2_abc`). Solo hace falta el análisis.
- **B-2** `[REAJUSTA]` Optimizar la receta de composición del pool (cuantiles
  de calidad, umbral de distancia, fracción, mezcla de generadores) contra
  el makespan final a presupuesto corto, en instancias generadas; evaluar en
  Taillard. Con irace o CMA-ES por encima. Depende de que I-001 muestre
  señal.
- **B-3** `[REAJUSTA]` Entrenar el generador (política RL de `v2`) con
  recompensa = resultado del ABC, contra un sustituto ajustado en B-2. Solo si
  B-2 muestra que la palanca es grande.
- **B-4** Primera mejora con orden aleatorio en el tabú, en vez de
  mejor-de-todos. Diff pequeño, sin parámetros nuevos; es la única forma en
  que "ordenar los vecinos" puede cambiar algo con el bucle actual
  (`LocalSearch.cpp:213-238` evalúa todos y coge el mejor no tabú).
- **B-5** `[REAJUSTA]` Política aprendida de selección de movimiento en el
  tabú (RL / hiperheurística). Solo si B-4 muestra que la regla de selección
  importa.
- **B-6** Intentos de récord: muchas tiradas cortas con la configuración
  vigente sobre la lista corta `ta29 (0) ta30 (4) ta22 (13) ta23 (14) ta27
  (16) ta26 (17) ta45 (19)`; afinar sobre las propias instancias es legítimo
  ahí y se declara.
