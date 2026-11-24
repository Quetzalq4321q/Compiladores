# Informe técnico: segunda entrega

## Alcance

El proyecto recibe un archivo LP, valida la entrada, obtiene tokens, analiza las producciones sintácticas, construye un AST y emite reportes de aceptación o rechazo. Cuando el programa es aceptado y no se indica `--no-run`, el intérprete ejecuta el AST. El árbol se presenta como grafo nativo en la interfaz de Windows y como SVG exportable. También se conserva una versión textual.

## Arquitectura y decisiones

`FileConverter` obtiene el código fuente; `Lexer` genera tokens con línea; `Parser` usa descenso recursivo y devuelve nodos definidos en `AST`; `ASTGraph` convierte esos nodos en vértices y conexiones, coloca los hijos por niveles bajo sus padres y escribe SVG; `ASTGraphView` dibuja el mismo grafo con GDI en Windows, ajusta el árbol completo a la ventana y permite ampliar alrededor del puntero. `ErrorHandler` reúne diagnósticos; `Interpreter` ejecuta el árbol. `cli_main` integra los módulos y escribe los reportes en `output/`. `main` presenta las mismas etapas en la interfaz WinAPI.

La jerarquía de funciones de expresión en `Parser` representa la precedencia de operadores. Los operadores binarios repetibles se construyen de izquierda a derecha. Los signos unarios y la negación se procesan recursivamente, también ante paréntesis o llamadas. Las listas de `println` usan expresiones completas y los índices de `scanf` quedan incluidos en el árbol impreso.

Cada error sintáctico guarda línea, token encontrado, construcción esperada y mensaje. La recuperación avanza hasta un punto y coma, cierre de bloque o inicio reconocible de otra sentencia o declaración, para informar varios problemas en una pasada cuando es viable. Un programa con errores no se ejecuta.

## Verificación

`tests/test_lexer.cpp` comprueba la tokenización. `tests/test_parser.cpp` comprueba producciones, precedencia, AST, estructura del grafo, escape de texto en SVG, ejecución y recuperación de varios errores. Los ejemplos `tests/entregable2_integral.lp`, `tests/entregable2_lectura_arreglo.lp` y `tests/entregable2_invalido_*.lp` permiten repetir las pruebas desde el ejecutable de consola.

## Limitaciones

No hay producción para `for`, aunque el lexer lo reconoce. La comprobación semántica de tipos y declaraciones no es completa. El intérprete no realiza lectura interactiva en `scanf`; asigna cero. La conversión de PDF depende de Microsoft Word en Windows. La interfaz gráfica se compila solo en Windows.

## Distribución del trabajo

Los cambios de esta entrega se registran en Git con la identidad `Quetzalq4321q`. El trabajo se organiza por módulos: análisis léxico (`Lexer`, `Token`, `SymbolTable`), análisis sintáctico y AST (`Parser`, `AST`), visualización (`ASTGraph`, `ASTGraphView`), diagnósticos (`ErrorHandler`), ejecución (`Interpreter`), integración (`cli_main`, `main`), pruebas y documentación. No se asignan nombres individuales a módulos porque el repositorio no contiene una distribución verificable por integrante; el equipo puede completar ese dato antes de presentar la entrega.
