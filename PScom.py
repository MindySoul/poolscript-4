#!/usr/bin/env python3
"""
PScom.py - Motor de execução da linguagem PoolScript v4.0.0
Lê arquivos .pool, transpila via Ollama e executa o Python gerado.
"""

import sys
import os
import re
import subprocess
import tempfile
import importlib.util
import json
import hashlib

OLLAMA_URL = "http://localhost:11434/api/generate"
OLLAMA_MODEL = "deepseek-r1:8b"

SYSTEM_PROMPT = """Você é o núcleo de execução da linguagem PoolScript. Sua única função é traduzir a intenção do usuário em código Python executável e otimizado.

REGRAS CRÍTICAS:
Responda APENAS com código Python puro. Não use explicações, não use blocos de Markdown (```python), não escreva # comentários no código.
Se a intenção for ambígua ou faltar algo necessário, deixe o Python mandar o erro para o terminal do usuário.
Se o usuário mencionar arquivos locais, use caminhos relativos compatíveis com Windows.
Se o usuário mencionar bibliotecas Python, instale-as automaticamente.
Adicione tratamento de erros try-except básico para reportar falhas de execução de forma limpa.
Ao manipular arquivos, use apenas o nome do arquivo ou caminhos relativos. O ambiente de execução já está configurado no diretório correto do script."""

CACHE_FILE = os.path.join(tempfile.gettempdir(), "poolscript_cache.json")

def load_cache():
    try:
        if os.path.exists(CACHE_FILE):
            with open(CACHE_FILE, "r", encoding="utf-8") as f:
                return json.load(f)
    except:
        pass
    return {}

def save_cache(cache):
    try:
        with open(CACHE_FILE, "w", encoding="utf-8") as f:
            json.dump(cache, f)
    except:
        pass

def get_hash(content):
    return hashlib.sha256(content.encode()).hexdigest()

def transpilar(codigo_pool, arquivo_nome, contexto_extra=""):
    try:
        import urllib.request
        import urllib.error

        prompt_completo = f"{SYSTEM_PROMPT}\n\nArquivo: {arquivo_nome}\nCódigo PoolScript:\n{codigo_pool}"
        if contexto_extra:
            prompt_completo += f"\n\nContexto adicional (outros arquivos .pool):\n{contexto_extra}"

        body = json.dumps({
            "model": OLLAMA_MODEL,
            "prompt": prompt_completo,
            "stream": False,
            "options": {"temperature": 0.1}
        }).encode("utf-8")

        req = urllib.request.Request(
            OLLAMA_URL,
            data=body,
            headers={"Content-Type": "application/json"},
            method="POST"
        )

        with urllib.request.urlopen(req, timeout=120) as resp:
            resultado = json.loads(resp.read().decode("utf-8"))
            codigo = resultado.get("response", "").strip()

            # Remove blocos markdown se a IA ignorar as regras
            match = re.search(r"```(?:python)?\s*([\s\S]*?)```", codigo)
            if match:
                codigo = match.group(1).strip()

            return codigo

    except Exception as e:
        print(f"[PoolScript] Erro ao conectar com Ollama: {e}")
        print("[PoolScript] Verifique se o Ollama está rodando: ollama serve")
        sys.exit(1)

def pre_scanner_dependencias(codigo_python):
    """Escaneia imports e instala pacotes faltantes silenciosamente."""
    imports = []
    for linha in codigo_python.splitlines():
        linha = linha.strip()
        if linha.startswith("import "):
            pacote = linha.split()[1].split(".")[0]
            imports.append(pacote)
        elif linha.startswith("from "):
            pacote = linha.split()[1].split(".")[0]
            imports.append(pacote)

    stdlib = {
        "os", "sys", "re", "json", "math", "time", "datetime", "random",
        "string", "pathlib", "subprocess", "tempfile", "hashlib", "base64",
        "urllib", "http", "socket", "threading", "logging", "copy", "io",
        "collections", "functools", "itertools", "operator", "struct",
        "pickle", "csv", "html", "xml", "email", "smtplib", "sqlite3",
        "unittest", "abc", "typing", "enum", "dataclasses", "contextlib",
        "traceback", "inspect", "gc", "platform", "shutil", "glob",
        "fnmatch", "linecache", "tokenize", "ast", "dis", "cProfile",
        "pprint", "textwrap", "difflib", "uuid", "hmac", "secrets",
        "zipfile", "tarfile", "gzip", "bz2", "lzma", "stat", "errno",
        "signal", "ctypes", "mmap", "array", "queue", "heapq", "bisect",
        "weakref", "builtins", "importlib", "pkgutil", "site"
    }

    for pacote in set(imports):
        if pacote in stdlib:
            continue
        if importlib.util.find_spec(pacote) is None:
            print(f"[PoolScript] Instalando dependência: {pacote}...")
            subprocess.run(
                [sys.executable, "-m", "pip", "install", pacote, "--quiet"],
                capture_output=True
            )

