# INFORME DE REVISIÓN

- **Revista:** Computers & Operations Research
- **Manuscrito:** «Neighbourhood Structures and Ranking Operators for the Interval Job Shop Scheduling Problem»
- **Autor:** Hernán Díaz Rodríguez (Department of Computing, University of Oviedo)
- **Tipo de contribución:** Artículo de investigación (Full Research Paper)
- **Revisor:** Catedrático en Inteligencia Artificial
- **Fecha de la revisión:** 19 de mayo de 2026

---

## 1. Síntesis del manuscrito

El artículo aborda el Problema de Programación Job Shop con duraciones de procesamiento intervalares (Interval Job Shop Scheduling Problem, IJSP). Las aportaciones principales declaradas por el autor son cinco:

1. Una formalización del concepto de **criticalidad extrema** (un arco u operación es extremo crítico si pertenece a un camino crítico del grafo del mejor caso G⁻(σ) o del peor caso G⁺(σ)), con la prueba de su contención respecto de los conjuntos de tareas posible y necesariamente críticas en el sentido de Fortin et al. (2010) y Artigues et al. (2015).
2. La definición de una vecindad H(σ) basada en arcos extremo críticos y la prueba formal de tres propiedades estructurales —factibilidad, no pérdida de vecinos mejorables y conectividad— válidas simultáneamente para cuatro operadores de ranking intervalar (EV, LEX1, LEX2 y YX).
3. La instanciación de cinco variantes (N₁, N₂, N₃, N_ext y la novedosa N₈, esta última adaptada de Xie et al. 2022).
4. Un estudio experimental en tres fases sobre 82 instancias (hasta 1.000 operaciones) y 12.300 ejecuciones.
5. El descubrimiento empírico, con explicación estructural, de que el operador LEX2 produce intervalos de makespan entre un 5 % y un 11 % más estrechos que las alternativas.

---

## 2. Valoración global

El manuscrito presenta una contribución coherente con la línea editorial de *Computers & Operations Research*: combina un avance teórico riguroso (definiciones formales y tres demostraciones completas) con un estudio empírico extenso y metodológicamente correcto. El hallazgo del **efecto LEX2–anchura**, junto con su interpretación estructural a través del análisis de flotantes en G⁺(σ), constituye a mi juicio el resultado más novedoso y de mayor interés para la comunidad. La observación complementaria de que N₈ es competitivo en el caso determinista pero perjudicial en el caso intervalar —y la razón estructural que lo explica— es otro hallazgo valioso porque revierte la intuición habitual de transferir vecindades del JSP clásico al IJSP.

Dicho esto, el manuscrito contiene varias deficiencias —algunas estrictamente editoriales pero bloqueantes, otras de fondo— que deben ser corregidas antes de su eventual aceptación. La sección de disponibilidad de datos y los agradecimientos no están completos (contienen marcadores `[TODO]`); la comparación con el estado del arte algorítmico para IJSP es inexistente; la sensibilidad a otros niveles de incertidumbre no se reporta; y algunos aspectos algorítmicos (procedimientos `isViableSwap`, propagación BFS de N₈, integración con fEABC) necesitan más detalle para garantizar la reproducibilidad.

---

## 3. Aspectos positivos

