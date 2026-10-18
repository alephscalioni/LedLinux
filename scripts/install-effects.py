#!/usr/bin/python3
"""Instala o motor de efeitos e seu ambiente Python, preservando o backend existente."""
from pathlib import Path
import os
import shutil
import subprocess
import sys
project=Path(__file__).resolve().parents[1]
data=Path(os.environ.get('XDG_DATA_HOME',Path.home()/'.local/share'))/'led-control'
(data/'src').mkdir(parents=True,exist_ok=True)
shutil.copy2(project/'src/rgb_engine.py',data/'src/rgb_engine.py')
shutil.copy2(project/'requirements.txt',data/'requirements.txt')
runtime=data/'runtime'
if not (runtime/'bin/python').exists():subprocess.run([sys.executable,'-m','venv',str(runtime)],check=True)
subprocess.run([str(runtime/'bin/python'),'-m','pip','install','--disable-pip-version-check','-r',str(data/'requirements.txt')],check=True)
print('Motor RGB instalado:',data/'src/rgb_engine.py')
