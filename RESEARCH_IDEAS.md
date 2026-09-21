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
3. **El filtro se queda con 30 runs**, y también troceados. Con 10 runs, el
   error típico de la media de las cuatro instancias sube a 1.8 unidades y la
   regla "descarta si la diferencia supera +2" empezaría a descartar ideas
   neutras. Lo que se recorta en el filtro son las celdas, no los runs. Los
   30 runs se parten en 6 trozos de 5 (`fc1..fc6`, semillas 1, 6, ..., 26),
   por la misma razón que la confirmación va por oleadas: ver el aviso de
   abajo.

**Aviso, y corrección de esta sección (2026-09-21, medido)**: el reloj de una
tanda **no** es su coste en horas-CPU partido por 14. Eso solo vale cuando
hay más trabajos que huecos. El suelo real es **la duración del trabajo más
largo**, porque un trabajo es un proceso de un solo hilo. El filtro de I-002,
8 trabajos en 14 huecos, se anunció aquí como 19 minutos y tardó **82**: sus
30 runs de `ta45` a 150 s son 75 minutos de un solo proceso y los otros 13
núcleos miraban. La regla correcta, que es la que ya cumplen las oleadas por
casualidad, es **trocear los runs hasta que ningún trabajo pase de unos 13
minutos**: 5 runs por trabajo. Con eso el filtro son 48 trabajos, 4.5 h-CPU y
unos 20 minutos de reloj de verdad. Una oleada son 42 trabajos, ninguno de
más de 750 s, y sus 26 minutos sí eran correctos.

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

**Antes de preinscribir una idea, buscarla en el repositorio.** Regla nueva
(2026-09-21), a raíz de I-002: el historial de abajo solo tenía los tres
negativos que el PI me nombró, y con eso preinscribí una repetición de un
experimento que estaba **en este mismo árbol**,
`experiments/cor_tabu_2026/`, el paquete de la fase B del paper de COR. No
basta con leer el código para saber si algo se ha probado; hay que mirar
todos los paquetes de `experiments/`, los tags (`cor-tabu-2026-submission`,
`ijsp-neigh-ranking-v1`) y las ramas, y preguntar al PI. El historial queda
sembrado con H-4 para que N8 no vuelva a proponerse.

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
- Referencia en el **régimen de tiradas cortas**: la celda `control` de
  I-001, `results/I-001_full_control/`, 21 instancias x 30 runs a 40/100/150 s
  (2026-09-21, 05:22-14:23). Media de la distancia del mejor de sus 30 runs
  al BKS: **37.4 unidades**. Por instancia (mejor verificado, distancia al
  BKS): ta22 1619 (19), ta23 1574 (17), ta25 1620 (25), ta26 1660 (17),
  ta27 1703 (23), ta29 1628 (3), ta30 1607 (23), ta32 1815 (31), ta33 1848
  (57), ta34 1856 (27), ta40 1711 (42), ta41 2069 (64), ta42 1983 (46),
  ta43 1905 (59), ta44 2011 (32), ta45 2024 (24), ta46 2062 (58), ta47 1951
  (62), ta48 1998 (61), ta49 1997 (36), ta50 1982 (59). Cifras completas en
  `iter/I-001/full_analysis.txt`. Contra esta celda se mide todo lo que
  venga, y no contra los mejores históricos, que salen de muchas más tiradas
  y de presupuestos más largos.
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
| H-4 | N8 contra N2 (y contra N1, N3, N_ext), fase B del paper de COR | un vecindario más rico gana | -- | 82 instancias x 30 runs, 2460 bloques pareados: N2 1846.50 contra N8 1847.94, dif −1.45, p_adj = 3.9e−4, r = 0.077 (**despreciable**); rangos de Friedman N2 2.1315 el mejor de cinco, N8 2.2400 | **descartada** (antes del bucle; `experiments/cor_tabu_2026/`) |
| H-5 | profundidad del tabú como **parámetro global** (`bad-iterations`) | más profundo es mejor | -- | dentro del espacio de irace **dos veces**: rango (5, 30) en el paper de COR para los cinco vecindarios, rango (5, 40) en los dos brazos de este proyecto. Las configuraciones ganadoras eligieron **15** para el ABC (config. 136) y **23** para el memético (config. 164), no el tope | **contestada** por el afinado, no hace falta experimento |
| I-002 | vecindario N8 en vez de N2, **repetición de H-4** | la misma que H-4 | n8−control = −1.44, pasa (no descarta) | 3 mirillas de 6, 15 runs: **+1.52**, N8 mejor en 7 de 21, p = 0.054; reproduce H-4 en el régimen corto y crisp | **retirada** (2026-09-21): la pregunta ya estaba contestada |
| I-001 | sembrar la población inicial en tiradas cortas desde el banco; composición vs calidad | ver abajo | mix−control = −2.75 en 4 inst. (regla: > +2 descarta) → pasa | 21 inst. x 5 celdas x 30 runs: mix−control = −0.90, mejor en 13 de 21, W = 88.5, **p = 0.348**; ninguna celda separa (la mejor, `v2rand`, −1.87, p = 0.079) | **descartada** (2026-09-21) |

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
- **B-2** ~~Optimizar la receta de composición del pool~~ **archivada
  (2026-09-21)**, y no por falta de ganas sino por aritmética: I-001 midió
  que una ventaja de **294 unidades de makespan en la generación 0**
  sobrevive como **0.9 unidades al final**, el 0.3 %. El techo de cualquier
  receta de siembra es esa fracción de lo que se gane en el arranque, y
  para que la receta valiera tres unidades habría que mejorar el arranque en
  mil. Depende de que algo cambie primero la parte de la búsqueda que borra
  el arranque (B-9, B-10).
