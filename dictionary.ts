import * as vscode from "vscode";
import * as path from "path";
import * as fs from "fs";

const DICTIONARY_FILE = "poolscript-dictionary.json";

/**
 * Gerencia o dicionário persistente de palavras/sintaxes do PoolScript.
 * Armazena em globalStorageUri para sobreviver entre sessões.
 */
export class PoolDictionary {
  private storageDir: string;
  private dictionaryPath: string;
  private words: Set<string>;

  constructor(context: vscode.ExtensionContext) {
    this.storageDir = context.globalStorageUri.fsPath;
    this.dictionaryPath = path.join(this.storageDir, DICTIONARY_FILE);
    this.words = new Set(this.load());
  }

  /** Carrega o dicionário do disco */
  private load(): string[] {
    try {
      if (!fs.existsSync(this.storageDir)) {
        fs.mkdirSync(this.storageDir, { recursive: true });
      }
      if (fs.existsSync(this.dictionaryPath)) {
        const data = fs.readFileSync(this.dictionaryPath, "utf-8");
        const parsed = JSON.parse(data);
        if (Array.isArray(parsed)) {
          return parsed;
        }
      }
    } catch (e) {
      // Ignora erros de leitura — começa com dicionário vazio
    }
    return [];
  }

  /** Salva o dicionário no disco */
  private save(): void {
    try {
      if (!fs.existsSync(this.storageDir)) {
        fs.mkdirSync(this.storageDir, { recursive: true });
      }
      fs.writeFileSync(this.dictionaryPath, JSON.stringify(Array.from(this.words), null, 2), "utf-8");
    } catch (e) {
      // Ignora erros de escrita
    }
  }

  /**
   * Extrai palavras de um conteúdo .pool e as adiciona ao dicionário.
   * Retorna true se novas palavras foram adicionadas.
   */
  public learnFromContent(content: string): boolean {
    const before = this.words.size;

    // Extrai identificadores: letras (incluindo acentuadas), dígitos e underscore
    // Mínimo 2 caracteres, começa com letra ou underscore
    const matches = content.match(/[a-zA-ZÀ-ÿ_][a-zA-ZÀ-ÿ0-9_]{1,}/g);
    if (matches) {
      matches.forEach((w) => {
        // Filtra palavras muito curtas ou puramente numéricas
        if (w.length >= 2 && !/^\d+$/.test(w)) {
          this.words.add(w);
        }
      });
    }

    const added = this.words.size > before;
    if (added) {
      this.save();
    }
    return added;
  }

  /** Retorna todas as palavras do dicionário */
  public getWords(): string[] {
    return Array.from(this.words);
  }

  /** Retorna o número de palavras no dicionário */
  public get size(): number {
    return this.words.size;
  }

  /** Limpa o dicionário */
  public clear(): void {
    this.words.clear();
    this.save();
  }

  /** Retorna o caminho do arquivo de dicionário */
  public getDictionaryPath(): string {
    return this.dictionaryPath;
  }
}

/**
 * Provedor de completions para arquivos .pool.
 * Sugere palavras do dicionário conforme o usuário digita.
 */
export class PoolCompletionProvider implements vscode.CompletionItemProvider {
  constructor(private dictionary: PoolDictionary) {}

  provideCompletionItems(
    document: vscode.TextDocument,
    position: vscode.Position
  ): vscode.CompletionItem[] {
    const wordRange = document.getWordRangeAtPosition(position, /[a-zA-ZÀ-ÿ_][a-zA-ZÀ-ÿ0-9_]*/);
    const prefix = wordRange ? document.getText(wordRange).toLowerCase() : "";

    const words = this.dictionary.getWords();

    // Aprende também do documento atual em tempo real
    const currentContent = document.getText();
    this.dictionary.learnFromContent(currentContent);

    return words
      .filter((w) => prefix === "" || w.toLowerCase().startsWith(prefix))
      .map((w) => {
        const item = new vscode.CompletionItem(w, vscode.CompletionItemKind.Text);
        item.detail = "PoolScript";
        item.documentation = new vscode.MarkdownString(
          `Palavra aprendida do seu código PoolScript.\n\n*Dicionário adaptativo — cresce conforme você escreve.*`
        );
        item.sortText = `zzz_${w}`; // Aparece após sugestões nativas
        return item;
      });
  }
}
