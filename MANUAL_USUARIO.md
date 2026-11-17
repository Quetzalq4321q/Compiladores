# Manual de usuario de Compiladores LP

## Preparación

En Windows x64, extraiga todo el ZIP portátil en una carpeta con permisos de escritura. Abra `abrir_gui.bat` para usar la interfaz o `ejecutar_cli.bat` para usar la consola. Los archivos `.lp` de `tests/` contienen ejemplos para la demostración.

## Interfaz gráfica

La aplicación tiene seis pestañas: código fuente, consola de ejecución, secuencia de tokens, tabla de símbolos, AST gráfico y diagnóstico de errores.

1. Escriba el programa LP en **Entrada (Código Fuente)** o use **Abrir Archivo**. Se admiten archivos `.lp`, `.txt`, `.docx` y `.pdf`; la conversión de PDF requiere Microsoft Word en Windows.
2. Pulse **Analizar**. Si el programa es válido, se abrirá **AST Gráfico**. Si hay errores, se abrirá **Diagnóstico y Errores**.
3. En **AST Gráfico**, las tarjetas representan funciones, bloques, sentencias, expresiones y valores. Las líneas muestran la relación entre nodos. Use la rueda para desplazarse verticalmente, `Ctrl` más rueda para ampliar o reducir y las barras de desplazamiento para recorrer el árbol.
4. Pulse **Ejecutar LP** para interpretar un programa aceptado y consultar su salida en **Consola de Ejecución**.

Los botones **Ejemplo OK** y **Ejemplo Error** cargan programas de demostración. **Limpiar** vacía el editor y los paneles. La representación gráfica no requiere instalar Graphviz.

## Archivos de salida

La aplicación guarda reportes en `output/`. En el ZIP portátil esa carpeta está junto a los ejecutables; al compilar desde CLion se usa la carpeta `output/` del proyecto.

| Archivo | Contenido |
| --- | --- |
| `tokens.txt` | Secuencia y tabla de tokens. |
| `tabla_simbolos.txt` | Identificadores registrados por el analizador léxico. |
| `ast.svg` | Grafo vectorial del AST, abrible en un navegador o visor SVG. |
| `ast.txt` | Árbol en texto para consulta rápida. |
| `errores.txt` | Resultado de aceptación o rechazo y detalles de errores. |
| `ejecucion.txt` | Salida y estado del intérprete cuando se ejecuta el programa. |

## Consola

Desde una terminal situada en la carpeta del ZIP:

```text
compilador_lp.exe --no-run tests\entregable2_integral.lp
compilador_lp.exe tests\entregable2_integral.lp
```

`--no-run` realiza el análisis y genera `ast.svg` sin ejecutar el programa. Si no se indica un archivo, la consola usa `input/ejemplo.lp`. El código de salida es 0 para proceso correcto, 1 para error de entrada, 2 para error del programa y 3 si falla la escritura de reportes.

## Alcance de la lectura

El analizador reconoce `scanf` y sus índices de arreglo. El intérprete actual asigna cero a los destinos de `scanf` y no solicita datos por teclado. Para revisar programas que usan lectura sin ejecutarlos, use `--no-run`.