def mapear_arquivos_locais(pool_dir, codigo_pool):
    """Mapeia arquivos locais mencionados no código .pool."""
    extensoes_permitidas = {".txt", ".csv", ".png", ".jpg", ".jpeg", ".html", ".css", ".json", ".pdf", ".docx", ".pool"}
    extensoes_codigo = {".py", ".js", ".ts", ".java", ".cpp", ".c", ".cs", ".go", ".rs"}

    arquivos_encontrados = []
    palavras = re.findall(r'[\w\-]+\.\w+', codigo_pool)

    for palavra in palavras:
        ext = os.path.splitext(palavra)[1].lower()
        if ext in extensoes_codigo:
            print(f"[PoolScript] Erro: arquivos de código ({palavra}) não são suportados como dependência.")
            continue
        if ext in extensoes_permitidas:
            caminho = os.path.join(pool_dir, palavra)
            if os.path.exists(caminho):
                arquivos_encontrados.append(caminho)

    return arquivos_encontrados

def coletar_contexto_pool(pool_dir, arquivo_principal):
    """Coleta outros arquivos .pool do diretório para contexto."""
    contexto = []
    for f in os.listdir(pool_dir):
        if f.endswith(".pool") and f != os.path.basename(arquivo_principal):
            caminho = os.path.join(pool_dir, f)
            try:
                with open(caminho, "r", encoding="utf-8") as fp:
                    conteudo = fp.read()
                contexto.append(f"--- {f} ---\n{conteudo}")
            except:
                pass
    return "\n\n".join(contexto)

def executar_python(codigo_python, pool_dir):
    """Executa o código Python gerado numa pasta temporária."""
    tmp_dir = tempfile.mkdtemp(prefix="poolscript_")

    try:
        tmp_file = os.path.join(tmp_dir, "_pool_exec.py")
        with open(tmp_file, "w", encoding="utf-8") as f:
            f.write(f"import sys, os\n")
            f.write(f"sys.path.insert(0, r'{pool_dir}')\n")
            f.write(f"os.chdir(r'{pool_dir}')\n")
            f.write(codigo_python)

        resultado = subprocess.run(
            [sys.executable, tmp_file],
            capture_output=False,
            cwd=pool_dir
        )

        return resultado.returncode

    finally:
        try:
            import shutil
            shutil.rmtree(tmp_dir, ignore_errors=True)
        except:
            pass

def main():
    if len(sys.argv) < 2:
        print("Uso: pool <arquivo.pool>")
        sys.exit(1)

    arquivo_pool = sys.argv[1]

    if not os.path.exists(arquivo_pool):
        print(f"[PoolScript] Arquivo não encontrado: {arquivo_pool}")
        sys.exit(1)

    if not arquivo_pool.endswith(".pool"):
        print(f"[PoolScript] Erro: o arquivo deve ter extensão .pool")
        sys.exit(1)

    arquivo_pool = os.path.abspath(arquivo_pool)
    pool_dir = os.path.dirname(arquivo_pool)
    arquivo_nome = os.path.basename(arquivo_pool)

    with open(arquivo_pool, "r", encoding="utf-8") as f:
        codigo_pool = f.read()

    if not codigo_pool.strip():
        print("[PoolScript] Arquivo vazio.")
        sys.exit(0)

    # Cache — se nada mudou, reutiliza transpilação anterior
    cache = load_cache()
    codigo_hash = get_hash(codigo_pool)

    if codigo_hash in cache:
        print(f"[PoolScript] Cache: {arquivo_nome} não mudou. Executando versão anterior...")
        codigo_python = cache[codigo_hash]
    else:
        print(f"[PoolScript] Transpilando {arquivo_nome}...")
        contexto = coletar_contexto_pool(pool_dir, arquivo_pool)
        codigo_python = transpilar(codigo_pool, arquivo_nome, contexto)

        if not codigo_python:
            print("[PoolScript] Erro: transpilação retornou vazio.")
            sys.exit(1)

        cache[codigo_hash] = codigo_python
        save_cache(cache)

    # Pré-scanner de dependências
    pre_scanner_dependencias(codigo_python)

    # Mapeia arquivos locais mencionados
    mapear_arquivos_locais(pool_dir, codigo_pool)

    # Executa
    print(f"[PoolScript] Executando...\n")
    sys.exit(executar_python(codigo_python, pool_dir))

if __name__ == "__main__":
    main()
