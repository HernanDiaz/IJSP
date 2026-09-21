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
Se corre **por oleadas de media hora**, no de una sola vez: ver "Cadencia"
más abajo. De una sola vez, 5 celdas x 30 runs son 90.8 h-CPU, unas 6.5 h en
14 núcleos, y eso es más de lo que un paso del bucle puede durar.

**Filtro barato**

Mismo comando sobre las instancias del filtro, **fijadas de antemano y para
siempre**: `ta29 ta30 ta23` (20x20, las tres más cerca del récord) y `ta45`
(30x20, para no ajustarse a un solo tamaño). Con 2 celdas x 30 runs, 4.5
h-CPU, unos 19 minutos en 14 núcleos; las variantes de una misma idea se
separan aquí, que es donde son baratas. **El filtro solo descarta, nunca
acepta.**

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
rango medio**. Siempre sobre la evaluación completa y nunca sobre el filtro. Corrida por
oleadas, que es lo normal, el umbral por mirilla es **0.0142** y no 0.05
(ver "Cadencia").
Un p significativo dice que difieren, no en qué dirección: la dirección la
dan los rangos. Los endpoints secundarios se reportan y no deciden.

**Cadencia: oleadas de media hora.** Decisión del PI (2026-09-21): cada paso
del bucle tiene que caber en media hora. La máquina tiene 14 hilos, así que
el reloj es el coste en horas-CPU partido por 14, y media hora son **7
h-CPU**. Un run sobre cada una de las 21 instancias, con los presupuestos
por clase, cuesta 0.61 h-CPU. La clase 30x20 (10 instancias a 150 s) es el
69 % del coste de una tanda completa. De ahí tres reglas:

1. **Dos celdas, no cinco.** Una tanda de confirmación lleva `control` y la
   celda del endpoint predeclarado, nada más. Las variantes de una misma
   idea (calidad contra dispersión contra mezcla, un parámetro en tres
   valores) se separan en el filtro. De 90.8 a 36.3 h-CPU sin tocar la
   decisión, porque el criterio de aceptación solo mira esas dos celdas.
2. **La confirmación se parte en 6 oleadas de 5 runs** sobre las 21
   instancias: 6.1 h-CPU, unos 26 minutos de reloj cada una. La oleada `w`
   corre con `runs = 5`, `seed = 1 + 5*(w-1)` y, si la idea siembra,
   `creation.seed.offset = 5*(w-1)`. Como el run `r` de un proceso usa
   `seed + r` (`EvoLauncher.cpp:95`), las seis oleadas son **exactamente**
   los mismos 30 runs que una tanda monolítica con `seed = 1`: partirla no
   cambia el experimento, solo cuándo se puede mirar. Cada oleada escribe en
   su propio tag `results/<id>_full_<celda>_w<w>` (el guardia de reanudación
   de `queue_jobs.sh` cuenta runs por directorio, así que dos oleadas no
   pueden compartirlo) y el análisis suma los directorios `_w*`.
3. **El filtro se queda con 30 runs.** Con 10, el error típico de la media
   de las cuatro instancias del filtro sube a 1.8 unidades y la regla
   "descarta si la diferencia supera +2" empezaría a descartar ideas
   neutras. Lo que se recorta en el filtro son las celdas, no los runs.

**Mirillas intermedias y su precio.** Analizar al final de cada oleada es un
contraste repetido sobre los mismos datos, y eso infla el error de tipo I.
Con 6 mirillas igualmente espaciadas, el umbral por mirilla que mantiene el
global en 0.05 es **p <= 0.0142** (frontera constante de Pocock, 1977), y
ese es el umbral que se aplica en las seis, la última incluida. El número de
oleadas (6) y el umbral quedan fijados de antemano como todo lo demás: si a
la sexta no se declara, la idea se descarta, y mirar una séptima no es una
opción.

Lo que cuesta y lo que da, simulado con la dispersión real (sd entre runs
~ 8 unidades de makespan, medida en el filtro de I-001; 1500 tiradas,
`experiments/classic_jsp_2026/scripts/wave_power.py`), como porcentaje acumulado de tandas que
declaran al cerrar cada oleada:

| efecto real | ol. 1 | ol. 2 | ol. 3 | ol. 4 | ol. 5 | ol. 6 |
|-------------|-------|-------|-------|-------|-------|-------|
| ninguno     |   0.5 |   1.1 |   1.5 |   1.9 |   2.1 |   2.3 |
| -1.5        |   8.3 |  17.2 |  26.6 |  34.8 |  39.4 |  44.9 |
| -3.0        |  38.5 |  69.5 |  84.8 |  92.2 |  95.1 |  97.2 |
| -6.0        |  97.6 | 100.0 | 100.0 | 100.0 | 100.0 | 100.0 |

Es decir: el diseño por oleadas no es una rebaja. Mantiene el error de tipo
I por debajo del 5 % (sale 2.3 %, conservador), conserva la potencia de los
30 runs para el efecto que perseguimos (97 % a la sexta, contra 99 % de la
tanda monolítica a 0.05) y, sobre todo, **la mayoría de las decisiones caen
en la primera hora**: un efecto de -3 se declara en la primera o la segunda
oleada el 70 % de las veces, y uno del doble en la primera. Las seis oleadas
completas son el caso peor, no el normal, y son 2.6 h en vez de 6.5.

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
- I-001 se cierra **como se preinscribió**: una sola tanda de 5 celdas x 30
  runs, lanzada a las 05:22 del 2026-09-21 y ya por encima de dos tercios
  cuando se escribió esta sección. Cortarla para relanzarla por oleadas
  tiraría cuatro horas y media de cómputo y, sobre todo, cambiaría el diseño
  después de ver el filtro. La cadencia por oleadas rige desde I-002.
- Registro del filtro: `experiments/classic_jsp_2026/iter/I-001/filter_analysis.txt`
  (2026-09-21, 03:13-~04:05). Medias de 30 runs, control / mix: ta23 1587.8 / 1582.1,
  ta29 1641.1 / 1640.0, ta30 1623.8 / 1619.7, ta45 2039.4 / 2039.4. Las cuatro
  celdas sembradas quedan entre −2.4 y −3.6 de media respecto al control; con
  n = 4 ningún contraste separa nada (p ≥ 0.25) y no tiene que hacerlo: el filtro
  solo descarta.

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
| I-001 | sembrar la población inicial en tiradas cortas desde el banco; composición vs calidad | ver abajo | mix−control = −2.75 en 4 inst. (regla: > +2 descarta) → **pasa** | pendiente (lanzada 2026-09-21 05:2x, ~6.5 h) | **en curso** |

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
