# Registro de cambios

## 2.3.1 - 2026-11-24 - Distribución del AST

### Motivo

La distribución anterior asignaba una fila distinta a cada nodo y colocaba los descendientes progresivamente a la derecha. En programas medianos el resultado parecía una escalera y obligaba a recorrer el grafo antes de comprender su estructura.

### Cambios por archivo

- `src/ASTGraph.cpp`: `position` asigna la coordenada vertical según la profundidad. Las hojas reciben espacios consecutivos en el eje horizontal; cada padre queda centrado entre su primer y último hijo. `fromProgram` calcula el tamaño del lienzo a partir del número de hojas y la profundidad máxima. `toSVG` conecta la parte inferior del padre con la parte superior de cada hijo mediante tramos verticales y horizontales. El SVG conserva los nombres y líneas de los nodos.
- `src/ASTGraph.h`: se ajustó el ancho de las tarjetas a 170 unidades y se renombró el parámetro de posicionamiento para expresar que cuenta hojas.
- `src/ASTGraphView.cpp`: el dibujo GDI usa las mismas coordenadas y conexiones que el SVG. `fitGraph` calcula la escala necesaria para mostrar el grafo completo, centra el lienzo y restablece el desplazamiento. El ajuste se aplica al recibir un grafo, al mostrar la pestaña y al cambiar el tamaño de la ventana mientras está activa la vista completa. La rueda con `Ctrl` amplía alrededor del puntero; el doble clic restablece el ajuste. Al señalar una tarjeta se muestra su nombre y número de línea. En escalas pequeñas se omite el texto dentro de las tarjetas para evitar manchas ilegibles.
- `src/ASTGraphView.h`: se expuso `fit` para que la ventana principal pueda restablecer la vista completa.
- `src/main.cpp`: se añadió **Ajustar AST** a la barra superior y se enlazó con `ASTGraphView::fit`. El botón también abre la pestaña gráfica.
- `tests/test_parser.cpp`: la prueba del grafo verifica que el padre esté centrado sobre sus hijos, que los hermanos compartan nivel, que cada hijo esté debajo de su padre y que las tarjetas de un mismo nivel no se solapen.
- `README.md`: se actualizó el uso de la vista gráfica y el nombre del ZIP portátil.
- `MANUAL_USUARIO.md`: se documentaron el ajuste inicial, la ampliación, la inspección de nodos y el retorno a la vista completa.
- `INFORME_ENTREGABLE2.md`: se actualizó la descripción técnica de la distribución por niveles y del ajuste de la vista.

### Comprobación

Se compiló la aplicación en modo Release, pasaron las pruebas de lexer y parser, y el ejemplo integral generó un SVG válido con 73 nodos y 72 conexiones. Se verificó en la interfaz de Windows la vista completa y el botón de ajuste.

## 2.3.0 - 2026-11-17 - AST gráfico

- Se añadió un grafo nativo del AST a la interfaz de Windows, con nodos por categoría, conexiones, desplazamiento y ampliación.
- Se añadió `output/ast.svg` como representación vectorial del árbol en la consola y en la interfaz gráfica.
- Se añadió una prueba de estructura del grafo y escape de caracteres en SVG.
- Se actualizaron el README, el manual y el informe técnico para describir el nuevo flujo.

## 2.2.0 - 2026-11-17 - Entregable 2

### Corregido

- `println` analiza expresiones completas en cada elemento de su lista, incluidas operaciones, índices y llamadas.
- Las asignaciones combinan llamadas a funciones con otros operadores de expresión.
- Los signos unarios aceptan factores entre paréntesis y llamadas a funciones.
- La recuperación sintáctica conserva el comienzo de la siguiente sentencia cuando encuentra un error.
- El árbol textual incluye los índices de los destinos de `scanf`.
- Los reportes sintácticos indican la construcción esperada junto a la línea y el token encontrado.

### Añadido

- Pruebas integrales y archivos `.lp` válidos e inválidos para funciones, arreglos, lectura, control de flujo, precedencia, retorno y recuperación de errores.
- Comentarios breves al inicio de los módulos fuente y las pruebas.
- README con compilación, uso, módulos, sintaxis y limitaciones; informe técnico de la segunda entrega.
- ZIP portátil de Windows actualizado con ambos ejecutables, pruebas y documentación.

## 2.1.0 - 2026-11-12

- Se incorporó el intérprete de AST, con funciones, arreglos, control de flujo y reportes de ejecución.
- Se añadieron protecciones ante división por cero y ciclos que exceden el límite de pasos.
- Se amplió la interfaz WinAPI con una consola de ejecución y se configuró el empaquetado portátil.

## 2.0.0 - 2026-11-11

- Se incorporaron el analizador sintáctico por descenso recursivo, el AST y las primeras pruebas del parser.
- La interfaz gráfica pasó a mostrar el resultado sintáctico y el árbol.

## 1.0.0 - 2026-09-29

- Se completó el analizador léxico y la interfaz de Windows, con reportes de tokens, símbolos y errores.

## 0.3.0 - 2026-09-28

- Se integraron formatos adicionales de entrada y exportación automática de reportes.

## 0.2.0 - 2026-09-27

- Se añadió la interfaz gráfica WinAPI.

## 0.1.0 - 2026-09-25

- Se creó el prototipo del analizador léxico.
