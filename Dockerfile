# ==============================================================================
# Dockerfile para Compiladores LP (Analizador Léxico, Tabla de Símbolos y Errores)
# Construcción multi-etapa optimizada en Linux (GCC 13 / C++20)
# ==============================================================================

# ------------------------------------------------------------------------------
# Etapa 1: Compilación y Verificación de Pruebas Unitarias
# ------------------------------------------------------------------------------
FROM gcc:13-bookworm AS builder

WORKDIR /build

# Instalar dependencias de compilación
RUN apt-get update && apt-get install -y --no-install-recommends \
    cmake \
    ninja-build \
    && rm -rf /var/lib/apt/lists/*

# Copiar código fuente y recursos del proyecto
COPY CMakeLists.txt ./
COPY src/ ./src/
COPY tests/ ./tests/
COPY input/ ./input/
COPY res/ ./res/

# Compilar los ejecutables de CLI y pruebas en modo Release
RUN cmake -B cmake-build -G Ninja -DCMAKE_BUILD_TYPE=Release && \
    cmake --build cmake-build --target compilador_lp test_lexer

# Ejecutar las 6 pruebas unitarias durante el build para validar cero errores
RUN ./cmake-build/test_lexer

# ------------------------------------------------------------------------------
# Etapa 2: Imagen Final Ultraligera para Ejecución
# ------------------------------------------------------------------------------
FROM debian:bookworm-slim AS runtime

LABEL maintainer="Compiladores LP - Catedra Compiladores" \
      description="Analizador Lexico, Tabla de Simbolos y Contencion de Errores para el Lenguaje LP" \
      version="1.0.0"

WORKDIR /app

# Crear directorios de trabajo
RUN mkdir -p /app/input /app/output /app/res

# Copiar ejecutables y assets desde la etapa de construcción
COPY --from=builder /build/cmake-build/compilador_lp /app/compilador_lp
COPY --from=builder /build/cmake-build/test_lexer /app/test_lexer
COPY --from=builder /build/input/ /app/input/
COPY --from=builder /build/res/ /app/res/

# Otorgar permisos de ejecución
RUN chmod +x /app/compilador_lp /app/test_lexer

# Definir volúmenes para entrada y salida con la máquina anfitriona
VOLUME ["/app/input", "/app/output"]

# Configuración de terminal con colores ANSI
ENV TERM=xterm-256color

# Punto de entrada predeterminado: analizador léxico LP
ENTRYPOINT ["/app/compilador_lp"]

# Parámetro por defecto: procesar input/ejemplo.lp
CMD ["input/ejemplo.lp"]
