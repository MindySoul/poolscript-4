#include "llama.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

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
    std::system(cmd.c_str());

    // Limpa arquivo temporário
    std::remove(tmp.c_str());
}

// Tokeniza e faz inferência com llama.cpp
std::string inferir(llama_context* ctx, llama_model* model, const std::string& prompt) {
    std::string resultado;

    std::vector<llama_token> tokens(prompt.size() + 32);
    int n_tokens = llama_tokenize(
        model,
        prompt.c_str(),
        prompt.size(),
        tokens.data(),
        tokens.size(),
        true,
        false
    );
    tokens.resize(n_tokens);

    llama_batch batch = llama_batch_init(512, 0, 1);

    for (int i = 0; i < n_tokens; i++) {
        llama_batch_add(batch, tokens[i], i, {0}, false);
    }
    batch.logits[batch.n_tokens - 1] = true;

    if (llama_decode(ctx, batch) != 0) {
        llama_batch_free(batch);
        return "[ERRO DE INFERENCIA]";
    }

    int n_cur = batch.n_tokens;
    int n_max = 1024;

    while (n_cur < n_max) {
        auto* logits = llama_get_logits_ith(ctx, batch.n_tokens - 1);
        int n_vocab = llama_n_vocab(model);

        llama_token new_token = 0;
        float max_logit = logits[0];
        for (int i = 1; i < n_vocab; i++) {
            if (logits[i] > max_logit) {
                max_logit = logits[i];
                new_token = i;
            }
        }

        if (new_token == llama_token_eos(model)) break;

        char buf[256];
        int len = llama_token_to_piece(model, new_token, buf, sizeof(buf), 0, false);
        if (len > 0) resultado.append(buf, len);

        llama_batch_clear(batch);
        llama_batch_add(batch, new_token, n_cur, {0}, true);
        llama_decode(ctx, batch);
        n_cur++;
    }

    llama_batch_free(batch);
    return resultado;
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

    // Prompt para a IA
    std::string prompt =
        "Você é o núcleo de execução da linguagem PoolScript. "
        "Sua única função é traduzir a intenção do usuário em código Python executável. "
        "Responda APENAS com código Python puro. Sem explicações, sem blocos de markdown, sem comentários.\n\n"
        "Código PoolScript:\n" + codigo_pool + "\n\nCódigo Python:";

    // Inicializa llama.cpp
    llama_backend_init();

    auto mparams = llama_model_default_params();
    // Ajuste: i5 4a geração com 8GB RAM — modelo pequeno
    std::string model_path = "models/qwen2.5-coder-0.5b.gguf";

    std::cout << "[PoolScript] Carregando modelo..." << std::endl;
    llama_model* model = llama_load_model_from_file(model_path.c_str(), mparams);
    if (!model) {
        std::cerr << "[PoolScript] Erro: modelo nao encontrado em " << model_path << std::endl;
        std::cerr << "[PoolScript] Baixe o modelo e coloque na pasta models/" << std::endl;
        llama_backend_free();
        return 1;
    }

    auto cparams = llama_context_default_params();
    cparams.n_ctx = 512;      // Janela pequena — economiza RAM no i5
    cparams.n_threads = 4;    // 4 núcleos do i5 4a geração

    llama_context* ctx = llama_new_context_with_model(model, cparams);

    std::cout << "[PoolScript] Transpilando..." << std::endl;
    std::string codigo_python = inferir(ctx, model, prompt);

    if (codigo_python.empty() || codigo_python == "[ERRO DE INFERENCIA]") {
        std::cerr << "[PoolScript] Erro na transpilação." << std::endl;
    } else {
        std::cout << "[PoolScript] Executando...\n" << std::endl;
        executar_python(codigo_python, dir_pool);
    }

    // Limpeza
    llama_free(ctx);
    llama_model_free(model);
    llama_backend_free();

    return 0;
}