- **B-3** ~~Entrenar el generador con recompensa = resultado del ABC~~
  **archivada (2026-09-21)**, por lo mismo que B-2 y con más razón: la
  recompensa que se querría optimizar conserva el 0.3 % de la señal del
  arranque, así que el ruido de asignación de crédito de una tirada entera
  del ABC la enterraría.
- **B-4** Primera mejora con orden aleatorio en el tabú, en vez de
  mejor-de-todos. Diff pequeño, sin parámetros nuevos. Corrección al
  razonamiento original (2026-09-21, leyendo `LS_Tabu::apply`,
  `LocalSearch.cpp:316-435`): el bucle **ya ordena** los vecinos
  (`sortByEstimation`) y **ya poda** (`localsearch.filter = yes` corta el
  barrido en el primer vecino cuya estimación no mejora al mejor real
  encontrado). Como el estimador es `heads&tails`, una cota inferior del
  makespan tras el intercambio, la poda es exacta: el movimiento elegido es
  el mejor de todo el vecindario aunque solo se evalúe un prefijo. Ordenar,
  por tanto, no puede cambiar la trayectoria, solo el coste, y el coste ya
  está podado; primera-mejora sí la cambiaría, pero el ahorro es menor de lo
  que parecía. Prioridad baja.
- **B-5** `[REAJUSTA]` Política aprendida de selección de movimiento en el
  tabú (RL / hiperheurística). Solo si B-4 muestra que la regla de selección
  importa.
- **B-6** Intentos de récord: muchas tiradas cortas con la configuración
  vigente sobre la lista corta `ta29 (0) ta30 (4) ta22 (13) ta23 (14) ta27
  (16) ta26 (17) ta45 (19)`; afinar sobre las propias instancias es legítimo
  ahí y se declara. No es una idea sino el objetivo: conviene intercalarlo
  cada pocas iteraciones, no dejarlo para el final.
