"""
instalar.py - Adiciona PScom.exe ao PATH do usuário no Windows
Execute: python instalar.py
"""
import os
import sys
import subprocess

def instalar():
    pasta_atual = os.path.dirname(os.path.abspath(__file__))
    bin_path = os.path.join(pasta_atual, "bin")

    # Compila PScom.py para PScom.exe
    print("[PoolScript] Compilando PScom.exe...")
    result = subprocess.run(
        [sys.executable, "-m", "PyInstaller", "--onefile", "--name", "PScom",
         "--distpath", bin_path, os.path.join(pasta_atual, "PScom.py")],
        capture_output=True, text=True
    )

    if result.returncode != 0:
        print("[PoolScript] Erro ao compilar. Instale o pyinstaller: pip install pyinstaller")
        print(result.stderr)
        return

    # Adiciona ao PATH do usuário
    import winreg
    try:
        key = winreg.OpenKey(
            winreg.HKEY_CURRENT_USER,
            r"Environment",
            0, winreg.KEY_READ | winreg.KEY_WRITE
        )
        try:
            path_atual, _ = winreg.QueryValueEx(key, "PATH")
        except FileNotFoundError:
            path_atual = ""

        if bin_path not in path_atual:
            novo_path = path_atual + ";" + bin_path if path_atual else bin_path
            winreg.SetValueEx(key, "PATH", 0, winreg.REG_EXPAND_SZ, novo_path)
            print(f"[PoolScript] PATH atualizado: {bin_path}")
            print("[PoolScript] Reinicie o terminal para aplicar.")
        else:
            print("[PoolScript] PATH já configurado.")

        winreg.CloseKey(key)
    except Exception as e:
        print(f"[PoolScript] Erro ao atualizar PATH: {e}")
        print(f"[PoolScript] Adicione manualmente ao PATH: {bin_path}")

    print("[PoolScript] Instalação concluída! Use: pool arquivo.pool")

if __name__ == "__main__":
    instalar()