- **Solidez teórica.** Las Definiciones 1 y 2, la Proposición 1 y los Teoremas 1 y 2 son correctos y están escritos con el grado de formalidad que cabe esperar en COR. La conexión entre criticalidad extrema y los conceptos de posible/necesariamente crítico de Fortin et al. (2010) y Artigues et al. (2015) está bien establecida.
- **Marco unificado.** La propiedad de *no loss of improving neighbours* se demuestra para los cuatro operadores intervalares simultáneamente, gracias al hecho de que todos ellos refinan el preorden producto ≤₂. Esta unificación es elegante y operativamente útil.
- **Diseño experimental.** Tres fases con objetivos diferenciados (comparación de operadores en una configuración común, comparación de vecindades tras ajuste con irace, y validación CRISP). 82 instancias y 30 ejecuciones por configuración proporcionan potencia estadística suficiente.
- **Rigor estadístico.** Uso adecuado del test de Friedman, tests de Wilcoxon pareados, corrección de Holm–Bonferroni y tamaños de efecto *r* con umbrales convencionales (small / medium / large). El reporte de significación junto con tamaño de efecto es buena práctica que muchos trabajos del área omiten.
- **Análisis estructural del efecto LEX2.** En lugar de limitarse a reportar la diferencia, el autor formula dos hipótesis competidoras (H1: LEX2 «tensa» G⁺(σ) directamente; H2: LEX2 deja más holgura estructural) y las contrasta con un análisis de flotantes. Los resultados de las Figuras 3(b) y 3(c) apoyan H1 y falsan H2 de manera convincente.
- **Honestidad sobre N₈.** El autor incluye N₈ como una variante exploratoria y reporta abiertamente su mal comportamiento en el caso intervalar, incluso aportando una explicación causal coherente (la incompatibilidad entre el filtro lower-bound basado en midpoint y la asimetría G⁻/G⁺ típica de IJSP). Este tipo de resultado negativo, bien argumentado, es de utilidad para la comunidad.
- **Reproducibilidad parcial.** Hardware, software, banderas de compilación, configuraciones tuneadas por irace (Tabla `tab:tuned`) y presupuesto computacional están reportados con un nivel de detalle suficiente.

---

## 4. Comentarios mayores

### M1. Ausencia de comparación con el estado del arte algorítmico para IJSP

El estudio compara cinco vecindades entre sí (Fase B) y valida sobre los problemas CRISP frente a las BKS de Taillard y la OR-Library (Fase C). No se incluye, sin embargo, ninguna comparación frente a algoritmos publicados específicamente para IJSP. Sin esa comparación, no es posible situar a fEABC con la mejor configuración (LEX2, N_ext) dentro del estado del arte. Solicito al autor que incluya una tabla comparativa frente, al menos, a sus propios trabajos previos para IJSP (DiazFEABC2023, DiazICAE2023, DiazIPMU2020) y, si los datos son accesibles, frente a Lei (2012c) y al estimador de riesgo por aprendizaje automático de Calamita et al. (2025). La comparación debe ser sobre métricas comunes (midpoint, anchura, mejor solución conocida intervalar si la hubiera) y, preferentemente, sobre las mismas instancias.

### M2. Un único nivel de incertidumbre (±7,5 %)

Todos los experimentos usan p_ij ∈ [0,925 p*, 1,075 p*]. El autor reconoce esta limitación en la Sec. 7.5, pero, dado que el principal resultado empírico de la fase A (efecto LEX2–anchura) depende intrínsecamente de la magnitud de la incertidumbre, considero necesario verificar la robustez del efecto sobre al menos dos o tres anchuras adicionales (p. ej., ±2,5 %, ±15 % y ±25 %). Un subconjunto reducido (15–20 instancias de tamaño medio) sería suficiente y consumiría aproximadamente un 10 % del presupuesto experimental ya invertido. Sin esta verificación, la generalización del resultado queda comprometida.

### M3. Cambio de búsqueda local entre la Fase A y la Fase B

La Fase A utiliza hill-climbing de primera mejora y la Fase B tabu search ajustado por irace. La justificación dada (Sec. 6.5) —que hill-climbing oculta las diferencias estructurales entre vecindades al converger rápidamente— es razonable, pero introduce un factor de confusión sobre la comparación cruzada entre fases. Recomiendo:

- **(a)** Reportar también la Fase A con tabu search no ajustado en un apéndice o material suplementario, para verificar que el efecto LEX2–anchura persiste con la búsqueda local empleada en la Fase B.
- **(b)** Discutir explícitamente que fijar LEX2 en la Fase B introduce un sesgo de selección y que la conclusión «N_ext es la vecindad más fuerte tras el ajuste» se condiciona a este operador. En estricto rigor, una comparación neutra de la Fase B exigiría ajustar la pareja (vecindad, operador) en lugar de fijar LEX2.

