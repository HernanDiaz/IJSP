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

**Dos clases de iteración: de posición y de cola.** Decisión del PI
(2026-09-22), tras diez iteraciones y dos sustos. Hasta ahora toda idea se
juzgaba por el desplazamiento de la **media**, y eso tiene un motivo que sigue
siendo bueno: con una dispersión entre tiradas de ~8 unidades, el error típico
agregado sobre las 21 instancias es de **~0.45 unidades** para la media,
**~0.7** para la media de los mejores por bloques de 5, y **~1.0** y además
sesgado para el mejor absoluto. El mínimo de una muestra es un estadístico de
orden extremo: cuesta el doble de datos ver lo mismo. Y si un mecanismo
desplaza la distribución, **el mínimo baja con ella**: una mejora en la media
*es* una mejora en la cola, medida con un instrumento más fino.

Esta sesión lo aprendió por las malas dos veces. I-005 empeoraba la media y
mejoraba el mejor en `ta30`, 1595 contra 1607; con 600 tiradas en I-006 el
efecto desapareció (26 bloques contra 30, p = 0.689). I-007 tocó 1625 con el
presupuesto largo en 27 tiradas; con 84 más en I-008 no se repitió.

Pero media y cola **sí** divergen cuando un mecanismo no mueve el centro sino
la **forma**, es decir cuando aumenta la varianza, y el protocolo no sabía
premiar eso: el filtro descarta por empeorar la media, que es exactamente el
aspecto de un mecanismo que ensancha la cola. Por ahí se cayó I-005. Desde
ahora:

- Una iteración **de posición** (las diez primeras y el caso por defecto)
  decide por la media, como hasta ahora.
- Una iteración **de cola** se declara como tal **antes de correr**, y
  entonces: su endpoint primario es la **media de los mejores por bloques de
  5 tiradas** (lo que consume de verdad una campaña de récord, y no el mejor
  absoluto, que solo vale como titular); su regla de filtro se aplica sobre
  ese endpoint y no sobre la media; y **se le permite empeorar la media**,
  que se reporta sin valor decisorio. La frontera es la misma de Pocock,
  p <= 0.0142 en cada una de las seis mirillas, simétrica.
- Lo que **no** cambia: el filtro solo descarta, los presupuestos por clase
  son intocables, y la métrica sigue siendo el makespan recomputado desde el
  horario.

La aritmética que justifica abrir esta vía: en `ta29` la media está en 1641 y
el BKS en 1625, dos desviaciones típicas. Al récord se llega bajando la media
dos unidades, que es lo que **no ha conseguido nada** en diez iteraciones, o
**subiendo la dispersión**, que nadie ha intentado como objetivo. Son dos
caminos al mismo sitio y solo hemos recorrido uno.

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

**Un directorio de resultados por trabajo, nunca compartido entre trabajos de
la misma instancia.** Regla nueva (2026-09-22), pagada con el primer filtro de
I-011. El solver nombra su salida con una marca de tiempo de **resolución de
un segundo**, así que dos trabajos concurrentes sobre la misma instancia que
escriban en el mismo directorio **colisionan en el nombre del fichero y se
pisan**. Aquel filtro mandó 60 trabajos de la celda `portfolio` a un solo
directorio y dejó **25 certificados**: 35 tiradas desaparecidas y 4 de las
supervivientes ilegibles. Lo detectó el análisis, que verifica **todos** los
certificados; la cola no, porque su verificación mira solo el más reciente de
cada par (tag, instancia) y con 15 trabajos por par comprueba el mismo fichero
quince veces. **`queue exit 0` no garantiza nada cuando varios trabajos
comparten directorio.** Desde ahora el par (tag de resultados, instancia) es
único por construcción y se comprueba antes de lanzar.

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
- **Mejores verificados de la línea, igualando el BKS** (la prueba es el
  horario, y los horarios están guardados):
  - `ta30` = **1584**, **iguala el BKS**, 2026-09-22, celda `control` del
    filtro de I-012, 40 s por tirada.
    `iter/I-012/evidence/ta30_1584_control_I-012_fc1_control.csv`.
    **Primera vez de la línea**: el mejor anterior era 1588, del régimen
    largo, y en el régimen corto era 1595.
  - `ta29` = **1625**, iguala el BKS, tres veces: régimen largo, celda de
    300 s de I-007 (`iter/I-007/evidence/`), y celda `escape` del filtro de
    I-012 (`iter/I-012/evidence/`).
  - `ta23` = **1564**, a 7 del BKS, I-006, `iter/I-006/evidence/`.
  Ninguno es un récord: **récord es batir el BKS**, y no hay nada
  estrictamente por debajo.
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
| I-003 | el explorador del ABC reinyecta un **elite pateado** en vez de una solución aleatoria | un arranque aleatorio a mitad de tirada no puede alcanzar a la población; uno dentro de una cuenca buena sí | kick−control = **+1.43** (ta23 −0.27, ta29 +0.70, ta30 −0.50, ta45 **+5.80**); regla > +2 descarta → **pasa, por poco y en contra** | 4 mirillas de 6: −1.02, −0.57, **−0.03**, +0.44; kick mejor en 9-10 de 21 siempre; p entre 0.55 y 0.88, la frontera nunca se acerca | **detenida en la 4ª** (2026-09-21) por cambio de dirección del PI, no por sus datos. Sin aceptación: es un cero |
| H-4 | N8 contra N2 (y contra N1, N3, N_ext), fase B del paper de COR | un vecindario más rico gana | -- | 82 instancias x 30 runs, 2460 bloques pareados: N2 1846.50 contra N8 1847.94, dif −1.45, p_adj = 3.9e−4, r = 0.077 (**despreciable**); rangos de Friedman N2 2.1315 el mejor de cinco, N8 2.2400 | **descartada** (antes del bucle; `experiments/cor_tabu_2026/`) |
| I-012 | **escapar del estado todo-tabú, solo eso** | la celda informativa de I-010 dio −1.11 y mejor en 15 de 21 (p = 0.033) sin frontera | esc−control = −1.82, pasa | 6 mirillas, 30 runs: +1.30, +1.09, +0.59, +0.27, +0.39, **+0.07**; mejor en 10 de 21, p = 0.835 | **descartada** (2026-09-22). La señal de I-010 **no se reprodujo** |
| I-016 | **siembra desde el círculo** (idea del PI) y, de paso, **el generador aleatorio hecho uniforme** | que la población inicial represente todas las zonas del espacio; medido antes: el círculo cubre igual que el azar, pero el generador aleatorio del solver está sesgado y empieza un 6-7 % peor | mecanismo: las dos arrancan **~156 mejor** en la generación 0; al final circle−control = **+0.00**, uniform−control = **+0.71**; pasan las dos (descartaba si > +2.0) | mirillas media: w1: circle **+1.94** (8 de 21, p = 0.186), uniform **+3.22** (4 de 21, p = 0.018), w2: circle **+0.98** (9 de 21, p = 0.297), uniform **+1.95** (6 de 21, p = 0.048), w3: circle **+1.38** (7 de 21, p = 0.089), uniform **+1.74** (6 de 21, p = 0.036) | **en oleadas** (2026-09-23), tras cerrar I-015 |
| I-015 | **levantar el veto de meseta** (B-14): conservar la mejora que iguala exactamente el makespan del incumbente, en el cruce y en la escritura lamarckiana del tabú | I-014 dice que la búsqueda no sale de su cuenca; en JSP moverse de lado por la meseta del incumbente es cómo se sale, y el veto lo prohíbe | mecanismo **~3000 admisiones/tirada**, sin coste; allow−control = **−2.38** (ta45 −3.83, ta30 −3.00, ta23 −2.67, ta29 −0.03), pasa (descartaba si > +2.0) | mirillas media: **+1.46** (w1, 7 de 21, p = 0.237), **+0.62** (w2, 8 de 21, p = 0.848), **+0.22** (w3, 9 de 21, p = 0.674), **+0.50** (w4, 13 de 21, p = 0.715), **-0.38** (w5, 13 de 21, p = 0.434), **-0.78** (w6, 14 de 21, **p = 0.122**) | **descartada** (2026-09-23) por no cruzar en la sexta; el final más favorable del bucle, a diez veces la frontera |
| I-014 | **reiniciar la población alrededor del incumbente** cuando la tirada lleva 0.2 del presupuesto sin mejorar | entre el 18 y el 44 % de cada tirada se gasta con riesgo de mejora medido en ~0 %; convertir ese tramo en búsqueda nueva **sin perder el incumbente** debe bajar la media | mecanismo **1.36 reinicios/tirada**; restart−control = **−0.70**, pasa (descartaba si > +2.0) | mirillas media: **-0.70** (w1, 13 de 21, p = 0.186), **-0.37** (w2, 12 de 21, p = 0.271), **-0.15** (w3, 9 de 21, p = 0.805), **-0.17** (w4, 11 de 21, p = 0.664), **-0.10** (w5, 10 de 21, p = 0.702), **-0.16** (w6, 10 de 21, **p = 0.516**) | **descartada** (2026-09-23) por no cruzar en la sexta, con el mecanismo disparándose en todas las mirillas |
| I-013 | **primera mejora sobre un barrido rotatorio de N2**: sin ordenar, sin podar, desde una posición que rota, el primer vecino elegible que mejore | con la regla *el mejor* el orden es irrelevante, y por eso I-004, el defecto de las colas e I-005 dieron cero; al abandonarla, el orden decide el movimiento | mecanismo **al 69.7 %**; first−control = **+21.55** (ta45 +30.83, ta23 +29.27, ta30 +16.90, ta29 +9.20); regla > +2 → **DESCARTA** | -- | **descartada** (2026-09-23) en el filtro, y con una cifra que mide algo |
| I-011 | `[COLA]` **cartera de configuraciones**: cada tirada sortea una combinación de los interruptores ya medidos como neutros | una mezcla de componentes neutros conserva la media y **suma varianza entre componentes**, así que alarga la cola por construcción | cola: bo5 cartera−control = **−2.54**, pasa (descartaba si > +2.0). Mecanismo **plano**: razón de dispersión 1.03, más ancha en 2 de 4 | mirillas bo5: **+1.43** (w1, 7 de 21, p = 0.574), **-0.43** (w2, 12 de 21, p = 0.602), **-1.46** (w3, 15 de 21, p = 0.106), **-0.79** (w4, 14 de 21, p = 0.213), **-0.79** (w5, 13 de 21, p = 0.192), **-0.32** (w6, 10 de 21, **p = 0.777**) | **descartada** (2026-09-23) por no cruzar en la sexta; y su mecanismo nunca se ejerció |
| I-010 | **escapar del estado todo-tabú** y con ello hacer alcanzable la profundidad | la profundidad la limita el callejón sin salida, no el parámetro | dscp−control = −0.86, pasa | 6 mirillas, 30 runs: +0.16, −0.32, −0.13, −0.06, −0.41, **−0.25**; mejor en 10 de 21, p = 0.639 | **descartada** (2026-09-22) por no cruzar en la sexta |
| I-009 | una llamada profunda al tabú sobre el incumbente, **una sola por tirada** | nunca hay una trayectoria profunda | deep−control = −0.02, pasa | -- | **retirada** (2026-09-22): infradimensionada por construcción, la llamada era el 0.06-2.3 % del trabajo del tabú |
| I-008 | **caza en `ta29` a 300 s**, 84 tiradas | el mínimo lo da el presupuesto largo (I-007), y con el triple de tiradas debería bajar de 1625 | -- | 84 tiradas: **sin récord y sin igualada**, mínimo **1628**. Con I-007 suman 111 tiradas de 300 s y **1 igualada**. Rebaja la conclusión de I-007 sobre la duración de tirada | **cerrada** (2026-09-22): cierra la fuerza bruta en `ta29` |
| I-007 | **intento concentrado en `ta29`**, y qué duración de tirada da el mínimo más bajo a igual CPU | los 40 s son el mejor corte fijo **por la media**; un récord vive en el mínimo | -- | 307 tiradas: **sin récord**. **Iguala el BKS, 1625**, verificado, solo la celda de 300 s. 40 s y 100 s se quedan en 1627. Medias iguales (1640.5 / 1639.8 / 1639.4) | **cerrada** (2026-09-22): sin récord, con igualada y con la duración de tirada medida |
| I-006 | **intento de récord** sobre la lista corta, 75 tiradas por celda e instancia | el récord vive en la cola, no en la media | -- | 600 tiradas: **sin récord**. Casi en `ta29` 1627 (BKS 1625, +2). **Mejor propio nuevo en `ta23`: 1564** contra 1571, verificado. Cola entre celdas plana: 26 contra 30 bloques, p = 0.689 | **cerrada** (2026-09-22), sin récord y con un mejor propio |
| I-005 | desempate **dirigido por frecuencia** entre los vecinos empatados de N2 | elegir *mejor* dentro de N2 no cambia nada (I-004); elegir **dirigidamente distinto** sí puede | freq−control = **+2.55** (ta45 +7.70, ta23 +4.20, ta29 +0.60, ta30 −2.30); regla > +2 descarta → **DESCARTA** | -- | **descartada** (2026-09-22) en el filtro |
| I-004 | reparar las colas que alimentan la estimación de N2 | con colas correctas la estimación vuelve a ser cota inferior, el orden del vecindario es el bueno y la poda deja de tirar movimientos mejores | ftails−control = −1.40, pasa | 6 mirillas, 30 runs, 21 inst.: +2.02, +0.84, +0.92, +1.23, +0.33, **+0.01**; mejor en 13 de 21, W = 103.5, p = 0.677; mejor-de-30 mejora en 9 y empeora en 11 | **descartada** (2026-09-22) por no cruzar la frontera en la sexta |
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
- **B-14** **Levantar el veto de meseta**, con su mecanismo ya medido. En
  `ArtificialBeeColonyPSO` hay dos sitios donde un descendiente se rechaza
  por **igualar** el makespan del incumbente, y el que importa está dentro de
  la escritura lamarckiana (`ArtificialBeeColonyPSO.cpp:765`): cuando el tabú
  mejora un individuo hasta el valor del incumbente, **la mejora se
  descarta** y se cuenta un intento fallido. Medido: 1.324 descartes por
  tirada en ta29 y 3.415 en ta41, contra 54.440 y 140.020 mejoras
  conservadas, o sea el **2,4 %** del trabajo útil del tabú tirado, con una
  varianza enorme entre tiradas (71 a 8.073). En JSP las mesetas al valor del
  incumbente son enormes y moverse de lado por ellas es cómo se sale de una
  cuenca, así que el veto prohíbe justo el movimiento que interesa. Es un
  filtro de duplicados hecho sobre el fitness en vez de sobre el genotipo:
  rechaza soluciones distintas por empatar y admite clones con makespan
  distinto. Diff de dos condiciones, **cero parámetros nuevos**.
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

### I-003 — el explorador del ABC: elite pateado en vez de solución aleatoria

**Hipótesis** (una frase): a igual tiempo de reloj, reemplazar una fuente de
comida agotada por **un clon de un elite al azar con tres mutaciones encima**
da un makespan final menor que reemplazarla por una solución aleatoria
nueva, que es lo que hace el ABC clásico.

De dónde sale, y esto es lo importante: **de nuestra propia medida**. I-001
midió que un arranque aleatorio está 294 unidades por encima en la generación
0 y que de esa distancia sobrevive el **0.3 %** al final de la tirada. El
mecanismo de abandono (`maxnumtrials = 35`,
`ArtificialBeeColony.cpp:755-757`) inyecta exactamente ese arranque
aleatorio, y lo hace repetidamente durante toda la tirada. Un individuo
inyectado en la generación g nace en torno a 2100 cuando la población ya está
en 1640, y no le queda presupuesto para alcanzarla: es diversidad que no
puede competir. La patada lo pone dentro de una cuenca prometedora, a tres
mutaciones de un elite, de modo que el abandono pasa de inyectar ruido a
**reiniciar una trayectoria**, que es lo único que este proyecto tiene medido
como positivo (JOURNAL 2026-09-21: reiniciar gana en 21 de 22 instancias).

