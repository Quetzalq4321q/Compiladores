param(
    [Parameter(Position = 0)]
    [string]$Version,

    [string]$Title = "",

    [string]$Remote = "origin",

    [switch]$NoOpenBrowser,

    [switch]$Docker
)

$ErrorActionPreference = "Stop"

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
    $Version = Read-Host "Ingresa la version de release (ej. 0.1.0)"
}
$Version = Normalize-Version -InputVersion $Version
$tag = "v$Version"

if (-not $Title) {
    $Title = "Compiladores LP $tag"
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

# 4. Compilar proyecto antes del release
Write-Step "Compilando Compiladores.exe"
$cmakeExe = "C:\Users\User\AppData\Local\Programs\CLion\bin\cmake\win\x64\bin\cmake.exe"
if (-not (Test-Path $cmakeExe)) {
    $cmakeExe = (Get-Command cmake -ErrorAction SilentlyContinue).Source
}

$buildDir = Join-Path $projectRoot "cmake-build-debug"
$exePath  = Join-Path $buildDir "Compiladores.exe"

if (Test-Path $cmakeExe) {
    Write-Host "Compilando con CMake..." -ForegroundColor Yellow
    & $cmakeExe --build $buildDir --target Compiladores
} else {
    Write-Host "Aviso: No se encontro cmake.exe directo, verificando existencia de binario..." -ForegroundColor Yellow
}

if (-not (Test-Path $exePath)) {
    throw "No se encontro el ejecutable en $exePath. Compila primero el proyecto en CLion."
}
Write-Host "Binario listo: $exePath" -ForegroundColor Green

# 5. Si se pidio Docker o se encuentra Docker instalado, construir imagen
if ($Docker -or (Get-Command docker -ErrorAction SilentlyContinue)) {
    Write-Step "Construyendo imagen Docker (Dockeado)"
    if (Get-Command docker -ErrorAction SilentlyContinue) {
        docker build -t "compiladores:$tag" -t "compiladores:latest" .
        if ($LASTEXITCODE -eq 0) {
            Write-Host "Imagen Docker compiladores:$tag creada con exito." -ForegroundColor Green
        } else {
            Write-Host "Advertencia: La construccion con Docker retorno error." -ForegroundColor Yellow
        }
    } else {
        Write-Host "Docker no esta instalado localmente. Saltando etapa Docker." -ForegroundColor Yellow
    }
}

# 6. Preparar paquete ZIP de distribucion
Write-Step "Empaquetando release en dist/"
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

# Copiar archivos a empaquetar
Copy-Item -Path $exePath -Destination $stagingDir
if (Test-Path (Join-Path $projectRoot "tests")) {
    Copy-Item -Path (Join-Path $projectRoot "tests") -Destination $stagingDir -Recurse
}
if (Test-Path (Join-Path $projectRoot "input")) {
    Copy-Item -Path (Join-Path $projectRoot "input") -Destination $stagingDir -Recurse
}
if (Test-Path (Join-Path $projectRoot "README.md")) {
    Copy-Item -Path (Join-Path $projectRoot "README.md") -Destination $stagingDir
}

# Crear ZIP
Compress-Archive -Path "$stagingDir\*" -DestinationPath $zipPath -Force
Remove-Item -Recurse -Force $stagingDir

$zipSizeMb = [Math]::Round((Get-Item $zipPath).Length / 1MB, 2)
Write-Host "ZIP generado: $zipPath ($zipSizeMb MB)" -ForegroundColor Green

# 7. Generar notas de release
$notesPath = Join-Path $distDir "RELEASE_NOTES_$tag.md"
$fecha = Get-Date -Format "yyyy-MM-dd"
$notas = @"
# $Title

Fecha: $fecha

## Contenido de la Entrega (Semana 1)
- Analizador lexico para el lenguaje LP en C++20.
- Soporte para lexemas de numeros enteros (NUM_INT) y decimales (NUM_DEC).
- Interfaz visual en Windows (WinAPI) con pestañas:
  - Lista de Tokens reconocidos.
  - Tabla de Simbolos (preparada para proximas entregas).
  - Errores Lexicos con ubicacion por linea.
- Soporte para lectura de archivos .lp, .txt, .docx y .pdf.
- Exportacion automatica de resultados a carpeta output/ (tokens.txt, tabla_simbolos.txt, errores.txt).
- Contenedorizacion mediante Dockerfile incluido.

## Descarga
Descarga y descomprime el archivo **Compiladores-$tag-windows-x64.zip** para ejecutar la aplicacion.
"@
$utf8 = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllText($notesPath, $notas, $utf8)

# 8. Git Commit & Tag
Write-Step "Confirmando cambios en Git"
git add CMakeLists.txt src/ tests/ Dockerfile release.ps1 .gitignore README.md 2>$null

# Evitar commitear basura o binarios pesados si quedaron staged
git reset HEAD cmake-build-debug/ 2>$null
git reset HEAD .ninja_deps 2>$null

git diff --cached --quiet
$hayCambios = ($LASTEXITCODE -ne 0)

if ($hayCambios) {
    git commit -m "release: Compiladores $tag"
} else {
    Write-Host "No habia archivos nuevos staged para commit." -ForegroundColor Yellow
}

Write-Step "Subiendo rama y tag a GitHub"
git push -u $Remote $branch

# Crear tag
git tag -f -a $tag -m $Title
git push --force $Remote "refs/tags/$tag"

# 9. Publicar en GitHub Releases con GitHub CLI
Write-Step "Publicando GitHub Release"
gh release view $tag *> $null
$existeRelease = ($LASTEXITCODE -eq 0)

if ($existeRelease) {
    Write-Host "Actualizando release existente $tag..." -ForegroundColor DarkYellow
    gh release edit $tag --title $Title --notes-file $notesPath
    gh release upload $tag $zipPath --clobber
} else {
    Write-Host "Creando nueva release $tag..." -ForegroundColor Green
    gh release create $tag $zipPath --title $Title --notes-file $notesPath --latest
}

Write-Step "RELEASE COMPLETADA CON EXITO"
Write-Host "Tag    : $tag" -ForegroundColor Green
Write-Host "Titulo : $Title" -ForegroundColor Green
Write-Host "ZIP    : $zipPath" -ForegroundColor Green

if (-not $NoOpenBrowser) {
    gh release view $tag --web
}
