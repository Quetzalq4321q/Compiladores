#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "FileConverter.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

std::string FileConverter::error_ = "";

/* Obtiene extension en minusculas para determinar el formato */
std::string FileConverter::getExtension(const std::string& ruta) {
    size_t punto = ruta.rfind('.');
    if (punto == std::string::npos) return "";
    std::string ext = ruta.substr(punto);
    std::transform(ext.begin(), ext.end(), ext.begin(),
              [](unsigned char c){ return std::tolower(c); });
    return ext;
}

/* Lectura basica para .lp y .txt */
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

/* Ejecuta script PowerShell en segundo plano sin mostrar ventana de consola */
std::string FileConverter::ejecutarPS(const std::string& script) {
    char tempDir[MAX_PATH];
    GetTempPathA(MAX_PATH, tempDir);

    std::string scriptPath = std::string(tempDir) + "lexlp_convert.ps1";
    std::string outputPath = std::string(tempDir) + "lexlp_output.txt";

    {
        std::ofstream f(scriptPath);
        if (!f) { error_ = "No se pudo crear script temporal."; return ""; }
        f << script;
    }

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

    std::string resultado;
    {
        std::ifstream f(outputPath);
        if (f) {
            std::ostringstream ss;
            ss << f.rdbuf();
            resultado = ss.str();
        }
    }

    DeleteFileA(scriptPath.c_str());
    DeleteFileA(outputPath.c_str());

    return resultado;
}

/* Descomprime el DOCX y extrae el contenido textual de word/document.xml */
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

/* Abre PDF usando COM de Word para convertirlo a texto */
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

/* Dispatcher de lectura segun el tipo de archivo */
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
