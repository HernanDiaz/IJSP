# Informes de revisión del manuscrito

**Manuscrito:** *Seeding metaheuristics with learned and heuristic solution pools for the Interval Job Shop Scheduling Problem: an anytime study of when initialisation helps*

**Autor:** Hernán Díaz Rodríguez (Universidad de Oviedo)

**Fecha de revisión:** 25 de agosto de 2026

**Revisores:** Revisor 1 — Claude (Fable 5) · Revisor 2 — OpenAI Codex (gpt-5.6)

**Recomendación de ambos revisores:** *Major revision*


# Informe del Revisor 1 (Claude)


## 1. Resumen

El manuscrito presenta el que probablemente sea el mayor estudio controlado de seeding de poblaciones para el Job Shop con tiempos de intervalo (IJSP): 4 solvers publicados (GA, ABC elitista, ABC memético con hill-climbing, híbrido tabú estado del arte), 7 brazos de inicialización (control sin semillas, política deep-RL, escalera de constructivos aleatorizados MOR/GT/GP, y un pool mixto), 61 instancias Taillard de intervalo y 30 ejecuciones emparejadas por celda (51.240 runs). Un estudio piloto documenta tres artefactos metodológicos que corrompen habitualmente las evaluaciones de seeding (parada por estancamiento, ganancias dependientes del presupuesto que se invierten, y métricas de velocidad autorreferenciales), y el diseño final los elimina. Resultados principales: el signo del efecto del seeding no lo predice la fuerza del solver sino la compatibilidad de sus operadores de variación con una población estructuralmente heterogénea; la magnitud la predice la ratio tamaño-de-instancia/presupuesto; el pool mixto domina a los de generador único pese a partir de peor calidad media, aislando la diversidad de composición (no la calidad de las semillas) como variable operativa; y a presupuestos cortos (10%) el seeding beneficia a los cuatro solvers, incluido el que perjudica en convergencia.

## 2. Originalidad y relevancia

La pregunta ("¿ayuda sembrar?") es antigua, pero el ángulo es genuinamente original: el paper no propone un algoritmo nuevo sino que audita la *metodología* con la que la comunidad evalúa el seeding, y demuestra con datos que tres prácticas estándar fabrican resultados positivos espurios. Esa contribución trasciende el IJSP y es relevante para cualquier estudio de hibridación learning-+ -búsqueda. El hallazgo de que el GA es perjudicado por un fallo de *consolidación* (y no por convergencia prematura, que los datos de diversidad Hamming refutan directamente) es novedoso y bien argumentado. La auditoría semántica de los pools (desajuste lexicográfico vs. componente-a-componente en la cota inferior, hasta un 22% de líneas afectadas) es una lección práctica valiosa para pipelines híbridos.

## 3. Fortalezas

1. **Rigor estadístico ejemplar**: runs emparejados por RNG, Wilcoxon con corrección de Holm, tamaños de efecto Vargha–Delaney, Friedman con post-hoc, unidad de análisis la instancia. Sigue las buenas prácticas de Derrac et al.
2. **Diseño anytime completo** con dos métricas de time-to-target complementarias y la proporción de instancias que alcanzan el objetivo — la crítica a la métrica A autorreferencial (Sección 5.3, 7.5) es lúcida y debería ser lectura obligada en el área.
3. **Honestidad inusual**: la concentración del 94,8% de la ganancia de TSN2 en la clase 50×20 se destaca como "la salvedad más importante del paper" en lugar de ocultarse; la no-replicación de ABCE3 se declara; la contabilidad del coste de entrenamiento del RL se explicita.
4. **Argumento de validez para las cotas** (desigualdad 1) con predicción falsable: 0 violaciones en 51.240 runs y 6 instancias certificadas óptimas. Elegante.
5. **Análisis mecanicista**: la Tabla 9 (diversidad Hamming) convierte una especulación habitual (convergencia prematura) en una hipótesis refutada y propone un mecanismo alternativo (fallo de consolidación bajo JOX) coherente con el contraste GA/ABCE3.
6. Guía práctica clara y accionable (Sección 8.3).

## 4. Objeciones mayores

**M1 — Generalidad del mecanismo (N=1 por categoría).** La afirmación central — "el signo lo predice la compatibilidad del operador" — descansa en el contraste entre *un* GA y *un* ABC. GA y ABCE3 difieren en mucho más que el crossover (selección, elitismo, scout bees…), por lo que la atribución causal al operador de recombinación está infra-determinada. El experimento de aislamiento (marco GA + búsqueda local de TS-N2) que la apoyaría se menciona solo en el piloto (5.2). Se necesita al menos un experimento de ablación en el diseño final: p. ej., el GA con un crossover que preserve bloques (u orden relativo) vs. JOX, manteniendo todo lo demás. Sin ello, la Sección 8.2 debe rebajarse explícitamente a hipótesis (la 8.4 lo reconoce, pero el abstract y las conclusiones lo afirman con más fuerza de la que los datos soportan).

**M2 — Sección "Data and code availability" vacía.** El encabezado existe pero no hay contenido. Para un paper cuya contribución 4 es una "auditoría de reproducibilidad" y que promete "pools corregidos, código y datos completos", es imprescindible el enlace (Zenodo/OSF/GitHub con DOI). Igualmente, se afirma pre-registro del diseño y de las hipótesis, pero no consta dónde está depositado; un pre-registro no verificable públicamente pierde gran parte de su valor evidencial.

**M3 — No-replicación de ABCE3.** El solver que más se beneficia del seeding es precisamente el que corre 1,5–2,5 pp por encima de sus resultados publicados y la brecha no pudo cerrarse. El argumento de que las comparaciones son intra-solver es correcto, pero no elimina el riesgo de que la implementación difiera del original justamente en lo que gobierna el efecto (p. ej., intensidad de diversificación de los scouts). Se pide: descripción de los intentos de cierre de la brecha, y análisis de sensibilidad con alguna variante plausible.

**M4 — Reproducibilidad de los generadores de semillas.** La política RL "v2" (deep-sets, 3 checkpoints, entrenada en TA11–TA14) y la regla GP evolucionada con irace se describen en un párrafo. No hay arquitectura, recompensa, espacio de estados/acciones ni hiperparámetros, y no se cita un paper propio que los contenga. Tal como está, un tercero no puede reproducir dos de los cinco generadores. Además el solver estado del arte TS-N2 se apoya en la referencia [7], "under review": el estudio completo es irreproducible hasta que [7] se publique — como mínimo debe incluirse descripción suficiente en un apéndice.

**M5 — Soporte empírico estrecho para el análisis de diversidad.** La Tabla 9 usa 4 instancias medianas, 10 runs y 2 solvers. Dado que sostiene el mecanismo central del paper, debería ampliarse (más instancias, los 4 solvers, e idealmente correlacionar la caída de consolidación con la pérdida de calidad por instancia).

**M6 — Extrapolación de tamaño sin la clase mayor.** El mensaje "el beneficio crece con la ratio tamaño/presupuesto" se apoya en que 50×20 domina, pero TA71–TA80 (100×20) se excluye por calibración. Es una decisión defendible, pero entonces la predicción de escalado queda sin el punto de datos que más la pondría a prueba; añadir aunque fuera esa clase con calibración separada (etiquetada como estudio adicional) reforzaría notablemente la conclusión principal.

## 5. Comentarios menores