**Qué no es**, porque tres ideas vecinas ya están descartadas: no es el
back-jump de H-1, que volvía a un punto ya visitado **dentro de una misma
llamada al tabú**; no es el path relinking de H-2, que **recombinaba** dos
elites; y no es la siembra de I-001, que actúa **una sola vez** en la
generación 0 y cuyo efecto se borra. Esta actúa durante toda la tirada y en
el momento que el propio algoritmo señala como agotado.

**Qué se toca**: el ABC, no el setup. `abc.scout = kick` y
`abc.scout.kicks = 3` en `ArtificialBeeColony.{h,cpp}`. Por defecto el
parámetro no existe y el comportamiento es el clásico, así que la celda de
control es idéntica en comportamiento a las de I-001 e I-002. Los tres
golpes se fijan **de antemano** y no se ajustan; los parámetros del ABC
siguen congelados en la configuración 136. Reutiliza la mutación ya
configurada (`swap`), sin operador nuevo.

**Búsqueda previa** (regla del 2026-09-21, aplicada): no hay nada de esto en
el árbol. Ni clearing, ni niching, ni crowding; ni racing ni reinicio dentro
de una tirada; ni memoria de frecuencias en `TabuList`; ni fijación de arcos
por consenso. La fase de explorador no aparece en ningún paquete de
`experiments/`, ni en los tags, ni como parámetro de irace en ninguno de los
siete ficheros de parámetros del repositorio.

**Celdas**: `control` (ABC clásico) y `kick`.

**Endpoint primario, fijado antes de correr**: `kick` contra `control`, media
por instancia, Wilcoxon pareado por las 21 instancias abiertas, frontera de
Pocock **p <= 0.0142** en cada una de las seis mirillas.

**Frontera simétrica**, que es el hueco que destapó I-002: cruzarla con
`kick` en el rango menor **acepta**; cruzarla con `control` en el rango menor
**rechaza ahí mismo** y las oleadas restantes no se corren. No cruzarla en la
sexta también rechaza. Queda fijado ahora, antes de ver un solo dato.

**Regla del filtro**: se descarta si la media de `kick − control` sobre las
cuatro instancias del filtro supera **+2.0**. Solo descarta, y su magnitud no
es una previsión: I-001 leyó −2.75 y luego −0.90, I-002 leyó −1.44 y luego
+1.52.

**Si se acepta**: `abc.scout = kick` pasa a la configuración vigente, y la
siguiente pregunta es si el número de golpes y la elección del elite (al
azar contra el mejor contra el más distante) valen algo, que ya sería
afinado y no idea.

### RETRACTACIÓN (2026-09-21): la sección siguiente estaba equivocada

Lo que decía la sección de abajo, que la fase de explorador del ABC nunca se
ejecuta, es **falso**, y el error es de método, no de aritmética.

**Qué pasó.** El setup dice `algorithm = ABCPSO`, y la clase
`ArtificialBeeColonyPSO` **hereda de `GeneticAlgorithm`, no de
`ArtificialBeeColony`**: es una implementación aparte, con sus propias fases.
Implementé la patada, y después el veto de meseta, en
`ArtificialBeeColony.{h,cpp}`, que **no se ejecuta**. Por eso las dos pruebas
de humo daban celdas idénticas. Lo interpreté como "el mecanismo está
apagado" cuando lo que estaba apagado era mi propio código.

**Y el número en el que me apoyé no era una medida.** `Total replacements in
ABC` salía 0 porque en la clase que sí corre el contador `abc_replacements`
**solo se inicializa a cero y nunca se incrementa** (`ArtificialBeeColonyPSO`
lo declaraba heredado y no lo tocaba). Leí una cifra que el solver reportaba
sin comprobar que estuviera calculada: exactamente el error contra el que
existe la regla de recomputar cada makespan desde su propio horario. La regla
estaba escrita para los makespans; queda extendida a **cualquier** contador
del solver antes de usarlo como dato.

**Lo que se mide de verdad**, con el contador ya instrumentado
(`ArtificialBeeColonyPSO.cpp`, 3 tiradas, 2026-09-21):

| | ta29 | ta41 |
|---|---|---|
| reemplazos de explorador por tirada | **496** | **815** |
| vetos de meseta en el cruce | 28 | 27 |
| vetos de meseta en la búsqueda local | **1.324** | **3.415** |
| mejoras conservadas por la búsqueda local | 54.440 | 140.020 |

Así que el abandono se dispara cientos de veces por tirada y **la premisa de
I-003 era correcta desde el principio**. I-003 revive, ya portada a la clase
que corre y con la rama verificada viva: los contadores de reemplazo
difieren entre celdas (426 contra 371 en ta29, 702 contra 492 en ta33) y los
makespans cambian en tres de cuatro instancias, todo factible.

**Lo que sobrevive de la sección de abajo**: solo la medida de la profundidad
del tabú, 26 a 41 iteraciones por llamada, porque `iterationsLS` **sí** se
incrementa en la clase que corre.

### I-003 retirada antes de correr, y la medida que la retira — **ERRÓNEA, ver arriba**

La prueba de humo, cuatro instancias de tres clases y dos celdas, salió
factible y con los **mismos makespans** en tres de las cuatro instancias. Eso
no era casualidad. Los contadores que el propio solver ya escribe en su CSV
(`ArtificialBeeColony.cpp:239-243`) lo explican:

| instancia | `Total replacements in ABC` | `Avg. Iterations per LS` |
|---|---|---|
| ta29 | 0 | 26.7 |
| ta23 | 0 | 30.9 |
| ta33 | 0 | 35.7 |
| ta41 | 0 | 41.0 |

**La fase de explorador del ABC no se ejecuta nunca.** Cero reemplazos, en
las ocho tiradas, en las cuatro instancias, en las tres clases de tamaño.
Con `maxnumtrials = 35` ninguna fuente de comida se agota, porque su contador
de intentos se pone a cero cada vez que la fuente mejora y, con la búsqueda
tabú cayendo sobre el 46 % de la población en cada generación y con
lamarckismo, ninguna fuente sobrevive 35 fallos consecutivos. El ABC, tal
como está afinado, **no abandona nada**. La patada no puede ejecutarse, así
que I-003 se retira por invalidez, antes de correr y sin gastar una tanda.

Esto no es un fracaso de la idea, es una medida sobre el algoritmo, y vale
más que la idea:

1. **`maxtrials = 35` no es un parámetro, es un interruptor apagado.** irace
   lo barrió y eligió un valor que desactiva el mecanismo. Es decir, el
   afinado prefirió un ABC sin exploradores. Cualquier idea que cuelgue del
   abandono está muerta de antemano, y eso incluye media literatura de ABC.
2. **La población nunca pierde un miembro.** Sin abandono, la única presión
   de diversidad que queda es la del cruce y la mutación, y el estudio de
   siembra ya documentó que la población colapsa. Eso apunta a la siguiente
   idea, que no depende de ningún mecanismo apagado: **obligar a una
   distancia mínima en la población** (clearing), que actúa en cada
   inserción. Las estadísticas de `hamming` y `neri` que el setup ya calcula
   y que nadie usa son justo su instrumento de medida.
3. **De paso, la profundidad del tabú queda medida** sin gastar la sonda que
   había lanzado: una llamada dura entre **26 y 41 iteraciones** de media
   según la clase. No mueren en un puñado de iteraciones, y no mueren en el
   tope de 2 s, que permitiría muchas más: mueren en el contador de 15 sin
   mejora, tras haber hecho 26 a 41 movimientos. Sigue siendo poco profundo
   al lado de TSAB, pero es diez veces más de lo que la revisión externa
   suponía al hablar de "15 movimientos".

**Qué se revierte**: nada, y el código de la patada se queda con un comentario
en `ArtificialBeeColony.cpp` que dice que es rama muerta mientras el abandono
no se active, con la medida y la fecha. Borrarlo perdería el hallazgo.

### I-003, filtro: pasa, y hay que decir cómo

`iter/I-003/filter_analysis.txt`, 48 trabajos, cero infactibles. Medias de 30
runs, `kick` menos `control`: ta23 −0.27, ta29 +0.70, ta30 −0.50, ta45
**+5.80**; media **+1.43**. La regla preinscrita descartaba por encima de
+2.0, así que **pasa**, pero pasa **por poco y con la dirección en contra de
la hipótesis**. La regla solo descarta y se aplica literalmente: va a las
oleadas. Lo que acota el coste es la frontera simétrica, que rechazará en la
mirilla en que cruce en lugar de gastar las seis.

**Tercera reproducción del control**, y ya es un registro: la celda `control`
de este filtro es la misma configuración y las mismas semillas que las de
I-001 y I-002, corrida en otro momento del día. ta23 1587.3 contra 1587.3,
ta29 1640.8 contra 1641.3, ta30 1623.4 contra 1623.4, ta45 2041.9 contra
2041.9. Media décima de unidad de diferencia sobre makespans de 1587 a 2042.
El ruido entre tandas de este banco de pruebas es despreciable al lado de los
efectos que perseguimos, y eso es lo que hace que un efecto de tres unidades
sea detectable.

### I-003, cierre en la cuarta mirilla

Cuatro mirillas, 20 tiradas acumuladas por celda, cero infactibles en 840
tiradas. `kick` menos `control`: **−1.02, −0.57, −0.03, +0.44**, con `kick`
mejor en 9 o 10 de las 21 en las cuatro, y p entre 0.55 y 0.88. La frontera
de Pocock, 0.0142, no se acercó en ningún momento ni en ningún sentido.

**Por qué se detiene en la cuarta y no en la sexta**: por decisión del PI de
cambiar de dirección, no por lo que dicen los datos. La distinción importa y
por eso se escribe: **no se reclama aceptación ni rechazo estadístico**, se
deja el registro de un **cero** medido con 20 tiradas por celda sobre las 21
instancias. Detener una tanda preinscrita porque la estimación va hacia donde
a uno le gusta sería trampa; detenerla sobre una estimación que lleva cuatro
mirillas pegada al cero, para gastar esas dos horas en otra cosa, no lo es, y
queda anotado para que cualquiera lo juzgue.

**Qué deja medido**, que es lo que vale: reemplazar entre 500 y 800 arranques
aleatorios por tirada por elites pateados **no cambia el resultado**. Junto a
I-001, que midió que 294 unidades de ventaja en la generación 0 se quedan en
0.9 al final, el patrón es consistente y ya son dos medidas independientes:
en este algoritmo, **de dónde parte un individuo es irrelevante**, tanto al
principio como a mitad de tirada. Lo que queda por mirar no es de dónde se
sale sino cómo se elige el paso siguiente, que es donde apunta el PI.

### I-004 — la estimación de N2 no es una cota inferior, y por eso el orden decide

**El hallazgo, primero**, porque la idea es su consecuencia. El PI insistió en
que la clave está en los vecinos de N2, en cómo se ordenan y en qué orden se
visitan. Tenía razón, y la razón es un defecto, no una preferencia de diseño.

`LS_Tabu` ordena el vecindario por la estimación `heads&tails` y, con
`localsearch.filter = yes`, corta el barrido en el primer vecino cuya
estimación deja de ser mejor que el mejor valor real encontrado. Eso solo es
correcto si la estimación es una **cota inferior**. Medido en `ta29`, 10 s,
evaluando todos los vecinos:

| colas | vecinos evaluados | estimación **por encima** del valor real |
|---|---|---|
| incrementales, como corre hoy | 1.362.270 | **882.481 (64,8 %)** |
| recalculadas enteras | 1.352.160 | **0** |

O sea que **no es una cota inferior en dos tercios de los casos**, y la causa
son las colas: `NB_ParallelN2_MakespanJSP::acceptNeighbour` las mantiene con
un barrido hacia atrás que **solo continúa mientras una cola cambia**, el
mismo patrón de siembra incompleta que causaba el fallo de N8 (ver la entrada
de I-002). Con las colas rehechas desde cero la estimación vuelve a ser cota
inferior exacta: 0 violaciones de 1,35 millones.

Los mismos cinco vecinos, con colas obsoletas y con colas correctas:

```
est=2308 real=2276      est=2238 real=2276
est=2322 real=2247      est=2247 real=2247
est=2323 real=2284      est=2284 real=2284
est=2349 real=2310      est=2310 real=2310
est=2362 real=2323      est=2323 real=2323
```

Las colas viejas inflan la estimación hasta 75 unidades y mandan al final del
orden a vecinos cuyo valor real es **mejor** que el de los que sí se evalúan.
Como la poda corta antes de llegar a ellos, **no se evalúan nunca**. Decenas
de millones de veces por tirada. El tabú no está cogiendo el mejor movimiento
de N2: coge el mejor de un prefijo truncado y mal ordenado.

**Hipótesis** (una frase): a igual tiempo de reloj, rehacer las colas antes de
cada estimación da un makespan final menor, porque el vecindario se ordena
bien y la poda deja de descartar movimientos mejores.

**Qué se toca**: `localsearch.tails = full`, una línea de setup respaldada por
un método nuevo `recomputeAllTails()` que repite el cálculo que ya hace
`setInitialSolution`. **Ningún parámetro del algoritmo cambia**: esto no
ajusta nada, repara el dato con el que se ordena y se poda. Por defecto el
comportamiento es el de hoy.

**El coste, medido**: rehacer todas las colas en cada iteración evaluó
1.352.160 vecinos contra 1.362.270 en el mismo tiempo, **menos del 1 %**. La
versión bruta del arreglo ya es casi gratis; una incremental correcta sería
gratis del todo, y es lo siguiente si esto gana.

**Celdas**: `control` y `fulltails`. **Endpoint primario**: Wilcoxon pareado
por las 21, frontera de Pocock **simétrica** p <= 0.0142 en las seis
mirillas. **Regla del filtro**: descartar si la media de `fulltails − control`
sobre las cuatro instancias supera +2.0.

**Pendiente de mirar, fuera del bucle**: si la línea de intervalos
(`NeighbourhoodIJSP_*`, la del paper de COR) tiene el mismo defecto en su
mantenimiento de colas. Aquí está medido en el árbol crisp; allí no lo he
mirado, y no se afirma nada.

### I-004, filtro: pasa, y con el signo a favor

`iter/I-004/filter_analysis.txt`, 48 trabajos, cero infactibles. Medias de 30
runs, `fulltails` menos `control`: ta30 **−4.10**, ta29 −1.33, ta45 −1.30,
ta23 +1.13; media **−1.40**. La regla descartaba por encima de +2.0, así que
pasa.

Lo que no mide el filtro pero conviene anotar, porque el objetivo de la línea
es un récord y los récords viven en la cola inferior: **el mejor de las 30
tiradas también baja** en tres de las cuatro, ta23 1567 contra 1573, ta30
1603 contra 1607 y ta45 2021 contra 2024. Es descriptivo y no decide nada,
pero es la primera vez en cuatro iteraciones que el mejor mejora a la vez que
la media.

Aviso repetido por tercera vez: la magnitud del filtro **no** anticipa la de
la confirmación. I-001 leyó −2.75 y luego −0.90, I-002 −1.44 y luego +1.52,
I-003 +1.43 y luego −1.02. Este −1.40 no dice nada sobre lo que dirán las 21.

### I-004, cierre: el estimador estaba roto y arreglarlo no cambia nada

