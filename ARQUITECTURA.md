# Arquitectura del Sistema - Analizador Léxico LP

Este documento describe la arquitectura de software, los modelos de autómatas finitos deterministas (AFD), los flujos de datos y los principios de diseño aplicados en el desarrollo del compilador para el lenguaje **LP (Lenguaje de Programación)**.

---

## 1. Visión General de la Arquitectura

El sistema está diseñado bajo un enfoque modular desacoplado en C++20, estructurado en capas con responsabilidades delimitadas:

```mermaid
flowchart TD
    subgraph Entrada["Capa 1: Ingesta y Contención"]
        A["Archivo (.lp, .txt, .docx, .pdf)"] --> B["FileConverter"]
        C["Texto Escrito en GUI"] --> D["ErrorHandler::validarCodigoFuente"]
        B --> E["ErrorHandler::validarArchivo"]
        E -- "Inválido (Imagen/Binario/Vacio)" --> F["Rechazo Inmediato (MessageBox)"]
        E -- "Válido" --> G["Texto Normalizado"]
        D -- "Válido" --> G
    end

    subgraph Nucleo["Capa 2: Motor Léxico y Símbolos"]
        G --> H["Lexer (AFD)"]
        H <--> I["SymbolTable (Identificadores)"]
        H --> J["Vector de Tokens"]
        H --> K["Vector de Errores Léxicos"]
    end

    subgraph Sintactico["Capa 3: Motor Sintáctico (Parser) y AST"]
        J --> L["Parser (Descenso Recursivo)"]
        L --> M["Árbol de Sintaxis Abstracta (AST)"]
        L --> N["Vector de Errores Sintácticos"]
        L --> O["Estado: Aceptado vs Rechazado"]
    end

    subgraph Presentacion["Capa 4: Presentación y Salida"]
        J --> P["Secuencia y Tabla de Tokens"]
        I --> Q["Tabla de Símbolos"]
        M --> R["Renderizador Visual de AST"]
        K & N --> S["Diagnóstico Integral Unificado"]
        P & Q & R & S --> T["GUI WinAPI (5 Pestañas + Banner)"]
        P --> U["output/tokens.txt"]
        Q --> V["output/tabla_simbolos.txt"]
        R --> W["output/ast.txt"]
        S --> X["output/errores.txt"]
    end
```

---

## 2. Diagrama de Estados Finitos (AFD) del Analizador Léxico

El núcleo de escaneo léxico implementa un autómata finito determinista para clasificar el flujo continuo de caracteres:

```mermaid
stateDiagram-v2
    [*] --> INICIO

    INICIO --> NUM_ENTERO : Dígito [0-9]
    NUM_ENTERO --> NUM_ENTERO : Dígito [0-9]
    NUM_ENTERO --> NUM_PUNTO : Punto [.]
    NUM_PUNTO --> NUM_DECIMAL : Dígito [0-9]
    NUM_DECIMAL --> NUM_DECIMAL : Dígito [0-9]
    NUM_DECIMAL --> ERROR_NUM : Punto adicional [.]
    NUM_ENTERO --> ERROR_ID_NUM : Letra [a-zA-Z_]

    INICIO --> IDENTIFICADOR : Letra [a-zA-Z_]
    IDENTIFICADOR --> IDENTIFICADOR : Alfanumérico [a-zA-Z0-9_]
    IDENTIFICADOR --> EVAL_PALABRA_CLAVE : Delimitador / Espacio

    INICIO --> CADENA_TEXTO : Comilla ["]
    CADENA_TEXTO --> CADENA_TEXTO : Caracteres != ["]
    CADENA_TEXTO --> FIN_CADENA : Comilla ["]
    CADENA_TEXTO --> ERROR_CADENA : Fin de línea / EOF

    INICIO --> COMENTARIO_LINEA : Barra [/] + [/]
    COMENTARIO_LINEA --> INICIO : Salto de línea [\n]

    INICIO --> COMENTARIO_BLOQUE : Barra [/] + [*]
    COMENTARIO_BLOQUE --> INICIO : [*] + [/]

    INICIO --> OPERADOR : [=, +, -, *, /, %, <, >, !, &]
    OPERADOR --> TOKEN_EMITIDO : Simbolo simple o compuesto

    INICIO --> DELIMITADOR : [(, ), [, ], {, }, ,, ;]
    DELIMITADOR --> TOKEN_EMITIDO : Token especial

    INICIO --> ERROR_CARACTER : Caracter no reconocido (@, $, #)
```

---

## 3. Descripción de Módulos y Responsabilidades