- El abstract es demasiado largo y denso para Elsevier (~250 palabras recomendadas); condensar.
- p\* = 100% se selecciona con un sub-estudio sobre 8 instancias y solo para v2 en 2 solvers; matizar que el "no hay óptimo interior" es específico de ese contexto.
- Figuras 2–3: gráficos base de R con leyendas minúsculas y colores poco distinguibles en impresión; rehacer con mayor tamaño de fuente y estilos de línea diferenciados.
- El solapamiento del 22% entre bloques de semillas con k=250 (4.3) merece una frase cuantificando su efecto sobre la varianza entre runs.
- Tablas al final del manuscrito: para revisión es preferible flotarlas en el texto.
- La referencia [7] debería actualizarse o complementarse con un preprint citable.
- Errata: "84/83/83 of the 250" — comprobar la aritmética de la ventana en 4.1 vs. 8.1.

## 6. Metodología y estadística

Sólida y por encima del estándar del área: presupuestos idénticos por celda calibrados sobre la convergencia del control, flag *anytime-only* cuando el control no converge, RPD contra cotas publicadas (no contra best-known propios), verificación independiente del decodificador (3255 soluciones re-decodificadas, discrepancias ≤0,11% y siempre pesimistas), y control de contaminación train/test (exclusión de TA11–TA20). Las principales reservas son de alcance (M1, M5), no de ejecución.

## 7. Recomendación

**Major revision.** El paper es metodológicamente excelente, honesto y con un mensaje claro y útil; ninguna objeción parece insalvable. M1 (ablación del operador o rebaja explícita de la afirmación causal), M2 (repositorio y pre-registro públicos) y M4 (reproducibilidad de v2/GP/TS-N2) son condiciones necesarias; M3, M5 y M6 elevarían el trabajo de "muy bueno" a "referencia del área".


# Informe del Revisor 2 (OpenAI Codex)

## 1. Resumen del artículo en mis palabras

El manuscrito estudia cuándo resulta beneficioso inicializar —"sembrar"— metaheurísticas con soluciones generadas externamente para el Job Shop Scheduling Problem con tiempos de procesamiento intervalares (IJSP). Se comparan cuatro algoritmos, desde un algoritmo genético relativamente simple hasta un híbrido con búsqueda tabú presentado como estado del arte, y seis estrategias de inicialización, incluyendo una política de aprendizaje por refuerzo, varias heurísticas constructivas aleatorizadas y una mezcla de tres generadores.

La tesis principal es que el efecto de la siembra no depende simplemente de la calidad del algoritmo receptor ni de la calidad media de las semillas. Según los resultados, la siembra mejora claramente ABCE3, perjudica al GA, apenas afecta a fEABC-LS y mejora TS-N2 principalmente en las instancias \(50\times20\), donde el presupuesto temporal parece insuficiente para que la búsqueda desde una población aleatoria alcance regiones comparables. A presupuestos cortos, sin embargo, la siembra proporciona ventajas sustanciales en los cuatro algoritmos.

El artículo también propone recomendaciones metodológicas para evaluar inicializaciones: evitar criterios de parada basados en estancamiento, comprobar la convergencia del control, presentar curvas anytime y distinguir entre tiempo hasta el plateau propio y tiempo hasta un objetivo común. Finalmente, atribuye el comportamiento del GA a una supuesta incompatibilidad entre poblaciones estructuralmente heterogéneas y el operador JOX, más que a convergencia prematura.

## 2. Valoración de originalidad y relevancia

La pregunta de investigación es relevante para Inteligencia Artificial, metaheurísticas e Investigación Operativa. La inicialización heurística se utiliza de forma rutinaria, pero con frecuencia se evalúa mediante experimentos insuficientes, especialmente usando un único presupuesto y sin estudiar la evolución temporal. El énfasis en el carácter dependiente del presupuesto y en la interacción entre semillas y operadores constituye una aportación potencialmente interesante.

La originalidad no reside en el mecanismo de siembra —que es deliberadamente elemental— ni en introducir aprendizaje por refuerzo en scheduling, sino en la amplitud del estudio comparativo y en la metodología anytime. Esta contribución puede ser publicable en una revista Q1 si las afirmaciones se ajustan mejor a la evidencia y si se corrigen varias debilidades estadísticas y de diseño.

En su forma actual, algunas conclusiones centrales son más fuertes que lo que permite el experimento. En particular, no queda demostrado que el cociente tamaño/presupuesto sea "la variable gobernante", que la diversidad del pool sea el mecanismo causal, ni que JOX sea responsable de la degradación del GA. Los datos muestran asociaciones compatibles con esas explicaciones, pero no aíslan causalmente dichos factores.

También preocupa que TS-N2, utilizado como referencia de estado del arte, proceda de un trabajo aún "under review" del mismo autor. Esto no invalida su inclusión, pero hace necesario moderar esa caracterización, documentar completamente el algoritmo y compararlo con referencias públicas e independientes.

## 3. Fortalezas

- El artículo aborda una creencia práctica importante y evita asumir de antemano que la siembra siempre ayuda.

- El diseño factorial es amplio: 61 instancias, cuatro algoritmos, siete brazos y 30 ejecuciones por celda. La presentación de resultados negativos, en particular para el GA, es valiosa.

- El estudio piloto identifica problemas reales en la evaluación de metaheurísticas: parada por estancamiento, reversión del resultado al ampliar el presupuesto y métricas de velocidad autorreferenciales.

- Las curvas anytime y el análisis a diferentes fracciones del presupuesto aportan bastante más información que una tabla única de resultados finales.

- La comparación entre generadores aprendidos y heurísticos se hace mediante un protocolo común de inyección, lo cual reduce varias fuentes de confusión.

- La auditoría de consistencia entre decodificadores es una buena práctica. Detectar la discrepancia entre la agregación lexicográfica y el máximo componente a componente constituye una lección reproducible para sistemas híbridos.

- El manuscrito reconoce varias limitaciones y evita presentar el resultado agregado de TS-N2 como uniformemente válido para todas las clases.

- El argumento de validez del lower bound crisp para el objetivo midpoint intervalar es interesante y, bajo las definiciones proporcionadas, parece esencialmente correcto.

- El texto es generalmente claro, la narrativa experimental es coherente y las tablas permiten reconstruir buena parte de los resultados.

## 4. Debilidades y objeciones mayores

### 1. Las afirmaciones causales sobre diversidad y compatibilidad de operadores no están demostradas

El manuscrito afirma que "pool diversity rather than raw seed quality drives the gain" y que el resultado negativo del GA se debe a una incompatibilidad con JOX. Sin embargo, no se realiza una intervención experimental que separe calidad y diversidad.

El pool MIX difiere de los pools individuales en más aspectos que su diversidad: mezcla distribuciones generadas por procedimientos distintos, puede cubrir distintas cuencas de atracción y puede contener complementariedades estructurales no capturadas por la distancia utilizada. Que su media inicial sea peor que la de GP o V2 no basta para "aislar" diversidad como causa. También podrían importar la cola de la distribución, la cobertura de vecindarios, la calidad condicional de subgrupos o la interacción con selección y búsqueda local.

Asimismo, observar que la población del GA mantiene una distancia de Hamming elevada no demuestra que JOX destruya bloques útiles. La ausencia de contracción puede ser consecuencia, y no causa, del bajo progreso. La comparación GA–ABCE3 tampoco aísla el operador de cruce: ambos algoritmos difieren en selección, reemplazo, mutación, dinámica poblacional y evaluación de candidatos.

Acciones necesarias:

