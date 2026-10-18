"""Binário Qt real, motor JSON simulado e HOME temporário; não acessa hardware."""
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest
BINARY=Path(__file__).resolve().parents[1]/'build/led-control'

class SyncTests(unittest.TestCase):
    def run_sync(self,fail=False,enabled=True,effect='breathing',independent=False,mode=1):
        with tempfile.TemporaryDirectory() as directory:
            home=Path(directory);data=home/'.local/share/led-control';data.mkdir(parents=True)
            driver=data/'engine.py'
            driver.write_text('''import json,pathlib,sys
print(json.dumps({'event':'ready','devices':['GPU','RAM','Nollie']}),flush=True)
for line in sys.stdin:
 command=json.loads(line)
 pathlib.Path(__file__).with_name('command.json').write_text(json.dumps(command))
 if '''+repr(fail)+''': print(json.dumps({'event':'error','id':command['id'],'message':'teclado indisponível'}),flush=True)
 else: print(json.dumps({'event':'applied','id':command['id'],'devices':['RAM','RAM','GPU','G502','ASUS','Nollie'],'keyboard':'receiver_response_matched'}),flush=True)
''')
            config=home/'.config/Aleph/led-control.conf';config.parent.mkdir(parents=True)
            config.write_text(f'[General]\ncolor=#8000ff\nbrightness=30\nrgbPython=/usr/bin/python3\nrgbEngine={driver}\nm87Enabled={str(enabled).lower()}\nm87Connection=auto\nglobalEffect={effect}\nglobalSpeed=40\nm87Independent={str(independent).lower()}\nm87Mode={mode}\nm87Speed=5\nm87Direction=1\nm87Rainbow=true\n')
            runtime=home/'runtime';runtime.mkdir(mode=0o700)
            env=os.environ|{'HOME':str(home),'XDG_CONFIG_HOME':str(home/'.config'),'XDG_DATA_HOME':str(home/'.local/share'),'XDG_RUNTIME_DIR':str(runtime),'QT_QPA_PLATFORM':'offscreen'}
            result=subprocess.run([str(BINARY),'--check-sync'],env=env,capture_output=True,timeout=15)
            command=json.loads((data/'command.json').read_text())
            return result,command,config.read_text()

    def test_global_effect_and_color_reach_engine(self):
        result,command,config=self.run_sync(effect='wave')
        self.assertEqual(result.returncode,0,result.stderr.decode())
        self.assertEqual((command['effect'],command['color'],command['brightness'],command['speed']),('wave','#8000ff',30,40))
        self.assertFalse(command['keyboard']['independent'])
        self.assertIn('globalEffect=wave',config)

    def test_failure_returns_failure(self):
        result,command,config=self.run_sync(fail=True)
        self.assertEqual(result.returncode,1,result.stderr.decode())
        self.assertNotIn('m87Driver=',config)

    def test_keyboard_can_be_disabled(self):
        result,command,config=self.run_sync(enabled=False)
        self.assertEqual(result.returncode,0,result.stderr.decode())
        self.assertFalse(command['keyboard']['enabled'])

    def test_independent_keyboard_options_preserved(self):
        result,command,config=self.run_sync(independent=True,mode=7)
        self.assertEqual(result.returncode,0,result.stderr.decode())
        self.assertEqual({k:command['keyboard'][k] for k in ['independent','mode','speed','direction','rainbow']},
                         {'independent':True,'mode':7,'speed':5,'direction':1,'rainbow':True})

if __name__=='__main__':unittest.main()
