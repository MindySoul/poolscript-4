#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <filesystem>
#include <cstdio>
#include <curl/curl.h>

namespace fs = std::filesystem;

// Variáveis globais para as credenciais do Supabase
const std::string SUPABASE_URL = "https://vyrgjdoixpghdsehashs.supabase.co";
const std::string SUPABASE_API_KEY = "sb_publishable_uNjraTVSDlGyT6p72N41xg_S1gV76wg";
const std::string EDGE_FUNCTION_URL = SUPABASE_URL + "/functions/v1/transpile";

// Callback para capturar a resposta do curl
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Lê arquivo .pool do disco
std::string ler_arquivo(const std::string& caminho) {
    std::ifstream f(caminho);
    if (!f.is_open()) {
        std::cerr << "[PoolScript] Erro: nao foi possivel abrir " << caminho << std::endl;
        std::exit(1);
    }
    std::stringstream buf;
    buf << f.rdbuf();
    return buf.str();
}

// Salva código Python gerado em arquivo temporário e executa
void executar_python(const std::string& codigo, const std::string& dir_pool) {
    std::string tmp = std::string(std::getenv("TEMP")) + "\\poolscript_exec.py";

    std::ofstream f(tmp);
    f << "import sys, os\n";
    f << "sys.path.insert(0, r'" << dir_pool << "')\n";
    f << "os.chdir(r'" << dir_pool << "')\n";
    f << codigo;
    f.close();

    std::string cmd = "python \"" + tmp + "\"";
    int result = std::system(cmd.c_str());
    (void)result; // Suppress unused variable warning

    // Limpa arquivo temporário
    std::remove(tmp.c_str());
}

// Função para desescapar strings JSON
std::string unescape_json(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '\\' && i + 1 < str.length()) {
            switch (str[++i]) {
                case 'n': result += '\n'; break;
                case 't': result += '\t'; break;
                case 'r': result += '\r'; break;
                case '\\': result += '\\'; break;
                case '"': result += '"'; break;
                case '/': result += '/'; break;
                case 'b': result += '\b'; break;
                case 'f': result += '\f'; break;
                default: result += str[i]; break;
            }
        } else {
            result += str[i];
        }
    }
    return result;
}

// Função para fazer a requisição HTTP para a Edge Function do Supabase usando libcurl
std::string transpile_remote(
    const std::string& pool_code,
    const std::string& file_name
) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "[PoolScript] Erro: nao foi possivel inicializar curl." << std::endl;
        return "[ERRO DE INFERENCIA]";
    }

    // Monta o corpo da requisição JSON
    // Escapar aspas duplas no código
    std::string escaped_code = pool_code;
    size_t pos = 0;
    while ((pos = escaped_code.find('"', pos)) != std::string::npos) {
        escaped_code.replace(pos, 1, "\\\"");
        pos += 2;
    }
    // Escapar quebras de linha
    pos = 0;
    while ((pos = escaped_code.find('\n', pos)) != std::string::npos) {
        escaped_code.replace(pos, 1, "\\n");
        pos += 2;
    }

    std::string json_payload = "{\"code\": \"" + escaped_code + "\", \"fileName\": \"" + file_name + "\"}";

    // Configurar curl
    std::string response_body;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("apikey: " + SUPABASE_API_KEY).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, EDGE_FUNCTION_URL.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    // Executar requisição
    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        std::cerr << "[PoolScript] Erro ao chamar a Edge Function do Supabase: " << curl_easy_strerror(res) << std::endl;
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return "[ERRO DE INFERENCIA]";
    }

    // Verificar código HTTP
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200) {
        std::cerr << "[PoolScript] Erro HTTP " << http_code << " da Edge Function: " << response_body << std::endl;
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return "[ERRO DE INFERENCIA]";
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    // Extrair o campo 'transpiledCode' do JSON
    std::string search_str = "\"transpiledCode\":\"";
    size_t start_pos = response_body.find(search_str);
    if (start_pos == std::string::npos) {
        if (response_body.find("\"error\":") != std::string::npos) {
            std::cerr << "[PoolScript] Erro da Edge Function: " << response_body << std::endl;
        }
        return "[ERRO DE INFERENCIA]";
    }

    start_pos += search_str.length();
    size_t end_pos = response_body.find("\"", start_pos);
    if (end_pos == std::string::npos) {
        return "[ERRO DE INFERENCIA]";
    }

    std::string transpiled_code = response_body.substr(start_pos, end_pos - start_pos);
    // Desescapar o JSON
    transpiled_code = unescape_json(transpiled_code);

    return transpiled_code;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Uso: pool arquivo.pool" << std::endl;
        return 1;
    }

    std::string arquivo_pool = argv[1];
    if (arquivo_pool.size() < 5 || arquivo_pool.substr(arquivo_pool.size() - 5) != ".pool") {
        std::cerr << "[PoolScript] Erro: o arquivo deve ter extensao .pool" << std::endl;
        return 1;
    }

    std::string codigo_pool = ler_arquivo(arquivo_pool);
    if (codigo_pool.empty()) {
        std::cout << "[PoolScript] Arquivo vazio." << std::endl;
        return 0;
    }

    // Diretório do arquivo .pool para sys.path
    fs::path p(arquivo_pool);
    std::string dir_pool = fs::absolute(p.parent_path()).string();
    std::string file_name = p.filename().string();

    std::cout << "[PoolScript] Transpilando via Supabase Edge Function..." << std::endl;
    std::string codigo_python = transpile_remote(codigo_pool, file_name);

    if (codigo_python.empty() || codigo_python == "[ERRO DE INFERENCIA]") {
        std::cerr << "[PoolScript] Erro na transpilação ou resposta vazia." << std::endl;
        return 1;
    } else {
        std::cout << "[PoolScript] Executando...\n" << std::endl;
        executar_python(codigo_python, dir_pool);
    }

    return 0;
}