Seis mirillas completas, 30 tiradas por celda, 21 instancias, 1.260 tiradas,
cero infactibles. `fulltails` menos `control` por mirilla: **+2.02, +0.84,
+0.92, +1.23, +0.33, +0.01**. En la sexta: media **+0.01**, mejor en 13 de
21, W = 103.5, p = 0.677. No cruza la frontera de Pocock y **no hay séptima
mirada**: por el criterio predeclarado, **descartada**.

En la cola inferior tampoco hay nada: el mejor de las 30 tiradas mejora en 9
instancias, empeora en 11 y empata en 1.

**Lo que queda medido, que es el resultado y no el veredicto**: el estimador
`heads&tails` viola la cota inferior en el **64,8 %** de las evaluaciones por
culpa de las colas obsoletas, y arreglarlo del todo, pasando a 0 violaciones
de 1,35 millones, **no mueve el makespan final ni una unidad**. Dos hechos
que hay que sostener juntos:

1. **El defecto existe.** No es una opinión sobre heurísticas: la poda del
   tabú es incorrecta tal como está, descarta movimientos cuyo valor real es
   mejor, y el orden del vecindario está mal. Medido, no razonado.
2. **A la búsqueda le da igual.** Con 1.260 tiradas, el efecto sobre el
   resultado es una centésima de unidad. La conclusión es que este algoritmo
   es **insensible a cuál de los buenos vecinos de N2 elige**, y no que el
   defecto sea inocuo por casualidad.

Eso convierte la intuición del PI en una pregunta más afilada, y es la que
queda viva: si elegir *mejor* dentro de N2 no cambia nada, lo que puede
cambiar algo es elegir **deliberadamente distinto**. No un orden más
correcto, sino uno perturbado a propósito, o un desempate con criterio
propio. El fallo llevaba haciendo esa perturbación por accidente en dos
tercios de los casos, y el resultado era el mismo, lo cual acota también la
esperanza de esa vía.

**Decisión sobre el arreglo, separada del veredicto.** `localsearch.tails =
full` se queda disponible y documentado, pero **no** pasa a ser el valor por
defecto. La razón no es el rendimiento, que cuesta menos del 1 %, sino la
continuidad de la referencia: la celda `control` es el patrón del bucle y se
ha reproducido cuatro veces a media décima de unidad (I-001, I-002, I-003,
I-004). Cambiar el comportamiento por defecto rompería esa comparabilidad a
cambio de un beneficio medido como nulo. Se revisará si alguna idea futura
necesita que la cota sea válida, por ejemplo cualquiera que use la estimación
como guía en vez de como filtro. Queda anotado para que la decisión no se
pierda.

**Qué se revierte**: nada de los registros, y del código solo el volcado por
evaluación a `stderr` que se usó para el diagnóstico. Los contadores
(`N2 ties at best`, `N2 estimate above real value`, vetos de meseta,
reemplazos de explorador) se quedan: son inertes y son la única forma de
volver a ver esto.

### I-005 — desempate dirigido por frecuencia entre los empatados de N2

**Hipótesis** (una frase): entre los movimientos de N2 que empatan en el
mejor makespan, tomar el arco que **menos veces se ha usado en la tirada**, en
vez del primero que el orden coloque, da un makespan final menor a igual
tiempo de reloj.

De dónde sale, y esta vez de tres medidas propias:

1. **N2 empata a menudo y el empate lo rompe el azar.** Contado con los
   contadores de I-004: entre **1,5 y 2,0** vecinos elegibles empatados en el
   mejor valor de media, con máximos de **27**, y `isBetterThan` es estricto,
   así que gana el primero del orden. El orden lo fija un quicksort de
   **pivote aleatorio**. Hoy esa decisión la toma el generador de números
   aleatorios.
2. **Elegir mejor no sirve.** I-004 reparó la cota inferior, de 882.481
   violaciones a 0, y el resultado se movió **+0.01** unidades sobre 1.260
   tiradas.
3. **Perturbar el orden al azar tampoco.** El propio defecto de las colas
   llevaba desordenando el vecindario en dos tercios de los casos, y el
   resultado era el mismo. Lo que **no** se ha probado nunca es una elección
   con criterio, y es lo que el PI viene señalando desde el principio.

**Qué se toca**: `LS_Tabu`. Un contador de usos por arco, y entre los
empatados se toma el de menor cuenta. Es **memoria de frecuencias a largo
plazo**, la pieza de la familia TSAB que `TabuList` no tiene: la lista tabú
prohíbe, pero no recuerda cuánto se ha usado cada arco. **Ningún parámetro
numérico nuevo**: el criterio es un mínimo, sin umbral que ajustar.

**Por qué la celda lleva dos líneas y no una.** Para ver a todos los
empatados el barrido no puede cortarse en una estimación *igual* al mejor
valor, y eso solo es correcto si la estimación es cota inferior, es decir
exige `localsearch.tails = full`. La atribución queda limpia precisamente
porque I-004 midió esa mitad por separado y dio un cero exacto: cualquier
efecto aquí es del desempate.

**Comprobado antes de correr** (2026-09-22, 4 instancias, 3 clases, 4
tiradas): horarios factibles, las dos celdas difieren en las cuatro
instancias, la celda nueva tiene **0** violaciones de la cota, y el contador
de empates sube de ~1,2 a entre 1,4 y 2,0, que es la prueba de que el barrido
ahora **alcanza** los empatados que la poda hacía invisibles.

**Celdas**: `control` y `freqtie`. **Endpoint primario**: Wilcoxon pareado por
las 21, frontera de Pocock **simétrica** p <= 0.0142 en las seis mirillas.
**Regla del filtro**: descartar si la media de `freqtie − control` sobre las
cuatro instancias supera +2.0.

**Si sale cero**, y hay que decirlo de antemano: con I-004 y con el accidente
de las colas ya medidos, un tercer cero aquí cerraría la vía del orden de los
vecinos de N2 con tres medidas independientes, y el foco tendría que irse a
qué se hace con el movimiento elegido, no a cuál se elige.

### I-005, descartada en el filtro, y una cola que apunta al revés

`iter/I-005/filter_analysis.txt`, 48 trabajos, cero infactibles. Medias de 30
runs, `freqtie` menos `control`: ta45 **+7.70**, ta23 +4.20, ta29 +0.60, ta30
−2.30; media **+2.55**. La regla preinscrita descartaba por encima de +2.0.
**Descartada**, sin oleadas. La regla se aplica literalmente y no se discute
después de verla.

**Pero la cola inferior se mueve al contrario, y hay que anotarlo**: el mejor
de las 30 tiradas mejora en dos de las cuatro, y en una de ellas mucho. En
`ta30`, **1595** contra 1607 del control, doce unidades mejor, y el mejor
valor que este régimen corto ha producido en esa instancia en toda la línea
(BKS 1584, o sea a 11). En `ta23`, 1571 contra 1574. En `ta29` y `ta45` la
cola empeora.

Eso no cambia el veredicto, y decirlo no es reabrirlo: el endpoint es la
media y estaba fijado. Lo que señala es un hueco del protocolo que la
revisión externa del 2026-09-21 ya identificó y que yo no había cerrado: **el
objetivo de la línea es un récord, y un récord vive en la cola inferior,
mientras todos los criterios que hemos usado miden un desplazamiento de la
media**. Un mecanismo que empeora la media y ensancha la cola hacia abajo es
exactamente lo que un cazador de récords querría, y nuestro filtro lo tira.

**Cambio de protocolo, declarado ahora y por tanto válido solo para lo que
venga** (I-006 en adelante), no aplicable retroactivamente a I-005:

- Toda tanda reporta, **además** del endpoint de la media, dos endpoints de
  cola predeclarados: el **mejor de las 30 tiradas** por instancia, pareado
  por instancia con un contraste de signos; y el **déficit esperado del 10 %
  inferior** de las tiradas, o sea la media de las tres peores... de las tres
  **mejores**, que en minimización es la media de los tres valores más bajos.
- La **regla del filtro no cambia** y sigue siendo solo sobre la media, para
  que siga siendo barata y solo descarte.
- Una idea descartada por la media **cuyos endpoints de cola mejoren** no se
  acepta, pero se anota en el historial con la etiqueta `[COLA]` y entra en
  el backlog como candidata a **intento de récord** (B-6), que es el marco
  donde afinar sobre las propias instancias es legítimo.

`I-005` recibe esa etiqueta retroactivamente **solo como anotación**, sin
efecto sobre su veredicto: `[COLA]` por `ta30` 1595.

**Qué se revierte**: nada de los registros. El código del desempate se queda,
inerte, porque `localsearch.tiebreak` es `first` por defecto y la
configuración vigente no lo menciona. Igual que con `jsp.seeded`,
`abc.scout = kick` y `localsearch.tails = full`: cuatro mecanismos medidos y
disponibles, ninguno encendido.

### I-006 — intento de récord, que es el objetivo y no una idea

**No hay hipótesis que aceptar ni rechazar, y por eso no hay filtro ni
oleadas.** El objetivo de la línea es batir o igualar una mejor solución
conocida, y la prueba de un récord es el propio horario. Cinco iteraciones
midiendo desplazamientos de la media han dado cinco ceros (I-001 a I-005), y
lo que I-005 dejó a la vista es que el criterio estaba mirando al sitio
equivocado: el récord vive en la **cola inferior**.

**Lista corta**, por distancia de nuestro mejor verificado al BKS: `ta29`
(igualado una vez en 1625 en el régimen largo; a 3 en este), `ta30` (BKS
1584, mejor de este régimen **1595**, a 11), `ta23` (a 14) y `ta22` (a 13).
Se dejan fuera las 30x20, que están a 47-65 y donde 75 tiradas no compran
nada.

**Presupuesto**: 75 tiradas por celda e instancia, 600 tiradas en total, 8.3
h-CPU, en 120 trabajos de 5 tiradas para que ninguno pase de unos minutos.
Presupuesto por tirada el de la clase, 40 s, que es el que el análisis de
reinicios señala para la cola inferior en 20x20.

**Dos celdas, y la elección de la segunda se declara**: `control` es la
configuración vigente. `freqtie` es la celda que I-005 **descartó por la
media** y cuya cola fue al contrario, 1595 contra 1607 en `ta30`. Elegirla por
eso es selección sobre los datos, y en un intento de récord es legítimo
justamente porque **no se infiere nada**: un récord es un horario, no una p.
Queda escrito para que nadie tenga que adivinarlo. De paso, el intento duplica
como los datos extra que la pregunta de la cola necesita.

**Endpoints, predeclarados y en este orden**:

1. **Récord**: cualquier tirada cuyo makespan verificado quede en el BKS o por
   debajo. Se imprime el horario y el índice de la tirada de todo lo que quede
   a dos unidades o menos, para poder perseguir un casi.
2. **Mejor de las 75** por instancia y celda, y su distancia al BKS.
3. **Media de los tres valores más bajos**, el sustituto de déficit esperado
   que el protocolo pide desde I-006.
4. **Contraste de signos** entre celdas sobre el mejor de bloques de 5
   tiradas consecutivas, que es lo que un intento de récord consume de
   verdad.

Reportada y sin valor decisorio: la media por instancia, para poder poner esta
tanda al lado de las cinco anteriores.

**Qué pasa si sale un récord**: se verifica el horario con
`verify_certificate.py`, se guarda el certificado, y la prueba es él. Nada de
lo que haya aquí se incorpora a la configuración vigente por este resultado:
un récord no valida un mecanismo, y `freqtie` sigue descartada por la media.

### I-006, cierre: sin récord, un casi a dos unidades y un mejor propio

600 tiradas verificadas, cero infactibles, 120 trabajos, de 05:37 a 06:12.
`iter/I-006/attempt_analysis.txt`; los horarios de los mejores valores están
guardados y reverificados uno a uno en `iter/I-006/evidence/`.

**1. Récord: no.** Un casi: `ta29` en **1627** con el control, BKS 1625, a
**+2**. Nada más entró en la ventana de dos unidades.

**2. Mejor de las 75, y contra nuestro propio mejor histórico:**

| inst | BKS | control | freqtie | mejor propio anterior | ¿mejora? |
|---|---|---|---|---|---|
| ta22 | 1600 | 1613 | 1613 | 1613 | iguala |
| ta23 | 1557 | **1564** | 1570 | 1571 | **sí, −7** |
| ta29 | 1625 | 1627 | 1630 | 1625 (régimen largo) | no |
| ta30 | 1584 | 1604 | 1597 | 1588 (régimen largo) | no |

**`ta23` = 1564 es un mejor propio nuevo**, siete unidades por debajo de
1571, verificado desde su horario y con el certificado guardado. Sigue a 7 del
BKS, así que **no es un récord**, y se anota como lo que es.

**3. Media de los tres más bajos**: `freqtie` mejor en `ta30` (1599.7 contra
1606.7) y `ta22`, peor en `ta23` y `ta29`.

**4. Contraste de signos sobre el mejor de bloques de 5**: `freqtie` mejor en
26 bloques, peor en 30, empatados 4; **p = 0.689**. Plano.

**Lo que esto resuelve, y es la razón de haber corrido las dos celdas**: la
ventaja de cola de `freqtie` que I-005 dejó a la vista **no generaliza**. Se
repite en dirección en `ta30`, la única instancia donde se vio, y desaparece
al agregar cuatro instancias y 600 tiradas. Era un efecto de una instancia
más ruido. La etiqueta `[COLA]` de I-005 queda **retirada**: el cambio de
protocolo que trajo se queda, porque medir la cola sigue siendo lo correcto
para este objetivo, pero la candidatura de `freqtie` a intento de récord se
cierra aquí.

**Dónde deja la caza.** `ta29` a +2 con 75 tiradas de 40 s es el sitio más
cerca al que hemos llegado en el régimen corto, y nuestro histórico ya igualó
1625 una vez con 300 s. Las dos cosas juntas dicen que en `ta29` el récord
está al alcance de más volumen, no de otro mecanismo. Un intento concentrado
solo en `ta29`, con todo el presupuesto en una instancia en vez de repartido
en cuatro, es el siguiente paso natural del backlog (B-6) y no necesita
ninguna idea nueva.

### I-007 — todo el presupuesto en `ta29`, y la duración de tirada contra el mínimo

**Por qué `ta29` y solo `ta29`.** I-006 la dejó a **dos unidades**: 1627
contra un BKS de 1625, con 75 tiradas de 40 s. Y la línea ya igualó 1625 una
vez, en el régimen de 300 s. Es el punto más cerca al que hemos estado, y
repartir el presupuesto entre cuatro instancias fue lo que hizo I-006. Aquí va
entero a una.

**Y la pregunta que aprovecha el viaje.** El análisis de reinicios eligió 40 s
para 20x20 **por la media**. Un récord no vive en la media, vive en el
mínimo, y **qué duración de tirada produce el mínimo más bajo a igual CPU
total no se ha medido nunca**. Nuestro 1625 salió de 300 s; el mejor de 75
tiradas de 40 s es 1627. Los dos datos sueltos apuntan en direcciones
distintas y esto los enfrenta:

| celda | s por tirada | tiradas | h-CPU |
|---|---|---|---|
| `b040` | 40 | 200 | 2.22 |
| `b100` | 100 | 80 | 2.22 |
| `b300` | 300 | 27 | 2.25 |

Misma instancia, misma configuración congelada, mismo CPU total, una sola cosa
distinta. **No hay filtro ni oleadas**: no se acepta nada.

**Endpoints, predeclarados y en este orden**:

1. **Récord**: cualquier makespan verificado en 1625 o por debajo. Se imprime
   el certificado y el trozo de todo lo que quede en 1627 o menos.
2. **El mínimo por celda**, con su distancia al BKS y el CPU gastado.
3. **Media de los tres más bajos** por celda, y la cola entera hasta BKS+5
   para ver su forma.
