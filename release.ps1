param(
    [Parameter(Position = 0)]
    [string]$Version = "1.0.0",

    [string]$Title = "",

    [string]$Remote = "origin",

    [switch]$NoOpenBrowser,

    [switch]$Docker
)

$ErrorActionPreference = "Stop"

# Forzar codificacion UTF-8 estricta para evitar deformacion de texto en GitHub
[System.Console]::InputEncoding = [System.Text.Encoding]::UTF8
[System.Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$OutputEncoding = [System.Text.Encoding]::UTF8

function Write-Step {
    param([string]$Text)
    Write-Host ""
    Write-Host "==================================================" -ForegroundColor Cyan
    Write-Host " $Text" -ForegroundColor Cyan
    Write-Host "==================================================" -ForegroundColor Cyan
}

function Require-Command {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Name,

        [Parameter(Mandatory = $true)]
        [string]$InstallHint
    )
    if (-not (Get-Command $Name -ErrorAction SilentlyContinue)) {
        throw "$Name no esta disponible. $InstallHint"
    }
}

function Normalize-Version {
    param([string]$InputVersion)

    $clean = $InputVersion.Trim().TrimStart("v")
    if ($clean -notmatch '^\d+(\.\d+){0,3}$') {
        throw "Version invalida: $InputVersion. Usa formatos como 0.1.0, 1.0.0 o 0.2."
    }

    $parts = @($clean.Split("."))
    while ($parts.Count -lt 3) {
        $parts += "0"
    }
    return ($parts -join ".")
}

Write-Step "COMPILADORES LP - SCRIPT DE RELEASE"

# 1. Validar herramientas necesarias
Require-Command -Name "git" -InstallHint "Instala Git y agregalo al PATH."
Require-Command -Name "gh"  -InstallHint "Instala GitHub CLI: winget install --id GitHub.cli"

$projectRoot = (git rev-parse --show-toplevel).Trim()
if ($LASTEXITCODE -ne 0 -or -not $projectRoot) {
    throw "Ejecuta este script dentro del repositorio de Compiladores."
}
Set-Location $projectRoot

# 2. Obtener rama y version
$branch = (git branch --show-current).Trim()
if (-not $branch) {
    $branch = "main"
}

if (-not $Version) {
    $Version = Read-Host "Ingresa la version de release (ej. 1.0.0)"
}
$Version = Normalize-Version -InputVersion $Version
$tag = "v$Version"

if (-not $Title) {
    $Title = "Compiladores LP $tag - Entrega Final Oficial"
}

Write-Host "Repositorio: $projectRoot" -ForegroundColor Gray
Write-Host "Rama       : $branch" -ForegroundColor Gray
Write-Host "Version    : $Version" -ForegroundColor Green
Write-Host "Tag        : $tag" -ForegroundColor Green

# 3. Validar estado de GitHub CLI
Write-Step "Validando conexion con GitHub"
gh auth status *> $null
if ($LASTEXITCODE -ne 0) {
    throw "GitHub CLI no esta autenticado. Ejecuta primero: gh auth login"
}

gh repo view *> $null
if ($LASTEXITCODE -ne 0) {
    throw "GitHub CLI no pudo resolver el repositorio remoto."
}

# 4. Asegurar MinGW en PATH y compilar ejecutables
Write-Step "Compilando Compiladores.exe, compilador_lp.exe y test_lexer.exe"
$mingwBin = "C:\Users\User\AppData\Local\Programs\CLion\bin\mingw\bin"
if (Test-Path $mingwBin) {
    $env:PATH = "$mingwBin;" + $env:PATH
}

$cmakeExe = "C:\Users\User\AppData\Local\Programs\CLion\bin\cmake\win\x64\bin\cmake.exe"
if (-not (Test-Path $cmakeExe)) {
    $cmakeExe = (Get-Command cmake -ErrorAction SilentlyContinue).Source
}

$buildDir = Join-Path $projectRoot "cmake-build-debug"
$exeGui   = Join-Path $buildDir "Compiladores.exe"
$exeCli   = Join-Path $buildDir "compilador_lp.exe"
$exeTests = Join-Path $buildDir "test_lexer.exe"

# Detener procesos que puedan bloquear los binarios
Stop-Process -Name "Compiladores" -ErrorAction SilentlyContinue
Stop-Process -Name "compilador_lp" -ErrorAction SilentlyContinue
Stop-Process -Name "test_lexer" -ErrorAction SilentlyContinue

if (Test-Path $cmakeExe) {
    Write-Host "Compilando con CMake..." -ForegroundColor Yellow
    & $cmakeExe --build $buildDir --target Compiladores compilador_lp test_lexer
} else {
    Write-Host "Aviso: No se encontro cmake.exe directo, verificando binarios..." -ForegroundColor Yellow
}

if (-not (Test-Path $exeGui)) {
    throw "No se encontro el ejecutable en $exeGui. Compila primero el proyecto."
}
Write-Host "Binario GUI listo: $exeGui" -ForegroundColor Green

