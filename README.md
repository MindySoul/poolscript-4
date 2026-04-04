# PoolScript language 4.0.0

## Release 4.0.0

Arquitetura completamente refeita. Motor de execução agora é o `PScom.py` compilado para `.exe` via PyInstaller. A extensão do VS Code funciona como bridge simples — só mostra o botão de run e chama o PScom.

## Como instalar

1. Instala o Ollama: https://ollama.com
2. Baixa o modelo: `ollama pull deepseek-r1:8b`
3. Roda o instalador: `python instalar.py`
4. Instala a extensão `.vsix` no VS Code

## Como usar

Abre qualquer arquivo `.pool`, aperta **Ctrl+Shift+Enter** ou clica no botão ▶ no topo do editor.

## O que acontece por baixo

1. A extensão chama `PScom.exe arquivo.pool`
2. PScom lê o arquivo e manda pro Ollama com o system prompt
3. Ollama devolve Python puro
4. PScom escaneia imports e instala dependências faltantes automaticamente
5. PScom executa o Python numa pasta temporária
6. Resultado aparece no terminal do VS Code

## Cache

Se você rodar o mesmo arquivo sem alterar nada, PScom reutiliza a transpilação anterior sem chamar o Ollama. Mais rápido.

## Modelo

Padrão: `deepseek-r1:8b`. Troca no `PScom.py` se quiser outro modelo.