- **B-7** `[REAJUSTA]` **Longitud de tirada adaptativa**: terminar la tirada
  tras `S` segundos sin mejorar el mejor global, en vez de agotar un
  presupuesto fijo, y dejar que el arnés arranque la siguiente. Es la
  continuación natural de lo que ya medimos (JOURNAL 2026-09-21: la última
  mejora llega en la mediana al 0.32 del presupuesto, el 61.8 % de cada
  tirada se gasta después, y reiniciar gana en 21 de 22 instancias). Los
  presupuestos por clase son el mejor `L` *fijo*; uno adaptativo debería
  batirlo sin tocar nada más. Diff pequeño, en la regla de parada; `S` se
  fija de antemano en la mediana medida por clase, no se ajusta. Aviso de la
  revisión externa (2026-09-21): "segundos sin mejorar el mejor global"
  asigna al revés, porque una tirada que encuentra pronto un buen incumbente
  recibe *más* tiempo estéril que una mala que se corta antes; hay que
  añadir un mínimo de ejecución (`0.25 L`) y comprobar antes que el riesgo
  de mejora decae de verdad con la edad del estancamiento. Si ese riesgo no
  cae, el corte por estancamiento no tiene base.
- **B-8** `[REAJUSTA]` **Reparto de la búsqueda local**, a coste total igual.
  Hoy (`localsearch.target = 0.4645`, `MALS_SOME`, `period = 1`) el tabú cae
  cada generación sobre el **46 % de los 247 individuos elegido al azar**, y
  cada llamada es corta: 15 iteraciones sin mejora y 2 s de tope
  (`localsearch.bad-iterations = 15`, `localsearch.max-time = 2`). El
  incumbente no recibe trato especial salvo por azar. Dos celdas, la primera
  **el mejor siempre más el resto al azar, a número total de llamadas
  igual**. La revisión externa (2026-09-21) avisa de no empezar por
  `localsearch.target = best`, que es la variante sin código: con
  lamarckismo y élite de 86, pulir una sola estirpe destruiría la diversidad
  que nuestros propios resultados de reinicio y de siembra dicen que
  importa. Toca la pregunta de dónde gastar el tabú, distinta de cómo
  hacerlo mejor (H-1, B-4), y no toca la de si el tabú es bastante profundo
  (B-9).

### Revisión externa, 2026-09-21

Consulta a un modelo externo (OpenAI Codex, `gpt-5.6-sol`, con búsqueda web)
sobre el problema, la configuración congelada, las medidas, los tres
descartes y el backlog. Informe enviado y respuesta completa en
`experiments/classic_jsp_2026/reviews/2026-09-21_codex_briefing.md` y
`..._codex_review.md`. Su diagnóstico, en una frase: *la deficiencia no está
en el rendimiento de la población ni en la calidad de los arranques, sino en
que toda la intensificación usa la misma trayectoria N2 muy poco profunda*.
Su orden: B-1, B-6, B-7, B-8, B-9 (nueva), B-10 (nueva), B-2, y dejar caer
B-3, B-4 y B-5.

Dos de sus propuestas se comprobaron en el código antes de anotarlas, y la
comprobación cambió su precio:

- **Muestreo de varios caminos críticos**: **inaplicable**. `N2`
  (`NeighbourhoodJSP_N2.cpp:38-88`) no recorre *un* camino crítico: siembra
  la cola con todas las tareas últimas de máquina cuyo fin iguala el
  makespan y retrocede por todos los predecesores tensos, de máquina y de
  trabajo. Ya es el grafo crítico completo.
- **Vecindario más rico con reinserciones**: **ya está implementado**.
  `jsp.makespan.n8` (`NeighbourhoodJSP_N8.cpp:17-22`) son los intercambios
  de extremo de N2 *más* movimientos de reinserción fuera de bloque, que es
  justo el tipo Balas-Vazacopoulos que la revisión señala. Está registrado y
  se selecciona con una línea del setup, sin código. Con una advertencia:
  N8 todavía evalúa sobre una copia del horario, mientras N2 tiene la
  evaluación en sitio, así que a igual tiempo de reloj N8 paga un peaje de
  velocidad. La comparación a igual tiempo es la honesta y es la nuestra; si
  N8 asoma pese al peaje, portar la evaluación en sitio es lo siguiente.

