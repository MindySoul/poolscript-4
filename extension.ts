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
      vscode.window.showErrorMessage("PScom.exe não encontrado. Reinstale a extensão PoolScript.");
      return;
    }

    const terminal = getOrCreateTerminal();
    terminal.show(true);
    // Correção: Remover aspas externas ao redor de pscomPath e uri.fsPath para evitar falha no PowerShell
    terminal.sendText(`${pscomPath} "${uri.fsPath}"`);
  });

  context.subscriptions.push(runCmd);
}

function getPScomPath(context: vscode.ExtensionContext): string | null {
  const extensionDir = context.extensionPath;

  // Tenta PScom.exe primeiro (compilado com pyinstaller)
  const exePath = path.join(extensionDir, "bin", "PScom.exe");
  if (fs.existsSync(exePath)) return `"${exePath}"`;

  // Fallback: PScom.py via python
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
