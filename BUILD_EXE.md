# Guia de Compilação: PScom.exe (C++) 🏗️

Este guia explica como compilar o núcleo de execução do PoolScript (`PScom.cpp`) em um executável nativo para Windows (`.exe`), permitindo que a extensão funcione de forma mais rápida e sem depender diretamente do interpretador Python no momento da execução.

## 📋 Pré-requisitos
1. **Compilador C++**: [MinGW-w64](https://www.mingw-w64.org/) com suporte a C++17.
2. **CMake**: Versão 3.18 ou superior.
3. **libcurl**: Biblioteca para requisições HTTP.

## 🛠️ Passo a Passo para Compilação (Windows/MinGW)

1. Abra o terminal na raiz do projeto PoolScript.
2. Crie um diretório de build e compile:
   ```bash
   mkdir build
   cd build
   cmake .. -G "MinGW Makefiles"
   make
   ```

### O que os parâmetros fazem:
- `mkdir build`: Cria uma pasta separada para os arquivos de compilação.
- `cmake .. -G "MinGW Makefiles"`: Configura o projeto usando o CMake e o MinGW.
- `make`: Compila o código e gera o executável.

## 📂 Onde encontrar o executável?
Após a conclusão, o arquivo `PscompilerPoolscript.exe` estará dentro da pasta `build/`.

## 🔧 Como integrar com a Extensão
Para que a extensão utilize o executável automaticamente:
1. Renomeie o arquivo gerado para `PScom.exe`.
2. Mova o `PScom.exe` para a raiz da extensão.
3. A extensão PoolScript detectará o arquivo e o usará como prioridade máxima.

---
**Nota**: Se você fizer alterações no `PScom.cpp`, será necessário recompilar o executável para que as mudanças entrem em vigor.