- **B-9** `[REAJUSTA]` **Profundidad del tabú**, en su forma **asimétrica**,
  que es la única que queda viva. La forma global está contestada (H-5): subir
  `bad-iterations` a secas es reajustar un parámetro que irace ya barrió dos
  veces, en (5, 30) y en (5, 40), y que eligió en 15 para el ABC. Lo que irace
  **no** pudo explorar es un reparto desigual, porque en su espacio la
  profundidad era una sola para toda la población. Sigue el enunciado
  original abajo, entendido siempre como "llamadas cortas para la población,
  una profunda para el incumbente". Cada llamada muere a las 15
  iteraciones consecutivas sin mejorar, con tope de 2 s
  (`localsearch.bad-iterations = 15`). Para un tabú eso es rasísimo: TSAB e
  i-TSAB cruzan regiones peores durante cientos o miles de movimientos, y lo
  que tenemos se parece más a un descenso suavemente no monótono. Explicaría
  de paso por qué falló el back-jump (H-1): devolver a un punto mejor no
  cambia nada si la trayectoria nunca se alejó. Subir el parámetro a secas
  **no** es neutral en coste (46 % de 247 individuos por generación con tope
  de 2 s agotaría el presupuesto), así que la forma correcta es la que
  propone la revisión: mantener las llamadas cortas para la población y
  añadir **una** llamada profunda sobre el incumbente cuando la tirada se
  estanca. **Antes de eso, el diagnóstico**, que es casi gratis: instrumentar
  cada llamada con iteraciones hechas, si terminó por las 15 o por los 2 s,
  profundidad del peor empeoramiento antes de la siguiente mejora, y makespan
  de entrada y salida. Si casi todas mueren en 15-30 iteraciones, ahí está.
- **B-10** ~~N8 en vez de N2~~ **cerrada (2026-09-21)**: es H-4, medida ya
  en la fase B del paper de COR sobre 82 instancias, y reproducida por I-002.
  N8 no bate a N2; queda segundo de cinco por rangos de Friedman, a 1.45
  unidades. Lo que queda vivo de esta línea no es N8 sino un vecindario que
  el paper **no** probó: reinserciones que crucen máquinas o que trabajen en
  espacio de soluciones parciales (B-12), no más variantes de bloque.
- **B-11** **Patada estructurada y reoptimización** (ILS, no back-jump): al
  estancarse, aplicar tres movimientos críticos factibles al azar sin
  evaluar, limpiar la memoria tabú y volver al tabú. Se distingue de H-1 en
  que H-1 volvía a un punto ya visitado; esto sale de la cuenca a propósito,
  y se distingue del reinicio en que conserva la estructura de fondo.
- **B-12** **Reparación por ventana exacta / CP** sobre el incumbente: fijar
  casi todas las secuencias de máquina, liberar las dos máquinas que más
  arcos críticos aportan y darle medio segundo a un resolutor exacto, tarde
  en la tirada. Diff mayor, y la única familia con un mecanismo creíble para
  encontrar la unidad suelta que N2 no ve (Beck, Feng y Watson).
- **B-13** **Longitudes de tirada de cola pesada** en vez de adaptativas:
  sortear el límite de cada tirada en `{L/2, L, 2L}`. Sin parámetro
  continuo que ajustar, y protege contra las instancias donde la mejora rara
  llega tarde. Más simple que B-7 y se prueba antes.

**Crítica de la revisión a nuestras propias medidas**, aceptada y anotada
aquí para que conste:

1. El mejor `L` por clase se eligió sobre las mismas trazas con las que se
   midió la ganancia: hay optimismo de selección, y "21 de 22" ignora la
   magnitud y la dependencia entre instancias. La afirmación defendible es
   que las trazas dan evidencia fuerte de que el reinicio fijo domina a los
   300 s, no que los horizontes elegidos sean los buenos. B-1 tiene que ser
   con tiradas independientes de verdad, paradas y relanzadas, y reportando
   la distribución del mejor-de-k, no su media.
2. "El 61.8 % del tiempo se gasta después de la última mejora" es
   descriptivo, no prueba de desperdicio: toda búsqueda estocástica tiene un
   intervalo largo tras su última mejora, porque "última" se define con el
   futuro. Lo que decide es el riesgo condicional de mejora tras `s`
   segundos de estancamiento.
3. "El rendimiento no es la restricción" está sobreafirmado. El resultado
   del memético solo dice que el número de generaciones *en esa arquitectura*
   no predice la calidad. Las unidades útiles de rendimiento son entradas
   independientes a cuencas, movimientos de tabú y vecinos evaluados por
   segundo, no generaciones.
4. El criterio de aceptación (Wilcoxon sobre medias por instancia) mide un
   desplazamiento medio, y el objetivo es un récord, que vive en la **cola
   inferior**. Hay que añadir endpoints predeclarados de cola: probabilidad
   de igualar o batir el BKS en `ta30`, probabilidad de llegar a BKS+d por
   instancia de la lista corta, mejor makespan verificado tras un
   presupuesto total fijo, y el déficit esperado en el 5-10 % inferior de las
   tiradas. El Wilcoxon se queda para elegir algoritmo; la cola es el
   producto.

### Cierre de I-001, 2026-09-21

`iter/I-001/full_analysis.txt`. Decisión por el criterio predeclarado:
**descartada**. `mix` contra `control`, media por instancia de 30 runs,
Wilcoxon pareado por las 21: diferencia media **−0.90**, mejor en 13 de 21,
W = 88.5, **p = 0.348**. Ninguna celda separa: `v2rand` −1.87 (14 de 21,
p = 0.079), `v2top` −1.32 (p = 0.274), `v2maxmin` −0.67 (p = 0.876). Con
mejor-de-7 por bloques, entre −0.5 y −1.8, igual de mudo.

**Lo que sí quedó medido, y es el resultado de verdad de la iteración**: la
siembra entró con una fuerza enorme y no sirvió de nada. En la generación 0
las celdas sembradas arrancan **294 unidades por debajo** del control de
media, y de esa ventaja queda **el 0.3 %** al final. La calidad del arranque
se borra. Es una cifra, no una opinión, y es la que archiva B-2 y B-3.

**Descriptivo, post hoc, no decide nada**: la diferencia `mix − control` se
concentra en 20x20 (−2.17, mejor en 6 de 7), es nula en 30x15 (+0.72) y casi
nula en 30x20 (−0.65). Tres de las cuatro instancias del filtro son 20x20,
así que el filtro está sesgado hacia la clase donde el efecto es mayor y por
eso dio −2.75 donde las 21 dan −0.90. El filtro sigue valiendo, porque solo
descarta, pero **su magnitud no es una previsión de la magnitud final** y no
debe leerse como tal.

**Qué se revierte**: nada. `jsp.seeded` es opcional y se activa desde el
fichero de setup; la configuración vigente conserva `creation = jsp.random`,
así que la idea descartada no deja nada encendido. El código y los pools se
quedan porque B-9 y B-10 podrían querer un arranque controlado por otras
razones.

**B-1 ya tiene sus datos** por un lado: la celda `control` de I-001 es el
régimen corto con tiradas reales. El otro lado, `results/prereg2_abc/`, son
10 runs a 300 s. Los presupuestos totales no coinciden (20x20: 30 x 40 s
= 1200 s contra 10 x 300 s = 3000 s), así que el análisis tiene que igualar
tiempo de CPU antes de comparar, o la comparación no dice nada.