### M4. Detalle algorítmico insuficiente para reproducibilidad plena

Aunque la base fEABC se remite a DiazFEABC2023, los lectores de COR esperan que el manuscrito sea autocontenido en lo esencial. Solicito un apéndice (o ampliación de la Sec. 5) que incluya pseudocódigo de:

- **(a)** La función `isViableSwap` utilizada por N_ext (¿se evalúa por separado en G⁻(σ) y G⁺(σ)? ¿cómo se combinan ambos lower bounds para decidir la viabilidad?).
- **(b)** La propagación BFS de cabezas/colas empleada por las reinserciones extra-block de N₈.
- **(c)** La integración del local search en el ciclo de fEABC (propagación lamarckiana, fracción de individuos receptora, momento de aplicación).

Sin estos detalles, el lector no puede reimplementar las variantes con seguridad.

### M5. Datos sin enlazar y reproducibilidad bloqueada por marcadores `[TODO]`

La sección «Data availability» contiene literalmente `[TODO: Confirm or replace.]` y un placeholder `https://github.com/<TODO-org>/<TODO-repo>`. COR aplica la Option C de la política de datos de Elsevier, que exige depósito en un repositorio público y enlace permanente, o bien una justificación explícita de por qué no es posible compartir los datos. En su forma actual, el manuscrito no es admisible para publicación. Es imprescindible publicar el código fuente, los scripts de irace y los datos brutos (rutas Friedman / Wilcoxon, ficheros de configuración y de salida) en un repositorio (Zenodo, figshare o GitHub con DOI) antes de la aceptación.

### M6. Sección de Acknowledgements incompleta

Los marcadores `[TODO: Funding sources, project IDs, computing resources.]` también deben resolverse. Si el trabajo ha recibido financiación pública o competitiva, los códigos de proyecto deben aparecer; si no la ha recibido, indicarlo explícitamente. La omisión puede generar incompatibilidad con compromisos de las agencias financiadoras del autor.

### M7. *Heavy self-citation* y posible sesgo de cobertura bibliográfica

El manuscrito cita seis trabajos propios (DiazIPMU2020, DiazHAIS2020, DiazICAE2023, DiazIGPL2022, DiazFEABC2023 y la referencia DiazICAE2023 vía Sec. 7.1). Aunque parte de ellos son indispensables (en particular DiazFEABC2023, base del algoritmo), considero excesivo el número y rogaría al autor revisar si todas las citas son estrictamente necesarias. En paralelo, no se citan trabajos relevantes recientes que han atacado el IJSP o problemas próximos desde otros ángulos: por ejemplo, los trabajos de Mou et al., Zheng et al. o Wang et al. en metaheurísticas para IJSP/IJSPS publicados entre 2022 y 2025 deberían examinarse. Esta laguna afecta también al posicionamiento del trabajo (M1).

### M8. Limitada conexión con la línea Tamssaouet (2023) / García Gómez (2026)

El artículo cita estos dos trabajos como dirección futura (Sec. 2.3), pero la conexión es más profunda de lo que se reconoce: los lower bounds de García Gómez et al. (2026) podrían integrarse directamente en el filtro de N₈, atacando la causa raíz del problema diagnosticado en la Sec. 7.4 (el filtro midpoint descarta moves que reducen C⁺_max independientemente de C⁻_max). Una discusión más sustantiva en la Sec. 7.5 reforzaría el manuscrito y le abriría una continuación natural.

---

## 5. Comentarios menores

