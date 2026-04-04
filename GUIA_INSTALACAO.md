# PoolScript v4.3.0 — Guia de Instalação

## O que você vai instalar

1. MSYS2 / MinGW-w64 — compilador C++ para Windows
2. llama.cpp — biblioteca de IA
3. Modelo GGUF — o "cérebro" da PoolScript
4. CMake — organiza a compilação

---

## Passo 1 — Instalar MSYS2 (compilador C++)

1. Acessa: https://msys2.org
2. Baixa e instala o instalador
3. Abre o terminal **MSYS2 MINGW64** (não o comum, tem que ser o MINGW64)
4. Roda esses comandos um por um:

```
pacman -Syu
pacman -S --needed base-devel mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake git
```

5. Adiciona ao PATH do Windows:
   - Pesquisa "variáveis de ambiente" no Windows
   - Abre "Variáveis de Ambiente"
   - Em "Variáveis do sistema" clica em PATH → Editar → Novo
   - Adiciona: `C:\msys64\mingw64\bin`
   - Reinicia o VS Code

---

## Passo 2 — Baixar llama.cpp

Abre o terminal do VS Code na pasta do projeto e roda:

```
git clone https://github.com/ggerganov/llama.cpp
cd llama.cpp
git submodule update --init --recursive
cd ..
```

---

## Passo 3 — Baixar o modelo

1. Cria uma pasta chamada `models` dentro do projeto
2. Baixa o modelo: **qwen2.5-coder-0.5b-instruct-q4_k_m.gguf**
   - Link: https://huggingface.co/Qwen/Qwen2.5-Coder-0.5B-Instruct-GGUF
   - Arquivo: `qwen2.5-coder-0.5b-instruct-q4_k_m.gguf`
3. Coloca o arquivo dentro da pasta `models/`
4. Renomeia para: `qwen2.5-coder-0.5b.gguf`

---

## Passo 4 — Compilar o projeto

No terminal do VS Code dentro da pasta do projeto:

```
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

O arquivo `PscompilerPoolscript.exe` vai aparecer dentro da pasta `build/`.

Renomeia ele para `PScom.exe` e coloca na raiz do projeto.

---

## Passo 5 — Adicionar ao PATH

Para rodar `pool arquivo.pool` de qualquer lugar:

1. Pesquisa "variáveis de ambiente" no Windows
2. PATH → Editar → Novo
3. Adiciona o caminho da pasta onde o `PScom.exe` está
4. Reinicia o VS Code

---

## Estrutura final do projeto

```
PScomProject/
├── llama.cpp/          ← clonado do GitHub
├── models/
│   └── qwen2.5-coder-0.5b.gguf  ← baixado do HuggingFace
├── build/              ← gerado pelo cmake
├── PScom.cpp           ← código do motor
├── CMakeLists.txt      ← configuração de compilação
└── PScom.exe           ← executável final (copiar do build/)
```

---

## Como usar depois de instalado

Abre um arquivo `.pool` no VS Code e aperta o botão ▶, ou no terminal:

```
pool meuarquivo.pool
```

---

## Observações para o i5 4ª geração

- A primeira execução demora ~5-10 segundos para carregar o modelo na RAM
- Depois que carrega, traduções de 10-20 linhas ficam prontas em 3-8 segundos
- Não fecha o terminal entre execuções para o modelo ficar em cache
- Se travar, diminui outros programas abertos — o modelo usa ~1GB de RAM
