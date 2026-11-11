# Compiladores - Analizador Léxico y Sintáctico (Parser) para Lenguaje LP

<p align="center">
  <img src="res/compiler.png" width="128" height="128" alt="Icono Compilador LP" />
  <br>
  <strong>Compilador, Analizador Léxico, Parser por Descenso Recursivo y Árbol AST para Lenguaje LP (C++20)</strong>
</p>

Compilador desarrollado en C++20 para el lenguaje formal **LP (Lenguaje de Programación)**, correspondiente al **Entregable 1 (30%) de la fase de Análisis Sintáctico**. Incluye analizador sintáctico por descenso recursivo puro, construcción del Árbol de Sintaxis Abstracta (AST), diferenciación formal entre programas aceptados y rechazados, recuperación de errores sintácticos en modo pánico, interfaz gráfica nativa en Windows (WinAPI) con 5 pestañas interactivas, versión CLI multiplataforma (Linux/Docker/Windows), tabla de símbolos integrada, soporte multiformato (.lp, .txt, .docx, .pdf) y suites de pruebas automatizadas.

---

##  Documentación del Proyecto

Para consultar los detalles técnicos, guías y registros de evolución del proyecto, consulta los siguientes documentos:

* 📄 **[Changelog (Registro de Cambios)](CHANGELOG.md)**: Historial completo de versiones (incluyendo versión 2.0.0 - Entregable 1), nuevas características, correcciones y notas de lanzamiento.
* 🔧 **[Documentación Técnica de Funciones](DOCS_FUNCIONES.md)**: Detalle exhaustivo de todas las clases (`Parser`, `ASTNode`, `Lexer`, `ErrorHandler`, `SymbolTable`, `FileConverter`), métodos y algoritmos.
* 🏛️ **[Arquitectura del Sistema](ARQUITECTURA.md)**: Modelo arquitectónico en 4 capas, diagramas Mermaid, árbol sintáctico y estrategia de contención de errores en 4 niveles.
* 📖 **[Manual de Usuario](MANUAL_USUARIO.md)**: Guía paso a paso para operar la interfaz gráfica de 5 pestañas, la versión CLI, Docker y consultar los reportes en `output/`.


---

## 1. Especificación del Entregable 1 (Analizador Sintáctico)

### 1.1 Gramática Libre de Contexto de LP
El analizador sintáctico implementa directamente la gramática no ambigua mediante descenso recursivo:
* `PROGRAMA -> LISTA_FUNCIONES PRINCIPAL`
* `LISTA_FUNCIONES -> DECLARACION_FUNCION LISTA_FUNCIONES | ε`
* `DECLARACION_FUNCION -> [func] TIPO_RETORNO id ( PARAMETROS ) BLOQUE`
* `TIPO_RETORNO -> TIPO | void`
* `TIPO -> TIPO_BASE DIMENSION`
* `TIPO_BASE -> int | float | char | boolean | string`
* `DIMENSION -> [ num_int ] DIMENSION | ε`
* `PARAMETROS -> TIPO id LISTA_PARAMETROS | ε`
* `PRINCIPAL -> [TIPO_RETORNO] main ( ) BLOQUE`
* `BLOQUE -> { LISTA_VARIABLES COMANDOS }`
* `LISTA_VARIABLES -> TIPO id DIMENSION LISTA_ID ; LISTA_VARIABLES | ε`
* `COMANDOS -> COMANDO COMANDOS | ε`
* `COMANDO -> ASIGNACION | RETORNO | LECTURA | ESCRITURA | SELECCION | MIENTRAS | FUNCION`
* `ASIGNACION -> id [DIMENSION] = COMPLEMENTO ;`
* `COMPLEMENTO -> EXPRESION | FUNCION`
* `FUNCION -> func id ( ARGUMENTOS )`
* Jerarquía de expresiones: `EXPRESION -> EXPRESION_O -> EXPRESION_Y -> EXPRESION_NO -> EXPRESION_COMP -> EXPRESION_ADITIVA -> EXPRESION_MULT -> TERMINO_EXPR`.
* `RETORNO -> return EXPRESION ;`

### 1.2 Programas de Prueba del Entregable 1
* **Programa Mínimo Válido (Sección 4.2)**: [`tests/entregable1_valido.lp`](tests/entregable1_valido.lp) &rarr; **Aceptado (0 errores)**.
* **Programa Mínimo Inválido (Sección 4.3 - Falta `;`)**: [`tests/entregable1_invalido1.lp`](tests/entregable1_invalido1.lp) &rarr; **Rechazado (detecta falta de punto y coma)**.
* **Programa Inválido por Bloque Abierto**: [`tests/entregable1_invalido2.lp`](tests/entregable1_invalido2.lp) &rarr; **Rechazado**.
* **Programa con Funciones y `func`**: [`tests/entregable1_funciones.lp`](tests/entregable1_funciones.lp) &rarr; **Aceptado con AST completo**.

## 1. Especificación Léxica de LP