- **m1.** La cabecera de `references.bib` indica literalmente «Target: Applied Soft Computing», mientras que el manuscrito está dirigido a *Computers & Operations Research*. Corregir antes del envío para evitar dudas al editor.
- **m2.** Sec. 6.1: la elección de la métrica común («midpoint of the makespan interval») es razonable como *tertium comparationis*, pero conviene reportar también, en material suplementario, los resultados con la métrica intrínseca de cada operador (LEX1, LEX2, YX). De lo contrario, podría argumentarse que la métrica favorece indirectamente a EV.
- **m3.** Proposición 1: las contenciones «extreme critical ⊆ possibly critical» y «necessarily critical ⊆ extreme critical» se enuncian pero no se aclara si la inclusión es estricta en general. Un contraejemplo breve (incluso a pie de página) confirmaría que el «approximation» mencionado en la Sec. 3.3 es real y no degenerado.
- **m4.** Teorema 2: en el lema clave se invoca «every ranking R refines ≤₂». EV es un preorden total y no orden total estricto. Conviene aclarar que el refinamiento se entiende en sentido débil (no estricto) para EV; la prueba sigue siendo válida, pero merece una nota.
- **m5.** Ecuación (2): el `max` intervalar se define como `[max(a⁻,b⁻), max(a⁺,b⁺)]`. Esta operación coincide con la max para la propagación de longitudes en G⁻ y G⁺ por separado, pero no es la max en el sentido del orden ≤₂. Conviene precisarlo: lo que se está calculando son las longitudes de caminos en cada grafo extremo, no la max intervalar como objeto algebraico.
- **m6.** Sec. 5.4: la descripción de `isViableSwap` es muy esquemática. ¿Se aplica el procedimiento de Nowicki & Smutnicki (2005) sobre cada grafo extremo por separado? ¿Cómo se combinan los dos lower bounds? ¿Se exige que el lower bound mejore en ambos extremos, en al menos uno, o que sea mejor según LEX2? Aclarar.
- **m7.** Tablas 1 y 5: añadir desviación estándar o IQR junto al midpoint medio y la mediana de tiempo de ejecución, aunque sea en material suplementario.
- **m8.** Figuras 1 y 2: el código LaTeX usa `\figorplaceholder`, lo que sugiere que en la versión revisada del fuente las figuras podrían no estar presentes. Verificar que los PDFs definitivos estén incluidos al subir el manuscrito.
- **m9.** Sec. 6.6: la afirmación «to the best of my knowledge, the first experimental evidence that a CRISP-good neighbourhood can be IJSP-bad for purely structural reasons» es interesante y plausible, pero conviene matizarla: en el contexto de FJSP fuzzy se han observado fenómenos cualitativamente similares. Recomiendo suavizar el lenguaje o citar trabajos cercanos que se hayan revisado al respecto.
- **m10.** Voz autoral: hay alternancia entre primera persona singular («I formalise», «I prove») y plural impersonal («we prove», «we obtain»), especialmente al final de las pruebas del Teorema 1 y la Proposición 2. Unificar a la primera persona singular —dado que se trata de un artículo de autor único— o, si se prefiere, a la voz impersonal.
- **m11.** Notación: una notación (Algoritmo, N, R) explícita ayudaría a navegar la Sección 6 (p. ej., «(fEABC, N_ext, LEX2)»). Eliminaría ambigüedades sobre qué configuración se compara en cada tabla.
- **m12.** Sec. 6.5: la frase «C_ref es el mínimo midpoint alcanzado por las cinco vecindades en la misma ejecución» debe aclararse: ¿la misma ejecución se refiere a la misma semilla, o al agregado de 30 ejecuciones? Esto afecta la interpretación de la RPD.
- **m13.** Sec. 6.5: sería ilustrativo un breve análisis de por qué irace selecciona GOX y swap exclusivamente para N₃, mientras los demás convergen a JOX. ¿Hay correlación con el tamaño esperado de la vecindad?
- **m14.** Sec. 7.1: la recomendación práctica de usar N₂ para instancias grandes está bien sustentada, pero conviene reportarla también en términos de coste/beneficio cuantitativo (p. ej., calidad por segundo de CPU) en una pequeña tabla resumen.
- **m15.** El uso del intervalo 15 % de anchura relativa (±7,5 %) debe justificarse explícitamente con una referencia a la literatura previa o a un escenario industrial concreto. Aparece como decisión razonable pero arbitraria.
- **m16.** Abstract: cuenta aproximadamente 290 palabras; la guía de Elsevier sugiere un máximo de 250. Recortar (sugiero podar la enumeración detallada de findings y dejar solo los dos más relevantes).
- **m17.** Highlights: 5 ítems; verificar que cada uno respeta el límite estricto de 85 caracteres con espacios; un par parecen estar al filo del límite.
- **m18.** Graphical Abstract: COR lo requiere a la presentación. No aparece en el material remitido. Preparar y adjuntar.
- **m19.** Sección 1.2 («Paper Organisation»): redacción telegráfica; integrar como párrafo final de la Introducción con prosa fluida es preferible al listado actual.

