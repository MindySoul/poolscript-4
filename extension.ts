import * as vscode from "vscode";
import * as path from "path";
import * as fs from "fs";

let poolTerminal: vscode.Terminal | undefined;

export function activate(context: vscode.ExtensionContext) {

  // Comando de execução
  const runCmd = vscode.commands.registerCommand("poolscript.run", async (fileUri?: vscode.Uri) => {
    const editor = vscode.window.activeTextEditor;
    const uri = fileUri || editor?.document.uri;

    if (!uri || !uri.fsPath.endsWith(".pool")) {
      vscode.window.showErrorMessage("Nenhum arquivo .pool selecionado.");
      return;
    }

    const pscomPath = getPScomPath(context);
    if (!pscomPath) {
      vscode.window.showErrorMessage("PScom não encontrado. Verifique se o PScom.py está na raiz da extensão.");
      return;
    }

    const terminal = getOrCreateTerminal();
    terminal.show(true);
    // Correção: Enviar o comando formatado para o terminal (PowerShell/CMD)
    terminal.sendText(`${pscomPath} "${uri.fsPath}"`);
  });

  context.subscriptions.push(runCmd);
}

/**
 * Resolve o comando de execução do PScom.
 * Prioridade:
 * 1. bin/PScom.exe (Executável nativo)
 * 2. python PScom.py (Interpretador Python - Padrão Windows)
 */
function getPScomPath(context: vscode.ExtensionContext): string | null {
  const extensionDir = context.extensionPath;

  // 1. Tenta PScom.exe primeiro (compilado com pyinstaller) na pasta bin
  const exePath = path.join(extensionDir, "bin", "PScom.exe");
  if (fs.existsSync(exePath)) return `"${exePath}"`;

  // 2. Fallback: PScom.py via python (Padrão Windows: 'python')
  const pyPath = path.join(extensionDir, "PScom.py");
  if (fs.existsSync(pyPath)) return `python "${pyPath}"`;

  return null;
}

function getOrCreateTerminal(): vscode.Terminal {
  if (!poolTerminal || poolTerminal.exitStatus !== undefined) {
    poolTerminal = vscode.window.createTerminal("PoolScript");
  }
  return poolTerminal;
}

export function deactivate() {}
