# El texto del bucle de investigación

Esto es lo que había programado en el cron, **literal**. Para reanudarlo basta
con lanzar `/loop 30m` seguido del texto entre las marcas, sin cambiar nada.

Se guarda aquí porque el cron vive solo en la sesión de Claude: al cerrar la
aplicación desaparece, y reconstruir el texto de memoria es exactamente la
clase de deriva silenciosa que este protocolo existe para evitar.

## Cadencia

`30m`, es decir `/loop 30m <texto>`. Media hora es la duración medida de una
oleada (210 trabajos, 6.1 horas de CPU, 14 ranuras, ~35 minutos), así que el
bucle despierta aproximadamente cuando hay algo nuevo que mirar.

## El texto, entre las marcas

<<<BEGIN LOOP PROMPT>>>
Bucle de investigación (protocolo en /opt/ijsp/crisp/IJSP/RESEARCH_IDEAS.md, distro WSL "ijsp", árbol /opt/ijsp/crisp/IJSP; ejecutar todo con `wsl.exe -d ijsp -- bash <script>` escribiendo antes el script en /opt/scratch/, nunca comillas anidadas). Comprobar el estado: (1) si hay solvers FuzzyFW o un queue_jobs.sh vivos, la iteración sigue corriendo: informar en una línea y no tocar nada; (2) si una tanda ha terminado (iter/<id>/filter_run.log o full_run.log con línea "queue exit"), correr su análisis predeclarado (iter/<id>/analyze.py filter|full), aplicar literalmente la regla del filtro o el criterio de aceptación de RESEARCH_IDEAS.md, anotar cifras y decisión en la tabla del historial y en experiments/classic_jsp_2026/JOURNAL.md, hacer commit (autor Hernan Diaz), y lanzar el siguiente paso (la confirmación completa si el filtro pasa; si se descarta, revertir SOLO el código de la idea, nunca los registros); (3) si no hay nada corriendo ni pendiente, tomar la siguiente idea del backlog de RESEARCH_IDEAS.md que no esté en el historial (o proponer una nueva, diff pequeño, hipótesis medible, parámetros congelados), asignarle el siguiente id I-00N, anunciar idea e hipótesis al usuario, implementarla, escribir su iter/I-00N/ (make_jobs, analyze, setups) y lanzar su filtro. Reglas duras: nunca dos experimentos a la vez en la máquina; nunca tocar verify_certificate.py, taillard_bounds.csv, las instancias, el banco de semillas ni los presupuestos por clase; el filtro solo descarta, nunca acepta; toda idea probada queda en el historial con sus números; commits y JOURNAL en inglés, RESEARCH_IDEAS.md en español.
<<<END LOOP PROMPT>>>

## Las restricciones permanentes del PI, que el texto de arriba no recoge

No están en el prompt del cron porque se dieron en conversación, y valen tanto
como él:

- **Regla del PI**: trabajar siempre dentro de `C:\ijsp-runner\` y no tocar
  `C:\researchci-runner\`, que es de otro proyecto.
- Cuidado con no modificar cosas de otros experimentos.
- **Ejecutar en secuencial**, nunca dos experimentos apilados en la máquina.
- **Parámetros congelados** de momento, en la configuración 136 de irace.
- El banco de semillas en `C:\Users\diazhernan\CLionProjects\RL Seeds\` es
  **de solo lectura**: es el árbol de otro experimento.
- Que se ha encontrado una solución mejor **no se demuestra con un
  certificado, se demuestra con la propia solución**.
- Las iteraciones deben caber en **media hora**.
