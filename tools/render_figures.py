"""Optional figure regeneration: Python 3.10+, Pillow, Chinese-capable font.

Windows uses Microsoft YaHei. On Linux set TUTORIAL_FONT to a Noto CJK font.
Outputs PNG for viewing and SVG for editing; no screenshots or measured data.
"""
from pathlib import Path
import html
import math
import os

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
FONT = os.environ.get('TUTORIAL_FONT', 'C:/Windows/Fonts/msyh.ttc')
if not Path(FONT).exists():
    raise SystemExit('Set TUTORIAL_FONT to an installed Chinese-capable font.')
W, H = 1440, 900
INK, BLUE, TEAL, PALE = '#18334c', '#2572a5', '#098882', '#f0f6fa'


class Figure:
    def __init__(self, title, subtitle):
        self.im = Image.new('RGB', (W, H), 'white')
        self.draw = ImageDraw.Draw(self.im)
        self.svg = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" viewBox="0 0 {W} {H}">',
                    '<rect width="100%" height="100%" fill="white"/>']
        self.text(65, 42, title, 39)
        self.text(65, 106, subtitle, 22, '#526d80')

    def text(self, x, y, value, size=25, color=INK):
        self.draw.text((x, y), value, font=ImageFont.truetype(FONT, size), fill=color)
        self.svg.append(f'<text x="{x}" y="{y + size}" font-family="Microsoft YaHei,Noto Sans CJK SC,sans-serif" font-size="{size}" fill="{color}">{html.escape(value)}</text>')

    def box(self, x, y, w, h, lines, fill=PALE, border=BLUE):
        self.draw.rounded_rectangle((x, y, x+w, y+h), radius=16, fill=fill, outline=border, width=2)
        self.svg.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="16" fill="{fill}" stroke="{border}" stroke-width="2"/>')
        for i, line in enumerate(lines):
            self.text(x+20, y+18+i*39, line, 25 if i == 0 else 22)

    def arrow(self, x1, y1, x2, y2, color=BLUE):
        self.draw.line((x1,y1,x2,y2), fill=color, width=4)
        angle = math.atan2(y2-y1,x2-x1)
        pts=[(x2,y2), (x2-14*math.cos(angle-0.45),y2-14*math.sin(angle-0.45)),
             (x2-14*math.cos(angle+0.45),y2-14*math.sin(angle+0.45))]
        self.draw.polygon(pts,fill=color)
        self.svg.append(f'<path d="M{x1},{y1} L{x2},{y2}" fill="none" stroke="{color}" stroke-width="4"/>')
        self.svg.append('<polygon points="'+' '.join(f'{x},{y}' for x,y in pts)+f'" fill="{color}"/>')

    def save(self, name, lang):
        out=ROOT/'docs/assets'/lang
        out.mkdir(parents=True,exist_ok=True)
        self.im.save(out/f'{name}.png',optimize=True)
        (out/f'{name}.svg').write_text('\n'.join(self.svg+['</svg>']),encoding='utf-8')


