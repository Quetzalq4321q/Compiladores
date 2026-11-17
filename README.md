# Compiladores LP

Analizador léxico y sintáctico para programas LP, con construcción de un árbol de sintaxis abstracta (AST), diagnósticos y ejecución básica mediante intérprete. El ejecutable de consola procesa un archivo `.lp` de principio a fin; en Windows también hay una interfaz gráfica.

## Organización del código

| Archivo o módulo | Responsabilidad |
| --- | --- |
| `src/Token.*` | Define las clases de tokens y sus datos de ubicación. |
| `src/Lexer.*` | Convierte el texto fuente en tokens y registra errores léxicos. |
| `src/SymbolTable.*` | Almacena los identificadores encontrados por el analizador léxico. |
| `src/Parser.*` | Aplica la gramática por descenso recursivo, recupera errores y construye el AST. |
| `src/AST.*` | Define los nodos del programa y su representación textual en árbol. |
| `src/ASTGraph.*` | Transforma los nodos del AST en un grafo y exporta una imagen SVG. |
| `src/ASTGraphView.*` | Dibuja el grafo en la interfaz de Windows con desplazamiento y ampliación. |
| `src/ErrorHandler.*` | Valida la entrada y produce los reportes de tokens, símbolos y errores. |
| `src/Interpreter.*` | Ejecuta el AST cuando el programa fue aceptado. |
| `src/FileConverter.*` | Lee archivos de entrada admitidos por la aplicación. |
| `src/cli_main.cpp` | Coordina lectura, análisis, salida de consola y archivos de reporte. |
| `src/main.cpp` | Implementa la interfaz gráfica para Windows. |
| `tests/test_lexer.cpp`, `tests/test_parser.cpp` | Verifican el análisis léxico, sintáctico, AST e intérprete. |

Los archivos `.lp` de `tests/` incluyen ejemplos válidos e inválidos. `INFORME_ENTREGABLE2.md` resume la arquitectura, las decisiones y las limitaciones. `CHANGELOG.md` registra las modificaciones.

## Requisitos y compilación

- Compilador C++20 y CMake 3.20 o posterior.
- En Windows, MinGW-w64 para compilar la interfaz WinAPI y la consola. CLion puede abrir directamente `CMakeLists.txt`.
- En Linux, se compila la aplicación de consola y las pruebas. La interfaz WinAPI solo está disponible en Windows.

Desde una terminal en la raíz del repositorio:

```text
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

En Windows, los binarios se llaman `Compiladores.exe` (interfaz), `compilador_lp.exe`, `test_lexer.exe` y `test_parser.exe`. En Linux no tienen extensión `.exe` y no se genera `Compiladores`.

## Uso

```text
compilador_lp --no-run tests/entregable2_integral.lp
compilador_lp tests/entregable2_integral.lp
test_lexer
test_parser
```

En Windows, anteponer `build\` o la ruta real de compilación a cada ejecutable. `--no-run` analiza y genera reportes sin ejecutar el programa. Si no se indica un archivo, la consola busca `input/ejemplo.lp`. Los reportes se guardan en `output/` del directorio desde el que se inicia el ejecutable: `tokens.txt`, `tabla_simbolos.txt`, `ast.txt`, `ast.svg`, `errores.txt` y, al ejecutar, `ejecucion.txt`. La consola devuelve código 0 si todo fue correcto, 1 ante errores de entrada, 2 ante errores del programa y 3 si no pudo guardar reportes.

En la interfaz de Windows, el botón **Analizar** abre la pestaña **AST Gráfico**. Los nodos y sus conexiones se dibujan de izquierda a derecha. La rueda desplaza verticalmente, `Ctrl` más rueda cambia la escala y las barras permiten recorrer el resto del árbol. La interfaz también guarda `output/ast.svg`, que puede abrirse en un navegador o visor SVG; `output/ast.txt` conserva una versión textual para inspección rápida. El grafo se genera sin instalar Graphviz ni otra dependencia externa.

Para probar el ZIP portátil en Windows x64, extraerlo completo en una carpeta con permisos de escritura y abrir `abrir_gui.bat` o `ejecutar_cli.bat`. El paquete se genera después de compilar en modo Release:

```powershell
cmake -S . -B cmake-build-release-portable -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release-portable
.\package_portable.ps1 -PackageName Compiladores-portable-windows-x64-ast-grafico
```

El resultado es `dist/Compiladores-portable-windows-x64-ast-grafico.zip`.

## Sintaxis implementada

El programa contiene declaraciones de funciones opcionales y un bloque `main`. Las llamadas usan `func`; las declaraciones también pueden llevar ese prefijo. Se admiten tipos `int`, `float`, `char`, `boolean` y `string`, además de `void` para retorno; las variables y los parámetros tienen tipos básicos y pueden declarar dimensiones de arreglo. Las sentencias admitidas son asignación, llamada, `scanf`, `println`, `if` con `else` opcional, `while` y `return` con expresión. Los bloques se delimitan con llaves y las sentencias indicadas terminan en punto y coma.

De mayor a menor precedencia, las expresiones admiten paréntesis, `!` y signos unarios `+`/`-`; multiplicación, división y módulo; suma y resta; comparación; `&&`; y `||`. Los operadores binarios repetibles se agrupan de izquierda a derecha. La comparación admite una operación por nivel, conforme a la producción actual de la gramática. `println` acepta una lista de expresiones; `scanf` acepta identificadores con índices de arreglo opcionales.

Ejemplo mínimo:

```lp
func int doble(int n) {
    return n * 2;
}

void main() {
    int valor;
    valor = func doble(3 + 4);
    println("Resultado:", valor + 1);
    return valor;
}
```

`tests/entregable2_integral.lp` añade lectura, arreglo multidimensional, condicional, ciclo y operadores con distinta precedencia. Los archivos `tests/entregable2_invalido_*.lp` verifican rechazo y recuperación de errores. El reporte sintáctico incluye línea aproximada, token encontrado, construcción esperada y descripción. El AST se muestra como grafo nativo en Windows y se exporta como SVG en todas las plataformas.

## Limitaciones conocidas

- `for` se reconoce como palabra reservada en el analizador léxico, pero no tiene producción sintáctica implementada.
- El proyecto analiza sintaxis y ejecuta programas básicos; no implementa una comprobación semántica completa de tipos o usos de identificadores.
- En la ejecución actual, `scanf` no solicita datos por teclado: asigna un valor inicial de cero. Para revisar la sintaxis de programas con lectura, usar `--no-run`.
- La conversión de PDF requiere Microsoft Word instalado en Windows. Para la entrega y las pruebas se recomienda usar archivos `.lp`.
- El intérprete limita el número de pasos para evitar ciclos indefinidos.