---

## 6. Observaciones editoriales

- Verificar la consistencia del estilo de cita numérica en todas las llamadas (`\cite{}` en el fuente). Aparecen algunas con ortografía castellana de los apellidos que conviene comprobar.
- Comprobar la coherencia tipográfica de subíndices y superíndices (N₁ vs `N1`, G⁻ vs `G^-`) tras la compilación final.
- Revisar el espaciado de unidades: «s», «ms», «GB», «GHz» con espacio insecable antes del valor numérico.
- CRediT statement: como autor único, la enumeración de roles es correcta; verificar que el formato cumple las indicaciones de COR.

---

## 7. Preguntas concretas al autor

- **P1.** ¿Puede el autor describir con precisión cómo combina `isViableSwap` los lower bounds de los dos grafos extremos en N_ext?
- **P2.** ¿Persiste el efecto LEX2–anchura cuando se sustituye fEABC por una metaheurística diferente (p. ej., tabu search puro o ILS)? Esto separaría el efecto del operador del efecto del algoritmo base.
- **P3.** En la Tabla `tab:phaseB_groups`, N₁ supera a N_ext en las dos clases más grandes (tai50×15 y tai50×20). ¿Conjetura el autor una explicación estructural o se trata de ruido experimental? Si lo primero, ¿es replicable variando el seed?
- **P4.** ¿Por qué se ha excluido del estudio la familia tai100×20? ¿Es estrictamente un problema de tiempo de cómputo, o hay alguna razón metodológica adicional?
- **P5.** ¿Cuál es la varianza inter-ejecución del resultado dentro de cada (instancia, configuración)? La sola diferencia de medias —que es lo reportado en buena parte de las tablas— no permite juzgar la estabilidad de las configuraciones.

---

## 8. Recomendación

Mi recomendación es **REVISIÓN MAYOR (Major Revision)**.

**Justificación.** La contribución teórica (criticalidad extrema y vecindad H(σ) con sus tres garantías estructurales demostradas) es genuina, técnicamente correcta y bien escrita. El descubrimiento del efecto LEX2–anchura, junto con la prueba estructural mediante el análisis de flotantes, es novedoso y de relevancia para la comunidad. La metodología experimental y estadística es sólida.

No obstante, antes de poder recomendar la aceptación es necesario abordar cuatro cuestiones bloqueantes (**M1**: comparación con estado del arte ausente; **M2**: un único nivel de incertidumbre; **M5** y **M6**: secciones obligatorias incompletas con marcadores `[TODO]`) y un conjunto de cuestiones secundarias que mejorarán sustancialmente la claridad y reproducibilidad del trabajo (**M3, M4, M7, M8** y los comentarios menores **m1–m19**).

Una vez subsanadas estas cuestiones, considero que el manuscrito tendría una contribución relevante en línea con los estándares de *Computers & Operations Research*. Animo encarecidamente al autor a preservar el núcleo teórico y experimental del trabajo, que es de calidad notable.

---

## 9. Confidencialidad y conflictos de interés

El revisor declara no tener conflictos de interés con el autor ni con la institución. La presente revisión se ha realizado bajo confidencialidad de la información recibida.
