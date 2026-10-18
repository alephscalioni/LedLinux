#!/usr/bin/env python3
"""Motor persistente de efeitos globais: OpenRGB SDK local + efeito nativo M87."""
from collections import Counter
import colorsys
import importlib.util
import json
import math
import os
from pathlib import Path
import select
import signal
import socket
import subprocess
import sys
import time

EFFECTS=('static','breathing','spectrum','wave','chase','color_shift','off')
EXPECTED=Counter({'Corsair Vengeance RGB Pro DDR4':2,'Gigabyte GeForce RTX 3060 Gaming OC':1,
                  'G502 LIGHTSPEED Wireless Gaming Mouse':1,'ASUS TUF GAMING B550M-PLUS':1,'Nollie 8CH':1})
M87_MODES={'static':1,'breathing':7,'spectrum':8,'wave':10,'chase':14,'color_shift':16,'off':0}


def validate_command(data):
    if not isinstance(data,dict):raise ValueError('Configuração inválida')
    if type(data.get('id')) is not int:raise ValueError('Identificador de operação inválido')
    if data.get('effect') not in EFFECTS:raise ValueError('Efeito global inválido')
    if not isinstance(data.get('color'),str):raise ValueError('Cor RGB inválida')
    color=data['color'].removeprefix('#')
    if len(color)!=6 or any(c not in '0123456789abcdefABCDEF' for c in color):raise ValueError('Cor RGB inválida')
    for key,maximum in [('brightness',100),('speed',100)]:
        if type(data.get(key)) is not int or not 0<=data[key]<=maximum:raise ValueError(key+' inválido')
    keyboard=data.get('keyboard')
    if not isinstance(keyboard,dict) or type(keyboard.get('enabled')) is not bool:raise ValueError('Configuração do teclado inválida')
    if keyboard['enabled']:
        if keyboard.get('connection') not in ('auto','usb','wireless'):raise ValueError('Conexão M87 inválida')
        if type(keyboard.get('independent')) is not bool:raise ValueError('Seleção M87 inválida')
        for key,maximum in [('mode',19),('speed',5),('direction',1)]:
            if type(keyboard.get(key)) is not int or not 0<=keyboard[key]<=maximum:raise ValueError('Parâmetro M87 inválido: '+key)
        if type(keyboard.get('rainbow')) is not bool:raise ValueError('Multicolorido M87 inválido')
        if not isinstance(keyboard.get('driver'),str):raise ValueError('Adaptador M87 inválido')
    return data


def frame_colors(effect,color,brightness,speed,elapsed,count):
    if effect not in EFFECTS or not 0<=brightness<=100 or not 0<=speed<=100 or count<1:
        raise ValueError('Parâmetro de efeito inválido')
    rgb=tuple(int(color.removeprefix('#')[i:i+2],16)/255 for i in (0,2,4))
    phase=elapsed/(12-speed*.1)
    level=brightness/100
    def scaled(value):return tuple(max(0,min(255,round(v*level*255))) for v in value)
    if effect=='off':return [(0,0,0)]*count
    if effect=='static':return [scaled(rgb)]*count
    if effect=='breathing':
        pulse=.08+.92*(.5-.5*math.cos(phase*2*math.pi))
        return [scaled(tuple(v*pulse for v in rgb))]*count
    hue=colorsys.rgb_to_hsv(*rgb)[0]
    if effect=='spectrum':return [scaled(colorsys.hsv_to_rgb((hue+phase)%1,1,1))]*count
    if effect=='color_shift':
        mix=.5-.5*math.cos(phase*2*math.pi);other=colorsys.hsv_to_rgb((hue+1/3)%1,1,1)
        return [scaled(tuple(a*(1-mix)+b*mix for a,b in zip(rgb,other)))]*count
    if effect=='wave':return [scaled(colorsys.hsv_to_rgb((hue+phase+i/count)%1,1,1)) for i in range(count)]
    # O rastro percorre cada zona; dispositivos de um LED ainda variam no tempo.
    return [scaled(tuple(v*max(.03,1-((i/count-phase)%1)/.28) for v in rgb)) for i in range(count)]


def keyboard_effect(command):
    k=command['keyboard'];effect=command['effect']
    if effect=='off' or command['brightness']==0:
        return dict(mode=0,speed=3,direction=0,rainbow=False)
    if k['independent']:
        return {name:k[name] for name in ('mode','speed','direction','rainbow')}
    return dict(mode=M87_MODES[effect],speed=(command['speed']*5+50)//100,
                direction=0,rainbow=effect in ('spectrum','wave','color_shift'))