4. **Cuántas tiradas de cada celda quedan en o por debajo de cada umbral**
   desde BKS hasta BKS+5. Es la pregunta de la duración dicha como le importa
   a una caza de récords: a igual CPU, qué presupuesto pone más masa abajo.

Reportado y sin valor decisorio: media, desviación y peor valor por celda.

**Qué se hace con el resultado.** Si sale un récord, el certificado es la
prueba y se guarda. Si no, el endpoint 4 decide **dónde se gasta el siguiente
intento**, y esa es la parte que vale aunque no haya récord: los presupuestos
por clase del protocolo son intocables desde el bucle y seguirán siendo los
de la media, pero un intento de récord podrá declarar el suyo con esta medida
detrás en vez de por inercia.

### I-007, cierre: sin récord, igualada verificada, y la duración de tirada resuelta

307 tiradas en `ta29`, cero infactibles, 70 trabajos, de 06:24 a 07:04.
`iter/I-007/attempt_analysis.txt`.

**Récord: no.** Nada quedó **estrictamente** por debajo de 1625.

**Igualada: sí, y verificada.** La celda `b300` produjo un **1625**, que
iguala el BKS. El horario está en
`iter/I-007/evidence/ta29_1625_I-007_b300_c01.csv` y `verify_certificate.py`
lo confirma por su cuenta: *"matches the best known solution 1625"*. La línea
ya había igualado 1625 antes, en el régimen largo, así que esto lo **reproduce**
y no lo amplía.

**Corrección de mi propio análisis, antes de las cifras.** La primera versión
de `analyze.py` imprimió esa igualada como `*** RECORD ***` y remató con
`RECORD FOUND`. La condición era `value <= BKS` para la etiqueta de récord, lo
que dejaba inalcanzable la rama de igualdad. **Un récord es batir el BKS, no
igualarlo.** Corregido en el script y anotado aquí, porque el error iba en la
dirección de sobrevender el resultado y ese es justo el que hay que dejar
escrito.

**La duración de tirada, a igual CPU, sobre `ta29`:**

| celda | s/tirada | tiradas | mínimo | distancia | ≤1625 | ≤1628 | ≤1630 | media |
|---|---|---|---|---|---|---|---|---|
| `b040` | 40 | 200 | 1627 | +2 | 0 | 2 | 5 | 1640.5 |
| `b100` | 100 | 80 | 1627 | +2 | 0 | 4 | 6 | 1639.8 |
| `b300` | 300 | 27 | **1625** | **+0** | **1** | 2 | 3 | 1639.4 |

Y aquí está el resultado que vale aunque no haya récord: **las tres medias son
indistinguibles, 1640.5 contra 1639.8 contra 1639.4, y los mínimos no lo
son.** Solo el presupuesto largo llegó a 1625, con **siete veces menos
tiradas**. El presupuesto elegido por la media es el equivocado para cazar un
récord, y ahora está medido en la instancia que importa en vez de supuesto.

Matiz honesto: es **una** instancia y **una** tirada afortunada de 27. Con
n = 1 en la celda que gana, esto no es un contraste, es un indicio fuerte
sobre dónde gastar. Lo que autoriza es declarar 300 s en el próximo intento de
récord, no reescribir los presupuestos por clase, que siguen intocables desde
el bucle y siguen siendo los de la media.

**Lo siguiente, sin idea nueva**: un intento con todo el presupuesto en
`ta29` a 300 s por tirada. Con 7 h-CPU son 84 tiradas, tres veces las 27 de
aquí, en la única configuración que ha tocado 1625 dos veces.

### I-008 — `ta29` a 300 s, 84 tiradas, y nada más

**Sin idea nueva. Es la caza, y la dirección la fijó una medida.** I-007 midió
en `ta29`, a igual CPU total, que 40 s por 200 tiradas y 100 s por 80 se
quedan en 1627 mientras 300 s por 27 llega a **1625**, y que las tres medias
son indistinguibles. El presupuesto afinado sobre la media es el equivocado
para un récord. Esta tanda gasta las 7 h-CPU enteras en la única
configuración que ha tocado 1625 **dos veces**: 84 tiradas de 300 s, el triple
de lo que I-007 le dio.

**Semillas nuevas**, de la 101 a la 184, para que sean tiradas nuevas y no
casi-repeticiones de las 1 a 27 de I-007.

**Un récord es un makespan estrictamente por debajo de 1625.** Igualar 1625 es
una igualada, y esta línea ya lo ha hecho dos veces. La distinción costó un
titular mal etiquetado en I-007 y está escrita en el `analyze.py` de esta
iteración para que no vuelva a pasar.

**Endpoints**: cualquier cosa en 1627 o menos con su trozo, para poder sacar
y reverificar el horario; el mínimo de la tanda; la cola hasta 1630 y la media
de los tres más bajos; y los recuentos por umbral **al lado de la celda de
300 s de I-007**, para poder juntar por ojo las dos tandas de la misma
configuración, que sumarán 111 tiradas de 300 s sobre `ta29`.

**Qué se aprende si no sale.** Con 111 tiradas de 300 s y dos igualadas pero
ningún 1624, la lectura sería que `ta29` está en una barrera dura en 1625 para
esta configuración, y que batirla pide algo distinto del volumen. Eso cerraría
la caza por fuerza bruta en esta instancia y devolvería el foco al mecanismo,
pero con un dato que ahora no tenemos.

### I-008, cierre: la fuerza bruta en `ta29` se cierra, y I-007 se rebaja

84 tiradas de 300 s en `ta29`, 7.0 h-CPU, cero infactibles.
`iter/I-008/attempt_analysis.txt`.

**Sin récord y esta vez sin igualada.** El mínimo de la tanda es **1628**,
a +3. La cola hasta 1630: 1628, 1629, 1629, 1630, 1630, 1630.

**Rebaja de la conclusión de I-007, y es lo importante de esta tanda.** Ayer
escribí que el presupuesto largo da el mínimo más bajo a igual CPU, apoyado en
que la celda de 300 s llegó a 1625 con 27 tiradas mientras 40 s y 100 s se
quedaban en 1627. Con 84 tiradas más de la misma configuración, **el 1625 no
se repite**. Era una tirada afortunada, y yo ya había avisado de que era n = 1,
pero la conclusión estaba escrita con más peso del que soportaba. Puesto en
aciertos por hora de CPU, que es la comparación que I-007 debió hacer:

| celda | h-CPU | ≤1625 /h | ≤1627 /h | ≤1628 /h | ≤1630 /h |
|---|---|---|---|---|---|
| 40 s x 200 | 2.22 | 0.00 | 0.45 | 0.90 | 2.25 |
| 100 s x 80 | 2.22 | 0.00 | 0.45 | 1.80 | 2.70 |
| 300 s x 111 | 9.25 | 0.11 | 0.11 | 0.32 | 0.97 |

En todos los umbrales salvo el único suceso de 1625, los presupuestos corto y
medio producen **más** aciertos por hora de CPU. La afirmación "el
presupuesto largo es el bueno para el mínimo" **no está establecida**, y la
elección de 40 s por la media no queda refutada. Lo que queda es que la única
vez que se tocó 1625 fue con 300 s, con una frecuencia de 1 en 111.

**Y lo que sí queda establecido, que era el otro resultado previsto.** Entre
I-007 e I-008 hay **111 tiradas de 300 s** sobre `ta29` con **una** igualada y
**ningún** 1624. Sumando las 200 de 40 s y las 80 de 100 s, son **391 tiradas
verificadas** sobre esta instancia sin un solo valor por debajo de 1625.
`ta29` está contra una barrera dura en 1625 para esta configuración, y
batirla **no es cuestión de volumen**. La caza por fuerza bruta en `ta29` se
cierra aquí, y con un número detrás en vez de por cansancio.

**Dónde deja el bucle.** El foco vuelve al mecanismo, pero con el mapa mucho
mejor dibujado que al empezar: de dónde arranca un individuo no importa
(I-001, I-003), cuál de los buenos vecinos de N2 se elige tampoco (I-004,
I-005), un vecindario más rico no ayuda (H-4, reproducido en I-002), y el
volumen no rompe la barrera de `ta29` (I-006, I-007, I-008). Lo que ninguna
iteración ha tocado todavía es **qué se hace con el movimiento una vez
elegido**: la profundidad efectiva de la trayectoria, que H-5 cerró solo como
parámetro global y que sigue abierta en su forma asimétrica (B-9), y las
familias que la revisión externa señaló y que exigen salir del vecindario
completo, reparación exacta sobre una ventana (B-12) o búsqueda en espacio de
soluciones parciales.

### I-009 — una llamada profunda sobre el incumbente al estancarse

**Hipótesis** (una frase): a igual tiempo de reloj, disparar **una** llamada
profunda al tabú sobre el incumbente la primera vez que la tirada pasa diez
generaciones sin mejorar su mejor global da un makespan final menor.

**Por qué es lo único que queda del mapa.** Ocho iteraciones han cerrado casi
todo: de dónde arranca un individuo no importa (I-001, I-003), cuál de los
buenos vecinos de N2 se elige tampoco (I-004, I-005), un vecindario más rico
no ayuda (H-4, reproducido en I-002), y el volumen no rompe la barrera de
`ta29` (I-006 a I-008). Lo que ninguna ha tocado es **qué se hace con el
movimiento una vez elegido**. Medido el 2026-09-21: la búsqueda local son unas
242 zambullidas por generación, cada una muriendo a las 15 iteraciones sin
mejora tras 26 a 41 movimientos en total. **Nunca hay una sola trayectoria
profunda**, y eso es justo lo que distingue a TSAB, que cruza regiones peores
durante cientos o miles de movimientos.

**Por qué no lo contesta ya H-5.** H-5 cerró la profundidad como **parámetro
global**: irace la barrió entre 5 y 40 y eligió 15. Pero en ese espacio la
profundidad era **una sola para toda la población**, así que subirla encarecía
las 242 llamadas a la vez y el afinado estaba eligiendo en una curva de
compromiso. El reparto **desigual**, corto para la población y profundo para
el incumbente, no era expresable ahí. Es lo que esto prueba.

**Qué se toca**: `ArtificialBeeColonyPSO` y un par de accesores en `LS_Tabu`.
Una línea de setup, `abc.deepls = 10`. El disparador (10 generaciones) y la
profundidad (1000 iteraciones sin mejora contra las 15 congeladas) quedan
**fijados de antemano y no se ajustan**; el tope de 2 s por llamada se deja
como está. Parámetros del ABC congelados.

**Dispara una sola vez por tirada, y eso es diseño, no pereza.** Si se
repitiera en cada episodio de estancamiento, en el peor caso se comería la
mitad del presupuesto y la comparación a igual tiempo de reloj mediría el
gasto en vez de la idea. Una vez por tirada acota el coste en un 5 % de una
tirada de 40 s y deja la prueba interpretable.

**Comprobado antes de correr** (2026-09-22, 4 instancias, 3 clases, 4
tiradas): horarios factibles, las dos celdas difieren en las cuatro
instancias, y el contador nuevo `Deep LS calls` marca exactamente **1** por
tirada en la celda y **0** en el control. Las iteraciones medias por llamada
suben un poco en la celda (26.21 a 26.23 en `ta29`, 39.83 a 40.76 en `ta41`),
que es la firma de que la profundidad se usa. Después de I-003 no vuelvo a
preinscribir nada sin poder demostrar que la rama se ejecuta.

**Celdas**: `control` y `deep`. **Endpoint primario**: Wilcoxon pareado por las
21, frontera de Pocock **simétrica** p <= 0.0142 en las seis mirillas.
**Endpoints de cola**, obligatorios desde I-006: mejor de las 30 por instancia
y media de los tres más bajos. **Regla del filtro**: descartar si la media de
`deep − control` sobre las cuatro instancias supera +2.0.

### La profundidad del tabú está limitada por el callejón sin salida, no por su parámetro (2026-09-22)

Este es el hallazgo del que sale I-010, y reencuadra dos entradas del
historial. Se llegó a él por tres rediseños fallidos de I-009, cada uno
forzado por un número y no por una intuición, y los tres habrían producido un
cero que yo habría anotado como "la profundidad no ayuda".

**Primer intento (I-009).** Una llamada profunda por tirada. El filtro dio
−0.02. Midiendo qué fracción del trabajo del tabú hacía esa llamada,
despejada del aumento en iteraciones medias: entre el **0.06 % y el 2.3 %**.
Retirada por infradimensionada, no por su signo.

**Segundo intento.** Cuota del 25 % del tiempo de búsqueda local en vez de una
llamada. La cuota alcanzada fue el **0.03 %**: la puerta de "una vez por
episodio de estancamiento" limitaba las oportunidades, no el coste, y los
episodios son pocos (1.3 en `ta29`, 7.7 en `ta41` por tirada). Confundí
limitar oportunidades con limitar coste.

**Tercer intento.** Que la cuota sea el único límite. Subió a 40-150 llamadas
por tirada y la cuota seguía en el **0.3 %**. La razón es de escala: una
iteración de tabú cuesta ~1.7 us porque la poda evalúa uno o dos vecinos, así
que las zambullidas cortas suman del orden de **20 millones de iteraciones por
tirada** y mil movimientos son el 0.005 % de eso.

**Cuarto intento.** Profundidad a 30.000 movimientos. **No cambió nada**: ni la
cuota ni las iteraciones medias. Ahí estaba el hallazgo, y hubo que
instrumentar el porqué:

| instancia | llamadas profundas | iteraciones por llamada | mueren por callejón | por tope de tiempo |
|---|---|---|---|---|
| ta29 | 52 | 75 | 93 de 93 | 0 |
| ta41 | 158 | 92 | 217 de 217 | 0 |

**Todas mueren en callejón sin salida, tras 75 a 92 movimientos.** Llegan a un
estado donde **ningún** vecino es admisible: todo movimiento de bloque crítico
es tabú sin cumplir la aspiración, o es el inverso del último. `LS_Tabu` trata
eso como fin de llamada (`this->badIterations = this->maxBadIterations`).

**Consecuencias, que son tres y ninguna es pequeña:**

1. **`localsearch.bad-iterations` tiene un techo efectivo en ~80.** Por encima
   de eso da igual lo que valga: 15, 1.000 o 30.000 producen la misma
   trayectoria porque muere antes. El barrido de irace entre 5 y 40 (H-5) cayó
   entero por debajo del techo, así que **H-5 sigue siendo válido para el
   rango que barrió**, pero no dice nada sobre profundidades mayores, que eran
   inalcanzables por construcción.
2. **Explica el fracaso del back-jump (H-1).** Devolver la búsqueda a un punto
   mejor no puede ayudar si la trayectoria nunca se aleja: con un techo de 80
   movimientos no hay excursión desde la que volver.
3. **El escape clásico funciona, y mucho.** Tomar el mejor vecino aunque sea
   tabú cuando no hay ninguno admisible, en vez de terminar, lleva las
   trayectorias de **47-102 a 286-1909 movimientos** por llamada, y la cuota
   declarada del 25 % pasa a morder de verdad: 14.0 % en `ta29`, **25.3 %** en
   `ta41`, con 21.333 y 282.854 escapes por tirada.

### I-010 — escapar del todo-tabú, y con ello probar la profundidad

**Hipótesis** (una frase): a igual tiempo de reloj, permitir que el tabú salga
de un estado todo-tabú tomando el mejor movimiento de todos modos, y gastar
hasta el 25 % del tiempo de búsqueda local en trayectorias profundas sobre el
incumbente, da un makespan final menor.

**Tres celdas en el filtro, que es donde van las variantes**, y encajadas una
en otra para que la atribución sea limpia:

| celda | qué añade |
|---|---|
| `control` | nada, la configuración congelada |
| `escape` | solo `localsearch.deadend = escape` |
| `deepescape` | el escape **más** la cuota del 25 % en llamadas profundas |

Así el filtro separa lo que aporta el escape por sí solo de lo que aporta la
profundidad que el escape habilita. **Dos celdas en la confirmación**,
`control` y `deepescape`, que es el mecanismo completo y el endpoint primario.

**Endpoint primario**: Wilcoxon pareado por las 21, frontera de Pocock
**simétrica** p <= 0.0142 en las seis mirillas. **Endpoints de cola**,
obligatorios desde I-006. **Regla del filtro**: descartar si la media de
`deepescape − control` sobre las cuatro instancias supera +2.0.

**Regla nueva del protocolo, que estas cuatro rondas han ganado**: ninguna
idea se preinscribe sin un **contador que demuestre que el mecanismo recibe la
parte del cómputo que la hipótesis supone**. Verificar que la rama se ejecuta
(lección de I-003) no basta; hay que verificar cuánto pesa. Aquí ese contador
es `Deep LS share of LS time %`, y sin él habría lanzado cuatro filtros y
anotado cuatro ceros.

### I-010, filtro: pasa, y su secundario señala al escape y no a la profundidad

`iter/I-010/filter_analysis.txt`, 72 trabajos, tres celdas, cero infactibles.
Medias de 30 runs:

| instancia | `control` | `escape` | `deepescape` |
|---|---|---|---|
| ta23 | 1587.8 | **1584.6** | 1586.6 |
| ta29 | 1640.8 | 1641.0 | 1641.9 |
| ta30 | 1623.6 | **1618.5** | 1619.1 |
| ta45 | 2043.0 | 2044.0 | 2044.2 |
| media − control | — | **−1.78** | **−0.86** |

La regla descartaba por encima de +2.0 sobre `deepescape`, así que **pasa**.

**Lo que dice el secundario, y lo que no voy a hacer con él.** El escape por sí
solo sale mejor que el escape con la cuota de profundidad encima, en tres de
las cuatro instancias y en la media. Si se sostiene, el ingrediente activo es
**salir del estado todo-tabú**, y la profundidad que el escape habilita no
aporta o estorba. Es una hipótesis atractiva y por eso mismo **no cambio el
endpoint primario**: sigue siendo `deepescape` contra `control` en la frontera
de Pocock. Dejar que el secundario de un filtro redirija una decisión
preinscrita es exactamente lo que este protocolo existe para impedir, y ya
llevo cuatro iteraciones donde la magnitud del filtro no anticipó la de la
confirmación.

**Lo que sí hago, declarado ahora y antes de que exista un solo dato de
oleada**: la celda `escape` viaja en las oleadas como **información**. Cuesta
9.2 h-CPU por oleada en vez de 6.1, unos 50 minutos en vez de 40, y contesta
en la misma tanda la pregunta que el filtro acaba de abrir. No se infiere nada
de ella más allá de una diferencia reportada: no tiene frontera, no puede
aceptarse y no puede rechazarse. Si `deepescape` se rechaza y `escape` se ve
mejor, eso será una preinscripción nueva con su propia confirmación, no una
conclusión de aquí.

### I-011 `[COLA]` — cartera de configuraciones neutras

**Primera iteración declarada de cola.** Su endpoint primario es la media de
los mejores por bloques de 5, y se le permite empeorar la media.

**Hipótesis** (una frase): si cada tirada de una campaña sortea su
configuración entre varias que ya sabemos **neutras en media**, la cola
inferior se alarga sin que la media se mueva, y el mejor de cada bloque de 5
tiradas mejora.

**Por qué debería funcionar, y por qué es casi aritmética.** Una mezcla de
componentes tiene por media la media de las medias, así que si cada componente
es neutro la mezcla también lo es. Pero su varianza es la varianza **dentro**
de los componentes **más** la varianza **entre** ellos. Si los componentes
recorren trayectorias distintas, la mezcla tiene cola más larga que cualquiera
de ellos por construcción. Es el mismo mecanismo que el estudio de siembra ya
documentó en el problema de intervalos con otras palabras: *un pool mezclado
arranca peor y acaba mejor que sus componentes*.

**De dónde salen los componentes, y esto es lo bonito**: de las diez
iteraciones anteriores. Cada una dejó un interruptor implementado, verificado
y **medido como neutro en media sobre las 21 instancias**, y ninguno está
encendido. La cartera los usa todos a la vez, sorteando uno de cada par por
tirada:

| interruptor | apagado | encendido | dónde se midió neutro |
|---|---|---|---|
| creación | `jsp.random` | `jsp.seeded` (pool `mix`, k = 25) | I-001: −0.90, p = 0.348 |
| colas | incremental | `localsearch.tails = full` | I-004: +0.01, p = 0.677 |
| explorador | aleatorio | `abc.scout = kick` | I-003: −0.03 a +0.44 en 4 mirillas |
| callejón | `stop` | `localsearch.deadend = escape` | I-010, **condicional** |

**Regla fijada ahora sobre el cuarto**: el interruptor del callejón entra en la
cartera **si y solo si** queda medido como neutro en media sobre las 21. La
composición no se decide mirando ningún dato de I-011.

**Resuelta (2026-09-22), en dos pasos y con la segunda medida mandando.** Al
cerrar I-010 quedó **fuera**: lo que aquella frontera confirmó neutro era la
*combinación* escape+profundidad, no el interruptor suelto, y el interruptor
suelto daba 15 de 21 a favor, que no parece neutro. Después I-012 lo midió
solo y con su propia frontera sobre las 21: **+0.07, p = 0.835**. Es neutro
como la regla pide, así que **entra**. La decisión anterior era la correcta
con lo que se sabía entonces; esta la sustituye con una medida mejor.

**Cobertura de las combinaciones, anotada por honestidad**: con 30 tiradas en
trozos de 2, el filtro recorre las combinaciones **0 a 14** y se deja la 15,
la de los cuatro interruptores encendidos a la vez. Las seis oleadas, con
trozos de 1 tirada y el índice avanzando, **sí cubren las 16**. El filtro solo
descarta, así que la ausencia de una de dieciséis no afecta a ninguna
decisión, pero queda dicho.

**Excluido explícitamente**: `localsearch.tiebreak = frequency`. Su única
medida es el filtro de I-005, que dio **+2.55** y descartó. Nunca se midió
sobre las 21, así que su neutralidad **no está establecida** y no puede entrar
en una cartera cuya propiedad clave es heredar la neutralidad de sus partes.

**Qué se toca**: **ni una línea del solver.** La cartera es un asunto de
generación de trabajos: la celda `portfolio` reparte sus 30 tiradas por
instancia entre las 8 o 16 combinaciones, y se agrupan al analizar. El
control son 30 tiradas de la configuración congelada. Mismo presupuesto,
mismo número de tiradas, mismo tiempo de reloj.

**Celdas**: `control` y `portfolio`.

**Endpoint primario** (de cola): la **media de los mejores por bloques de 5
tiradas** por instancia, Wilcoxon pareado por las 21, frontera de Pocock
simétrica p <= 0.0142 en las seis mirillas.

**Reportados y sin valor decisorio**: la media por instancia, que **puede
empeorar**; el mejor absoluto de las 30; y la desviación típica por celda e
instancia, que es la comprobación de mecanismo de esta iteración. Si la
dispersión de `portfolio` no sube respecto al control, la mezcla no está
mezclando y la hipótesis no llega a probarse, igual que pasó con la cuota de
I-009. **Ese contador va mirado antes de interpretar el endpoint.**

**Regla del filtro**: se descarta si la media de los mejores por bloques de
`portfolio` menos la de `control`, promediada sobre las cuatro instancias del
filtro, supera **+2.0** unidades. Sobre el endpoint de cola, no sobre la
media.

### I-010, cierre: la profundidad no aporta, y la celda informativa apunta a otra cosa

Seis mirillas, 30 tiradas por celda, 21 instancias, 1.890 tiradas, cero
infactibles. `deepescape` menos `control` por mirilla: **+0.16, −0.32, −0.13,
−0.06, −0.41, −0.25**. En la sexta: media **−0.25**, mejor en 10 de 21,
p = 0.639. No cruza la frontera: **descartada**.

**Este cero no es como los anteriores, y por eso vale.** Los de I-001 a I-005
eran de mecanismos que no llegaban a actuar o actuaban donde no importaba.
Aquí los contadores dicen que el mecanismo actuó con toda la fuerza pedida:
cientos de miles de escapes por tirada, trayectorias de **286 a 1909
movimientos** en vez de 47 a 102, y la cuota mordiendo en el 25 % declarado.
**La profundidad de la trayectoria tabú, que es lo que distingue a TSAB, no
aporta nada dentro de este ABC.** Es un resultado con contenido propio: junto
con I-004 dice que esta búsqueda tolera que su estimador viole la cota en dos
tercios de los casos y tolera que sus trayectorias sean cuarenta veces más
largas, sin que el makespan final se entere.

**La celda informativa, y lo que no se concluye de ella.** El escape **solo**
queda en media **−1.10** y mejor en **15 de 21**, contra el 10 de 21 del
mecanismo completo. Es un patrón mejor que el del endpoint primario. No se
concluye nada: la celda entró sin frontera y declarada como información, y la
preinscripción decía literalmente que si esto pasaba sería *"una preinscripción
nueva con su propia confirmación, no una conclusión de aquí"*. Se cumple.

**Consecuencia sobre la composición de I-011, resuelta explícitamente.** La
regla preinscrita decía que el interruptor del callejón entra en la cartera si
I-010 cierra sin cruzar su frontera, *"es decir si queda medido como neutro en
media sobre las 21"*. Las dos mitades de esa frase apuntan ahora a sitios
distintos: la frontera de I-010 se aplicó a `deepescape`, no al escape solo, y
el escape solo **no parece neutro**. Lo que quedó confirmado neutro es la
combinación, no el interruptor. Por tanto **el escape no entra en la cartera**,
y la razón es la intención escrita de la regla y no su letra. Queda anotado
así para que se vea que la letra y la intención divergieron y cuál se siguió.

**Y cambia el orden**: I-011 depende de saber si el escape es neutro o
beneficioso, así que **I-012 va antes**, con el escape solo como idea de
posición y su propia confirmación. Si sale neutro, entra en la cartera. Si
sale beneficioso, pasa a la configuración vigente y la cartera se construye
sin él.

### I-012 — el escape solo, esta vez con frontera

**De dónde sale, y por qué necesita su propia tanda.** La celda informativa de
I-010 dio, sobre las 21 instancias y 30 tiradas, media **−1.11**, mejor en
**15 de 21**, p = **0.0325**. Es la señal más fuerte que ha producido
cualquier celda en once iteraciones. Y no vale como resultado por dos razones
que hay que decir juntas: entró **sin frontera**, declarada como información,
y se miró **porque destacaba**, que es selección sobre los datos. Un p de
0.033 escogido entre las celdas que llamaron la atención no es un p de 0.033.

**Hipótesis** (una frase): a igual tiempo de reloj, permitir que la búsqueda
tabú salga de un estado todo-tabú tomando el mejor movimiento de todos modos,
en vez de terminar la llamada, da un makespan final menor.

**Qué se toca**: una línea, `localsearch.deadend = escape`. **Nada más**: sin
llamadas profundas, sin cuota, sin ningún parámetro. Es la mitad de I-010 que
la otra mitad estaba tapando.

**Semillas independientes**, 201 a 230, no las 1 a 30 de I-010. La
confirmación tiene que ser una muestra nueva: reutilizar las mismas tiradas
que motivaron la sospecha sería medir dos veces el mismo ruido.

**Por qué es plausible más allá del número.** El escape es la salida clásica
del estado todo-tabú y está en la familia TSAB desde siempre; lo que I-010
midió es que sin él las trayectorias mueren en torno al movimiento 80, y que
con él llegan a 286-1909. I-010 probó que **la profundidad** que eso habilita
no aporta. Queda la otra mitad: que lo que aporta no sea la profundidad sino
**no terminar la llamada**, es decir que el valor esté en seguir buscando desde
donde se estaba en vez de reiniciar la llamada desde el siguiente individuo.
Son dos mecanismos distintos y I-010 solo refutó uno.

**Celdas**: `control` y `escape`. **Endpoint primario**: Wilcoxon pareado por
las 21, frontera de Pocock **simétrica** p <= 0.0142 en las seis mirillas.
**Endpoints de cola**, obligatorios desde I-006. **Regla del filtro**:
descartar si la media de `escape − control` sobre las cuatro instancias del
filtro supera +2.0.

**Comprobación de coherencia añadida al procedimiento** (2026-09-22): antes de
generar nada se verifica que la lista de celdas del generador y la del
analizador son idéntica. Dos veces en esta sesión derivé ficheros de la
iteración anterior con sustituciones de texto y las listas quedaron
descolgadas: en I-005 los setups salieron sin la línea de la idea, y en I-010
el análisis leyó dos celdas de las tres que se corrieron. Son dos líneas de
comprobación y se me escapó dos veces.

### I-012, filtro: pasa, y aparece la primera igualada de `ta30`

`iter/I-012/filter_analysis.txt`, 48 trabajos, cero infactibles. Medias de 30
runs, `escape` menos `control`: ta45 −4.03, ta23 −3.53, ta30 −2.13, ta29
+2.43; media **−1.82**. La regla descartaba por encima de +2.0: **pasa**.

**Lo que importa más que el filtro.** En la columna de mejores aparecen dos
valores que igualan su BKS, los dos verificados de forma independiente
recomputando el makespan desde el horario, y los dos con el certificado
guardado en `iter/I-012/evidence/`:

| instancia | valor | BKS | celda | qué es |
|---|---|---|---|---|
| `ta30` | **1584** | 1584 | `control` | **iguala, y es la primera vez de la línea** |
| `ta29` | 1625 | 1625 | `escape` | iguala, tercera vez |

**`ta30` = 1584 es el mejor resultado que este proyecto ha producido en esa
instancia.** El anterior era 1588, cuatro unidades peor, y venía del régimen
largo; en el régimen corto lo mejor había sido 1595. **No es un récord**:
récord es batir el BKS y aquí se iguala.

**Y sale de la celda `control`**, es decir de la configuración congelada sin
tocar nada. No es evidencia a favor del escape y no se cuenta como tal.

**Lo que sí dice, y corrige una conclusión de I-006.** En I-006 el intento de
récord dio en `ta30` un mejor de 1604 con 75 tiradas de 40 s, y de ahí salió
la idea de que en el régimen corto esa instancia estaba a 20 del BKS. Ahora 30
tiradas del mismo régimen han dado 1584. La cola inferior de `ta30` es **mucho
más pesada** de lo que 75 tiradas sugerían, y eso reabre la instancia como
objetivo de intento de récord: si 30 tiradas llegan al BKS, batirlo puede
estar a una tanda de volumen, que es justo lo contrario de lo que concluimos
para `ta29` en I-008.

### I-012, cierre: la señal de I-010 no se reprodujo, y eso es el resultado

Seis mirillas, 30 tiradas por celda, 21 instancias, 1.260 tiradas, cero
infactibles. `escape` menos `control` por mirilla: **+1.30, +1.09, +0.59,
+0.27, +0.39, +0.07**. En la sexta: media **+0.07**, mejor en 10 de 21,
p = 0.835. **Descartada.**

**El contraste con lo que motivó la tanda:**

| | media | mejor en | p |
|---|---|---|---|
| celda informativa de I-010, 21 inst. x 30 runs | −1.11 | **15** de 21 | **0.0325** |
| I-012, mismas instancias, **semillas nuevas** | **+0.07** | 10 de 21 | 0.835 |