for lang in ['en', 'cn']:
    cn=lang=='cn'
    f=Figure('从采集到云端显示' if cn else 'From sensors to cloud values',
             '基于源码绘制的架构示意，不是实测截图' if cn else 'Source-derived architecture diagram; not a measured result or screenshot')
    f.box(65,210,320,150,['AHT20','温度 / 湿度' if cn else 'Temperature / humidity',
                         '地址 0x38' if cn else 'Address 0x38'])
    f.box(65,390,320,120,['BH1750','光照 · 0x23' if cn else 'Illuminance · 0x23'])
    f.box(500,250,360,170,['ESP32-S3','I²C: SDA=8 / SCL=9','采集 → 格式化 JSON' if cn else 'Read → format JSON'])
    f.arrow(385,270,500,300);f.arrow(385,450,500,370)
    f.box(500,570,360,120,['SSD1306 · 128 × 64','本地数值显示 · 0x3C' if cn else 'Local values · 0x3C'])
    f.arrow(680,420,680,570)
    f.box(980,250,385,170,['Wi-Fi → MQTT','mqtt.nlecloud.com:1883','普通鉴权 / 明文传输' if cn else 'Ordinary auth / plain TCP'])
    f.arrow(860,335,980,335)
    f.box(980,570,385,120,['NLECloud','传感器数值与历史记录' if cn else 'Sensor values and history'])
    f.arrow(1170,420,1170,570)
    f.text(65,790,'上传条件：本轮两类传感器读取成功，且 MQTT 已连接。' if cn else 'Upload gate: both sensor reads succeed and MQTT is connected.',25)
    f.save('architecture',lang)

    f=Figure('共享 I²C 总线接线' if cn else 'Wire one shared I²C bus',
             '接线示意：断电操作；确认模块丝印与 3.3 V 兼容性' if cn else 'Wiring schematic: power off first; verify module labels and 3.3 V compatibility')
    f.box(65,210,330,405,['ESP32-S3','3V3','GND','GPIO8 → SDA','GPIO9 → SCL'])
    for x,title,addr in [(560,'AHT20','0x38'),(845,'BH1750','0x23'),(1130,'SSD1306','0x3C')]:
        f.box(x,210,250,220,[title,'VCC / GND','SDA / SCL',addr])
        f.arrow(x+125,500,x+125,430,TEAL)
    f.arrow(395,500,1360,500,TEAL)
    f.text(470,535,'并联连接同名引脚，共用地线' if cn else 'Parallel same-name pins; common ground',27)
    f.text(470,590,'总线速率：100 kHz · 地址均为 7 位地址' if cn else 'Bus speed: 100 kHz · 7-bit addresses',24)
    f.text(65,720,'SDA / SCL 上拉到 3.3 V；先检查模块是否已有上拉电阻。' if cn else 'Pull SDA / SCL up to 3.3 V; check onboard pull-ups first.',25)
    f.text(65,775,'BH1750: ADDR 低电平 → 0x23；不要把器件地址左移一位。' if cn else 'BH1750: ADDR low → 0x23; do not left-shift device addresses.',24)
    f.save('wiring',lang)

    f=Figure('云端字段与固件配置一一对应' if cn else 'Map cloud fields to firmware',
             '配置示意：全部使用自己的设备信息，不包含真实账号' if cn else 'Configuration schematic: use your own device; no real account data shown')
    rows=[('云端设备 ID' if cn else 'Cloud numeric device ID','APP_MQTT_DEVICE_ID → username'),
          ('设备标识 / 自定义设备序列号' if cn else 'Device tag / chosen client identifier','APP_MQTT_CLIENT_ID → client_id'),
          ('设备传输密钥 SecretKey' if cn else 'Device transport SecretKey','APP_MQTT_SECRET_KEY → password')]
    for i,(left,right) in enumerate(rows):
        y=195+i*130
        f.box(65,y,470,90,[left]);f.box(665,y,710,90,[right]);f.arrow(535,y+45,665,y+45)
    f.box(65,615,1310,100,['Topic: /sys/<device-id>/<client-id>/sensor/datas',
                            '注意：device-id 是设备 ID，不是项目 ID。' if cn else 'Use the device ID, not the project ID.'])
    f.text(65,780,'传感器标识必须匹配大小写：Temperature / Humidity / Light' if cn else 'Sensor API tags are case-sensitive: Temperature / Humidity / Light',25)
    f.save('cloud-mapping',lang)

    f=Figure('main.c：app_main() 主循环' if cn else 'main.c: app_main() loop',
             '控制流程示意；初始化异步启动 Wi-Fi 和 MQTT' if cn else 'Control-flow diagram; Wi-Fi and MQTT initialization starts asynchronous connections')
    texts=[['启动 Wi-Fi → 建立 I²C 总线 → 启动 MQTT'] if cn else ['Start Wi-Fi → create I²C bus → start MQTT'],
           ['初始化 OLED → BH1750 → AHT20'] if cn else ['Initialize OLED → BH1750 → AHT20'],
           ['读 AHT20 与 BH1750，成功时更新 OLED'] if cn else ['Read both sensors; update OLED for successful reads'],
           ['两者成功且 MQTT 已连接 → 构造 JSON 并 publish'] if cn else ['Both reads OK and MQTT connected → JSON → publish'],
           ['延时 1000 ms → 下一轮（采样耗时另计）'] if cn else ['Wait 1000 ms → repeat (measurement time is additional)']]
    for i,lines in enumerate(texts):
        y=180+i*123;f.box(140,y,1160,86,lines)
        if i<4:f.arrow(720,y+86,720,y+123)
    f.text(140,825,'任一读取失败：本轮不上报；断网期间无离线补传机制。' if cn else 'Failed read: skip this upload. No offline backfill is implemented.',24)
    f.save('main.c__app_main',lang)

print('Generated 8 PNG figures and 8 editable SVG sources.')