class Backend:
    def __init__(self,base):
        self.base=Path(base);self.server=None;self.client=None;self.targets=[]

    def connect(self):
        from openrgb import OpenRGBClient
        port_socket=socket.socket();port_socket.bind(('127.0.0.1',0));port=port_socket.getsockname()[1];port_socket.close();self.port=port
        executable=self.base/'vendor/squashfs-root/AppRun'
        parent=os.getpid()
        def parent_death_signal():
            import ctypes
            if ctypes.CDLL(None).prctl(1,signal.SIGTERM,0,0,0)!=0 or os.getppid()!=parent:os._exit(1)
        with (self.base/'sdk-server.log').open('ab') as log:
            self.server=subprocess.Popen([str(executable),'--server','--server-host','127.0.0.1','--server-port',str(port),'--noautoconnect','--loglevel','1'],stdin=subprocess.DEVNULL,stdout=log,stderr=subprocess.STDOUT,preexec_fn=parent_death_signal)
        deadline=time.monotonic()+35;last='OpenRGB ainda iniciando'
        while time.monotonic()<deadline:
            if self.server.poll() is not None:raise RuntimeError('OpenRGB encerrou durante a inicialização; confira sdk-server.log')
            try:
                self.client=OpenRGBClient('127.0.0.1',port,'LedLinux — efeitos globais')
                self.targets=[d for d in self.client.devices if d.name in EXPECTED]
                seen=Counter(d.name for d in self.targets)
                if seen!=EXPECTED:raise RuntimeError('Controladores ausentes ou duplicados: '+str(EXPECTED-seen))
                for device in self.targets:
                    if not device.colors:raise RuntimeError('Dispositivo sem LEDs configurados: '+device.name)
                    device.set_mode('Direct')
                return
            except (OSError,RuntimeError,ValueError) as exc:
                last=str(exc)
                if self.client:
                    try:self.client.disconnect()
                    except Exception:pass
                    self.client=None
                time.sleep(.5)
        raise RuntimeError('Não foi possível iniciar os seis controladores RGB: '+last)

    def frame(self,command,elapsed):
        from openrgb.utils import RGBColor
        completed=[]
        for device in self.targets:
            colors=frame_colors(command['effect'],command['color'],command['brightness'],command['speed'],elapsed,len(device.colors))
            device.set_colors([RGBColor(*rgb) for rgb in colors],fast=True)
            completed.append(device.name)
        return completed

    def keyboard(self,command):
        k=command['keyboard']
        if not k['enabled']:return None
        driver=Path(k['driver']).resolve()
        if not driver.is_file():raise RuntimeError('Adaptador M87 não instalado')
        sys.path.insert(0,str(driver.parent))
        # Módulo local instalado; chamadas restritas ao adaptador M87 da conta.
        spec=importlib.util.spec_from_file_location('ledlinux_m87_adapter',driver)
        adapter=importlib.util.module_from_spec(spec);spec.loader.exec_module(adapter)
        result=adapter.apply(command['color'],command['brightness'],k['connection'],**keyboard_effect(command))
        (self.base/'last-m87-run.log').write_text(json.dumps(result,ensure_ascii=False)+'\n')
        return result['status']

    def close(self):
        if self.client:
            try:self.client.disconnect()
            except Exception:pass
        if self.server and self.server.poll() is None:
            self.server.terminate()
            try:self.server.wait(timeout=3)
            except subprocess.TimeoutExpired:self.server.kill();self.server.wait(timeout=2)


def emit(data):print(json.dumps(data,ensure_ascii=False),flush=True)


def main():
    base=Path(__file__).resolve().parents[1]
    backend=Backend(base);stopping=False
    def stop(*_):
        nonlocal stopping
        stopping=True
    signal.signal(signal.SIGTERM,stop);signal.signal(signal.SIGINT,stop)
    active=None;buffer=b'';epoch=time.monotonic();next_frame=0
    try:
        backend.connect();emit({'event':'ready','devices':[d.name for d in backend.targets],'port':backend.port,'led_counts':[len(d.colors) for d in backend.targets]})
        while not stopping:
            timeout=max(0,next_frame-time.monotonic()) if active and active['effect'] not in ('static','off') else .5
            ready,_,_=select.select([sys.stdin],[],[],min(timeout,.5))
            if ready:
                chunk=os.read(sys.stdin.fileno(),65536)
                if not chunk:break
                buffer+=chunk
                if len(buffer)>1024*1024:raise ValueError('Comando RGB excede limite')
                commands=[]
                while b'\n' in buffer:
                    line,buffer=buffer.split(b'\n',1)
                    if line.strip():commands.append(line)
                if commands:
                    command=None
                    try:
                        command=validate_command(json.loads(commands[-1]));epoch=time.monotonic()
                        devices=backend.frame(command,.2)
                        keyboard=backend.keyboard(command)
                        active=command;next_frame=time.monotonic()+.125
                        emit({'event':'applied','id':command['id'],'effect':command['effect'],
                              'devices':devices,'keyboard':keyboard,'physical_result_confirmed':False})
                    except Exception as exc:
                        active=None
                        emit({'event':'error','id':command.get('id') if command else None,'message':str(exc)})
            if active and active['effect'] not in ('static','off') and time.monotonic()>=next_frame:
                try:
                    backend.frame(active,time.monotonic()-epoch)
                    next_frame=time.monotonic()+.125
                except Exception as exc:
                    emit({'event':'error','id':active['id'],'message':str(exc)});active=None
    except Exception as exc:
        emit({'event':'fatal','message':str(exc)});return 1
    finally:backend.close()
    return 0

if __name__=='__main__':raise SystemExit(main())
