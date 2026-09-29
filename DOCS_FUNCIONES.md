# Documentación Técnica de Funciones y Módulos - Lenguaje LP

Esta guía técnica describe en detalle la arquitectura del código, clases, métodos, enumeraciones y funciones del analizador léxico para el lenguaje **LP (Lenguaje de Programación)** desarrollado en C++20.

---

## Índice General

1. [Módulo de Tokens (`Token.h` / `Token.cpp`)](#1-módulo-de-tokens-tokenh--tokencpp)
2. [Módulo de Tabla de Símbolos (`SymbolTable.h` / `SymbolTable.cpp`)](#2-módulo-de-tabla-de-símbolos-symboltableh--symboltablecpp)
3. [Módulo del Analizador Léxico (`Lexer.h` / `Lexer.cpp`)](#3-módulo-del-analizador-léxico-lexerh--lexercpp)
4. [Módulo de Contención y Reportes (`ErrorHandler.h` / `ErrorHandler.cpp`)](#4-módulo-de-contención-y-reportes-errorhandlerh--errorhandlercpp)
5. [Módulo de Conversión y Entrada Multiformato (`FileConverter.h` / `FileConverter.cpp`)](#5-módulo-de-conversión-y-entrada-multiformato-fileconverterh--fileconvertercpp)
6. [Módulo de Interfaz Gráfica WinAPI (`main.cpp`)](#6-módulo-de-interfaz-gráfica-winapi-maincpp)

---

## 1. Módulo de Tokens (`Token.h` / `Token.cpp`)

Representa la unidad básica de información léxica generada por el escáner.

### 1.1 Enumeración `LexTokenType`
Define las familias y tipos de componentes léxicos admitidos en LP:
* **Identificadores y literales**: `ID`, `TEXTO`, `NUM_INT`, `NUM_DEC`.
* **Palabras reservadas**: `KW_INT`, `KW_FLOAT`, `KW_CHAR`, `KW_BOOLEAN`, `KW_VOID`, `KW_IF`, `KW_ELSE`, `KW_FOR`, `KW_WHILE`, `KW_SCANF`, `KW_PRINTLN`, `KW_MAIN`, `KW_RETURN`, `KW_STRING`.
* **Operadores de asignación**: `OP_ASSIGN` (`=`).
* **Operadores aritméticos**: `OP_SUM` (`+`), `OP_SUB` (`-`), `OP_MUL` (`*`), `OP_DIV` (`/`), `OP_MOD` (`%`).
* **Operadores lógicos**: `OP_AND` (`&&`), `OP_OR` (`||`), `OP_NOT` (`!`).
* **Operadores relacionales / de comparación**: `OP_COMP` (`==`, `!=`, `<`, `<=`, `>`, `>=`).
* **Delimitadores**: `LPAREN`, `RPAREN`, `LBRACKET`, `RBRACKET`, `LBRACE`, `RBRACE`, `COMMA`, `SEMICOLON`.
* **Control y diagnóstico**: `LEX_ERROR`, `LEX_EOF`.

### 1.2 Estructura `Token`

#### Atributos
* `LexTokenType tipo`: Tipo o clasificación interna del token.
* `std::string lexema`: Texto exacto consumido del código fuente.
* `int linea`: Número de línea (base 1) donde inicia el token.
* `int symbolIndex`: Índice en la tabla de símbolos (solo aplica a `ID`, por defecto `-1`).
* `std::string mensaje`: Mensaje descriptivo de error en caso de `tipo == LexTokenType::LEX_ERROR`.

#### Métodos

##### `Token()`
* **Descripción**: Constructor por defecto. Inicializa el token en estado `LEX_EOF`, línea 1 y sin índice de símbolo.

##### `Token(LexTokenType tipo, const std::string& lexema, int linea, int symbolIndex = -1, const std::string& mensaje = "")`
* **Descripción**: Constructor con inicialización completa de campos.
* **Parámetros**:
  * `tipo`: Tipo del token.
  * `lexema`: Cadena con el texto fuente del token.
  * `linea`: Número de línea en el código fuente.
  * `symbolIndex`: Índice en la tabla de símbolos (opcional, por defecto `-1`).
  * `mensaje`: Descripción de error léxico (opcional).

##### `std::string toString() const`
* **Descripción**: Retorna la representación canónica en formato LP entre corchetes angulares.
* **Retorno**:
  * Palabras reservadas: `<INT>`, `<FLOAT>`, `<VOID>`, `<MAIN>`, etc.
  * Identificadores: `<ID,pos>` (ej. `<ID,0>`, `<ID,1>`).
  * Literales: `<NUM_INT>`, `<NUM_DEC>`, `<TEXTO>`.
  * Operadores: `<=>`, `<+>`, `<COMP>`, `<&&>`, etc.
  * Símbolos: `<(>`, `<)>`, `<{>`, `<;>`, etc.
  * Errores: `<ERROR: 'lexema' - mensaje>`.

##### `std::string getCategoria() const`
* **Descripción**: Mapea el token a la categoría formal requerida en las tablas de clase de la cátedra.
* **Retorno**: Cadenas como `"Palabra reservada"`, `"Identificador"`, `"Numero entero"`, `"Numero decimal"`, `"Constantes de texto"`, `"Operador de Asignacion"`, `"Operador aritmetico"`, `"Operador logico"`, `"Operador de Comparacion/Relacionales"`, `"Simbolo especial"`, o `"Error lexico"`.

---

## 2. Módulo de Tabla de Símbolos (`SymbolTable.h` / `SymbolTable.cpp`)

Estructura de datos que gestiona los identificadores unicos del programa fuente y almacena su historial de apariciones.

### 2.1 Estructura `SymbolEntry`
* `int pos`: Posición o índice único asignado (base 0).
* `std::string nombre`: Nombre del identificador (ej. `edad`, `promedio`).
* `int lineaIni`: Línea del código donde se declaró o apareció por primera vez.
* `std::vector<int> apariciones`: Lista cronológica de todas las líneas donde aparece el identificador.

### 2.2 Clase `SymbolTable`

##### `int agregar(const std::string& nombre, int linea = 1)`
* **Descripción**: Registra un identificador en la tabla. Si el identificador ya existe, añade la línea actual a su lista de apariciones y devuelve su posición previa (asegurando unicidad y persistencia del índice). Si no existe, genera una nueva entrada al final y le asigna la posición `entradas.size() - 1`.
* **Parámetros**:
  * `nombre`: Lexema del identificador.
  * `linea`: Número de línea actual.
* **Retorno**: Posición entera (base 0) asignada al identificador.

##### `int buscar(const std::string& nombre) const`
* **Descripción**: Realiza una búsqueda secuencial por nombre.
* **Parámetros**:
  * `nombre`: Nombre del identificador a consultar.
* **Retorno**: Índice entero del identificador o `-1` si no ha sido registrado.

##### `const std::vector<SymbolEntry>& getEntradas() const`
* **Descripción**: Obtiene la referencia constante a la colección completa de entradas registradas.
* **Retorno**: Vector de estructuras `SymbolEntry`.

##### `std::vector<std::string> getSimbolos() const`
* **Descripción**: Retorna una lista con únicamente los nombres de los identificadores registrados.
* **Retorno**: Vector de cadenas con los nombres.

##### `bool vacia() const`
* **Descripción**: Evalúa si la tabla no contiene ningún identificador.
* **Retorno**: `true` si no hay entradas; `false` en caso contrario.

##### `void limpiar()`
* **Descripción**: Vacía la colección de identificadores y restablece el contador de posiciones.

---

## 3. Módulo del Analizador Léxico (`Lexer.h` / `Lexer.cpp`)

Implementa el autómata finito determinista (AFD) que reconoce la gramática léxica de LP.

### 3.1 Clase `Lexer`

#### Atributos Privados
* `std::string fuente`: Cadena con el texto completo del código fuente a procesar.
* `size_t pos`: Puntero o índice del carácter actual en `fuente`.
* `int linea`: Contador de línea actual en el análisis (inicia en 1).
* `std::vector<Token> tokens`: Secuencia ordenada de tokens válidos extraídos.
* `SymbolTable tabla`: Instancia interna de la tabla de símbolos.
* `std::vector<Token> errores`: Lista de tokens de tipo `LEX_ERROR` recolectados durante el barrido.

#### Métodos Públicos

##### `explicit Lexer(const std::string& fuente)`
* **Descripción**: Inicializa el lexer cargando el texto fuente y ubicando el cursor en la posición 0 y línea 1.
* **Parámetros**:
  * `fuente`: Texto del programa a compilar.

##### `void analizar()`
* **Descripción**: Método principal que orquesta el análisis léxico completo. Itera carácter a carácter mediante un bucle de exploración y desvía la ejecución al analizador específico según el primer carácter de cada lexema:
  1. Descarta espacios en blanco y comentarios (`//` y `/* */`).
  2. Letras o `_`: Invoca `lexIdentificadorOPalabra()`.
  3. Dígitos: Invoca `lexNumero()`.
  4. Comillas dobles (`"`): Invoca `lexTexto()`.
  5. Caracteres especiales (`=`, `+`, `-`, `<`, `>`, `&`, `|`, etc.): Gestiona operadores de uno o dos caracteres.
  6. Caracteres desconocidos: Registra un `LEX_ERROR` y continúa la ejecución.

##### `const std::vector<Token>& getTokens() const`
* **Descripción**: Retorna la colección de tokens válidos reconocidos.

##### `const SymbolTable& getTabla() const`
* **Descripción**: Retorna la tabla de símbolos con todos los identificadores clasificados.

##### `const std::vector<Token>& getErrores() const`
* **Descripción**: Retorna la lista de incidencias y errores léxicos detectados durante el escaneo.

##### `std::string getSalidaPorLineas() const`
* **Descripción**: Formatea la secuencia de tokens agrupada por las líneas originales del código fuente, coincidiendo con la especificación de salida del lenguaje LP.
* **Retorno**: Cadena multilínea con formato `<VOID> <MAIN> <(> <)> <{>\r\n...`.

#### Métodos Privados y Autómatas

##### `char actual() const`
* **Descripción**: Inspecciona el carácter actual en `fuente[pos]` sin avanzar el puntero. Retorna `\0` si se alcanzó el fin del texto.

##### `char siguiente() const`
* **Descripción**: Inspecciona el carácter siguiente `fuente[pos + 1]` (lookahead de 1 carácter) sin avanzar el cursor.

##### `void avanzar()`
* **Descripción**: Desplaza el puntero de lectura una posición (`pos++`). Si el carácter superado era `\n`, incrementa el contador de línea (`linea++`).

##### `void saltarEspaciosYComentarios()`
* **Descripción**: Consume espacios, tabulaciones y retornos de carro. Si detecta `//`, consume todos los caracteres hasta el final de la línea. Si detecta `/*`, consume caracteres hasta encontrar `*/`. Si un bloque de comentario queda sin cerrar al final del archivo, registra el error léxico correspondiente.

##### `Token lexNumero(int linIni)`
* **Descripción**: Autómata para números enteros (`NUM_INT`) y decimales (`NUM_DEC`). Consume la secuencia continua de dígitos. Si encuentra un punto `.` seguido de dígitos, pasa al estado decimal. Si detecta múltiples puntos consecutivos (ej. `12.3.4`), genera un token `LEX_ERROR`. Si un dígito va seguido inmediatamente por letras (ej. `99var`), reconoce el error de identificador inválido iniciado con número.
* **Parámetros**:
  * `linIni`: Línea donde inició el número.
* **Retorno**: Estructura `Token` con el literal reconocido o el error respectivo.

##### `Token lexIdentificadorOPalabra(int linIni)`
* **Descripción**: Autómata para identificadores y palabras clave. Consume caracteres alfanuméricos y guiones bajos (`[a-zA-Z0-9_]`). Al terminar el lexema:
  1. Consulta el diccionario estático de palabras reservadas (`int`, `float`, `void`, `main`, etc.). Si coincide, produce el token de palabra reservada.
  2. Si no coincide con ninguna palabra clave, se clasifica como `ID`, se registra en `tabla` (mediante `tabla.agregar(lexema, linIni)`) y se produce el token `<ID,pos>`.
* **Parámetros**:
  * `linIni`: Línea inicial del identificador.
* **Retorno**: Estructura `Token` correspondiente.

##### `Token lexTexto(int linIni, char delim)`
* **Descripción**: Consume todos los caracteres encerrados entre comillas dobles (`"`). Soporta caracteres escapados (`\"`, `\\`, `\n`, `\t`). Si se llega al fin de línea o fin de archivo sin encontrar la comilla de cierre, produce un `LEX_ERROR`.
* **Parámetros**:
  * `linIni`: Línea de apertura de la cadena.
  * `delim`: Carácter delimitador (`"`).
* **Retorno**: Token de tipo `TEXTO` o `LEX_ERROR`.

---

## 4. Módulo de Contención y Reportes (`ErrorHandler.h` / `ErrorHandler.cpp`)

Módulo centralizado para blindar el compilador contra entradas anómalas, archivos no compatibles, valores muertos y generar los reportes formales de salida.

### 4.1 Enumeración `TipoFalloEntrada`
Clasificación de fallos antes de la etapa de análisis:
* `NINGUNO`: Archivo o texto válido.
* `ARCHIVO_NO_EXISTE`: Ruta inexistente en el sistema de archivos.
* `ARCHIVO_VACIO`: Archivo con tamaño de 0 bytes.
* `TEXTO_VACIO`: Contenido compuesto únicamente por espacios en blanco o saltos de línea (valor muerto).
* `IMAGEN`: Formatos gráficos (`.png`, `.jpg`, `.jpeg`, `.gif`, `.bmp`, `.webp`, `.svg`, etc.).
* `BINARIO_EJECUTABLE`: Binarios o librerías compiladas (`.exe`, `.dll`, `.bin`, `.zip`, etc.).
* `MULTIMEDIA`: Archivos de audio o video (`.mp3`, `.mp4`, etc.).
* `CONTENIDO_BINARIO`: Archivos de texto con caracteres de control nulos (`\0`).
* `FORMATO_NO_SOPORTADO`: Extensiones no procesables.

### 4.2 Estructura `DiagnosticoEntrada`
* `bool esValido`: Bandera lógica que indica si la entrada es apta para análisis.
* `TipoFalloEntrada tipo`: Categoría del fallo detectado.
* `std::string titulo`: Título legible para cuadros de diálogo o encabezados de error.
* `std::string mensaje`: Explicación detallada orientada al usuario de por qué se rechazó la entrada.
* `std::string sugerencia`: Acción recomendada para corregir la situación.

### 4.3 Métodos de la Clase `ErrorHandler`

##### `static DiagnosticoEntrada validarArchivo(const std::string& ruta)`
* **Descripción**: Inspecciona un archivo antes de intentar abrirlo o procesarlo:
  1. Verifica existencia física en disco.
  2. Extrae la extensión y comprueba si es imagen gráfica, ejecutable binario o multimedia.
  3. Verifica que el archivo no posea tamaño 0.
  4. Realiza un escaneo preliminar de los primeros 1024 bytes para verificar que no contenga bytes nulos (`\0`).
* **Parámetros**:
  * `ruta`: Ruta absoluta o relativa al archivo en disco.
* **Retorno**: Estructura `DiagnosticoEntrada`.

##### `static DiagnosticoEntrada validarCodigoFuente(const std::string& codigo)`
* **Descripción**: Valida el texto cargado o escrito en el editor para evitar valores muertos. Si el texto está vacío o solo contiene espacios, tabulaciones y saltos de línea, emite un diagnóstico de rechazo indicando que se debe ingresar código fuente en lenguaje LP.
* **Parámetros**:
  * `codigo`: Cadena con el texto a analizar.
* **Retorno**: Estructura `DiagnosticoEntrada`.

##### `static std::string generarReporteErrores(const std::vector<Token>& errores)`
* **Descripción**: Construye un reporte estructurado de los errores léxicos detectados durante el análisis, indicando la línea, el lexema con fallo y la descripción técnica de la causa. Si no hay errores, genera un mensaje de conformidad al 100%.
* **Parámetros**:
  * `errores`: Vector de tokens de tipo `LEX_ERROR`.
* **Retorno**: Cadena formateada para la interfaz y el archivo `errores.txt`.

##### `static std::string generarReporteTokens(const std::vector<Token>& tokens, const std::string& secuenciaLineas)`
* **Descripción**: Ensambla el reporte completo de tokens:
  1. Sección 1: Secuencia de tokens por línea (formato `<VOID> <MAIN>...`).
  2. Sección 2: Tabla detallada con las columnas `#`, `TOKEN`, `LEXEMA`, `CATEGORIA DEL TOKEN` y `LINEA`.
* **Parámetros**:
  * `tokens`: Colección de tokens reconocidos.
  * `secuenciaLineas`: Salida condensada por líneas generada por el lexer.
* **Retorno**: Cadena multilínea para la interfaz y `tokens.txt`.

##### `static std::string generarReporteTablaSimbolos(const SymbolTable& tabla)`
* **Descripción**: Genera la tabla formateada en texto para los identificadores registrados, mostrando `POSICION`, `IDENTIFICADOR`, `LINEA INICIAL` y la lista de `APARICIONES`.
* **Parámetros**:
  * `tabla`: Instancia de `SymbolTable`.
* **Retorno**: Cadena multilínea para la interfaz y `tabla_simbolos.txt`.

---

## 5. Módulo de Conversión y Entrada Multiformato (`FileConverter.h` / `FileConverter.cpp`)

Gestiona la importación y extracción de texto desde diversos formatos físicos de documento hacia una cadena en memoria normalizada.

### 5.1 Clase `FileConverter`

##### `static std::string leer(const std::string& ruta)`
* **Descripción**: Detecta la extensión del archivo y delega la lectura al método especializado correspondiente:
  * `.lp` o `.txt` &rarr; `leerTextoPlano()`.
  * `.docx` &rarr; `extraerDocx()`.
  * `.pdf` &rarr; `extraerPdf()`.
* **Parámetros**:
  * `ruta`: Ruta del archivo seleccionado.
* **Retorno**: Cadena con el contenido en texto plano.

##### `static std::string ultimoError()`
* **Descripción**: Retorna la última cadena de error producida durante los procesos de lectura o extracción.

##### `static std::string leerTextoPlano(const std::string& ruta)`
* **Descripción**: Abre el archivo mediante `std::ifstream` en modo binario, carga su contenido completo en un `std::string` y normaliza los saltos de línea `\r\n` o `\r` a `\n`.

##### `static std::string extraerDocx(const std::string& ruta)`
* **Descripción**: Los archivos `.docx` son contenedores ZIP con especificación OpenXML. Este método invoca un subproceso ligero en PowerShell para abrir el archivo como archivo comprimido ZIP (`System.IO.Compression.ZipFile`), leer el archivo interno `word/document.xml` y extraer el contenido textual de las etiquetas `<w:t>`.

##### `static std::string extraerPdf(const std::string& ruta)`
* **Descripción**: Ejecuta una extracción asistida por script en segundo plano para leer secuencias de texto legibles de documentos PDF y volcarlas en memoria.

##### `static std::string ejecutarPS(const std::string& script)`
* **Descripción**: Crea un proceso oculto invocando `powershell.exe -NoProfile -NonInteractive` a través de la API Win32 `CreateProcessA` con pipes anónimos para redirigir `stdout` y capturar la salida en memoria sin abrir consolas visibles al usuario.

---

## 6. Módulo de Interfaz Gráfica WinAPI (`main.cpp`)

Implementa la interfaz gráfica de usuario en modo nativo Win32 (sin requerir dependencias de terceros ni frameworks pesados).

### 6.1 Controles e Identificadores
* `IDC_TAB (100)`: Control de pestañas (`SysTabControl32`).
* `IDC_BTN_ABRIR (101)`: Botón "Abrir Archivo".
* `IDC_BTN_ANALIZAR (102)`: Botón "Analizar".
* `IDC_LABEL_FILE (103)`: Etiqueta estática con el nombre del archivo activo.
* `IDC_EDIT_ENTRADA (109)`: Editor multilínea para el código fuente editable.
* `IDC_EDIT_TOKENS (110)`: Panel de visualización de secuencia y tabla de tokens.
* `IDC_EDIT_TABLA (111)`: Panel de visualización de la tabla de símbolos.
* `IDC_EDIT_ERRORES (112)`: Panel de visualización de errores léxicos.

### 6.2 Funciones Principales

##### `int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)`
* **Descripción**: Punto de entrada de la aplicación Win32.
  1. Inicializa los controles comunes (`InitCommonControlsEx`).
  2. Registra la clase de ventana `LexLP_Clase`.
  3. Crea y muestra la ventana principal (`CreateWindowExA`).
  4. Ejecuta el bucle de mensajes estándar (`GetMessageA` &rarr; `TranslateMessage` &rarr; `DispatchMessageA`).

##### `LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)`
* **Descripción**: Procedimiento de ventana encargado de procesar los mensajes del sistema operativo:
  * `WM_CREATE`: Crea fuentes Segoe UI y Consolas, botones, etiqueta, pestañas y los cuatro paneles `EDIT`.
  * `WM_SIZE`: Invoca `redimensionarPaneles()` para diseño adaptable dinámico.
  * `WM_COMMAND`: Maneja las pulsaciones de los botones Abrir (`IDC_BTN_ABRIR`) y Analizar (`IDC_BTN_ANALIZAR`).
  * `WM_NOTIFY`: Responde a la notificación `TCN_SELCHANGE` para alternar la pestaña activa.
  * `WM_DESTROY`: Invoca `PostQuitMessage(0)` para cerrar la aplicación ordenadamente.

##### `static void ejecutarAnalisis(HWND hWnd)`
* **Descripción**: Obtiene el texto del control `hEditEntrada`, ejecuta las validaciones de contención en `ErrorHandler`, instancia el `Lexer`, procesa los tokens, actualiza los tres paneles de resultados en la GUI, conmuta automáticamente a la pestaña de tokens y guarda copias físicas en `output/tokens.txt`, `output/tabla_simbolos.txt` y `output/errores.txt`.

##### `static void mostrarPestana(int idx)`
* **Descripción**: Oculta los cuatro controles `EDIT` y hace visible exclusivamente el correspondiente al índice de la pestaña seleccionada (0: Entrada, 1: Tokens, 2: Tabla, 3: Errores).

##### `static void redimensionarPaneles(HWND hWnd)`
* **Descripción**: Calcula las coordenadas del área cliente de la ventana y ajusta proporcionalmente el tamaño y posición de la barra superior, el control de pestañas y el área interna de los paneles de edición.