- Construir pools con calidad controlada y niveles de diversidad manipulados explícitamente, idealmente mediante matching o estratificación por calidad.

- Comparar JOX con al menos uno o dos operadores alternativos sobre el mismo GA, manteniendo fijo todo lo demás.

- Incluir ablaciones sin crossover, con mating restringido por distancia y/o con estrategias explícitas de consolidación.

- Medir preservación de precedencias o arcos disyuntivos padre–hijo, no solo distancia de Hamming poblacional.

- Reformular como hipótesis interpretativa cualquier afirmación causal que no pueda verificarse mediante esas ablaciones.

### 2. "El cociente tamaño/presupuesto gobierna el efecto" es una conclusión excesiva

El manuscrito no analiza realmente un cociente continuo entre tamaño y esfuerzo computacional. El tamaño está confundido con la clase de instancia, el presupuesto calibrado, el efecto del límite de 900 segundos, el número de operaciones y posiblemente la dificultad intrínseca de cada clase. La evidencia principal consiste en que el beneficio de TS-N2 se concentra en \(50\times20\).

Además, la afirmación de que el presupuesto "no crece proporcionalmente" no se acompaña de medidas de trabajo algorítmico, evaluaciones, generaciones, movimientos tabu ni complejidad efectiva. Con presupuestos temporales, un aumento de tamaño también reduce el número de iteraciones ejecutables.

Acciones necesarias:

- Modelar estadísticamente la interacción entre siembra, tamaño, presupuesto y algoritmo.

- Ejecutar varias combinaciones de presupuesto para una selección estratificada de tamaños, no únicamente fracciones de un único presupuesto calibrado.

- Reportar métricas de esfuerzo independientes del hardware: evaluaciones, generaciones, movimientos de vecindario y llamadas al decodificador.

- Separar "efecto del tamaño" de "efecto del truncamiento por 900 s".

- Sustituir "governing variable" por una formulación asociativa si no se incorpora un experimento factorial tamaño–presupuesto.

### 3. Existe una tensión seria entre "calidad convergida" y celdas anytime-only

La Sección 6.3 establece que, cuando el control no converge antes del límite, la celda se marca como anytime-only y no se hacen afirmaciones de calidad final. Sin embargo, el artículo presenta resultados agregados de "converged final quality" sobre las 61 instancias y realiza afirmaciones finales precisamente sobre \(50\times20\), donde TS-N2 continúa descendiendo hasta el límite y la explicación propuesta es de starvation presupuestaria.

No se indica qué celdas fueron clasificadas como anytime-only, cuántas hubo ni cómo se excluyeron de las Tablas 2–4. Si \(50\times20\) es anytime-only, no debería entrar en una tabla titulada calidad convergida; si no lo es, debe demostrarse con diagnósticos de convergencia. Que las curvas estén separadas en el límite no prueba que la diferencia persista al converger.

Acciones necesarias:

- Enumerar todas las celdas anytime-only.

- Separar estrictamente resultados de convergencia y resultados a presupuesto fijo.

- Presentar las Tablas 2 y 3 tanto con todas las instancias como únicamente con celdas que satisfacen el criterio de convergencia.

- Extender el presupuesto en \(50\times20\), al menos para A0 y MIX de TS-N2, para comprobar si la diferencia persiste o es solo una ventaja transitoria.

### 4. El análisis estadístico tiene problemas de multiplicidad y de unidad experimental

Se realizan seis contrastes por instancia y solver, corrigiendo con Holm solo dentro de cada instancia. Esto supone al menos 244 familias separadas y hasta 1.464 contrastes elementales. Contar posteriormente cuántas instancias son "victorias" no controla el error global ni cuantifica si el número de victorias excede lo esperable bajo la hipótesis nula.

Por otra parte, se dice que la instancia es la unidad de análisis porque los bloques de semillas se solapan entre ejecuciones, pero los Wilcoxon de cada instancia utilizan las 30 ejecuciones como unidades. Ambas afirmaciones no son compatibles sin un modelo jerárquico explícito.

El \(\hat A_{12}\) de Vargha–Delaney es normalmente una medida para muestras independientes; con ejecuciones emparejadas debería justificarse su empleo o utilizarse una medida de efecto compatible con diferencias pareadas. Tampoco se explica cómo se calcula el \(\hat A_{12}\) "agregado": si se agrupan observaciones de distintas instancias, las escalas y dificultades heterogéneas pueden producir un estimador difícil de interpretar.

Acciones necesarias:

- Definir con precisión cada unidad experimental y cada familia de hipótesis.

- Realizar un análisis global por solver y brazo usando la instancia como bloque, seguido de análisis post-hoc controlados.

- Considerar modelos jerárquicos o pruebas sobre diferencias por instancia, con intervalos de confianza.

- Presentar distribuciones de efectos por instancia, no solo W/T/L.

- Justificar el estimador de tamaño de efecto o reemplazarlo por una medida pareada, como probabilidad de superioridad pareada o medianas estandarizadas de las diferencias.

- Añadir intervalos de confianza obtenidos mediante bootstrap jerárquico sobre instancias y ejecuciones.

### 5. El emparejamiento mediante la misma semilla RNG no está suficientemente justificado

Usar el mismo stream de números aleatorios no garantiza un emparejamiento informativo cuando los brazos consumen cantidades o secuencias diferentes de números aleatorios. La inicialización sembrada sustituye individuos aleatorios y, por tanto, puede desplazar todo el stream posterior. Los algoritmos también pueden tomar ramas diferentes y consumir aleatoriedad de manera divergente.

En esas condiciones, presentar las ejecuciones como "RNG-paired" puede ofrecer una impresión exagerada de control experimental.

Debe explicarse cómo se implementó common random numbers: streams separados por componente, restauración del estado después de la inicialización, seeds independientes para inicialización y búsqueda, o alguna estrategia equivalente. De lo contrario, los contrastes pareados necesitan una justificación empírica o deben tratarse como no emparejados.

### 6. La preinscripción no es verificable y parece parcialmente adaptativa

El artículo utiliza repetidamente el término "pre-registered", pero no proporciona repositorio, sello temporal, protocolo, hipótesis exactas ni registro de desviaciones. Además, el piloto parece haber informado las hipótesis y el diseño posterior, y la fracción \(p^*=100\%\) se selecciona mediante una "Phase 0" sobre ocho instancias. Esto puede ser legítimo, pero debe distinguirse claramente entre exploración, selección de diseño y confirmación.

Acciones necesarias:

- Proporcionar enlace permanente y fechado a la preinscripción.

- Reproducir las reglas de decisión completas y los análisis previstos.

- Identificar cualquier desviación del protocolo.

- Indicar si las ocho instancias de selección de \(p^*\) forman parte de las 61 instancias de evaluación. Si forman parte, existe reutilización adaptativa de datos y se necesita excluirlas del análisis confirmatorio o presentar sensibilidad sin ellas.

Sin evidencia documental, debe reemplazarse "pre-registered" por "specified before the final experiment".

### 7. La selección de \(p^*=100\%\) y el brazo V2H producen un diseño incompleto

La fracción óptima se elige usando solo ABCE3 y TS-N2, aunque luego se concluye sobre cuatro algoritmos. Los propios resultados del GA indican que V2 al 50% es menos perjudicial que V2 al 100%, sugiriendo una interacción fuerte entre fracción y algoritmo. No puede afirmarse que no existe óptimo interior de manera general.

