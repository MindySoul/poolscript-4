# PoolScript v4.1.0 🌊🚀

Linguagem baseada em intenção que transpila para **Python Puro** via **Supabase Edge Functions** e executa nativamente no seu terminal.

## 🛠️ Novidades da v4.1.0
- **Execução Remota**: A inferência da LLM agora é feita via Supabase Edge Functions, removendo a necessidade de rodar o Ollama localmente.
- **Executável C++**: O núcleo de execução (`PScom.exe`) foi reescrito em C++ para máxima performance e portabilidade.
- **Organização de Versões**: Nova pasta `versões/` contendo os pacotes `.vsix` históricos.
- **Padrão Windows**: O comando de execução padrão agora é `python` (em vez de `python3`).
- **Smart Dependency Resolver**: Resolução inteligente de imports locais e bibliotecas.
- **Agnosticismo de Domínio**: Foco total em ser um Runtime de Linguagem de propósito geral.

## 🚀 Como Funciona a Execução
A extensão segue uma ordem de prioridade para garantir a melhor performance no Windows:

1. **Executável Nativo (C++)**: Tenta encontrar o `PScom.exe` na raiz da extensão.
2. **Executável Nativo (PyInstaller)**: Tenta encontrar o `PScom.exe` na pasta `bin/` da extensão.
3. **Interpretador Python**: Se o executável não existir, utiliza o comando `python PScom.py`.

> **Nota para Windows**: O comando padrão utilizado é `python`. Certifique-se de que o Python está no seu PATH.

## 📦 Instalação
1. Baixe o arquivo `.vsix` mais recente na pasta `versões/`.
2. No VS Code, vá em Extensões > `...` > Install from VSIX.

## 🏗️ Compilação
Para gerar seu próprio executável `PScom.exe` a partir do código C++, consulte o guia [BUILD_EXE.md](./BUILD_EXE.md).

---

## Release 4.0.0 (Histórico)
Arquitetura completamente refeita. Motor de execução agora é o `PScom.py` compilado para `.exe` via PyInstaller. A extensão do VS Code funciona como bridge simples — só mostra o botão de run e chama o PScom.

1. Instala o Ollama: https://ollama.com
2. Baixa o modelo: `ollama pull deepseek-r1:8b`
3. Roda o instalador: `python instalar.py`
4. Instala a extensão `.vsix` no VS Code
