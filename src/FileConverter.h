#pragma once

#include <string>

/* Lee y convierte archivos de entrada (.lp, .txt, .docx, .pdf) a texto plano */
class FileConverter {
public:
    /* Lee el archivo y retorna su contenido en texto plano */
    static std::string leer(const std::string& ruta);

    /* Mensaje del ultimo error ocurrido */
    static std::string ultimoError();

private:
    static std::string error_;

    /* Extrae la extension en minusculas */
    static std::string getExtension(const std::string& ruta);

    /* Lee archivos de texto plano (.lp, .txt) */
    static std::string leerTextoPlano(const std::string& ruta);

    /* Extrae texto de un documento Word (.docx) */
    static std::string extraerDocx(const std::string& ruta);

    /* Extrae texto de un documento PDF (.pdf) */
    static std::string extraerPdf(const std::string& ruta);

    /* Ejecuta un comando PowerShell de forma oculta */
    static std::string ejecutarPS(const std::string& script);
};