Mismo mecanismo, mismo presupuesto, mismas 21 instancias, mismo número de
tiradas. Lo único distinto son las semillas. **La señal desapareció entera.**

Esto es el argumento del protocolo demostrado sobre un caso propio, y conviene
dejarlo escrito porque es el tipo de cosa que en un artículo se publica sin
saberlo. Aquel p de 0.0325 salió de una celda que viajaba **sin frontera**,
declarada como información, y que se miró **porque destacaba entre tres**. Un
p escogido así no es un p: es el mínimo de varios, y su distribución no es la
que el contraste supone. Costó una tanda de hora y media comprobarlo. Sin el
protocolo habría costado una afirmación falsa en un paper.

**Consecuencia sobre I-011, ahora sí resuelta con una medida y no con una
interpretación.** Al cerrar I-010 dejé el interruptor del callejón **fuera** de
la cartera, razonando que lo confirmado neutro era la combinación y no el
interruptor, y que el interruptor no parecía neutro. Ahora el interruptor
tiene su propia confirmación con frontera sobre las 21 instancias:
**+0.07, p = 0.835**. Es neutro en media, medido como pide la regla. **Entra
en la cartera.** Queda anotado que la decisión anterior era la correcta con la
información de entonces y que esta la sustituye con una medida mejor, no que
aquella fuera un error.

**Composición final de la cartera de I-011**, cuatro interruptores, 16
combinaciones, todos con neutralidad **confirmada sobre las 21 instancias**:

| interruptor | encendido | confirmación |
|---|---|---|
| creación | `jsp.seeded` (pool `mix`, k = 25) | I-001: −0.90, p = 0.348 |
| colas | `localsearch.tails = full` | I-004: +0.01, p = 0.677 |
| explorador | `abc.scout = kick` | I-003: −0.03 a +0.44, 4 mirillas |
| callejón | `localsearch.deadend = escape` | **I-012: +0.07, p = 0.835** |

### I-011, primer filtro anulado por colisión de ficheros (2026-09-22)

La cola salió con **0** y el análisis se paró en seco: `ta23 ... is
infeasible`. Dos problemas, y el segundo es peor que el primero.

**Qué pasó.** La celda `portfolio` reparte sus 30 tiradas en 15 trozos, cada
uno con su combinación, y los mandé **todos al mismo directorio de
resultados**. El solver nombra su salida `<inst>_<AAAAMMDDHHMMSS>_*.csv`, con
resolución de **un segundo**. Quince trabajos de la misma instancia arrancando
a la vez colisionan en el nombre y se sobrescriben. De **60 trabajos
quedaron 25 certificados**, y 4 de esos estaban a medio escribir y no
verifican. El control, con un trabajo por instancia, salió intacto.

**Por qué la cola no lo vio.** `queue_jobs.sh` verifica, por cada línea de
trabajo, **el certificado más reciente** de su par (tag, instancia). Con 15
líneas apuntando al mismo par, verificó quince veces el mismo fichero y nunca
los otros. Hasta ahora cada par tenía exactamente un trabajo y la verificación
era completa por accidente. **`queue exit 0` no garantiza nada cuando varios
trabajos comparten directorio**, y eso queda escrito en el protocolo.

**Lo que salvó la tanda** fue que el análisis recomputa **todos** los
makespans desde sus horarios, no una muestra. Es la tercera vez en la sesión
que esa regla detecta algo que ninguna otra comprobación habría visto: antes
fueron los horarios infactibles de N8 y el contador de reemplazos que nunca
contaba.

**Descartado como causa, y comprobado**: las 15 combinaciones se corrieron
por separado sobre `ta23` con 3 tiradas cada una y **todas dan horarios
factibles**. No hay ninguna interacción rota entre los interruptores; el fallo
era de mi generación de trabajos.

**Arreglo**: un tag de resultados por trozo, `I-011_<tanda>_p<NN>_<celda>`, de
modo que el par (tag, instancia) es único. Verificado sobre el filtro y sobre
la oleada 1: 64 y 126 trabajos, 64 y 126 pares distintos. Los resultados
corruptos quedan como `I-011_voidfilter_*`, no se borran, y el filtro se
relanza entero.

### I-011, filtro: pasa la regla, pero el mecanismo declarado no se movió

64 trabajos, 64 certificados, cero infactibles. Esta vez el par (tag de
resultados, instancia) era único y el análisis leyó las 240 tiradas que había
que leer.

**La comprobación de mecanismo va primero, como estaba preinscrito, y sale
plana.** La cartera dispersa más que el control en **2 de 4** instancias, razón
media **1.03**. La mezcla no está ensanchando nada. Era toda la hipótesis: una
mezcla de componentes neutros conserva la media y suma varianza entre
componentes, alargando la cola por construcción. Con razón 1.03 esa varianza
entre componentes es indistinguible de cero, así que **la hipótesis no llegó a
ejercerse**, y eso es lo primero que hay que saber al leer el endpoint.

**El endpoint, aun así, se movió a favor.** Media de los mejores por bloques de
cinco, cartera − control:

| instancia | bo5 control | bo5 cartera | d | media control | media cartera | d |
|---|---|---|---|---|---|---|
| ta23 | 1576.2 | 1574.2 | −2.00 | 1586.8 | 1584.0 | −2.80 |
| ta29 | 1635.7 | 1632.7 | −3.00 | 1640.8 | 1638.2 | −2.63 |
| ta30 | 1613.7 | 1606.8 | **−6.83** | 1623.1 | 1618.4 | −4.70 |
| ta45 | 2029.0 | 2030.7 | +1.67 | 2039.7 | 2041.2 | +1.50 |

Cola **−2.54**, regla del filtro *descartar si > +2.0*: **pasa**. La media se
movió **−2.16**, y esta iteración tenía permiso para empeorarla, así que ese
número no decide nada. El filtro no tiene frontera: **solo descarta**, y no
descartó. Nada más.

**De dónde no sale el movimiento.** Tres comprobaciones, todas sobre datos ya
en disco:

1. **Ningún interruptor lo carga.** Cada trozo de la cartera lleva una
   combinación fija, así que se puede desglosar. Contrastes activado−desactivado
   sobre las 15 combinaciones: semilla −0.48, colas −0.41, patada −0.28, escape
   **+1.83**. Ninguno explica −2.16, y el único grande va en contra.
2. **La combinación 0 es la configuración congelada**, es decir el control
   mismo, y midió −0.97 frente al control. Un desplazamiento que incluye a la
   celda idéntica al control no es un efecto del tratamiento.
3. **No hay deriva dentro del proceso**, que era la explicación estructural
   candidata: el control es un trabajo de 30 tiradas y la cartera quince de 2,
   y como el presupuesto es de reloj, una degradación con el índice de tirada
   habría regalado exactamente este desplazamiento. Pendiente medida
   **+0.011** unidades por tirada, **+0.31** en las treinta. Descartada.

**Lo que sí explica la tabla por combinación es su propio ruido.** La
combinación 0 corre las semillas 1 y 2 con la configuración del control, y las
tiradas 1 y 2 del propio control usan esas mismas semillas: difieren en **0.9
unidades**. El presupuesto es de reloj de pared, así que una tirada vale lo que
alcance a calcular y **ni la misma semilla reproduce el mismo número**. Con 2
tiradas por celda la dispersión esperada de las 15 medias es ~2.8 y la
observada es **3.13**. Esa tabla no resuelve nada y no se interpreta.

Queda entonces un −2.16 agregado a unas 1.9 veces su error típico, con el
mecanismo declarado plano. Eso no es un hallazgo; es exactamente el estado en
el que un filtro debe mandar la idea a las oleadas, que es donde está la
frontera. **Va a oleadas, con la nota de que si cruza habrá que explicar por
qué cruzó, porque la explicación preinscrita ya no sirve.**

**Cambio de troceado para las oleadas, solo de reparto.** En el filtro el
control era **un** trabajo de 30 tiradas y tardó 75 minutos ocupando una
ranura mientras las otras trece se vaciaban: la tanda entera duró lo que su
trabajo más largo. Desde la oleada 1 ambas celdas se trocean igual, una tirada
por trabajo. No cambia ni la configuración ni las semillas, baja la oleada de
~41 a ~29 minutos, y de paso deja las dos celdas **simétricas en troceado**,
que era la única asimetría estructural que quedaba entre ellas — medida
inocua arriba, pero mejor no tenerla.

### I-011, cierre: no cruzó, y el mecanismo declarado no se movió ni una vez

Seis oleadas, 1260 tiradas de confirmación, 30 por celda e instancia, cero
infactibles. Endpoint de cola, media de los mejores por bloques de cinco:

| mirilla | tiradas/celda | bo5 cartera−control | mejor en | p | dispersión |
|---|---|---|---|---|---|
| 1 | 5 | +1.43 | 7 de 21 | 0.574 | 1.03 |
| 2 | 10 | −0.43 | 12 de 21 | 0.602 | 1.03 |
| 3 | 15 | −1.46 | **15** de 21 | 0.106 | 1.14 |
| 4 | 20 | −0.79 | 14 de 21 | 0.213 | 1.09 |
| 5 | 25 | −0.79 | 13 de 21 | 0.192 | 1.04 |
| 6 | 30 | **−0.32** | 10 de 21 | **0.777** | 1.05 |

Frontera de Pocock 0.0142, simétrica. **No cruza, ni a favor ni en contra:
descartada.** La media informada se movió −0.32, y esta iteración tenía
permiso para empeorarla, así que tampoco decide.

**Lo que hay que leer no es el rechazo, es la última columna.** La razón de
dispersión cartera/control empieza en 1.03, sube a 1.14 en la tercera mirilla
y vuelve a 1.05. En siete lecturas independientes —el filtro sobre 4
instancias y seis mirillas sobre 21— **la cartera nunca dispersó más que el
control de forma apreciable**. La hipótesis era aritmética: una mezcla de
componentes neutros conserva la media y **suma la varianza entre
componentes**, así que su cola inferior es más larga por construcción. La
aritmética es correcta; lo que falla es la premisa empírica de que los cuatro
interruptores producen **distribuciones distintas**. Si los cuatro dan
esencialmente la misma distribución, la varianza entre componentes es cero y
la mezcla es la propia configuración congelada. Eso es exactamente lo que
midieron I-001, I-003, I-004 e I-012 al declararlos neutros, y yo leí
"neutro en media" donde los datos decían algo más fuerte.

**La lección, que vale más que la iteración**: *neutro en media* y
*equivalente en distribución* no son lo mismo, pero en estos cuatro casos
resultaron serlo, y una cartera solo puede alargar la cola si sus componentes
difieren. **Antes de volver a proponer una mezcla hay que comprobar primero
que los componentes tienen distribuciones distintas**, que es una medida
barata sobre datos ya existentes, y no darla por supuesta.

**Y el filtro queda en su sitio.** Dio −2.54 a favor con el mecanismo ya
plano; las seis mirillas dieron −0.32. Es la **segunda vez** en este bucle que
una medida sin frontera no se reproduce con una: antes fue la celda
informativa de I-010 (−1.11, 15 de 21, p = 0.0325) contra I-012 (+0.07, 10 de
21, p = 0.835). Dos de dos. El filtro está para descartar barato, **no para
sugerir hallazgos**, y cuando la mirilla 3 dio 15 de 21 —la misma forma que
engañó a I-010— la frontera hizo su trabajo y esperó.

**Sin código que revertir**: I-011 era cuestión de generación de trabajos y no
tocó el solver. Los cuatro interruptores siguen implementados y desactivados
por defecto, como estaban.

**Sin récords**: `iter/I-011/records.txt`, mejor por instancia sobre las 1260
tiradas, ninguna por debajo ni igualando su BKS. Era de esperar, porque las
oleadas corren con los presupuestos por clase (40/100/150 s) y no con los 300
s de los intentos de récord.

### Sondeo del barrido de N2: se ve el 20 %, y una de cada cuatro estimaciones miente

Medido 2026-09-23 con dos contadores nuevos (`N2 neighbours offered`,
`N2 fraction of neighbourhood scanned %`), una tirada por instancia con la
configuración congelada:

| | ta23, 40 s | ta45, 150 s |
|---|---|---|
| vecinos que N2 ofrece por iteración del tabú | 11.28 | 12.27 |
| **fracción del vecindario evaluada** | **20.08 %** | **19.79 %** |
| estimación **por encima** del valor real | 1.12 M de 4.52 M = **24.7 %** | 4.33 M de 15.94 M = **27.2 %** |
| empates en el mejor por iteración | 1.16 | 1.28 |
| iteraciones por llamada | 27.3 | 37.1 |

Dos hechos que el bucle no tenía escritos.

**El barrido ve una quinta parte del vecindario.** La poda
(`localsearch.filter = yes`) corta en el primer vecino cuya estimación no
mejora al mejor valor real encontrado, y con el vecindario ordenado por
estimación eso ocurre tras **2.3 vecinos de 11.5**. El tabú no elige entre
once movimientos, elige entre dos o tres.

**Y por defecto la poda es insegura.** La estimación está *por encima* del
valor real en una de cada cuatro evaluaciones, así que no es cota inferior y
el corte puede descartar —y descarta— vecinos cuyo valor real era mejor. Esto
solo se arregla con `localsearch.tails = full`, que es justo lo que midió
I-004: **882.481 violaciones a 0**.

**Por qué esto cierra la vía del *cuál* y abre la del *cómo*.** I-004 hizo que
el barrido viera el mejor de verdad y el resultado se movió **+0.01** en 1260
tiradas. Es decir: la elección dentro de N2 estaba equivocada el 25 % de las
veces, corregirla no cambió nada. Súmese a que el defecto de las colas
desordenaba el vecindario en dos tercios de los casos sin efecto, y a que el
desempate por frecuencia (I-005) dio +2.55. **Cuatro medidas independientes
dicen que da igual cuál de los vecinos de N2 se coja.**

Las cuatro comparten una premisa que ninguna ha tocado: **la regla sigue
siendo *coge el mejor***. Con cota exacta y regla del mejor, el orden de
visita es matemáticamente irrelevante, y por eso las cuatro dieron cero. El
orden solo puede decidir algo si se abandona la regla. Eso es I-013.

### I-013, descartada en el filtro: el orden importa, y el algoritmo ya está en el lado bueno

240 trabajos, cero infactibles, 30 tiradas por celda e instancia.

**El mecanismo se ejerció, y con holgura**, que es lo primero que había que
comprobar: la rama de primera mejora se lleva el **69.7 %** de los
movimientos, el barrido pasa de evaluar el 20 % del vecindario a evaluar el
55 %, y las iteraciones del tabú por llamada **suben** de 26-42 a 39-61,
porque al mejorar más a menudo se reinicia el contador de iteraciones
estériles. No es una idea que no llegara a probarse; es una idea que se probó
entera.

| instancia | media control | media first | d | bo5 control | bo5 first | d |
|---|---|---|---|---|---|---|
| ta23 | 1587.4 | 1616.6 | **+29.27** | 1576.7 | 1600.5 | +23.83 |
| ta29 | 1640.8 | 1650.0 | +9.20 | 1635.8 | 1643.0 | +7.17 |
| ta30 | 1623.2 | 1640.1 | +16.90 | 1613.7 | 1629.3 | +15.67 |
| ta45 | 2042.7 | 2073.5 | **+30.83** | 2030.8 | 2063.7 | +32.83 |

Media **+21.55**, regla *descartar si > +2.0*: **descarta**, por un factor de
diez y en las cuatro instancias.