Sería necesario estudiar varias fracciones para cada solver, al menos para MIX y V2. Alternativamente, el manuscrito debe presentar \(100\%\) como una decisión protocolaria específica, no como una fracción generalmente preferible.

### 8. La mezcla no está comparada con controles suficientes

El pool MIX combina V2, GT y GP, pero no se comparan mezclas de dos generadores, mezclas con proporciones alternativas ni una mezcla aleatoria de soluciones emparejada por calidad. Tampoco se incluye MOR, con la explicación de que es el peor generador. Por ello no se sabe si MIX funciona por diversidad, por complementariedad de un par específico o simplemente porque amortigua los fallos de cada generador.

Se requieren, como mínimo, ablaciones V2+GP, V2+GT y GP+GT, o una justificación más prudente que evite recomendar de manera general "usar un pool mixto".

### 9. La métrica de diversidad es insuficiente y parcialmente inconsistente

En la Sección 4 se habla de distancia normalizada de orden disyuntivo, mientras que en la Sección 8 se usa distancia de Hamming posicional. Estas métricas no son intercambiables. En una representación con repetición de identificadores de trabajo, la distancia de Hamming puede no corresponder adecuadamente a diferencias fenotípicas entre schedules ni a los building blocks relevantes para JOX.

Además, el análisis instrumental solo usa cuatro instancias y diez ejecuciones, una base pequeña para sostener el principal argumento mecanístico. Deben incluirse más clases, varias métricas genotípicas y fenotípicas, y su relación directa con calidad de descendientes y progreso.

### 10. TS-N2 no puede presentarse sin reservas como estado del arte

La única referencia de TS-N2 es un manuscrito del propio autor "under review" en 2026. Sus resultados no han sido validados mediante publicación y el algoritmo puede no ser reproducible a partir de una referencia disponible. También se retienen 20 instancias usadas para ajustar sus hiperparámetros. Aunque la comparación dentro de solver reduce cierto sesgo, esto no elimina todos los problemas: un algoritmo ajustado en determinadas instancias puede presentar dinámicas de interacción con la inicialización específicas de esas instancias.

Debe incluirirse:

- Una especificación autocontenida y reproducible de TS-N2.

- Resultados separados para instancias usadas y no usadas en tuning, no solo la afirmación de que la sensibilidad "replica" las conclusiones.

- Comparación con métodos publicados e independientes cuando se emplee la expresión "state of the art".

- Una descripción más moderada, por ejemplo "strong tabu-search hybrid", mientras su referencia siga sin estar publicada.

### 11. La reproducibilidad del componente de aprendizaje es insuficiente

La política v2 está descrita de forma muy superficial. Faltan arquitectura detallada, estados, acciones, función de recompensa, algoritmo de RL, hiperparámetros, distribución de entrenamiento, criterio de selección de checkpoints y mecanismo de aleatorización para producir 1.024 soluciones i.i.d. No está claro qué significa i.i.d. cuando se agregan tres checkpoints entrenados.

También falta información equivalente para GP: conjunto de entrenamiento, función de fitness, gramática, presupuesto evolutivo, configuración de irace y grado de reutilización de instancias.

La sección "Data and code availability" aparece vacía. En un artículo cuya contribución incluye una auditoría de reproducibilidad y que declara liberar código, pools y datos, esto es una deficiencia mayor. La aceptación debería condicionarse a la disponibilidad real de código, instancias perturbadas, seeds, trazas, pools corregidos, scripts estadísticos y documentación del entorno.

### 12. La comparación de costes está incompleta

Se excluye el entrenamiento del modelo RL porque se amortiza, pero tampoco se cuantifica el coste de generar los pools, ejecutar GP o producir las soluciones heurísticas. Si la recomendación es práctica, el coste de inicialización debe incluirse en las curvas o, como mínimo, presentarse por separado.

La frase "el coste de ejecutar tres generadores es negligible" no está respaldada por datos. Puede ser cierta respecto a 900 s, pero no necesariamente para presupuestos muy cortos, precisamente donde se afirma que la siembra tiene mayor valor.

### 13. La generalización es limitada y algunas recomendaciones son demasiado amplias

Solo se estudia una familia de problemas, una representación, cuatro algoritmos estrechamente relacionados —tres comparten el marco ABC o componentes afines— y pools producidos para un benchmark derivado de Taillard. No hay evidencia suficiente para recomendaciones generales sobre algoritmos recombinativos o sobre population seeding en metaheurísticas.

La conclusión debe restringirse al IJSP, a esta representación y a estos operadores. Una validación adicional en JSP determinista, otra distribución de incertidumbre o un benchmark externo fortalecería mucho la contribución.

## 5. Comentarios menores

1. Debe utilizarse con cuidado "expected makespan". El midpoint de un intervalo no es una esperanza matemática salvo que se introduzca explícitamente una distribución probabilística. Sería más preciso hablar de "midpoint criterion" o "interval expected-value ranking" conforme a la convención de la literatura.

2. La notación de \(p_o=[p_o^-,p_o^+]\), máximos componente a componente y LEX2 debería acompañarse de un ejemplo pequeño. El texto es demasiado compacto para lectores no especializados en aritmética intervalar.

3. Debe aclararse si \(\delta\) se muestrea una sola vez por operación y queda fijado para todas las ejecuciones. También deben publicarse las semillas usadas para construir las instancias.

4. La justificación del lower bound debería formalizar mejor la inducción sobre el DAG y distinguir entre un schedule fijo, el óptimo crisp y el óptimo intervalar. El argumento parece correcto, pero merece una proposición formal.

5. Que no haya soluciones por debajo del bound es solo una prueba de consistencia débil; no constituye "strong joint evidence" de corrección del mapeo y del decodificador. Muchos errores pueden conservar esa desigualdad.

6. La comprobación del mapeo mediante coincidencia con machine-load bounds en solo 19 instancias tampoco valida por completo el mapeo de las restantes.

7. Debe aclararse por qué solo se redecodificaron 3.255 soluciones de 51.240 ejecuciones y cómo se seleccionaron.

8. La explicación de las cinco discrepancias en la redecodificación no resulta convincente. Si el cromosoma y el decodificador son deterministas, el makespan debería reproducirse exactamente. Que un constructor activo "mejore" el schedule almacenado indica que no se está reproduciendo el mismo mapping genotipo–fenotipo o que se está comparando otra representación. Esto requiere investigación, no simplemente atribuirlo a una mejora esperable.

9. Se alternan "CPU-time" y "wall-clock CPU-time". Debe especificarse la función de reloj utilizada y diferenciar tiempo de proceso, tiempo de hilo y tiempo de pared.

10. "Four published solvers" es inexacto si TS-N2 solo está under review.

11. "All five pools" parece incluir MIX junto con cuatro generadores, pero MIX no es un generador independiente. Conviene mantener esa distinción terminológica.

12. La independencia de las 1.024 soluciones de cada pool debe justificarse. Las soluciones procedentes de una misma política o checkpoint no son necesariamente i.i.d. en el sentido estadístico fuerte.

13. Los bloques de 250 se solapan sustancialmente y, tras varias ejecuciones, reutilizan múltiples entradas. Debe reportarse el número efectivo de soluciones distintas observadas por brazo e instancia.

14. La frase "the cost of running three generators instead of one is negligible" necesita evidencia cuantitativa.

15. La Tabla 5 no coincide exactamente con la Tabla 2 al 100%; atribuirlo a resolución de trazas puede ser aceptable, pero deberían cuantificarse el intervalo de muestreo y el procedimiento de interpolación.

