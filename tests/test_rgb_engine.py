import copy
from pathlib import Path
import sys
import types
import unittest
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'src'))
from rgb_engine import frame_colors,validate_command,keyboard_effect,Backend,EFFECTS

COMMAND={'id':1,'color':'#8000ff','brightness':50,'speed':40,'effect':'breathing','keyboard':{'enabled':True,'driver':'/tmp/driver.py','connection':'auto','independent':False,'mode':1,'speed':3,'direction':0,'rainbow':False}}

class EffectTests(unittest.TestCase):
    def test_all_effects_have_correct_lengths_and_ranges_for_all_devices(self):
        for effect in EFFECTS:
            for count in [1,2,10,125,1008]:
                frame=frame_colors(effect,'#8000ff',30,40,1.7,count)
                self.assertEqual(len(frame),count)
                self.assertTrue(all(type(value) is int and 0<=value<=255 for pixel in frame for value in pixel))

    def test_zero_brightness_is_black_even_for_rainbow(self):
        for effect in EFFECTS:self.assertEqual(set(frame_colors(effect,'#ffffff',0,50,1.3,20)),{(0,0,0)})

    def test_effects_really_change_over_time_on_single_led_devices(self):
        for effect in set(EFFECTS)-{'static','off'}:
            self.assertGreater(len({tuple(frame_colors(effect,'#8000ff',100,40,i*.4,1)) for i in range(20)}),1,effect)

    def test_wave_and_chase_change_across_strip(self):
        for effect in ['wave','chase']:
            self.assertGreater(len(set(frame_colors(effect,'#ff0000',100,40,1.1,126))),20)

    def test_default_keyboard_tracks_global_and_off_overrides_custom(self):
        cmd=copy.deepcopy(COMMAND);cmd['effect']='wave'
        self.assertEqual(keyboard_effect(cmd),{'mode':10,'speed':2,'direction':0,'rainbow':True})
        cmd['keyboard']['independent']=True;cmd['keyboard']['mode']=7
        self.assertEqual(keyboard_effect(cmd)['mode'],7)
        cmd['effect']='off';self.assertEqual(keyboard_effect(cmd)['mode'],0)
        cmd['effect']='wave';cmd['brightness']=0;self.assertEqual(keyboard_effect(cmd)['mode'],0)

    def test_invalid_commands_rejected(self):
        for changes in [{'effect':'unknown'},{'brightness':101},{'color':None},{'speed':True},{'id':'bad'},{'keyboard':None}]:
            with self.assertRaises(ValueError):validate_command(COMMAND|changes)

    def test_frame_reaches_every_target_including_nollie(self):
        class Device:
            def __init__(self,name,count):self.name=name;self.colors=[None]*count;self.sent=[]
            def set_colors(self,colors,fast):self.sent.append((colors,fast))
        backend=Backend('/tmp');backend.targets=[Device('RAM',10),Device('GPU',1),Device('ASUS',125),Device('Nollie',1008)]
        colors_module=types.ModuleType('openrgb.utils');colors_module.RGBColor=lambda *rgb:rgb
        with patch.dict(sys.modules,{'openrgb.utils':colors_module}):
            self.assertEqual(backend.frame(COMMAND,1),['RAM','GPU','ASUS','Nollie'])
        for device in backend.targets:
            self.assertEqual(len(device.sent[0][0]),len(device.colors));self.assertTrue(device.sent[0][1])

if __name__=='__main__':unittest.main()