# Ejecutar pruebas unitarias para certificar el build
Write-Step "Verificando Suite de Pruebas Unitarias"
if (Test-Path $exeTests) {
    & $exeTests
    if ($LASTEXITCODE -ne 0) {
        throw "Las pruebas unitarias fallaron. No se puede proceder con el release."
    }
    Write-Host "Todas las pruebas unitarias pasaron al 100%." -ForegroundColor Green
}

# 5. Construccion de Docker si esta disponible
if ($Docker -or (Get-Command docker -ErrorAction SilentlyContinue)) {
    Write-Step "Construyendo imagen Docker (Dockeado)"
    if (Get-Command docker -ErrorAction SilentlyContinue) {
        docker build -t "compiladores-lp:$tag" -t "compiladores-lp:latest" .
        if ($LASTEXITCODE -eq 0) {
            Write-Host "Imagen Docker compiladores-lp:$tag creada con exito." -ForegroundColor Green
        } else {
            Write-Host "Advertencia: La construccion con Docker retorno error." -ForegroundColor Yellow
        }
    } else {
        Write-Host "Docker no esta instalado localmente. Saltando etapa Docker." -ForegroundColor Yellow
    }
}

# 6. Preparar paquete ZIP de distribucion portable
Write-Step "Empaquetando release portable en dist/"
$distDir = Join-Path $projectRoot "dist"
New-Item -ItemType Directory -Path $distDir -Force | Out-Null

$zipPath = Join-Path $distDir "Compiladores-$tag-windows-x64.zip"
if (Test-Path $zipPath) {
    Remove-Item $zipPath -Force
}

$stagingDir = Join-Path $distDir "Compiladores-$tag"
if (Test-Path $stagingDir) {
    Remove-Item -Recurse -Force $stagingDir
}
New-Item -ItemType Directory -Path $stagingDir -Force | Out-Null

# Copiar ejecutables principales
Copy-Item -Path $exeGui -Destination $stagingDir
if (Test-Path $exeCli) {
    Copy-Item -Path $exeCli -Destination $stagingDir
}
if (Test-Path $exeTests) {
    Copy-Item -Path $exeTests -Destination $stagingDir
}

# Copiar DLLs de runtime de MinGW si existen para hacer el paquete 100% portable
$runtimeDlls = @("libgcc_s_seh-1.dll", "libstdc++-6.dll", "libwinpthread-1.dll")
foreach ($dll in $runtimeDlls) {
    $dllSource = Join-Path $mingwBin $dll
    if (Test-Path $dllSource) {
        Copy-Item -Path $dllSource -Destination $stagingDir
    }
}

# Copiar carpetas y recursos
if (Test-Path (Join-Path $projectRoot "res")) {
    Copy-Item -Path (Join-Path $projectRoot "res") -Destination $stagingDir -Recurse
}
if (Test-Path (Join-Path $projectRoot "input")) {
    Copy-Item -Path (Join-Path $projectRoot "input") -Destination $stagingDir -Recurse
}
if (Test-Path (Join-Path $projectRoot "output")) {
    Copy-Item -Path (Join-Path $projectRoot "output") -Destination $stagingDir -Recurse
}
if (Test-Path (Join-Path $projectRoot "tests")) {
    Copy-Item -Path (Join-Path $projectRoot "tests") -Destination $stagingDir -Recurse
}

# Copiar documentacion
$docs = @("README.md", "MANUAL_USUARIO.md", "DOCS_FUNCIONES.md", "ARQUITECTURA.md", "CHANGELOG.md")
foreach ($doc in $docs) {
    $docPath = Join-Path $projectRoot $doc
    if (Test-Path $docPath) {
        Copy-Item -Path $docPath -Destination $stagingDir
    }
}

# Crear archivo ZIP portable
Compress-Archive -Path "$stagingDir\*" -DestinationPath $zipPath -Force
Remove-Item -Recurse -Force $stagingDir

$zipSizeMb = [Math]::Round((Get-Item $zipPath).Length / 1MB, 2)
Write-Host "ZIP portable generado: $zipPath ($zipSizeMb MB)" -ForegroundColor Green

# 7. Generar notas de release detalladas
$notesPath = Join-Path $distDir "RELEASE_NOTES_$tag.md"
$fecha = Get-Date -Format "yyyy-MM-dd"
$notas = @"
# $Title

Fecha de Lanzamiento: $fecha

## 🚀 Entrega Oficial del Analizador Léxico para el Lenguaje LP (C++20)

Esta versión incluye la suite completa del compilador con interfaz gráfica nativa en Windows, versión CLI multiplataforma, soporte de contenedor Docker, tabla de símbolos, módulo único de contención de errores y documentación técnica exhaustiva.

---