### I-002 — N8 en vez de N2

**Hipótesis** (una frase): a igual tiempo de reloj, hacer la búsqueda tabú
sobre `jsp.makespan.n8`, que son los intercambios de extremo de bloque de N2
**más** movimientos de reinserción fuera de bloque, da un makespan final
menor que hacerla sobre N2.

De dónde sale: (i) la revisión externa del 2026-09-21 señala que las tres
ideas descartadas (back-jump, path relinking, memético) y las dos que algo
hacen (reinicio, siembra) son todas consistentes con un problema de *acceso a
cuencas*, y que las dos palancas que quedan son la profundidad de la
trayectoria tabú y la **conectividad del vecindario**; (ii) I-001 acaba de
medir que 294 unidades de ventaja en la generación 0 se evaporan hasta 0.9 al
final, o sea que de dónde se parte no decide nada y lo que decide es por
dónde se puede ir; (iii) una reinserción hace de una vez lo que varios
intercambios adyacentes no alcanzan, que es el mecanismo plausible para 4-19
unidades y no para 3.

**Qué se toca**: **una línea** del setup,
`localsearch.neighbourhood = jsp.makespan.n8`. Ni una de código. El resto de
la configuración, congelada. La celda `control` es la misma de I-001 salvo
runs y semilla, así que las dos tandas se pueden mirar juntas.

**El peaje, declarado de antemano**: N8 todavía evalúa sobre una copia del
horario mientras N2 evalúa en sitio (JOURNAL, refactor crisp), así que a
igual tiempo de reloj N8 examina menos vecinos por segundo. La comparación a
igual tiempo es la honesta y es la nuestra: si N8 gana pese al peaje, el
siguiente paso evidente es portarle la evaluación en sitio; si pierde, no se
sabrá si fue el vecindario o el peaje, y eso se anotará como tal en vez de
declarar muerto al vecindario.

**Comprobado antes de correr** (2026-09-21, `ta29`, 4 runs x 40 s con las
mismas semillas): N8 produce horarios factibles y verificados, y su
trayectoria **no** es la de N2 (1634/1637/1631/1645 contra
1642/1634/1640/1639), o sea que los movimientos extra entran de verdad.
Cuatro tiradas no miden nada más que eso.

**Celdas**: `control` (N2) y `n8`. Dos, según la cadencia nueva.

**Endpoint primario, fijado antes de correr**: `n8` contra `control`, media
por instancia, Wilcoxon pareado por las 21 instancias abiertas, **p <= 0.0142
en cada una de las seis mirillas** (frontera de Pocock) y `n8` con menor
rango. **Secundario, reportado y no decisorio**: el mejor esperado por celda
sobre las tiradas acumuladas.

**Regla del filtro**: se descarta si la media de `n8 − control` sobre las
cuatro instancias del filtro supera **+2.0** unidades. El filtro solo
descarta. Y, con lo aprendido en I-001, su magnitud **no** es una previsión
de la magnitud final: tres de sus cuatro instancias son 20x20.

**Si se acepta**: `jsp.makespan.n8` pasa a la configuración vigente y la
siguiente idea es portarle la evaluación en sitio, que es rendimiento puro
sobre un vecindario que ya habría demostrado valer.

### I-002, primer filtro anulado por un fallo de N8 (2026-09-21)

El filtro corrió de 14:37 a 16:14 y la cola salió con estado **1**: la
verificación rechazó horarios. `N8` emitía soluciones **infactibles**, con
operaciones solapadas en una máquina: en `ta45`, **24 de 30** tiradas; en
`ta23`, 1 de 30; en `ta29` y `ta30`, ninguna. Todos los solapes de `ta45` son
en la máquina 7 y entre los mismos tres trabajos, o sea un fallo
sistemático, no ruido.

