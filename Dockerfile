# Dockerfile para Compiladores (Analizador Lexico LP)
# Permite compilar y ejecutar pruebas en entorno Linux contenedorizado

FROM gcc:13 AS build

WORKDIR /app

# Instalar dependencias basicas de compilacion
RUN apt-get update && apt-get install -y cmake ninja-build && rm -rf /var/lib/apt/lists/*

# Copiar archivos fuente del proyecto
COPY CMakeLists.txt ./
COPY src/ ./src/
COPY tests/ ./tests/
COPY input/ ./input/

# Compilacion sin GUI (para entorno Docker/Linux)
RUN mkdir -p build && cd build && \
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release .. && \
    ninja

CMD ["echo", "Compiladores LP - Imagen Docker lista."]