16. En la Tabla 3, "tie" significa aparentemente "no diferencia estadísticamente significativa", no igualdad. Debería denominarse NS para evitar una interpretación errónea.

17. Las expresiones "wins" y "loses" también deben reservarse para efectos con dirección y significación claramente definidas.

18. Deben proporcionarse intervalos de confianza para las diferencias de RPD y para los tiempos hasta objetivo.

19. Para el tiempo hasta objetivo, sería preferible un análisis de tiempo-a-evento con censura, por ejemplo curvas de Kaplan–Meier o modelos de riesgos, en vez de calcular medianas únicamente entre éxitos.

20. Normalizar el tiempo por presupuesto facilita agregar curvas, pero puede ocultar diferencias importantes entre clases. Deben mantenerse análisis estratificados, no solo el agregado.

21. El método de promediar RPD entre instancias pondera por igual instancias muy distintas. Es razonable, pero debe declararse y complementarse con medianas y distribuciones.

22. La supuesta correlación de Spearman entre rankings se calcula con únicamente seis o siete brazos; debería incluirse incertidumbre o tratarse como estadística descriptiva.

23. La afirmación "the choice of generator does not depend on the solver" es demasiado absoluta. Una correlación media de 0.88 con seis estrategias y cuatro algoritmos no demuestra invariancia.

24. La exclusión de TA71–TA80 es comprensible, pero limita precisamente la principal conclusión de escalabilidad. Sería muy valioso incluir algunas de estas instancias con una calibración separada.

25. La calidad tipográfica del texto suministrado muestra numerosos caracteres corruptos y palabras concatenadas. Si esto está presente en el manuscrito original, requiere una corrección completa.

## 6. Valoración de la metodología experimental y estadística

La metodología experimental es ambiciosa y superior a la de muchos trabajos de metaheurísticas: incluye múltiples algoritmos, una cantidad considerable de ejecuciones, un control no sembrado, varias fuentes de semillas, curvas anytime y análisis estratificado por tamaño. La calibración de presupuestos basada en controles y la igualdad de presupuesto dentro de cada celda son decisiones razonables. El piloto también cumple una función metodológica útil.

No obstante, hay cuatro problemas centrales.

Primero, la convergencia no está establecida con suficiente rigor. Un criterio de plateau basado en una ventana final no demuestra convergencia, y calibrarlo con cinco ejecuciones produce una estimación ruidosa. Además, no se informa qué celdas alcanzaron ese criterio ni se separan consistentemente de las celdas truncadas. Para un trabajo cuya tesis depende de distinguir calidad final y ventaja transitoria, esto es esencial.

Segundo, el tratamiento estadístico es fragmentario. Los tests por instancia con corrección local no justifican por sí solos conteos globales de victorias. La estructura jerárquica es clara: ejecuciones dentro de brazos, brazos dentro de instancias y clases, y algoritmos sobre las mismas instancias. Se necesita un análisis global que respete dicha estructura. Recomendaría:

- diferencias pareadas por ejecución cuando el emparejamiento sea válido;

- agregación por instancia con intervalos de confianza;

- un modelo jerárquico con efectos de solver, arm, tamaño, presupuesto e interacciones;

- corrección explícita de multiplicidad para las conclusiones confirmatorias;

- bootstrap por instancia para resultados agregados;

- análisis de supervivencia para time-to-target con ejecuciones no alcanzadas tratadas como censura.

Tercero, el diseño no identifica los mecanismos alegados. Para demostrar diversidad como causa se necesitan pools emparejados por calidad y diversidad manipulada. Para demostrar incompatibilidad con JOX se necesita cambiar el operador manteniendo fijo el resto del GA. Las medidas actuales son diagnósticas, no confirmatorias.

Cuarto, los presupuestos basados exclusivamente en tiempo dificultan la reproducibilidad. Mantener catorce workers no garantiza trabajo computacional idéntico por segundo, y el proceso puede verse afectado por frecuencia dinámica, memoria y scheduling. Esto no necesariamente invalida las comparaciones internas, pero sí exige registrar tanto tiempo como unidades de trabajo algorítmico.

También debe revisarse la noción de paired RNG. Si los brazos consumen los streams de forma divergente, el emparejamiento nominal no equivale a common random numbers y puede invalidar el uso directo de tests pareados.

En suma, la base experimental es prometedora, pero el análisis estadístico y la identificación causal no están todavía al nivel exigible para sostener las conclusiones más fuertes.

## 7. Recomendación editorial

**Recomendación: major revision.**

El manuscrito contiene una pregunta relevante, un conjunto de experimentos amplio, resultados negativos interesantes y una contribución metodológica potencialmente valiosa. No recomendaría rechazo directo porque la base empírica parece suficientemente rica como para producir un artículo sólido y porque varias deficiencias pueden corregirse mediante reanálisis, mayor transparencia y moderación de las afirmaciones.

Sin embargo, no puede aceptarse en su forma actual. Son imprescindibles:

1. Resolver la contradicción entre calidad "convergida" y celdas anytime-only.

2. Rehacer y documentar el análisis estadístico respetando la estructura jerárquica, la multiplicidad y la censura en time-to-target.

3. Aclarar la validez del emparejamiento RNG.

4. Proporcionar la preinscripción verificable o retirar esa caracterización.

5. Hacer disponible el código, datos, pools, instancias y scripts de análisis.

6. Moderar las afirmaciones causales sobre diversidad, JOX y el cociente tamaño/presupuesto, o respaldarlas con ablaciones específicas.

7. Investigar las discrepancias de redecodificación, que no deberían descartarse como un comportamiento normal.

8. Documentar de forma reproducible RL, GP y TS-N2.

Con estas revisiones —y, preferiblemente, experimentos adicionales sobre operadores, diversidad y presupuestos extendidos— el trabajo podría convertirse en una contribución relevante. Actualmente, la narrativa causal es más concluyente que el diseño experimental que la sustenta.


# Revisión de código — Revisor 1 (Claude)


## 1. ¿El código respalda las afirmaciones del paper?

En lo esencial, **sí**. Verifiqué directamente los puntos que sustentan las afirmaciones centrales:

- **Inyección de semillas** (`CreationIJSP.cpp:1426`, clase `CreationSeededSchedule::createPopulation`): coincide exactamente con la Sección 4.3 del paper. Reemplaza los primeros `k` individuos por líneas del pool en bloques consecutivos `[(r·k) mod L, r·k+k)`, el resto se genera aleatoriamente, y las llamadas parciales (scout bees, `reqSize != popSize`) caen a creación aleatoria pura. Las semillas entran como permutaciones crudas y se re-decodifican con el SGS y el encoder del solver receptor (`buildFromJobPerm`, `CreationIJSP.cpp:1381`), como afirma el paper ("no foreign objective values enter the search"). Detalle bien resuelto: el índice de bloque usa un `seedOffset` global para que los runs troceados en varios procesos reciban los mismos bloques que una corrida secuencial (`CreationIJSP.cpp:1441-1442`).
- **Aritmética de intervalos**: el máximo componente a componente está correctamente implementado (`Interval.cpp:283-287`) y es el usado por los constructivos y el SGS (`M_COMPONENT`).
- **Auditoría de pools**: `seed_consistency_test.cpp` y `run_all_pools.sh` implementan la comparación componentwise vs. lexicográfico-por-upper descrita en la Sección 4.2, incluida la opción `--rewrite` que genera los pools corregidos. `verify_solution.cpp` es el decoder independiente de la Sección 7.9.
- **Pre-registro interno**: `final/DESIGN.md` existe, está fechado (diseño cerrado 2026-07-22, decisión del criterio de parada 2026-07-26) y contiene las reglas de decisión de la Fase 0, la lista de instancias contaminadas y la justificación de abandonar `noimprovement=25`. Esto es exactamente lo que el informe del paper pedía hacer público: **publicar este fichero (con timestamp verificable, p. ej. OSF o el propio historial git) resolvería la objeción sobre el pre-registro**.
- **RNG**: Mersenne Twister estándar (`std::mt19937`, `RandomMT.cpp`), inicializado por semilla de run (`RandomPopulation.cpp:295`).
- Existe una batería notable de scripts de auditoría en `final/` (`check_below_lb.R`, `sanity_vs_paper.sh`, `audit_paper_numbers.R`, `verify_seed_use.sh`, `check_seed_reuse.sh`, `mix_census.sh`…) que da credibilidad al pipeline de verificación descrito en el paper.

