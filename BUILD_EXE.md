# Guia de Compilação: PScom.exe 🏗️

Este guia explica como transformar o script `PScom.py` em um executável nativo para Windows (`.exe`), permitindo que a extensão PoolScript funcione de forma mais rápida e sem depender diretamente do interpretador Python no momento da execução.

## 📋 Pré-requisitos
1. **Python** instalado e no PATH.
2. **PyInstaller** instalado:
   ```bash
   pip install pyinstaller
   ```

## 🛠️ Passo a Passo para Compilação

1. Abra o terminal na raiz do projeto PoolScript.
2. Execute o comando do PyInstaller:
   ```bash
   pyinstaller --onefile --name PScom PScom.py
   ```

### O que os parâmetros fazem:
- `--onefile`: Empacota tudo em um único arquivo `.exe`.
- `--name PScom`: Define o nome do executável final.

## 📂 Onde encontrar o executável?
Após a conclusão, o arquivo `PScom.exe` estará dentro da pasta `dist/`.

## 🔧 Como integrar com a Extensão
Para que a extensão utilize o executável automaticamente:
1. Crie uma pasta chamada `bin` na raiz da extensão.
2. Mova o `PScom.exe` para dentro desta pasta `bin/`.
3. A extensão PoolScript detectará o arquivo e o usará como prioridade máxima.

---
**Nota**: Se você fizer alterações no `PScom.py`, será necessário recompilar o executável para que as mudanças entrem em vigor.
