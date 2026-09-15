// ---------------------------------------------------------------
// FileConverter.cpp  –  Lector multi-formato de archivos.
//
// .lp / .txt  → se leen directamente con ifstream.
// .docx       → es un ZIP; se extrae word/document.xml con
//               PowerShell + .NET (no necesita Office).
// .pdf        → se abre con Word COM y se exporta como texto
//               (necesita Microsoft Word instalado).
//
// NOTA: en este archivo NO se usa "using namespace std" porque
// windows.h define 'byte' que colisiona con std::byte en C++17.
// Se usa std:: explícitamente en su lugar.
// ---------------------------------------------------------------

// IMPORTANTE: windows.h PRIMERO, antes de cualquier header de C++,
// para evitar la colisión std::byte vs byte de Windows.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "FileConverter.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

// Variable estática para almacenar el último error.
std::string FileConverter::error_ = "";

// ---------------------------------------------------------------
// getExtension() – Extrae la extensión en minúsculas.
// ---------------------------------------------------------------
std::string FileConverter::getExtension(const std::string& ruta) {
    size_t punto = ruta.rfind('.');
    if (punto == std::string::npos) return "";
    std::string ext = ruta.substr(punto);
    std::transform(ext.begin(), ext.end(), ext.begin(),
              [](unsigned char c){ return std::tolower(c); });
    return ext;
}

