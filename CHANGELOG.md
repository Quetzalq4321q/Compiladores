# Registro de cambios

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
