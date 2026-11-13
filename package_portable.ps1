param(
    [string]$BuildDir = "cmake-build-release-portable",
    [string]$PackageName = "Compiladores-portable-windows-x64"
)

$ErrorActionPreference = "Stop"
if ($PackageName -notmatch '^[A-Za-z0-9_-]+$') {
    throw "PackageName solo puede contener letras, numeros, guiones y guiones bajos."
}
$root = $PSScriptRoot
$build = Join-Path $root $BuildDir
$stage = Join-Path $root "dist\$PackageName"
$zip = Join-Path $root "dist\$PackageName.zip"

foreach ($name in @("Compiladores.exe", "compilador_lp.exe")) {
    if (-not (Test-Path (Join-Path $build $name))) {
        throw "Falta $name en $build. Compila primero en modo Release."
    }
}

$cache = Get-Content (Join-Path $build "CMakeCache.txt")
$compilerEntry = $cache | Where-Object { $_ -match '^CMAKE_CXX_COMPILER:(FILEPATH|STRING)=' } | Select-Object -First 1
if (-not $compilerEntry) { throw "No se encontro el compilador en CMakeCache.txt" }
$compiler = ($compilerEntry -split '=', 2)[1]
$mingwBin = Split-Path $compiler -Parent

New-Item -ItemType Directory -Path (Join-Path $root "dist") -Force | Out-Null
if (Test-Path $stage) { Remove-Item -LiteralPath $stage -Recurse -Force }
if (Test-Path $zip) { Remove-Item -LiteralPath $zip -Force }
New-Item -ItemType Directory -Path $stage | Out-Null
New-Item -ItemType Directory -Path (Join-Path $stage "output") | Out-Null
"Los reportes se generan aqui." | Set-Content (Join-Path $stage "output\LEEME.txt") -Encoding UTF8

Copy-Item (Join-Path $build "Compiladores.exe") $stage
Copy-Item (Join-Path $build "compilador_lp.exe") $stage
Copy-Item (Join-Path $root "input") $stage -Recurse
Copy-Item (Join-Path $root "tests") $stage -Recurse
foreach ($doc in @("README.md", "MANUAL_USUARIO.md")) {
    Copy-Item (Join-Path $root $doc) $stage
}
foreach ($dll in @("libgcc_s_seh-1.dll", "libstdc++-6.dll", "libwinpthread-1.dll")) {
    $source = Join-Path $mingwBin $dll
    if (Test-Path $source) { Copy-Item $source $stage }
}

@'
@echo off
cd /d "%~dp0"
start "" "%~dp0Compiladores.exe"
'@ | Set-Content (Join-Path $stage "abrir_gui.bat") -Encoding Ascii
@'
@echo off
cd /d "%~dp0"
"%~dp0compilador_lp.exe" %*
pause
'@ | Set-Content (Join-Path $stage "ejecutar_cli.bat") -Encoding Ascii
@'
COMPILADORES LP - VERSION PORTABLE PARA WINDOWS x64

1. Extrae todo el ZIP a una carpeta con permisos de escritura.
2. Abre abrir_gui.bat para usar la interfaz grafica.
3. Abre ejecutar_cli.bat para procesar input/ejemplo.lp.
4. Para procesar otro archivo desde una terminal:
   ejecutar_cli.bat "ruta\a\programa.lp"

Los reportes se guardan en output/. La conversion de PDF requiere Microsoft Word.
'@ | Set-Content (Join-Path $stage "LEEME_PORTABLE.txt") -Encoding UTF8

Compress-Archive -Path (Join-Path $stage '*') -DestinationPath $zip -CompressionLevel Optimal
Write-Output $zip
