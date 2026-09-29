# Manual de Usuario - Analizador Léxico para Lenguaje LP

Bienvenido al manual de uso del **Analizador Léxico para el Lenguaje LP (Lenguaje de Programación)**. Esta guía está diseñada para que evaluadores, docentes y estudiantes puedan operar todas las funciones del compilador con facilidad.

---

## 1. Requisitos del Sistema

* **Sistema Operativo**: Windows 10 o Windows 11 (64-bit).
* **Compilador (para desarrollo)**: MinGW-w64 (GCC 11+) o MSVC con soporte C++20.
* **Herramientas de compilación**: CMake 3.20+ y Ninja (incluidos por defecto en CLion).
* **PowerShell**: Versión 5.1 o superior (incluida de fábrica en Windows).

---

## 2. Inicio Rápido

### Ejecutar la Aplicación Visual (GUI)
1. Navega a la carpeta de compilación `cmake-build-debug/` o descarga el release empaquetado.
2. Haz doble clic sobre `Compiladores.exe` o ejecútalo desde la terminal:
   ```powershell
   .\cmake-build-debug\Compiladores.exe
   ```
3. Se abrirá la ventana principal con 4 pestañas interactivas y un código de ejemplo cargado por defecto.

---

## 3. Guía de Uso de la Interfaz Gráfica

La interfaz gráfica nativa está dividida en una barra de controles superior y un área central organizada en cuatro pestañas:

```
+-------------------------------------------------------------------------------+
| [ Abrir Archivo ]   [ Analizar ]   ejemplo.lp (codigo cargado)                |
+-------------------------------------------------------------------------------+
| [ Entrada (Codigo Fuente) ] [ Tokens ] [ Tabla de Simbolos ] [ Errores ]      |
|                                                                               |
|  void main() {                                                                |
|  int edad = 20;                                                               |
|  float promedio = 15.5;                                                       |
|  if (edad >= 18 && edad <= 60) {                                              |
|      println("Edad valida");                                                  |
|  }                                                                            |
|  return;                                                                      |
|  }                                                                            |
+-------------------------------------------------------------------------------+
```

### 3.1 Pestaña 1: Entrada (Código Fuente)
* **Edición directa**: Puedes escribir, editar, pegar o borrar código en cualquier momento dentro del editor de texto.
* **Botón "Abrir Archivo"**: Permite cargar archivos directamente desde tu disco duro.
  * Formatos soportados: `.lp`, `.txt`, `.docx` (Microsoft Word) y `.pdf`.
* **Protección contra archivos inválidos**:
  * Si intentas abrir un archivo de imagen (`.png`, `.jpg`, `.bmp`, etc.), un ejecutable (`.exe`, `.dll`) o un archivo vacío, el sistema mostrará una advertencia clara indicando por qué no se puede procesar y no afectará el estado del compilador.
* **Botón "Analizar"**: Ejecuta el análisis léxico completo sobre el texto visible en pantalla.

### 3.2 Pestaña 2: Secuencia de Tokens
Al pulsar el botón **[Analizar]**, el sistema conmuta automáticamente a esta pestaña para presentarte dos vistas:
1. **Secuencia de Tokens por Línea**: La representación canónica de los tokens entre corchetes angulares respetando las líneas del código fuente:
   ```text
   <VOID> <MAIN> <(> <)> <{>
   <INT> <ID,0> <=> <NUM_INT> <;>
   <FLOAT> <ID,1> <=> <NUM_DEC> <;>
   <IF> <(> <ID,0> <COMP> <NUM_INT> <&&> <ID,0> <COMP> <NUM_INT> <)>
   <{> <PRINTLN> <(> <TEXTO> <)> <;> <}>
   <RETURN> <;> <}>
   ```
2. **Tabla Detallada de Tokens**: Tabla analítica con el desglose formal de cada componente:
   * `#`: Número correlativo.
   * `TOKEN`: Token asignado (ej. `<VOID>`, `<ID,0>`, `<=>`, `<NUM_INT>`, `<;>`).
   * `LEXEMA`: Texto literal correspondiente en el código fuente.
   * `CATEGORIA DEL TOKEN`: Clasificación formal (Palabra reservada, Identificador, Operador de Asignación, etc.).
   * `LINEA`: Número de línea donde se encuentra ubicado.