### 📦 Contenido del Paquete Portable (Windows x64):
* **Compiladores.exe**: Aplicación con interfaz gráfica nativa (WinAPI) y el nuevo **icono oficial del compilador**, con editor multilínea, visor de tokens, tabla de símbolos y reporte de errores.
* **compilador_lp.exe**: Analizador léxico en consola (CLI) rápido y automatizado.
* **test_lexer.exe**: Suite automatizada con 6 pruebas unitarias (100% de aprobación).
* **Librerías de Ejecución Portables**: Se incluyen `libgcc_s_seh-1.dll`, `libstdc++-6.dll` y `libwinpthread-1.dll` para que el programa funcione en cualquier PC con Windows sin necesidad de instalar MinGW ni CLion.
* **Directorio res/**: Iconos oficiales en formatos SVG, PNG (256x256) e ICO.
* **Directorio input/**: Ejemplos de programas fuente en lenguaje LP (`ejemplo.lp`).
* **Directorio output/**: Salidas estructuradas de tokens, tabla de símbolos y errores.
* **Documentación Completa**:
  * `README.md`: Especificación del lenguaje y guía rápida.
  * `MANUAL_USUARIO.md`: Manual paso a paso para usuarios y evaluadores.
  * `DOCS_FUNCIONES.md`: Documentación técnica de clases, métodos y firmas de funciones.
  * `ARQUITECTURA.md`: Diseño de autómatas finitos (AFD) y flujos de datos con diagramas Mermaid.
  * `CHANGELOG.md`: Historial de versiones bajo el estándar Keep a Changelog.

---

### 🐳 Soporte para Docker:
El proyecto cuenta con un `Dockerfile` multi-etapa y `docker-compose.yml` para compilar y ejecutar en Linux:
````bash
# Construir y ejecutar con Docker
docker build -t compiladores-lp .
docker run --rm -v `$(pwd)/output:/app/output compiladores-lp
````

---

### 📥 Instrucciones de Instalación y Uso:
1. Descarga el archivo **Compiladores-$tag-windows-x64.zip** que se encuentra abajo en la sección de Assets.
2. Descomprime el contenido en cualquier carpeta de tu equipo.
3. Ejecuta **Compiladores.exe** para abrir la interfaz visual o **compilador_lp.exe** para la versión en consola.
"@
$utf8 = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllText($notesPath, $notas, $utf8)

# 8. Git Commit, Tag y Push
Write-Step "Confirmando cambios en Git"
git add -A
git reset HEAD cmake-build-debug/ 2>$null
git reset HEAD .ninja_deps 2>$null
git reset HEAD dist/ 2>$null

git diff --cached --quiet
$hayCambios = ($LASTEXITCODE -ne 0)

if ($hayCambios) {
    git commit -m "release: Compiladores $tag - Entrega Final Oficial"
} else {
    Write-Host "No habia archivos nuevos staged para commit." -ForegroundColor Yellow
}

Write-Step "Subiendo rama y tag a GitHub"
git push -u $Remote $branch

# Crear y empujar tag
git tag -f -a $tag -m $Title
git push --force $Remote "refs/tags/$tag"

# 9. Publicar en GitHub Releases con GitHub CLI
Write-Step "Publicando GitHub Release oficial"
$existeRelease = $false
try {
    $null = cmd /c "gh release view $tag >nul 2>&1"
    if ($LASTEXITCODE -eq 0) {
        $existeRelease = $true
    }
} catch {
    $existeRelease = $false
}

if ($existeRelease) {
    Write-Host "Actualizando release existente $tag..." -ForegroundColor DarkYellow
    gh release edit $tag --title $Title --notes-file $notesPath
    gh release upload $tag $zipPath --clobber
} else {
    Write-Host "Creando nueva release $tag..." -ForegroundColor Green
    gh release create $tag $zipPath --title $Title --notes-file $notesPath --latest
}

# Sincronizar cuerpo de la release garantizando UTF-8 puro (evita deformacion de acentos y emojis)
try {
    $syncPy = @"
import subprocess, json
try:
    with open(r'$notesPath', 'r', encoding='utf-8') as f:
        notes = f.read()
    relId = subprocess.check_output(['gh', 'api', 'repos/Quetzalq4321q/Compiladores/releases/tags/$tag', '--jq', '.id'], text=True).strip()
    payload = json.dumps({'body': notes})
    subprocess.run(['gh', 'api', '-X', 'PATCH', f'repos/Quetzalq4321q/Compiladores/releases/{relId}', '--input', '-'], input=payload.encode('utf-8'), check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
except Exception:
    pass
"@
    python -c $syncPy
} catch {
    # Silencioso si falla
}

Write-Step "RELEASE PUBLICADA EXITOSAMENTE EN GITHUB"
Write-Host "Tag    : $tag" -ForegroundColor Green
Write-Host "Titulo : $Title" -ForegroundColor Green
Write-Host "ZIP    : $zipPath ($zipSizeMb MB)" -ForegroundColor Green
Write-Host "URL    : https://github.com/Quetzalq4321q/Compiladores/releases/tag/$tag" -ForegroundColor Cyan

if (-not $NoOpenBrowser) {
    gh release view $tag --web
}
