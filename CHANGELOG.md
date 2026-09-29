# Changelog - Compiladores LP

Todos los cambios notables en el proyecto **Compiladores (Analizador Léxico LP)** están documentados en este archivo.

El formato está basado en [Keep a Changelog](https://keepachangelog.com/es-ES/1.0.0/) y este proyecto se adhiere a [Semantic Versioning](https://semver.org/lang/es/).

---

## [1.0.0] - 2026-09-29

### Entrega Final - Analizador Léxico Completo, GUI WinAPI y Contención de Errores

Esta versión representa la entrega formal y completa del proyecto según las directivas de la cátedra de Compiladores (Prof. Nury Yuleny Arosquipa Yanque) y los requerimientos del lenguaje LP (Lenguaje de Programación).

### Añadido
* **Secuencia de tokens en formato estándar LP**: Formateo por líneas que reproduce con exactitud la especificación de tokens del proyecto:
  ```text
  <VOID> <MAIN> <(> <)> <{>
  <INT> <ID,0> <=> <NUM_INT> <;>
  <FLOAT> <ID,1> <=> <NUM_DEC> <;>
  <IF> <(> <ID,0> <COMP> <NUM_INT> <&&> <ID,0> <COMP> <NUM_INT> <)>
  <{> <PRINTLN> <(> <TEXTO> <)> <;> <}>
  <RETURN> <;> <}>
  ```
* **Categorías oficiales en la Tabla de Tokens**:
  * *Palabra reservada*: `void`, `main`, `int`, `float`, `char`, `boolean`, `if`, `else`, `for`, `while`, `scanf`, `println`, `return`.
  * *Identificador*: Formato `<ID,pos>` vinculado a la tabla de símbolos.
  * *Número entero*: `<NUM_INT>` (`[0-9]+`).
  * *Número decimal*: `<NUM_DEC>` (`[0-9]+\.[0-9]+`).
  * *Constantes de texto*: `<TEXTO>` (`"..."`).
  * *Operador de Asignación*: `<=>` (`=`).
  * *Operador aritmético*: `<+>`, `<->`, `<*>`, `</>`, `<%>`.
  * *Operador lógico*: `<&&>`, `<||>`, `<!>`.
  * *Operador de Comparación/Relacionales*: `<COMP>` (`==`, `!=`, `<`, `<=`, `>`, `>=`).
  * *Símbolo especial*: `<(>`, `<)>`, `<[>`, `<]>`, `<{>`, `<}>`, `<,>`, `<;>`.
* **Módulo unificado de contención de errores (`ErrorHandler`)**:
  * Archivo centralizado (`src/ErrorHandler.h`, `src/ErrorHandler.cpp`) para validación antes de que el motor léxico procese entradas.
  * Detección temprana y rechazo explícito de formatos de imagen (`.png`, `.jpg`, `.jpeg`, `.gif`, `.bmp`, `.webp`, `.svg`, etc.) indicando que no son fuentes legibles.
  * Detección y rechazo de archivos binarios y ejecutables (`.exe`, `.dll`, `.bin`, `.zip`, `.rar`, etc.).
  * Detección y rechazo de archivos multimedia (`.mp3`, `.mp4`, etc.).
  * Validación de valores muertos: detección de archivos de 0 bytes o código fuente compuesto exclusivamente por espacios en blanco o saltos de línea.
  * Verificación de integridad de bytes nulos (`\0`) en archivos de texto.
* **Identidad Visual e Icono Oficial del Compilador (`res/`)**:
  * Diseño de icono distintivo representando el compilador: brackets de código `< / >` sobre engranaje de transformación y badge `LP`.
  * Generación en formatos `res/compiler.svg` (vectorial), `res/compiler.png` (256x256) y `res/compiler.ico` (multi-resolución).
  * Incrustación de `res/compiler.ico` en el binario ejecutable Windows `Compiladores.exe` mediante `res/resource.rc` y asociación de icono a la ventana y barra de tareas.
* **Aplicación CLI y Soporte Contenedorizado Docker**:
  * Ejecutable de línea de comandos `compilador_lp` compatible con Linux y Windows.
  * `Dockerfile` multi-etapa optimizado sobre GCC 13 y Debian bookworm-slim con ejecución de pruebas unitarias automáticas durante la construcción.
  * Configuración `docker-compose.yml` para ejecución inmediata y mapeo de volúmenes de entrada/salida (`input/` y `output/`).
  * Compatibilidad multiplataforma en `FileConverter` para compilar en Linux sin dependencias Win32.
* **Detección y Alertas de Anomalías Lógicas de Escritura (Carecen de Sentido)**:
  * El analizador reconoce y procesa tokens repetidos pero alerta explícitamente sobre secuencias anómalas que carecen de sentido léxico o lógico:
    * Secuencias masivas de puntos y comas (ej. `;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;`): genera los tokens `<;>` pero emite alerta de falta de sentido.
    * Secuencias de comas repetidas (ej. `,,`, `,,,`).
    * Operadores de asignación incongruentes (ej. `===`, `====`).
    * Operadores aritméticos y de módulo repetidos consecutivamente (ej. `+++`, `---`, `***`, `%%%`).
    * Operadores lógicos repetidos (ej. `&&&`, `|||`).
* **Suite de Pruebas Unitarias Automatizadas (`test_lexer.exe`)**:
  * Test 1: Verificación del caso de prueba oficial de la Sección 12 con secuencia exacta.
  * Test 2: Operadores aritméticos y descarte correcto de comentarios.
  * Test 3: Operadores relacionales (`<COMP>`) y lógicos.
  * Test 4: Gestión de identificadores repetidos en la Tabla de Símbolos.
  * Test 5: Detección y reporte de errores léxicos.
  * Test 6: Validación de contención de errores y categorías de tokens.
  * Test 7: Validación de lectura y alertas sobre anomalías de escritura carentes de sentido (`;;;;...`, `+++`, `===`).
* **Automatización de Builds y Publicación**:
  * Script PowerShell `release.ps1` con compilación automática, empaquetado en ZIP de distribución, generación de imagen Docker y publicación automatizada mediante GitHub CLI (`gh`).

### Modificado
* **Visualización de Tokens**: Se eliminó la columna de posición de caracteres (*columna*) de la interfaz y los reportes para alinearse a la tabla de la cátedra.
* **Tabla de Símbolos**: Indexación persistente en base 0 con registro de línea de primera aparición y trazabilidad de todas las líneas en las que aparece el identificador.
* **Manejo de Operadores de Comparación**: Se unificaron `==`, `!=`, `<`, `<=`, `>`, `>=` bajo el token único `<COMP>`, con categoría *"Operador de Comparacion/Relacionales"*.

### Corregido
* Conversión uniforme de saltos de línea (`\n` &rarr; `\r\n`) en controles WinAPI `EDIT` para evitar corrupción de visualización en Windows.
* Normalización UTF-8 en archivos de salida y scripts de despliegue.

---

## [0.3.0] - 2026-09-28

### Integración Multiformato y Exportación Automática

### Añadido
* **Módulo `FileConverter`**:
  * Lectura nativa de archivos `.lp` y `.txt`.
  * Extracción automatizada de contenido textual desde documentos Microsoft Word (`.docx`).
  * Extracción de texto desde documentos PDF (`.pdf`) utilizando utilidades nativas de PowerShell en segundo plano.
* **Exportación automática a disco**:
  * Guardado automático en la carpeta `output/`:
    * `tokens.txt`: Secuencia formateada por líneas y tabla clasificada.
    * `tabla_simbolos.txt`: Identificadores únicos y ubicaciones.
    * `errores.txt`: Detalle de incidencias léxicas detectadas.

---

## [0.2.0] - 2026-09-27

### Interfaz Gráfica Nativa de Usuario (WinAPI)

### Añadido
* **GUI nativa en C++ puro** sin dependencias externas pesadas (Qt, Electron, etc.).
* Sistema de navegación por pestañas (`SysTabControl32`):
  * *Entrada (Código Fuente)*: Editor multilínea con soporte de escritura manual o carga de archivos.
  * *Secuencia de Tokens*: Visualización de tokens clasificados.
  * *Tabla de Símbolos*: Listado de identificadores y sus índices.
  * *Errores Léxicos*: Listado detallado de incidencias encontradas.
* Diálogo de selección de archivo nativo `GetOpenFileNameA` con filtrado por extensiones.
* Tipografía de alta legibilidad (*Segoe UI* para controles y *Consolas* para código y tablas).

---

## [0.1.0] - 2026-09-25

### Prototipo Inicial del Motor Léxico

### Añadido
* Estructuras de datos base: enumeración `LexTokenType` y clase `Token`.
* Clase `SymbolTable` para almacenamiento asociativo de identificadores.
* Motor base `Lexer` implementando autómatas de estados finitos para:
  * Enteros y decimales.
  * Identificadores de variables y funciones.
  * Palabras reservadas fundamentales (`int`, `float`, `if`, `else`, `return`, `void`, `main`).
  * Descarte de comentarios de una línea (`//`) y de bloque (`/* ... */`).