## 2. Hallazgos

**H1 (mayor) — "RNG-paired" es en realidad "misma semilla inicial".** Los cuatro solvers comparten un único stream MT19937 por run. El brazo sembrado no consume extracciones del RNG para sus primeros `k` individuos (vienen del pool), así que su stream diverge del control inmediatamente después de la inicialización. El emparejamiento es válido como *common random numbers* débil (misma semilla, misma instancia), pero el paper debería describirlo así y no sugerir trayectorias emparejadas. No invalida los Wilcoxon pareados (el par instancia-semilla sigue siendo un bloque legítimo), pero la redacción actual promete más de lo que el código hace.

**H2 (mayor) — Base temporal mixta.** El paper dice "CPU-time budget, measured with the process clock". El código usa `clock()` en los bucles principales (`GeneticAlgorithm.cpp:438-502`, `ArtificialBeeColony.cpp:464+`), que en POSIX es tiempo de CPU del proceso pero **en Windows es tiempo de pared**; además hay comprobaciones explícitas de wall-clock dentro de la generación en `ArtificialBeeColonyPSO.cpp:523-528` y límites de pared en `LocalSearch.cpp:336` (`_lsWallLimit = maxTime*4+5`). Hay que documentar la plataforma de los experimentos (presumiblemente Linux) y qué componentes usan qué reloj; tal como está, una réplica en Windows mediría otra cosa. Nota adicional: `clock()` con `clock_t` de 32 bits envuelve a los ~2147 s — por debajo del cap de 900 s, pero los acumuladores por componentes podrían acercarse.

**H3 (mayor, riesgo latente) — El constructor de copia de `CreationSeededSchedule` resetea `runCounter` a 0** (`CreationIJSP.h:1078`). Si el framework copiara el objeto de creación entre runs, todos los runs recibirían el bloque 0 y el diseño de 30 bloques distintos se rompería en silencio. Los scripts `verify_seed_use.sh`/`check_seed_reuse.sh` sugieren que esto se comprobó empíricamente, pero el invariante merece un assert o un test unitario, no solo un script post-hoc.

**H4 (menor) — Reproducibilidad de terceros.** No hay `README` en la raíz: sin él, un revisor externo no puede saber qué binario compilar (`build_full.sh` vs `Makefile`), qué setup usar (`repro/setup_*.txt`) ni en qué orden lanzar `final/run_phase*.sh`. `final/DESIGN.md` es excelente como pre-registro pero está en español y mezcla decisiones con log operativo; conviene un README en inglés con el pipeline mínimo instancia→pool→run→análisis.

**H5 (menor) — Higiene de código heredado.** Ficheros muertos (`RouteFVRP - copia.cpp/h`), grandes bloques de debug comentado (p. ej. `CreationIJSP.cpp:307-331, 1062-1105`), familias de problemas ajenas al estudio (FVRP, FJSP) mezcladas en la raíz plana, y `throw new IJSPException(...)` — lanzar excepciones por puntero es un antipatrón C++ que fuga memoria salvo que todos los catch hagan delete. Nada de esto afecta a los resultados, pero para el repositorio "de release" que el paper promete convendría una rama limpia con solo lo necesario para reproducir el estudio.

**H6 (menor) — `RandomMT::getDouble(nDigits)`** (`RandomMT.cpp:38-50`) muestrea uniforme en ±INT_MAX y trunca dígitos — semántica extraña; verificar que ninguna ruta del estudio la usa (las rutas revisadas usan `getInteger(lower, upper)`, que es correcta).

## 3. Valoración

El código confirma la mecánica descrita en el paper en los puntos críticos (inyección, decodificación, auditoría de pools, aritmética de intervalos) y el material de `final/` demuestra una cultura de verificación inusualmente sólida. Los hallazgos mayores son de **precisión de redacción en el paper (H1, H2)** y de **robustez del invariante de bloques (H3)**, no errores demostrados en los resultados. Con un README de reproducción, la publicación del DESIGN.md como pre-registro y las aclaraciones de H1/H2 en el texto, el código pasaría una auditoría de revista sin problemas.


# Revisión de código — Revisor 2 (OpenAI Codex)

**Repositorio:** `HernanDiaz/IJSP`, rama `seeding-study`


## Alcance y dictamen ejecutivo

Se revisó la rama `seeding-study` (commit visible `aae0520`) como segundo revisor, contrastando el código C++, los *setups*, los *runners*, los scripts de calibración/análisis y los artefactos versionados con las afirmaciones metodológicas del manuscrito. La revisión es estática, complementada con comprobaciones de integridad de los CSV agregados. No fue posible recompilar ni ejecutar los tests en este entorno porque no están instalados `g++` ni `make`; `bash build_test.sh` llega a enumerar 104 fuentes y falla al invocar `g++`.

**Dictamen:** el código respalda razonablemente la aritmética básica de intervalos usada para el makespan, el máximo componente a componente, los rankings EV/LEX2 configurados y el mecanismo actual de sustitución de los primeros `k` individuos. Sin embargo, **no respalda en su formulación fuerte dos afirmaciones centrales**: los brazos no comparten realmente la misma corriente RNG (sólo usan el mismo entero como semilla), y `verify_solution.cpp`/`seed_consistency_test.cpp` no son decoders alternativos independientes, sino clientes de las mismas clases del solver. Además, la regla de presupuesto ejecutada difiere de la fórmula literal sin suelo y la parada sólo se comprueba con granularidad gruesa. La ausencia de pools, configuraciones/salidas crudas, manifiesto de entorno, README y depósito/DOI impide reconstruir y auditar de extremo a extremo los resultados del paper desde este checkout. En consecuencia, la rama ofrece **soporte parcial, no suficiente para validar sin reservas todas las afirmaciones del paper**.

## Hallazgos

### 1. [crítico] El "RNG pairing" es nominal, no un emparejamiento por corriente aleatoria

**Ficheros:** `EvoLauncher.cpp:94-98`; `CreationIJSP.cpp:1430-1451`; `CreationIJSP.cpp:48-75`; `final/run_phase2c.sh:73-86`; `final/paper/main.tex:1064-1065`.

