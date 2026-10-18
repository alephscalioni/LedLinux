"""Teste de integração: binário Qt real, backends e HOME temporários, sem hardware."""
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

BINARY = Path(__file__).resolve().parents[1]/'build/led-control'
NAMES = ['Corsair Vengeance RGB Pro DDR4']*2 + ['Gigabyte GeForce RTX 3060 Gaming OC', 'G502 LIGHTSPEED Wireless Gaming Mouse', 'ASUS TUF GAMING B550M-PLUS', 'Nollie 8CH']

class SyncTests(unittest.TestCase):
    def run_sync(self, enabled=True, fail=False):
        with tempfile.TemporaryDirectory() as directory:
            home=Path(directory)
            data=home/'.local/share/led-control'
            backend=data/'vendor/squashfs-root/AppRun'
            backend.parent.mkdir(parents=True)
            backend.write_text('''#!/usr/bin/python3
import json, os, pathlib, sys
home=pathlib.Path(os.environ['HOME'])
args=sys.argv
color=args[args.index('--color')+1]
r,g,b=bytes.fromhex(color)
controllers=[{'name':n,'modes':[{'color_mode':1}],'active_mode':0,'colors':[r|(g<<8)|(b<<16)]} for n in '''+repr(NAMES)+''']
path=home/'.config/OpenRGB/profiles/Aleph-LastApplied.json'
path.parent.mkdir(parents=True,exist_ok=True)
path.write_text(json.dumps({'controllers':controllers}))
''')
            backend.chmod(0o755)
            driver=data/'fake-m87.py'
            driver.write_text('''import json, pathlib, sys
pathlib.Path(__file__).with_name('args.json').write_text(json.dumps(sys.argv[1:]))
'''+("print('teclado indisponível'); sys.exit(1)" if fail else "print(json.dumps({'device_response_matched':True}))"))
            config=home/'.config/Aleph/led-control.conf'
            config.parent.mkdir(parents=True)
            config.write_text(f'[General]\ncolor=#0000ff\nbrightness=30\nm87Enabled={str(enabled).lower()}\nm87Driver={driver}\nm87Connection=auto\n')
            runtime=home/'runtime';runtime.mkdir(mode=0o700)
            env=os.environ | {'HOME':str(home),'XDG_CONFIG_HOME':str(home/'.config'),'XDG_DATA_HOME':str(home/'.local/share'),'XDG_RUNTIME_DIR':str(runtime),'QT_QPA_PLATFORM':'offscreen'}
            completed=subprocess.run([str(BINARY),'--check-sync'],env=env,capture_output=True,timeout=15)
            args=json.loads((data/'args.json').read_text()) if (data/'args.json').exists() else None
            log=(data/'last-m87-run.log').read_text() if (data/'last-m87-run.log').exists() else ''
            return completed, args, log

    def test_same_color_and_brightness_sent(self):
        result,args,log=self.run_sync()
        self.assertEqual(result.returncode,0,result.stderr.decode())
        self.assertEqual(args,['--color','#0000ff','--brightness','30','--connection','auto','--apply'])
        self.assertIn('device_response_matched',log)

    def test_keyboard_failure_is_not_reported_as_success(self):
        result,args,log=self.run_sync(fail=True)
        self.assertEqual(result.returncode,1,result.stderr.decode())
        self.assertIn('indisponível',log)

    def test_disabled_integration_preserves_previous_behavior(self):
        result,args,log=self.run_sync(enabled=False)
        self.assertEqual(result.returncode,0,result.stderr.decode())
        self.assertIsNone(args)

if __name__=='__main__': unittest.main()