**Qué falla**, en `NB_ParallelN8_MakespanJSP::evaluateNeighbour` para los
movimientos de reinserción (`tipo = 1`,
`NeighbourhoodJSP_N8.cpp:380-391`): la propagación de cabezas siembra la cola
con `oldMs` y con `T`, y le falta **`newMs`**. Al reinsertar `T` entre `newMp`
y `newMs`, el predecesor de máquina de `newMs` pasa de ser `newMp` a ser `T`,
así que su cabeza hay que recalcularla siempre. Tal como estaba, solo se
recalcula si la cabeza de `T` cambia, porque entonces la BFS empuja a sus
sucesores. Cuando la cabeza de `T` no cambia, `newMs` se queda con la cabeza
que tenía con su predecesor viejo, arranca antes de que `T` termine, y **el
makespan sale falsamente mejor**, que es justo por lo que la búsqueda
aceptaba esos movimientos.

**Qué hace esto por el método**: es la mejor defensa que ha tenido la regla
de recomputar el makespan desde el propio horario. El solver reportó esas
soluciones como buenas y como mejores; nada dentro del solver lo habría
detectado. También dice que el humo previo fue demasiado estrecho: probé
`ta29` con 4 tiradas y `ta29` es precisamente una de las dos instancias donde
el fallo no salió.

**Qué se hace**: el filtro queda **anulado por invalidez**, no descartado, y
la decisión de anularlo se toma **sin haber calculado la comparación**, que
sobre una celda con 24 de 30 tiradas infactibles no significaría nada. Se
arregla `N8`, se comprueba en `ta45`, que es la instancia que lo destapó, y
se relanza el filtro entero. La regla del filtro y el endpoint primario de
I-002 siguen siendo los preinscritos: no se toca ninguno.

### I-002, filtro (segundo, válido): pasa

`iter/I-002/filter_analysis.txt`, 2026-09-21 16:29-17:05, 48 trabajos, cero
infactibles. Medias de 30 runs, `n8` menos `control`: ta23 **−5.00**, ta30
**−5.57**, ta29 **+1.43**, ta45 **+3.37**; media **−1.44**. La regla
preinscrita descartaba si superaba +2.0, así que **pasa** y va a las oleadas.
Nada más se puede leer aquí: el filtro solo descarta, y I-001 acaba de
demostrar que su magnitud no anticipa la final.

**Control reproducido, de regalo**: la celda `control` de este filtro es la
misma configuración y las mismas semillas que la de I-001, corrida cinco
horas después y troceada en seis procesos en vez de uno. Medias: ta23 1587.3
contra 1587.3, ta29 1641.3 contra 1641.1, ta30 1623.4 contra 1623.4, ta45
2041.9 contra 2042.1. A dos décimas de unidad. No son idénticas porque el
presupuesto es tiempo y no generaciones, así que la carga de la máquina
cambia cuántas se hacen; que la diferencia sea de 0.2 unidades sobre 1600 a
2000 dice que el ruido entre tandas es mucho menor que los efectos que
perseguimos, y que trocear los runs no cambia nada.

**Segunda corrección al reloj, ahora medida y no estimada**: el suelo de una
tanda no es ni el coste partido por 14 ni el trabajo más largo, es **la suma
de los dos**, porque el último trabajo no puede empezar hasta que se haya
despachado el resto. Con 48 trabajos y 4.5 h-CPU: 19 minutos de despacho
más 16 del último trabajo de `ta45` = 35, y midió 36. Además un trabajo con
la máquina llena va un **28 %** más lento que su presupuesto nominal de CPU.
La fórmula que vale, y que hay que usar para prometer plazos:

```
reloj ~= (coste_total_CPU - trabajo_mas_largo) / huecos + trabajo_mas_largo * 1.28
```