### 3.3 Pestaña 3: Tabla de Símbolos
Muestra el catálogo de todos los identificadores únicos encontrados en el programa:
* `POSICION`: Índice único asignado en base 0 (coincidente con `<ID,0>`, `<ID,1>`, etc.).
* `IDENTIFICADOR`: Nombre de la variable o función declarada.
* `LINEA INICIAL`: Primera línea donde fue detectado.
* `APARICIONES`: Listado cronológico de todas las líneas donde se hace referencia al identificador (ej. `L2, L4, L4`).

### 3.4 Pestaña 4: Errores Léxicos
Si el código ingresado contiene caracteres no admitidos (como `@`, `$`), números con múltiples puntos (ej. `12.3.4`) o identificadores que inician con dígitos:
* La aplicación alertará mediante un cuadro emergente indicando la cantidad de errores encontrados.
* En esta pestaña podrás ver la línea exacta, el lexema causante y la explicación detallada de por qué es inválido para facilitar su corrección en el editor de entrada.

---

## 4. Archivos de Salida Automáticos

Cada vez que ejecutas el análisis, se generan automáticamente archivos de texto plano dentro de la carpeta `output/`:

* [`output/tokens.txt`](file:///c:/Users/User/CLionProjects/Compiladores/output/tokens.txt): Contiene la secuencia formateada por líneas y la tabla clasificada completa.
* [`output/tabla_simbolos.txt`](file:///c:/Users/User/CLionProjects/Compiladores/output/tabla_simbolos.txt): Contiene la tabla de identificadores únicos con sus posiciones y apariciones.
* [`output/errores.txt`](file:///c:/Users/User/CLionProjects/Compiladores/output/errores.txt): Contiene el reporte detallado de incidencias o la certificación de código 100% válido.

---

## 5. Ejecución de Pruebas Unitarias Automatizadas

El proyecto incluye una suite con 6 pruebas automatizadas que validan todos los casos límite y directivas del curso:

Para ejecutarlas desde PowerShell:
```powershell
# Compilar el ejecutable de pruebas
cmake --build cmake-build-debug --target test_lexer

# Ejecutar las pruebas
.\cmake-build-debug\test_lexer.exe
```

Salida esperada:
```text
========================================
 EJECUTANDO PRUEBAS UNITARIAS LEXER LP  
========================================

=== Test 1: Ejemplo Oficial LP (Seccion 12) ===
[PASS] Secuencia de tokens coincide exactamente con la especificacion.
[PASS] Tabla de simbolos: [0] = edad, [1] = promedio.
[PASS] 0 errores lexicos en codigo valido.

=== Test 2: Operadores Aritmeticos y Comentarios ===
[PASS] Operadores +, *, %, = y comentarios procesados correctamente.

=== Test 3: Operadores Relacionales (<COMP>) y Logicos ===
[PASS] Comparadores >=, !=, ==, >, < producen <COMP>, && y ! reconocidos.

=== Test 4: Identificadores Repetidos en Tabla de Simbolos ===
[PASS] Identificadores repetidos reutilizan la posicion existente (edad=0, promedio=1).

=== Test 5: Manejo de Errores Lexicos ===
[PASS] Errores detectados para @, $, multiples puntos y digito al inicio de identificador.

=== Test 6: Contencion de Errores de Entrada y Categorias de Tokens ===
[PASS] Imagen detectada y rechazada con mensaje explicito.
[PASS] Binario detectado y rechazado.
[PASS] Valor muerto detectado y rechazado.
[PASS] Categorias de la tabla oficial asignadas correctamente a cada token.

========================================
 TODAS LAS PRUEBAS PASARON EXITOSAMENTE 
========================================
```

---

## 6. Publicación y Empaquetado de Releases

Si deseas generar un release completo empaquetado para distribución en GitHub:
```powershell
.\release.ps1 1.0.0
```
Este script realiza automáticamente:
1. Verificación de herramientas (`git`, `gh`, `cmake`).
2. Compilación del binario optimizado `Compiladores.exe`.
3. Empaquetado de artefactos en `dist/Compiladores-v1.0.0-windows-x64.zip`.
4. Creación del tag Git `v1.0.0` y subida a GitHub.
5. Publicación del release oficial con notas de versión mediante GitHub CLI.