| Módulo | Archivos | Responsabilidad Primaria |
|---|---|---|
| **Token** | `Token.h`, `Token.cpp` | Definición de tipos léxicos (`LexTokenType`, incluyendo `KW_FUNC`), estructura de metadatos de tokens y mapeo a categorías oficiales de la cátedra. |
| **Tabla de Símbolos** | `SymbolTable.h`, `SymbolTable.cpp` | Almacenamiento asociativo de identificadores, garantizando que cada identificador conserve una posición única (base 0) e historial de apariciones. |
| **Lexer** | `Lexer.h`, `Lexer.cpp` | Implementación del autómata finito determinista (AFD) para la tokenización de texto y detección de incidencias léxicas. |
| **AST** | `AST.h`, `AST.cpp` | Jerarquía de nodos polimórficos (`ProgramNode`, `FunctionDeclNode`, `BlockNode`, etc.) y renderizado en árbol visual UTF-8. |
| **Parser** | `Parser.h`, `Parser.cpp` | Analizador sintáctico por descenso recursivo que valida las reglas gramaticales de LP, genera el AST y recupera errores en modo pánico. |
| **ErrorHandler** | `ErrorHandler.h`, `ErrorHandler.cpp` | Contención de entradas anómalas (imágenes, ejecutables, valores muertos), validación de precondiciones y generación estructurada de reportes léxicos y sintácticos. |
| **FileConverter** | `FileConverter.h`, `FileConverter.cpp` | Ingesta multiformato (`.lp`, `.txt`, `.docx`, `.pdf`) y descompresión de contenido textual en memoria. |
| **GUI WinAPI** | `main.cpp` | Interfaz gráfica nativa de Windows con 5 pestañas interactivas, banner de aceptación/rechazo y persistencia en disco. |
| **Suite de Pruebas** | `tests/test_lexer.cpp`, `tests/test_parser.cpp` | Baterías de pruebas unitarias automatizadas para certificar el 100% de cumplimiento léxico y sintáctico. |

---

## 4. Estrategia de Contención de Errores

Para garantizar la estabilidad del compilador y evitar comportamientos indefinidos o cuelgues, se diseñó una estrategia de **blindaje en cuatro niveles**:

### Nivel 1: Validación Preventiva de Archivos (Pre-ingesta)
Antes de que cualquier flujo de bytes ingrese al motor:
1. **Detección de formato gráfico**: Se interceptan extensiones como `.png`, `.jpg`, `.jpeg`, `.gif`, `.bmp`, `.webp`, `.svg`, etc., notificando inmediatamente al usuario mediante un cuadro de diálogo claro y explícito en lugar de intentar analizar píxeles como código.
2. **Detección de ejecutables y binarios**: Se bloquean archivos `.exe`, `.dll`, `.bin`, `.zip`, `.rar`, etc.
3. **Detección de archivos multimedia**: Bloqueo de archivos de audio y video.
4. **Inspección de integridad de bytes**: Se verifica la ausencia de caracteres nulos (`\0`) en archivos de texto para evitar inyecciones de bytes binarios corruptos.

### Nivel 2: Detección de Valores Muertos (Pre-análisis)
Si el usuario pulsa *Analizar* con el editor vacío o únicamente con espacios, saltos de línea y tabuladores:
* El módulo `ErrorHandler::validarCodigoFuente` intercepta la solicitud y previene la ejecución inútil del analizador, guiando al usuario con una sugerencia de acción.

### Nivel 3: Recuperación de Errores Léxicos (En escaneo)
Si durante el análisis léxico se detectan caracteres ajenos al alfabeto de LP:
* El analizador **no aborta la ejecución** (modo *panic-mode* recuperable).
* Registra el error en la colección de incidencias, almacena la línea y el lexema causante, y continúa con el siguiente carácter para identificar todos los errores presentes en el programa de una sola pasada.

### Nivel 4: Recuperación de Errores Sintácticos (Modo Pánico en Parser)
Si la estructura gramatical del programa falla (por ejemplo, falta de `;` o un bloque no cerrado):
* El `Parser` registra el error exacto con número de línea, token encontrado y construcción esperada.
* Ejecuta `sincronizar()` descartando tokens hasta encontrar un punto de sincronización seguro (`;`, `}`, palabras reservadas como `int`, `while`, etc.), permitiendo que el parser siga validando el resto del archivo sin colapsar.

---

## 5. Salidas Generadas y Persistencia

Al finalizar cada análisis exitoso o con advertencias, el sistema genera de forma concurrente cuatro representaciones en `output/`:

1. **Tokens (`output/tokens.txt`)**:
   * Secuencia en formato estándar LP (`<VOID> <MAIN>...`).
   * Tabla analítica con columnas: `#`, `TOKEN`, `LEXEMA`, `CATEGORIA DEL TOKEN`, `LINEA`.
2. **Tabla de Símbolos (`output/tabla_simbolos.txt`)**:
   * Listado de identificadores únicos con posición base 0, línea de primera aparición y trazabilidad de todas sus apariciones en el código.
3. **Árbol Sintáctico (`output/ast.txt`)**:
   * Estructura jerárquica visual del AST con conectores tipográficos UTF-8.
4. **Reporte de Diagnóstico (`output/errores.txt`)**:
   * Diagnóstico integral con secciones separadas para la Fase 1 (Léxica) y la Fase 2 (Sintáctica: Programa Aceptado vs. Programa Rechazado).