// ---------------------------------------------------------------
// leerTextoPlano() – Lee un archivo de texto completo.
// ---------------------------------------------------------------
std::string FileConverter::leerTextoPlano(const std::string& ruta) {
    std::ifstream f(ruta);
    if (!f) {
        error_ = "No se pudo abrir el archivo: " + ruta;
        return "";
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// ---------------------------------------------------------------
// ejecutarPS() – Ejecuta un script PowerShell de forma SILENCIOSA.
//
// 1. Guarda el script en un .ps1 temporal.
// 2. Ejecuta con CreateProcessA (sin ventana visible).
// 3. Lee la salida redirigida a un .txt temporal.
// 4. Limpia los temporales y retorna el resultado.
// ---------------------------------------------------------------
std::string FileConverter::ejecutarPS(const std::string& script) {
    char tempDir[MAX_PATH];
    GetTempPathA(MAX_PATH, tempDir);

    std::string scriptPath = std::string(tempDir) + "lexlp_convert.ps1";
    std::string outputPath = std::string(tempDir) + "lexlp_output.txt";

    // Paso 1: guardar el script .ps1
    {
        std::ofstream f(scriptPath);
        if (!f) { error_ = "No se pudo crear script temporal."; return ""; }
        f << script;
    }

    // Paso 2: construir y ejecutar el comando sin ventana
    std::string cmd = "cmd.exe /C powershell.exe -NoProfile -ExecutionPolicy Bypass -File \""
                    + scriptPath + "\" > \"" + outputPath + "\" 2>&1";

    STARTUPINFOA si = {};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi = {};

    BOOL ok = CreateProcessA(
        NULL, &cmd[0], NULL, NULL, FALSE,
        CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

    if (!ok) {
        error_ = "No se pudo ejecutar PowerShell.";
        DeleteFileA(scriptPath.c_str());
        return "";
    }

    WaitForSingleObject(pi.hProcess, 30000);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // Paso 3: leer resultado
    std::string resultado;
    {
        std::ifstream f(outputPath);
        if (f) {
            std::ostringstream ss;
            ss << f.rdbuf();
            resultado = ss.str();
        }
    }

    // Paso 4: limpiar temporales
    DeleteFileA(scriptPath.c_str());
    DeleteFileA(outputPath.c_str());

    return resultado;
}

// ---------------------------------------------------------------
// extraerDocx() – Extrae texto de un .docx usando PowerShell .NET.
// NO requiere Microsoft Office.
// ---------------------------------------------------------------
std::string FileConverter::extraerDocx(const std::string& ruta) {
    std::string rutaPS = ruta;
    size_t pos = 0;
    while ((pos = rutaPS.find('\'', pos)) != std::string::npos) {
        rutaPS.replace(pos, 1, "''");
        pos += 2;
    }

    std::string script =
        "Add-Type -AssemblyName System.IO.Compression.FileSystem\n"
        "try {\n"
        "    $zip = [System.IO.Compression.ZipFile]::OpenRead('" + rutaPS + "')\n"
        "    $entry = $zip.Entries | Where-Object { $_.FullName -eq 'word/document.xml' }\n"
        "    if ($entry -eq $null) { $zip.Dispose(); Write-Error 'No es un DOCX valido'; exit 1 }\n"
        "    $stream = $entry.Open()\n"
        "    $reader = New-Object System.IO.StreamReader($stream)\n"
        "    $xmlText = $reader.ReadToEnd()\n"
        "    $reader.Close(); $stream.Close(); $zip.Dispose()\n"
        "    $xdoc = [xml]$xmlText\n"
        "    $ns = New-Object System.Xml.XmlNamespaceManager($xdoc.NameTable)\n"
        "    $ns.AddNamespace('w','http://schemas.openxmlformats.org/wordprocessingml/2006/main')\n"
        "    $paragraphs = $xdoc.SelectNodes('//w:p', $ns)\n"
        "    $lines = @()\n"
        "    foreach ($p in $paragraphs) {\n"
        "        $runs = $p.SelectNodes('.//w:t', $ns)\n"
        "        $line = ($runs | ForEach-Object { $_.'#text' }) -join ''\n"
        "        $lines += $line\n"
        "    }\n"
        "    $lines -join \"`n\"\n"
        "} catch { Write-Error $_.Exception.Message; exit 1 }\n";

    std::string resultado = ejecutarPS(script);
    if (resultado.empty()) {
        error_ = "No se pudo extraer texto del DOCX.";
    }
    return resultado;
}

// ---------------------------------------------------------------
// extraerPdf() – Extrae texto de un .pdf usando Word COM.
// REQUIERE Microsoft Word instalado.
// ---------------------------------------------------------------
std::string FileConverter::extraerPdf(const std::string& ruta) {
    std::string rutaPS = ruta;
    size_t pos = 0;
    while ((pos = rutaPS.find('\'', pos)) != std::string::npos) {
        rutaPS.replace(pos, 1, "''");
        pos += 2;
    }

    char tempDir[MAX_PATH];
    GetTempPathA(MAX_PATH, tempDir);
    std::string tempTxt = std::string(tempDir) + "lexlp_pdf.txt";

    std::string tempPS = tempTxt;
    pos = 0;
    while ((pos = tempPS.find('\'', pos)) != std::string::npos) {
        tempPS.replace(pos, 1, "''");
        pos += 2;
    }

    std::string script =
        "try {\n"
        "    $word = New-Object -ComObject Word.Application\n"
        "    $word.Visible = $false\n"
        "    $doc = $word.Documents.Open('" + rutaPS + "')\n"
        "    $doc.SaveAs2('" + tempPS + "', 2)\n"
        "    $doc.Close(); $word.Quit()\n"
        "    [System.Runtime.Interopservices.Marshal]::ReleaseComObject($word) | Out-Null\n"
        "    Get-Content -Path '" + tempPS + "' -Raw\n"
        "} catch { Write-Error ('Se requiere Microsoft Word. ' + $_.Exception.Message); exit 1 }\n";

    std::string resultado = ejecutarPS(script);
    DeleteFileA(tempTxt.c_str());

    if (resultado.empty()) {
        error_ = "No se pudo extraer texto del PDF.\nSe requiere Microsoft Word instalado.";
    }
    return resultado;
}

// ---------------------------------------------------------------
// leer() – Punto de entrada. Detecta extensión y llama al método adecuado.
// ---------------------------------------------------------------
std::string FileConverter::leer(const std::string& ruta) {
    error_ = "";
    std::string ext = getExtension(ruta);

    if (ext == ".lp" || ext == ".txt") return leerTextoPlano(ruta);
    if (ext == ".docx")                return extraerDocx(ruta);
    if (ext == ".pdf")                 return extraerPdf(ruta);

    error_ = "Formato no soportado: " + ext + "\nUsa archivos .lp, .txt, .docx o .pdf";
    return "";
}

std::string FileConverter::ultimoError() { return error_; }