**Y esta cifra vale más que el descarte.** Doce iteraciones midiendo ceros
—+0.01, +0.07, 0.9 unidades, planos— y la primera que toca **la regla de
decisión** en vez de los datos que la alimentan salta a +21.55. Es decir: el
orden en que se visitan los vecinos de N2 **sí decide el resultado, y decide
mucho**; lo que pasa es que la regla vigente, *coger el mejor*, ya está en el
extremo bueno de esa palanca, y vale unas **21 unidades de makespan** frente
a la alternativa natural.

**La atribución es limpia.** La celda cambia tres cosas —regla, orden y
poda—, pero las dos últimas existen solo para encontrar el mejor: bajo la
regla del mejor, ordenar y podar no cambian *qué* movimiento se toma salvo
por la cota rota, y eso es exactamente lo que I-004 midió en **+0.01** sobre
1260 tiradas. Las 21 unidades son de la regla.

**Qué queda dicho sobre la vía del orden de los vecinos**, que el PI venía
señalando desde el principio y que ahora tiene cinco medidas:

| medida | qué cambia | resultado |
|---|---|---|
| I-004 | el barrido ve el mejor **de verdad** | +0.01 |
| defecto de las colas | desordena el vecindario en 2/3 de los casos | sin efecto |
| I-005 | desempata entre los empatados en el mejor | +2.55 |
| sondeo 2026-09-23 | la elección está equivocada el 25 % de las veces | corregirla no cambia nada |
| **I-013** | **deja de coger el mejor** | **+21.55** |

La lectura conjunta ya no es "el orden no importa" sino algo más útil:
**dentro de la regla del mejor todo da igual, y fuera de ella se pierde
mucho**. La superficie es plana alrededor del óptimo voraz y cae en cuanto se
sale. Eso cierra la vía de *cuál vecino se coge* con una medida positiva y no
con un cero, y manda el foco a **qué se hace con el movimiento elegido** y a
**cuánto tiempo se le da a cada tirada**, que es lo que dicen a la vez la
revisión externa y nuestras propias medidas de reinicio.

**Qué se revierte**: nada de los registros. El interruptor
`localsearch.select` se queda implementado y **desactivado por defecto**, como
`tails`, `scout`, `deadend` y `tiebreak`, y la configuración congelada recorre
exactamente el mismo camino que antes.

### El riesgo de mejora sí decae con la edad del estancamiento, y la cola estéril es menor de lo que decía B-7

Medido 2026-09-23 sobre datos ya en disco, sin correr nada: las 120 tiradas
de control del filtro de I-013, cuatro instancias, configuración congelada,
presupuestos por clase. Cada traza por generación trae `Runtime` y
`Best Cmax`, así que el riesgo se cuenta directamente
(`scripts/stall_hazard.py`).

El **riesgo** a edad `a` es la fracción de generaciones observadas con `a`
segundos desde la última mejora del mejor global que producen una mejora
nueva:

| edad del estancamiento | ta23 | ta29 | ta30 | ta45 |
|---|---|---|---|---|
| < 1 s | 38.75 % | 45.91 % | 41.20 % | 32.73 % |
| 1-2 s | 22.54 % | 24.83 % | 22.43 % | 28.36 % |
| 2-4 s | 15.56 % | 10.89 % | 15.13 % | 17.15 % |
| 4-8 s | 8.62 % | 3.55 % | 7.41 % | 12.77 % |
| 8-16 s | 2.53 % | 1.08 % | 2.76 % | 7.66 % |
| 16-32 s | **0.00 %** | **0.00 %** | 0.85 % | 2.23 % |
| 32-64 s | -- | -- | -- | 0.82 % |

**La comprobación que pedía la revisión externa pasa**: el riesgo decae
monótonamente y en un orden de magnitud, y a partir de los 16 segundos en las
instancias de 40 s es **cero o casi**. El tiempo que se gasta ahí es
demostrablemente estéril, no es una impresión.

**Y la misma medida corrige a la baja la cifra con la que B-7 se justificaba.**
El backlog dice que la última mejora llega en la mediana al **0.32** del
presupuesto y que el **61.8 %** de la tirada se gasta después. Con los
presupuestos que de verdad se usan:

| | ta23 | ta29 | ta30 | ta45 |
|---|---|---|---|---|
| última mejora, mediana del presupuesto | 0.75 | 0.56 | 0.73 | 0.82 |
| fracción de la tirada gastada después | 25.3 % | 44.2 % | 26.6 % | 18.0 % |

Aquella cifra se midió a **300 s**; a 40 y 150 segundos la cola estéril es del
**18 al 44 %**, no del 62 %. El margen de la idea es menor de lo que el
backlog prometía, y hay que decirlo antes de gastar máquina, no después.

**Consecuencia sobre la forma de la idea.** B-7 proponía *terminar la tirada*
antes y dejar que el arnés arranque la siguiente. Eso choca con dos
intocables del protocolo —la definición de la métrica y los presupuestos por
clase—, porque cambia cuántas tiradas caben en un trabajo y con ello el
endpoint. La misma evidencia admite una forma que no toca ninguno de los dos:
**reiniciar la población dentro de la tirada** cuando el estancamiento alcanza
la edad a la que el riesgo ya es cero, conservando el mejor global, y seguir
hasta agotar el mismo presupuesto. Mismo presupuesto, mismo número de
tiradas, mismo endpoint, y el tiempo estéril se convierte en búsqueda nueva.
Eso es I-014.

### I-014 — reiniciar alrededor del incumbente, y dos correcciones antes de gastar máquina

**La idea.** Cuando pasan **0.2 del presupuesto** sin mejorar el mejor global,
se reconstruye la población con el incumbente más copias suyas perturbadas
—entre 1 y 10 mutaciones cada una— y la tirada sigue hasta agotar el mismo
presupuesto. Reiniciar **dentro** de la tirada, y no acortarla, es lo que
permite no tocar ninguno de los intocables: mismo presupuesto por clase, mismo
número de tiradas, mismo endpoint.

**Ninguna constante se ajusta, las dos salen de una medida.** El 0.2 sale del
riesgo de estancamiento por una regla enunciada: *el primer borde en que el
riesgo cae por debajo de la décima parte de su valor en el primer segundo*, que
da 0.20 en ta23 y ta30, 0.10 en ta29 y 0.107 en ta45; se toma el más
conservador. El abanico de 1 a 10 se queda por debajo del alcance de una
llamada del tabú, medido en 26-41 movimientos, para que la perturbación no la
deshaga la búsqueda local que viene detrás.

**Primera corrección: el umbral estaba donde no pasaba nada.** Lo puse en 0.4,
la edad a la que el riesgo llega a **cero**. Pero para acumular 60 s de
estancamiento en una tirada de 150 la última mejora tiene que caer antes de los
90 s, y su mediana está en 123. La comprobación previa lo enseñó: el mecanismo
se disparó **una vez en cuatro tiradas** y los makespans salieron **idénticos**
a los del control. Un umbral donde no ocurre nada no mide nada, que es lo que
I-009 costó cuatro rediseños. Corregido a 0.2 **antes** de lanzar.

**Segunda corrección, y más seria: la forma fría de la idea se retira.** El
reinicio original reconstruía con el operador de creación. Las trazas que ya
estaban en disco dicen que eso no puede pagar
(`scripts/cold_catchup.py`, 120 tiradas de control):

| desde frío | ta23 | ta29 | ta30 | ta45 |
|---|---|---|---|---|
| al 20 % del presupuesto | +55.0 | +27.5 | +34.5 | +55.0 |
| al 40 % | +21.5 | +4.5 | +10.0 | +22.0 |
| al 60 % | +8.0 | +1.0 | +6.0 | +6.5 |
| al 80 % | +2.5 | +0.0 | +1.0 | +0.0 |

Una población fría necesita el **80 % del presupuesto** para ponerse a unas
pocas unidades de lo que la tirada acaba consiguiendo. Y el reinicio por
estancamiento se dispara **tarde por construcción**, porque exige 0.2 del
presupuesto de estancamiento *después* de la última mejora, que llega en la
mediana al 0.56-0.82. Le queda justo el tramo donde una población fría está más
atrás. **No es que la idea fuera mala, es que esa forma era imposible**, y se
retira igual que I-009, sin gastar las 4.5 horas de CPU del filtro. Poblar
alrededor del incumbente cuesta cero en recuperación, así que lo que se somete
a prueba pasa a ser la diversidad y no un lastre.

**La implementación está verificada, no supuesta.** Con el reinicio templado la
comprobación previa sigue dando makespans iguales en una tirada por instancia,
así que fui a la traza: las dos celdas son idénticas hasta t = 54 s y luego
divergen, **354 generaciones contra 372**. El reinicio ocurre y cambia la
trayectoria; lo que pasa en esa tirada es que encontró su respuesta final a los
50 s de 150 y los tres reinicios posteriores no mejoraron nada. Una tirada por
instancia no concluye; para eso está el filtro.

**Celdas**: `control` y `restart`. **Endpoint primario**: la media por
instancia, Wilcoxon pareado por las 21, frontera de Pocock simétrica
p <= 0.0142 en seis mirillas. **Regla del filtro**: descartar si la media de
`restart − control` sobre las cuatro instancias supera +2.0. **Comprobación de
mecanismo, primero**: reinicios por tirada por encima de cero, y generaciones
por tirada para ver qué cuestan las reconstrucciones.

### I-014, filtro: pasa, y el mecanismo por fin se ejerce

240 trabajos, cero infactibles, 30 tiradas por celda e instancia.

**Comprobación de mecanismo, leída primero y esta vez satisfactoria**: 1.36
reinicios por tirada de media (1.07 en ta23, 1.90 en ta29, 1.27 en ta30, 1.20
en ta45) contra 0.00 en el control. Con el umbral de 0.4 eran 0.25; con 0.2 el
mecanismo ocurre de verdad. Y **las reconstrucciones casi no cuestan**: las
generaciones por tirada suben en lugar de bajar (90.4 → 101.8 en ta23, 228.5 →
243.5 en ta45) y las iteraciones por llamada del tabú se mueven menos del 3 %.
Repoblar alrededor del incumbente es barato, como se esperaba de no tener que
recuperar terreno.

| instancia | media control | media restart | d | bo5 control | bo5 restart | d |
|---|---|---|---|---|---|---|
| ta23 | 1587.5 | 1587.0 | −0.50 | 1576.8 | 1576.8 | +0.00 |
| ta29 | 1641.1 | 1640.2 | −0.90 | 1636.5 | 1634.0 | −2.50 |
| ta30 | 1623.3 | 1623.6 | +0.37 | 1613.7 | 1614.0 | +0.33 |
| ta45 | 2042.2 | 2040.4 | −1.77 | 2029.3 | 2029.3 | +0.00 |

Media **−0.70**, regla *descartar si > +2.0*: **pasa**.

**Y eso es literalmente todo lo que dice.** −0.70 sobre cuatro instancias está
dentro del ruido —el error típico agregado del filtro ronda 1.0— y este bucle
ya lleva **dos** medidas sin frontera que no se reprodujeron con una: la celda
informativa de I-010 (−1.11, p = 0.0325) contra I-012 (+0.07, p = 0.835), y el
filtro de I-011 (−2.54) contra sus seis mirillas (−0.32). El filtro descarta
barato; **no sugiere hallazgos**. Deciden las oleadas.

### I-014, cierre: el tiempo estéril no se recupera ni reiniciando gratis

Seis oleadas, 1260 tiradas de confirmación, 30 por celda e instancia, cero
infactibles. Media por instancia, `restart − control`:

| mirilla | tiradas/celda | media | mejor en | p | reinicios/tirada |
|---|---|---|---|---|---|
| filtro | 30 (4 inst.) | −0.70 | -- | -- | 1.36 |
| 1 | 5 | −0.70 | 13 de 21 | 0.186 | 1.16 |
| 2 | 10 | −0.37 | 12 de 21 | 0.271 | 1.11 |
| 3 | 15 | −0.15 | 9 de 21 | 0.805 | 1.06 |
| 4 | 20 | −0.17 | 11 de 21 | 0.664 | 1.06 |
| 5 | 25 | −0.10 | 10 de 21 | 0.702 | 1.05 |
| 6 | 30 | **−0.16** | 10 de 21 | **0.516** | 1.01 |

Frontera de Pocock 0.0142, simétrica. **No cruza: descartada.** Las 21
diferencias finales están todas entre −1.67 y +1.30; ninguna instancia se
separa del cero.

**Esta vez el cero es limpio, y por eso vale.** Es la primera idea del bucle
que llegó a las oleadas con las dos mitades de la comprobación de mecanismo en
verde: se dispara en **todas** las mirillas, entre 1.01 y 1.36 veces por
tirada, y **no le quita trabajo al tabú** —las generaciones por tirada
incluso suben, porque repoblar alrededor del incumbente no obliga a recuperar
terreno—. No es una idea que no llegara a probarse ni una que costara más de lo
que daba. Se ejerció entera y no hace nada.

**Lo que dice, junto con lo anterior.** El riesgo de mejora se mide en ~0 %
durante el 18-44 % final de cada tirada, así que ese tiempo es de verdad
estéril. I-014 le ofreció a ese tramo la mejor salida disponible sin tocar la
métrica: arrancar de nuevo **desde el incumbente**, con diversidad y sin coste
de recuperación. No sirvió. Luego el incumbente de una tirada estancada está en
una cuenca de la que **ni la perturbación de 1 a 10 movimientos ni la búsqueda
que la sigue** sacan nada mejor en el tiempo que queda. Es coherente con I-003
(la patada del explorador, plana), con I-010 e I-012 (escapar del todo-tabú,
plano) y con I-013 (salir de la regla voraz, +21.55): **todo lo que se ha
intentado para mover la búsqueda fuera de donde ella misma se asienta, o no
hace nada o empeora.**

**Y la corrección que se hizo antes de lanzar queda justificada a posteriori.**
La forma fría —reconstruir con el operador de creación— se retiró porque las
trazas decían que no podía alcanzar al incumbente en lo que quedaba de
presupuesto. La forma templada, que no tenía ese problema, da cero. La fría
habría dado cero **o peor**, y habría costado las mismas 4.5 horas de filtro
más, probablemente, seis oleadas para medir algo que ya se sabía.

**Qué se revierte**: nada de los registros. El interruptor `abc.restart`
queda implementado y **desactivado por defecto**, como los anteriores; con él
apagado, la configuración congelada recorre el mismo camino que antes salvo
una lectura de reloj por generación.

**Sin récords**: `iter/I-014/records.txt`, el mejor de las 1260 tiradas por
instancia, ninguno igualando ni bajando su BKS; el más cerca es `ta29` a 1629
contra 1625. Esperable con los presupuestos por clase.

### I-015 — levantar el veto de meseta

**La idea** es B-14, que estaba medida desde el 21 de septiembre y nunca se
había probado. En `ArtificialBeeColonyPSO` hay dos sitios —el reemplazo tras el
cruce y la escritura lamarckiana del tabú— donde un descendiente que mejora a
su individuo **hasta igualar exactamente** el makespan del incumbente se
descarta y se cuenta como intento fallido. El interruptor
`abc.plateau = allow` quita esa condición en los dos. **Cero parámetros.**

**Por qué ahora.** I-014 acaba de cerrar con el cero más limpio del bucle y una
conclusión: el incumbente de una tirada estancada está en una cuenca de la que
ni una perturbación de 1 a 10 movimientos ni la búsqueda que la sigue lo sacan.
En JSP las mesetas al valor del incumbente son enormes y **moverse de lado por
ellas es precisamente cómo se sale de una cuenca**. El veto prohíbe ese
movimiento. Es un filtro de duplicados hecho sobre el fitness en lugar de sobre
el genotipo: rechaza soluciones **distintas** por empatar y admite clones con
makespan distinto.