Todos los brazos reciben los enteros de semilla 1..30, lo que sí empareja etiquetas de run. Pero A0 consume RNG al construir los 250 individuos aleatorios; V2/MOR/GT/GP/MIX con `k=250` no consume RNG en la población inicial, y V2H consume sólo el correspondiente a 125 individuos. Por ello, al comenzar selección, cruce, mutación o búsqueda local, el estado del generador es distinto entre brazos. Tampoco se guarda/restaura un estado RNG posterior a una creación común ni existen subcorrientes separadas para inicialización y evolución. La frase del manuscrito "all arms ... share the RNG stream" es, por tanto, falsa en sentido técnico. Los tests pareados pueden emparejar por `(instancia, run-id)`, pero no deben justificarse como *common random numbers* ni como comparación "like with like".

**Recomendación:** usar dos RNG explícitos (inicialización y evolución), o generar una población base común y sustituir después sus primeros `k` elementos sin cambiar el estado del RNG evolutivo. Registrar hashes/estados iniciales y formular el paper como "mismos índices de semilla" mientras no se haga esa corrección.

### 2. [crítico] La supuesta verificación con decoder independiente reutiliza la implementación bajo prueba

**Ficheros:** `verify_solution.cpp:21-27,48-50,85-88`; `seed_consistency_test.cpp:31-36,82-83,147-158`; `final/paper/main.tex:1051-1055`.

`verify_solution.cpp` incluye y ejecuta directamente `SGS_IJSP_Insertion`, `SGS_IJSP_Append`, `ScheduleIJSP` e `Interval`; `seed_consistency_test.cpp` hace lo mismo con `SGS_IJSP_Append`. No hay un segundo algoritmo de decodificación ni una implementación independiente de precedencias, inserción y máximo. Estos programas son útiles como pruebas de relectura, redecodificación con otra opción del propio framework y auditoría del formato, pero un defecto compartido en SGS, `ScheduleIJSP` o `Interval` pasaría inadvertido. Esto contradice tanto la afirmación solicitada (f) como la denominación "independent decoder" del manuscrito.

**Recomendación:** implementar un verificador pequeño sin enlazar clases del solver (DAG de precedencias de trabajo/máquina, orden topológico y propagación separada de ambos extremos), probarlo con casos manuales y diferenciales, y publicar entradas y salidas de la muestra de 3.255 soluciones.

### 3. [mayor] La regla de presupuesto ejecutada no es literalmente `min(1.5*tconv(A0),900)` por celda

**Ficheros:** `final/run_phase2.sh:3-7,15-25`; `final/run_phase2c.sh:23-30`; `final/phase1_budgets.sh:2-4,17-19,31-35`; `final/paper/main.tex:438-446`.

Los runners aplican presupuestos tabulados **por `(solver, clase de tamaño)`**, agregando una o dos instancias de calibración, y añaden un suelo de 60 s (`clamp`, no sólo `min`). Por ejemplo, tiempos de convergencia menores llevan a 60 s aunque `1.5*tconv` sea menor. `phase1_budgets.sh` calcula la mediana entre valores de instancia disponibles por clase, mientras el paper describe la mediana de cinco runs A0 de la celda; el script toma de cada CSV una única traza aparentemente agregada y no demuestra aquí la mediana run-a-run. La enmienda está comentada en el runner, pero no coincide con la fórmula literal de la afirmación (c).

**Recomendación:** describir exactamente `max(60,min(1.5*median_class(tconv),900))`, explicar qué se promedia en la traza y aportar la tabla derivada con hashes de las trazas fuente; o regenerar presupuestos estrictamente conforme a la regla publicada.

### 4. [mayor] El límite de CPU no es una barrera exacta y puede parar antes o después según el solver

**Ficheros:** `GeneticAlgorithm.cpp:469-530,554-574`; `ArtificialBeeColonyPSO.cpp:505-535,632-649`; `LocalSearch.cpp:69-80,331-361`.

El tiempo se acumula con `clock()` y la condición general se consulta entre generaciones. El GA incluso termina preventivamente cuando estima que media generación adicional alcanzaría el límite (`currentRuntime + runtimePerGen/2`), por lo que recibe menos CPU que el presupuesto. En ABCPSO, la comprobación interna compara el tiempo de pared transcurrido **desde el inicio de la generación** contra el presupuesto completo, no contra el CPU restante; normalmente no evita el sobrepaso. Una búsqueda local puede seguir hasta su propio límite antes de devolver control. Además, TS incorpora un fusible de pared (`4*maxTime+5` por llamada) que es una segunda regla de parada distinta. Así, "presupuesto fijo" significa techo aproximado con granularidad dependiente del algoritmo, no consumo fijo e idéntico.

**Recomendación:** mantener un deadline CPU absoluto por run, pasarlo a operadores/LS, consultar el remanente dentro de bucles costosos y eliminar la heurística de media generación. Publicar distribuciones de CPU realmente consumido por brazo, no sólo el parámetro solicitado.

### 5. [mayor] El operador actual implementa el bloque circular, pero la evidencia versionada no demuestra que todos los resultados se produjeran con él

**Ficheros:** `CreationIJSP.cpp:1430-1457`; `CreationIJSP.h:1045-1055`; `final/run_phase2c.sh:64-86`; `final/preflight.sh:6-12`; `.gitignore:26-38`.

La implementación actual sí sustituye los índices poblacionales `0..k-1`, usa `blockStart=((offset+runCounter)*k)%L`, envuelve cada índice módulo `L`, y deja scouts/creaciones parciales aleatorios. Para pools de `L=1024` coincide con (a). No obstante, el código usa `L` real, no 1024 fijo; no valida que `L==1024`. Más importante, los propios comentarios de `preflight.sh` documentan que campañas troceadas anteriores repetían los primeros bloques por reinicio de `runCounter`, y `run_phase2c.sh` acepta como terminada una celda monolítica antigua sin verificar versión/binario/setup. Como las carpetas crudas y sus `setup.txt` están ignoradas, los CSV agregados no permiten comprobar qué celdas se ejecutaron antes o después del arreglo del offset.

**Recomendación:** exigir `L==1024` para este protocolo (o corregir el texto), incrustar commit/hash de binario, pool y setup en cada salida, y distribuir un manifiesto por celda que certifique `creation.seed.offset` y procedencia.

### 6. [mayor] El cargador de semillas acepta cromosomas incompletos sin validar exhaustivamente la permutación

**Ficheros:** `CreationIJSP.cpp:1361-1369,1391-1408`; `seed_consistency_test.cpp:115-145`.

El operador valida rango y exceso de ocurrencias mediante `getTaskId`, pero no exige `jobs.size()==nTasks` ni comprueba al final que cada trabajo aparezca exactamente el número requerido. Una línea truncada puede convertirse en un schedule parcial, ser codificada y entrar en la población; la validación fuerte existe sólo en la utilidad externa y no es una precondición del runner. También se aceptan líneas sin `;[lower,upper]`, aunque el formato documentado lo incluye. Un pool corrupto puede así producir resultados plausibles sin fallo temprano.

**Recomendación:** validar cada línea al cargarla: longitud exacta, multiplicidades exactas, ausencia de basura tras el parseo y, si aplica, exactamente 1.024 líneas; abortar indicando fichero y línea.

### 7. [menor] La aritmética usada por el objetivo es correcta, pero la clase general de intervalos contiene una resta no intervalar

**Ficheros:** `Interval.cpp:128-152,280-287`; `EvaluationIJSP.cpp:22-27,110-117`; `repro/setup_ga.txt:18-19`; `repro/setup_abce3.txt:14-15`; `repro/setup_feabcls.txt:14-15`; `repro/setup_tsn2.txt:18-19`.

