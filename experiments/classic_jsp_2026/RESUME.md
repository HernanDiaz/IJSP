# Dónde se paró el bucle, y cómo seguir

Parada voluntaria el **2026-09-23** a petición del PI, para reiniciar Claude
Desktop. No hay nada roto, nada a medias y nada corriendo.

## Estado exacto

**I-014 está en la mirilla 5 de 6, sin decisión.** Eso es una parada legítima:
la frontera de Pocock está calculada para **seis** miradas y detenerse sin
cruzar no consume ninguna. La secuencia hasta aquí, media por instancia,
`restart − control`:

| mirilla | tiradas/celda | media | mejor en | p | reinicios/tirada |
|---|---|---|---|---|---|
| filtro | 30 (4 inst.) | −0.70 | -- | -- | 1.36 |
| 1 | 5 | −0.70 | 13 de 21 | 0.186 | 1.16 |
| 2 | 10 | −0.37 | 12 de 21 | 0.271 | 1.11 |
| 3 | 15 | −0.15 | 9 de 21 | 0.805 | 1.06 |
| 4 | 20 | −0.17 | 11 de 21 | 0.664 | 1.06 |
| 5 | 25 | **−0.10** | 10 de 21 | 0.702 | 1.05 | | | |

Frontera simétrica **p <= 0.0142**. El mecanismo se ejerce en todas las
mirillas, así que si no cruza no será por no haberse probado.

## Lo único que falta para cerrar I-014

    wsl.exe -d ijsp -- bash /opt/scratch/i014_run.sh w6
    wsl.exe -d ijsp -- bash -lc "cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026 && python3 iter/I-014/analyze.py wave 6"

La oleada 6 son 210 trabajos, 6.1 horas de CPU, unos 35 minutos. El análisis
imprime por sí mismo el veredicto: ACCEPT si cruza por abajo, REJECT NOW si
cruza el control, REJECT si la sexta no cruza. Después: anotar cifras y
decisión en la tabla del historial de `RESEARCH_IDEAS.md` y en `JOURNAL.md`,
commit como Hernan Diaz, y seguir con el backlog.

Los ficheros de trabajo de las seis oleadas **ya están generados y
verificados** (`iter/I-014/jobs_w*.tsv`, 210 trabajos y 210 pares
(tag, instancia) distintos cada uno).

## Guiones listos en /opt/scratch/

| guión | para qué |
|---|---|
| `i014_run.sh <filter|wN>` | lanza una tanda de I-014 con 14 ranuras |
| `i014_look.sh <"celda"> <"asunto"> <"cuerpo">` | anota una mirilla en la tabla y comitea |
| `stall_hazard.py`, `cold_catchup.py` | en `experiments/.../scripts/`, las dos medidas que fundan I-014 |

## Estado del repositorio

Rama `experiment/classic-jsp-crisp`, árbol limpio, todo comiteado. Hay
**commits sin empujar** desde `35a3c77`; el camino es bundle →
`/c/ijsp-runner/ijsp` → `git push origin crisp-push:experiment/classic-jsp-crisp`.

## El bucle de cron

Estaba en `/loop` cada 30 minutos y se **borró** al parar. Para reanudarlo,
volver a lanzar `/loop 30m <el mismo texto del protocolo>`.

## Lo que hay que recordar al volver, en dos líneas

Catorce ideas, **cero mecanismos aceptados**. El único número grande del
proyecto es **+21.55** de I-013: cambiar la regla del tabú de *coge el mejor* a
*coge el primero que mejora* cuesta 21 unidades de makespan, lo que significa
que la superficie es **plana alrededor del óptimo voraz y se desploma al
salir**. Las cinco medidas sobre el orden de los vecinos de N2 están cerradas.
El foco está ahora en **qué se hace con el movimiento elegido** y en **cómo se
reparte el tiempo**, que es donde vive I-014 y lo que queda del backlog
(B-1 análisis gratis, B-6 intentos de récord, B-8 reparto de la búsqueda
local).