**El riesgo contrario, declarado antes**: el veto puede estar haciendo trabajo
real como filtro de duplicados tosco, y sin él la población puede llenarse de
soluciones al valor del incumbente y perder diversidad. Si eso pasa, el filtro
lo verá como un empeoramiento.

**Comprobado antes de lanzar** (ta29 y ta41, una tirada por celda):

| | ta29 control | ta29 allow | ta41 control | ta41 allow |
|---|---|---|---|---|
| vetos en la búsqueda local | 1304 | 0 | 99 | 0 |
| vetos en el cruce | 93 | 0 | 14 | 0 |
| admitidos en la búsqueda local | 0 | **1898** | 0 | **74** |
| admitidos en el cruce | 0 | **287** | 0 | **94** |
| generaciones | 95 | 91 | 195 | 181 |

El interruptor convierte los vetos en admisiones, los horarios son factibles y
las celdas difieren en las dos instancias. Las generaciones bajan un 4-7 %:
conservar esos movimientos cuesta algo de trabajo, y eso entra en la
comprobación de mecanismo.

**Una verificación de paso sobre el binario**, porque su tamaño salía idéntico
build tras build: las claves nuevas `abc.plateau` y `abc.restart` están dentro,
pero `localsearch.select` no aparece como cadena contigua. Tampoco
`localsearch.tails`, `localsearch.deadend` ni `localsearch.filter`, que es parte
de la configuración congelada y funciona seguro: las cuatro se construyen en la
lista de inicialización del constructor y GCC las emite como inmediatos (14
`movabs` en `LocalSearch.o`). No afecta a I-013, cuyo mecanismo ya lo demostró
disparándose al 69.7 %.

**Y un defecto de documentación en I-014, corregido.** Su `analyze.py`
conservaba en el docstring el párrafo de mecanismo de I-013: la sustitución que
debía reemplazarlo usaba un `replace` sin comprobar, no encontró el texto
—faltaba el `a) ` inicial en el patrón— y falló en silencio. **El código estaba
bien** —la lista de estadísticas y el aviso eran los correctos, y son los que
imprimieron `stall restarts per run` en las seis mirillas—, así que ningún
resultado cambia; solo el texto describía otra idea. Los generadores de
ficheros usan desde ahora una sustitución que **falla si no encuentra el
patrón**.

**Celdas**: `control` y `allow`. **Endpoint primario**: la media por
instancia, Wilcoxon pareado por las 21, frontera de Pocock simétrica
p <= 0.0142 en seis mirillas. **Regla del filtro**: descartar si la media de
`allow − control` sobre las cuatro instancias supera +2.0. **Mecanismo,
primero**: admisiones por tirada en la búsqueda local, y generaciones por
tirada para ver qué cuestan.

### I-015, filtro: pasa con el resultado más favorable del bucle, y eso obliga a más cautela, no a menos

240 trabajos, cero infactibles, 30 tiradas por celda e instancia.

**Mecanismo, primero, y en verde en sus dos mitades.** La celda `allow` admite
por tirada 2457, 5252, 2010 y 2394 movimientos de meseta en la búsqueda local
(ta23, ta29, ta30, ta45) y entre 265 y 426 en el cruce; unos **3000 por
tirada** que el control tira. Y **no cuestan**: las generaciones por tirada
**suben**, 94.8 → 99.2, 100.1 → 103.4, 91.0 → 96.4, 227.9 → 234.5. La bajada
del 4-7 % que vio la comprobación previa era ruido de una tirada.

| instancia | media control | media allow | d | bo5 control | bo5 allow | d |
|---|---|---|---|---|---|---|
| ta23 | 1586.9 | 1584.3 | −2.67 | 1576.5 | 1575.3 | −1.17 |
| ta29 | 1641.1 | 1641.0 | −0.03 | 1636.5 | 1635.2 | −1.33 |
| ta30 | 1623.4 | 1620.4 | −3.00 | 1613.8 | 1607.7 | **−6.17** |
| ta45 | 2041.5 | 2037.7 | **−3.83** | 2029.3 | 2026.5 | −2.83 |

Media **−2.38**, regla *descartar si > +2.0*: **pasa**. Las cuatro instancias
van a favor o empatan, y la cola informada se mueve en la misma dirección
(−2.87).

**Es el filtro más favorable del bucle, y precisamente por eso hay que leerlo
con más cautela.** El de I-011 dio −2.54, casi lo mismo, y sus seis mirillas
acabaron en −0.32. Dos cosas lo distinguen de aquel, y ninguna es prueba: el
mecanismo de I-011 estaba **plano** en el propio filtro (dispersión 1.03), y el
de I-015 se ejerce con fuerza y sin coste; y el efecto de I-011 no lo cargaba
ningún interruptor, mientras que aquí hay una única causa posible. **Nada de
eso sustituye a la frontera.** Van las oleadas, y decide la sexta mirilla o el
cruce, lo que llegue antes.

### I-016 — la siembra desde el círculo, y lo que apareció al medirla

**La idea es del PI** (2026-09-23). Imaginar todo el espacio de soluciones como
un círculo, dividirlo en 247 partes iguales y que cada corte sea una solución
de la población inicial, de modo que **estén representadas todas las zonas del
espacio**; y girar el círculo para cada semilla, para que la selección no se
repita entre tiradas. Se pidió probarla en la práctica aunque la medida previa
la desaconsejara.

**Cómo se ha realizado, exactamente.** Las secuencias de operaciones (cada
trabajo aparece *m* veces) se numeran en orden lexicográfico, de 0 a
|S| − 1 con |S| = (nm)! / (m!)^n: unas 10^501 en 20x20 y 10^856 en 30x20. Esa
numeración es la posición en el círculo, y `unrank` la traduce a la secuencia
concreta con aritmética entera exacta (comprobado enumerando las 90 secuencias
de 3 trabajos x 2). El círculo g pone sus 247 puntos en o_g + ⌊k |S| / 247⌋, y el
giro o_g es la fracción áurea de un arco, así que ninguna semilla repite corte
(`scripts/circle_pool.py`). La tirada g lee el círculo g por la creación
sembrada.

**Primera medida: la cobertura, antes de decodificar.** Para 4000 secuencias
sonda al azar, la distancia a la semilla más cercana:

| | semillas | diversidad | distancia media a la más cercana | peor |
|---|---|---|---|---|
| ta23 | círculo | 0.9490 | 0.9169 | 0.930 |
| ta23 | azar | 0.9499 | 0.9168 | 0.930 |
| ta45 | círculo | 0.9659 | 0.9443 | 0.953 |
| ta45 | azar | 0.9666 | 0.9443 | 0.952 |

**Idénticas.** En un espacio de 10^500 puntos, 247 no cubren nada se pongan
como se pongan: cada sonda queda casi tan lejos de su semilla más cercana
como de cualquier otra. Y la población aleatoria ya está en el 98.5-99 % de la
diversidad máxima posible con la propia medida del solver
(`scripts/diversity_curve.py`).

**Segunda medida: la generación 0 después de decodificar, y la sorpresa.** El
círculo arranca **bastante mejor**: makespan medio ~2127 en ta23 y ~2754 en
ta45, contra ~2269 y ~2952 de la creación aleatoria congelada, con la misma
diversidad. Pero la hipótesis de que el círculo reparte mejor los trabajos a lo
largo de la secuencia es **falsa** —desequilibrio de prefijos 3.70 contra 3.72—,
y el control decisivo lo aclara: **secuencias uniformemente aleatorias por el
mismo camino arrancan igual**, ~2117 y ~2753. La mejora no es del círculo.

**Es del generador aleatorio del solver, que está sesgado.** `jsp.random` elige
cada siguiente trabajo **uniformemente entre los que aún tienen operaciones**,
sin mirar cuántas les quedan. Los trabajos que salen pronto se agotan antes, y
la cola de la secuencia se llena en bloque con las últimas operaciones de unos
pocos rezagados, lo que decodifica mal. Una permutación uniforme elige en la
práctica en proporción a lo que le queda a cada trabajo y se autocorrige. El
sesgo cuesta un **6-7 %** en la población inicial, y lo que lo hace
importante es que **los exploradores del ABC usan el mismo generador**, entre
500 y 800 veces por tirada.

**El arreglo**: `creation.random.draw = uniform`, que elige cada trabajo en
proporción a sus operaciones restantes. Por defecto nada cambia, y está
comprobado al número: la generación 0 del control repite exactamente los
valores anteriores con la misma semilla. Con el arreglo, la generación 0 cae
donde cayeron las secuencias uniformes (~2123 y ~2750).

**Celdas**: `control`; `circle`, la idea del PI, que siembra la población
inicial y deja los exploradores como están; y `uniform`, el generador
corregido, que toca la población inicial **y** los exploradores. **Endpoint**:
la media por instancia. **Filtro**: cada celda tratada por su cuenta, descartar
si su media contra el control supera +2.0; las que sobrevivan se escriben en
`wave_cells.txt`, que leen tanto el generador de trabajos como el análisis. Si
llegan las dos a las oleadas, la frontera se reparte entre ellas:
p <= 0.0071 cada una en lugar de 0.0142. **Mecanismo, primero**: el makespan
medio de la generación 0 de cada celda, que tiene que salir claramente por
debajo del control.

**El riesgo en contra está medido desde I-001**: una ventaja de 294 unidades en
la generación 0 se quedaba en 0.9 al final. Aquí es de 140 a 200. La celda
`uniform` tiene a su favor lo que ninguna siembra tenía: su efecto no se agota
en la generación 0, porque se repite en cada explorador.

**Orden de ejecución**: I-015 queda en pausa entre sus mirillas 2 y 3, lo que
el diseño de Pocock admite sin coste, y nunca hay dos experimentos a la vez.

### I-016, filtro: las dos celdas pasan, y lo que miden es la lección de I-001 otra vez

360 trabajos, cero infactibles, 30 tiradas por celda e instancia.

**Mecanismo, en verde en las dos celdas.** Makespan medio de la generación 0:

| instancia | control | círculo | uniforme |
|---|---|---|---|
| ta23 | 2263.0 | 2122.4 | 2121.2 |
| ta29 | 2398.6 | 2235.5 | 2234.1 |
| ta30 | 2308.1 | 2185.5 | 2183.3 |
| ta45 | 2949.2 | 2752.2 | 2751.7 |

Las dos arrancan unas **156 unidades mejor**, y **exactamente igual entre
sí**, como había anticipado la medida previa: lo que mejora el arranque no es el
círculo, es muestrear uniformemente.

**Al final de la tirada no queda nada.** Media por instancia contra el control:

| instancia | círculo | uniforme |
|---|---|---|
| ta23 | +1.43 | +4.10 |
| ta29 | −0.80 | −0.93 |
| ta30 | −1.90 | −1.63 |
| ta45 | +1.27 | +1.30 |
| **media** | **+0.00** | **+0.71** |

Ninguna supera +2.0: **pasan las dos** y van a oleadas con la frontera repartida,
p <= 0.0071 cada una. El filtro solo descarta; no acepta, y tampoco rechaza por
un cero.

**Lo que dice ya, sin frontera y sin pretender más.** Una ventaja de 156
unidades en la generación 0 se evapora entera, como se evaporaron las 294 de
I-001. La celda uniforme, que mejora además los 500-800 exploradores de cada
tirada, tampoco deja rastro en la media, así que el explorador que llega mejor
tampoco cambia el destino de la tirada. Es coherente con todo lo anterior: **lo
que decide el resultado es lo que hace la búsqueda local durante la tirada, no
desde dónde empieza ni con qué se la alimenta**.

**Orden**: primero se cierran las mirillas 3 a 6 de I-015, que está en pausa y
más cerca de decidir; después las seis oleadas de I-016, cuyos trabajos ya
están generados y verificados, con los círculos de las 21 instancias.

### I-015, cierre: el final más favorable del bucle, y aun así no cruza

Seis oleadas, 1260 tiradas de confirmación, 30 por celda e instancia, cero
infactibles. Media por instancia, `allow − control`:

| mirilla | tiradas/celda | media | mejor en | p | admisiones/tirada |
|---|---|---|---|---|---|
| filtro | 30 (4 inst.) | −2.38 | -- | -- | ~3000 |
| 1 | 5 | +1.46 | 7 de 21 | 0.237 | 2879 |
| 2 | 10 | +0.62 | 8 de 21 | 0.848 | 2299 |
| 3 | 15 | +0.22 | 9 de 21 | 0.674 | 2166 |
| 4 | 20 | +0.50 | 13 de 21 | 0.715 | 2149 |
| 5 | 25 | −0.38 | 13 de 21 | 0.434 | 2373 |
| 6 | 30 | **−0.78** | **14** de 21 | **0.122** | 2457 |

Frontera de Pocock 0.0142, simétrica. **No cruza: descartada.** No hay séptima
mirada.

**Es el final más favorable de todas las iteraciones**, y conviene decirlo en
los dos sentidos. A favor: la media va hacia abajo en las tres últimas miradas
(+0.50, −0.38, −0.78), mejora en 14 de 21, y las diferencias negativas grandes
(ta43 −4.33, ta45 −3.80, ta48 −3.60, ta23 −3.20) pesan más que las positivas,
que no pasan de +2.77. En contra: p = 0.122 está a casi **diez veces** la
frontera, el filtro prometía −2.38 y las primeras miradas fueron en contra, y
este bucle ya ha visto dos veces cómo una señal de esta forma no se reproduce
(I-010 frente a I-012; el filtro de I-011 frente a sus mirillas). **La decisión
es la que dice la regla.** Si más adelante se quisiera, lo correcto no sería
mirar otra vez estos datos sino **una réplica con semillas nuevas**, como I-012
hizo con la señal de I-010, preinscrita como idea propia.

**El mecanismo se ejerció siempre** —entre 2149 y 2879 movimientos de meseta
admitidos por tirada, sin coste en generaciones—, así que esto no es una idea
que no llegara a probarse.

**Qué se revierte**: nada de los registros. `abc.plateau` queda implementado y
**desactivado por defecto**.

**Sin récords, pero una igualada de `ta30`, en la celda `allow`, vista dos
veces.** El mejor conocido de `ta30`, 1584, aparece en el filtro
(`I-015_filter_p25_allow`) y en la oleada 5 (`I-015_w5_p05_allow`), a 40 s por
tirada, verificado con el comprobador independiente contra los datos
originales de OR-Library y guardado en `iter/I-015/evidence/`. **Los dos
horarios son idénticos**, y la razón es de diseño: el trabajo 25 del filtro y
el 5 de la oleada 5 llevan **la misma semilla, la 25**, porque el filtro usa
las semillas 1 a 30 y las seis oleadas vuelven a recorrer las mismas 1 a 30.
Con la misma semilla y la misma configuración, las dos tiradas hicieron el
mismo recorrido y llegaron al mismo horario pese a que el presupuesto es de
reloj. Es **una sola igualada**, no dos, y en el control ninguna: uno contra
cero no significa nada y no cambia la decisión. Se anota porque encaja con el
mecanismo y porque es la segunda igualada de `ta30` del bucle, tras la del
control de I-012.

**Y deja a la vista un hecho del diseño que vale para todas las iteraciones**:
en las cuatro instancias del filtro, sus tiradas y las de las oleadas **no son
independientes**, porque comparten semillas. No afecta a ninguna decisión —la
frontera se aplica solo a los datos de las oleadas—, pero explica en parte por
qué el filtro y las oleadas coinciden más de lo que coincidirían con semillas
distintas en esas cuatro instancias, y sugiere que el filtro de las próximas
iteraciones use semillas propias, fuera del rango de las oleadas.

La igualada se me escapó en el primer cierre: el guion de récords solo miraba
las oleadas.