### 1.1 Números e Identificadores
* **NUM_INT**: Dígitos enteros `[0-9]+` &rarr; `<NUM_INT>`
* **NUM_DEC**: Dígitos decimales `[0-9]+\.[0-9]+` &rarr; `<NUM_DEC>`
* **ID**: Identificadores `[a-zA-Z_][a-zA-Z0-9_]*` &rarr; `<ID,pos>` *(donde pos es su índice único en la tabla de símbolos)*
* **TEXTO**: Cadenas de caracteres `"..."` &rarr; `<TEXTO>`

### 1.2 Palabras Reservadas
| Lexema | Token | Lexema | Token |
|---|---|---|---|
| `int` | `<INT>` | `void` | `<VOID>` |
| `float` | `<FLOAT>` | `main` | `<MAIN>` |
| `char` | `<CHAR>` | `if` | `<IF>` |
| `boolean` | `<BOOLEAN>` | `else` | `<ELSE>` |
| `for` | `<FOR>` | `while` | `<WHILE>` |
| `scanf` | `<SCANF>` | `println` | `<PRINTLN>` |
| `return` | `<RETURN>` | `string` | `<STRING>` |

### 1.3 Operadores
* **Asignación:** `=` &rarr; `<=>`
* **Aritméticos:** `+` &rarr; `<+>`, `-` &rarr; `<->`, `*` &rarr; `<*>`, `/` &rarr; `</>`, `%` &rarr; `<%>`
* **Lógicos:** `&&` &rarr; `<&&>`, `||` &rarr; `<||>`, `!` &rarr; `<!>`
* **Comparación:** `>`, `>=`, `<`, `<=`, `!=`, `==` &rarr; `<COMP>`

### 1.4 Delimitadores y Símbolos Especiales
* `(` &rarr; `<(>`, `)` &rarr; `<)>`
* `[` &rarr; `<[>`, `]` &rarr; `<]>`
* `{` &rarr; `<{>`, `}` &rarr; `<}>`
* `,` &rarr; `<,>`, `;` &rarr; `<;>`

### 1.5 Comentarios
* De una línea: `//.*\n` &rarr; Reconocidos y descartados.
* De bloque: `/* ... */` &rarr; Reconocidos y descartados.

---

## 2. Ejemplo Oficial de Ejecución

### Código Fuente de Entrada:
```c
void main() {
int edad = 20;
float promedio = 15.5;
if (edad >= 18 && edad <= 60) {
println("Edad valida");
}
return;
}
```

### Secuencia de Tokens Resultante:
```text
<VOID> <MAIN> <(> <)> <{>
<INT> <ID,0> <=> <NUM_INT> <;>
<FLOAT> <ID,1> <=> <NUM_DEC> <;>
<IF> <(> <ID,0> <COMP> <NUM_INT> <&&> <ID,0> <COMP> <NUM_INT> <)> <{>
<PRINTLN> <(> <TEXTO> <)> <;>
<}>
<RETURN> <;>
<}>
```

### Tabla de Símbolos:
| Posición | Identificador | Línea Inicial | Apariciones |
|---|---|---|---|
| 0 | `edad` | Línea 2 | L2, L4, L4 |
| 1 | `promedio` | Línea 3 | L3 |

---

## 3. Compilación y Ejecución

### 3.1 Aplicación Visual (GUI Windows con 5 Pestañas y AST)
```powershell
cmake --build cmake-build-debug --target Compiladores
.\cmake-build-debug\Compiladores.exe
```

### 3.2 Aplicación por Consola (CLI Multiplataforma con AST y Diagnóstico)
```powershell
cmake --build cmake-build-debug --target compilador_lp
.\cmake-build-debug\compilador_lp.exe input/ejemplo.lp
```

### 3.3 Ejecutar Pruebas Automatizadas
```powershell
# Pruebas del Analizador Léxico (7 tests)
cmake --build cmake-build-debug --target test_lexer
.\cmake-build-debug\test_lexer.exe

# Pruebas del Analizador Sintáctico y AST - Entregable 1 (7 tests)
cmake --build cmake-build-debug --target test_parser
.\cmake-build-debug\test_parser.exe
```

---

## 4.  Entorno Docker (Dockeado)

El proyecto incluye soporte completo para ejecución contenedorizada en Linux mediante Docker y Docker Compose:

### Construir y Ejecutar con Docker:
```bash
# Construir la imagen optimizada
docker build -t compiladores-lp .

# Ejecutar el analizador sobre el archivo por defecto
docker run --rm -v $(pwd)/output:/app/output compiladores-lp

# Ejecutar sobre un archivo específico
docker run --rm -v $(pwd)/input:/app/input -v $(pwd)/output:/app/output compiladores-lp input/mi_codigo.lp

# Ejecutar la suite de pruebas unitarias
docker run --rm --entrypoint /app/test_lexer compiladores-lp
```

### Uso con Docker Compose:
```bash
# Levantar y procesar input/ejemplo.lp
docker compose up compilador

# Ejecutar pruebas unitarias
docker compose run --rm test
```

---

## 5. Publicar Release en GitHub:
```powershell
.\release.ps1 1.0.0
```