Para duraciones no negativas, la suma `[a,b]+[c,d]=[a+c,b+d]` y el máximo componente a componente están bien implementados. `EvaluationIJSP_Makespan` acumula el máximo de terminaciones de trabajo; GA/ABCE3/fEABCLS configuran EV (midpoint) y TSN2 LEX2, coherente con (d). Sin embargo, `operator-` y `operator-=` calculan `[a-c,b-d]`; la resta intervalar estándar es `[a-d,b-c]`. No parece intervenir directamente en el camino auditado de makespan/SGS, por lo que la severidad aquí es menor, pero es un riesgo si otros operadores usan la resta como aritmética de intervalos.

### 8. [menor] El SGS de inserción es coherente con la semántica componente a componente, pero carece de tests unitarios de huecos adversos

**Ficheros:** `SGS_IJSP_Insertion.cpp:68-120`; `ScheduleIJSP.cpp:128-184,316-361,365-399`; `Interval.cpp:193-195,227-229,254-262`.

El SGS respeta precedencia de trabajo, busca hacia atrás un hueco cuya cabecera sucesora domina componente a componente la terminación candidata, enlaza predecesor/sucesor de máquina y propaga retrasos por ambos extremos. Bajo la interpretación de dos escenarios correlacionados (extremo inferior con inferior y superior con superior), esto es consistente y la comprobación final detecta violaciones en cualquiera de los componentes. No se encontró un contraejemplo estático claro. Aun así, la decisión de hueco depende de una relación parcial y no hay tests pequeños versionados que cubran intervalos cruzados/incomparables, inserción entre dos tareas y cascadas de reparación; los verificadores reutilizan este mismo código.

**Recomendación:** añadir tests con soluciones esperadas calculadas a mano y un test diferencial contra el decoder realmente independiente.

### 9. [mayor] Hay defectos y deuda de memoria que afectan campañas largas

**Ficheros:** `CreationIJSP.h:1071-1087`; `CreationIJSP.cpp:1332-1339`; `SGS_IJSP.h:108-113`; `ScheduleIJSP.cpp:282-299,304-312`; `final/run_phase2c.sh:2-4`.

`CreationSeededSchedule` reserva un `sgs` pero su destructor vacío no lo libera; además contiene `randomSchedule`, que también posee un SGS y presenta un destructor vacío en esta jerarquía. `ScheduleIJSP::updateTopologicalOrder(NULL)` crea un `Random` con `new` sin liberarlo. `ScheduleIJSP::reset()` contiene el typo `mp = mp = -1` y no limpia `ms`; normalmente los enlaces se sobrescriben al reconstruir una secuencia completa, pero deja estado residual peligroso ante schedules parciales/excepciones. El runner troceado reconoce explícitamente un crecimiento aproximado de 400 MB a 1 GB durante 30 runs, lo que confirma que la fuga no es meramente teórica y llevó a cambiar la unidad de ejecución.

**Recomendación:** aplicar RAII (`unique_ptr`), corregir el reset (`mp=ms=-1`), ejecutar ASan/LSan sobre varios runs consecutivos y publicar un log limpio. El troceado mitiga el agotamiento, pero no corrige el defecto.

### 10. [mayor] El paquete no permite reproducir ni auditar de extremo a extremo el paper

**Ficheros:** `.gitignore:1-6,26-38,57-62`; `final/paper/main.tex:1210-1213`; `Makefile:1-17`; `build_test.sh:1-12`.

No existe README en la raíz. Los pools corregidos y MIX están excluidos (`pools_test/` no está en el checkout), al igual que las configuraciones, logs y cromosomas crudos de fase 2. La sección de disponibilidad del manuscrito sólo contiene comentarios TODO: no aporta DOI de Zenodo ni hash público verificable. Faltan un manifiesto de hardware/SO/compilador, versiones de R/paquetes y un comando único desde checkout limpio. El Makefile compila con `-march=native`, `-ffast-math` y `-fpermissive`, que reducen portabilidad y pueden ocultar problemas; además genera el ejecutable en `../FuzzyFW`, circunstancia que ya causó uso accidental de un binario antiguo según `preflight.sh:6-7`.

Los cuatro `results_*.csv` sí contienen 12.810 filas cada uno, exactamente `61*7*30`, lo que apoya la completitud aritmética del agregado, pero no su proveniencia. Sin pools/raw/setup no se pueden reproducir la corrección semántica (e), la muestra de verificación (f), los bloques realmente inyectados ni el tiempo consumido.

**Recomendación:** publicar el depósito antes de revisión final; añadir README con versión fijada de toolchain, checksums, licencia/datos, flujo `build -> validate pools -> run -> aggregate -> figures`, y un manifiesto inmutable que enlace cada fila agregada con su setup, log y solución.

### 11. [menor] La corrección semántica de pools está implementada como reescritura, pero no queda demostrada sobre los pools publicados

**Ficheros:** `seed_consistency_test.cpp:149-170,183-195`; `final/phase2_pools.sh:28-37`; `final/paper/main.tex:312-326`.

La utilidad calcula tanto el máximo componente a componente como la selección lexicográfica por extremo superior, y `--rewrite` conserva la permutación y sustituye el intervalo almacenado por el componente a componente. Esto es conceptualmente adecuado para (e). Sin embargo, devuelve éxito siempre que no haya errores de parseo, incluso si todas las líneas discrepan de la convención esperada, y los pools reescritos no están incluidos para verificar checksums o recuentos. El log versionado no sustituye una comprobación reproducible de los datos exactos usados.

**Recomendación:** hacer que el modo de validación falle cuando el pool "corregido" presenta discrepancias, versionar checksums antes/después y conservar un informe máquina-legible por pool.

## Valoración por afirmación del paper

| Afirmación | Valoración |
|---|---|
| (a) Sustitución de los primeros `k` y bloques circulares | **Respaldada por el código actual**, condicionada a `L=1024`; la procedencia de todos los resultados agregados no es auditable. |
| (b) Runs emparejados por RNG | **No respaldada en sentido real**: mismos enteros de semilla, estados/corrientes divergentes por consumo distinto durante la inicialización. |
| (c) `min(1.5*tconv(A0),900s)` CPU por celda | **Parcialmente respaldada**: se usa reloj de proceso y presupuesto común por brazo, pero con suelo 60 s, agregación por clase y parada aproximada. |
| (d) Máximo componente, EV en evolutivos y LEX2 en TS-N2 | **Respaldada** por implementación y setups revisados. |
| (e) Corrección lexicográfico vs componente a componente | **Implementada de forma plausible**, pero no verificable aquí sobre los pools exactos ausentes. |
| (f) Verificación independiente con decoder alternativo | **No respaldada**: ambos ejecutables reutilizan las clases de decodificación y aritmética del solver. |

## Conclusión final

El repositorio contiene una base técnica sustancial y varias salvaguardas honestas surgidas de fallos detectados durante la campaña. No obstante, en su estado actual **no basta para sostener sin matices las garantías metodológicas del artículo Q1**. Antes de aceptación deberían corregirse o reformularse al menos el pairing RNG y la independencia del decoder, reconciliarse la regla de presupuesto con el protocolo ejecutado y publicarse el paquete completo con trazabilidad por celda. La aritmética central y el mecanismo actual de inyección parecen sólidos; la principal debilidad no es la idea del experimento, sino que algunas garantías descritas son más fuertes que las que implementa y documenta el código.