Consecuencia para las oleadas: una oleada de 5 runs sobre las 21 instancias
son 6.1 h-CPU, o sea 26 minutos de despacho como suelo irreducible, más unos
16 del último trabajo de 30x20: **unos 35 minutos**, no 26. Las seis oleadas
son 3.5 h, no 2.6. Sigue siendo una decisión cada 35 minutos en vez de una
cada 6.5 horas, y el 70 % de las decisiones caen en las dos primeras, así
que el diseño se mantiene tal como está preinscrito; lo que se corrige es la
cifra que se anuncia.

### I-002 retirada, y lo que deja (2026-09-21)

**Por qué se retira**: la pregunta ya estaba contestada antes de empezar. La
fase B del paper de COR (`experiments/cor_tabu_2026/`, tag
`cor-tabu-2026-submission`) compara los cinco vecindarios afinados por irace
sobre 82 instancias con 30 runs, 2460 bloques pareados: N2 queda primero por
rangos de Friedman (2.1315) y N8 segundo (2.2400), con N2 mejor que N8 por
1.45 unidades, p_adj = 3.9e−4 y tamaño de efecto **despreciable** (r =
0.077). Se retira por eso y **no** por sus propias mirillas: la decisión no
depende de los datos de I-002.

**Lo que I-002 deja, que no es nada**:

1. **Una reproducción independiente de H-4.** Mis tres mirillas dan N8 peor
   que N2 por +1.82, +1.49 y +1.52 unidades con 5, 10 y 15 runs, sobre el
   problema **crisp**, las 21 instancias abiertas de Taillard y presupuestos
   de tiradas cortas. El paper da +1.45 sobre el problema de **intervalos**,
   82 instancias y presupuesto largo. Dos problemas, dos conjuntos de
   instancias, dos regímenes, misma cifra a una décima. Pocas veces se
   reproduce así un negativo.
2. **Un fallo real arreglado en el árbol crisp.** El defecto de N8 que
   encontró la verificación (24 de 30 tiradas infactibles en ta45) **no era
   nuevo**: es el primero de los cuatro parches de siembra de BFS que la
   línea de intervalos ya había encontrado y corregido para el paper
   (`statistical_results/REGENERATION_NOTES.md`: *"eval tipo=1: missing
   arc->x, arc->z seeds"*; `arc->z` es `newMs`, exactamente lo que faltaba).
   El N8 crisp se escribió con la lógica de antes del arreglo (`32660f4`) y
   nunca recibió el porte, así que **el refactor crisp regresó un fallo ya
   resuelto**. Tras su arreglo, la línea de intervalos validó 2460 de 2460
   tiradas de N8; el árbol crisp estaba otra vez en 24 de 30 infactibles en
   ta45. El arreglo (`b1ad79e`) es el mismo, y se queda.
3. **La comprobación de que el vecindario de producción está sano.** El N2
   crisp siembra su propagación con `x` **y** con `y`
   (`NeighbourhoodJSP_N2.cpp:145-146`), así que el tercero de aquellos cuatro
   parches (*"eval tipo=0: missing arc->x seed"*) no está presente aquí. Y
   las 3.780 tiradas verificadas de I-001 e I-002 son todas factibles.

**Dos cosas del paquete del paper que hay que mirar**, ajenas al bucle:

- La columna `winner` de `statistical_results/wilcoxon_table.csv` está
  **invertida**: nombra siempre el brazo de media **mayor**, o sea el peor.
  En `gen_phaseB_tables.py:233` es `p["A"] if p["sign"] > 0 else p["B"]`, con
  `diff = meanA − meanB`, así que con `diff > 0` el brazo A es el peor y sale
  como ganador. **No afecta al paper**: `tab4_wilcoxon.tex` no usa esa
  columna. Pero ese CSV viaja en el dataset de Zenodo, y ahí sí es una
  trampa para quien lo lea.
- El README del paquete dice *Computers & Operations Research*, y el PI
  habló de ASOC. O son dos artículos distintos o uno de los dos datos hay
  que corregir.
