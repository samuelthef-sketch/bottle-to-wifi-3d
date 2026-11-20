#!/usr/bin/env python3
"""Draws the BottleKonek wiring diagram as an SVG (net-label schematic style).

Every wire end carries a net label. Pins with the same label are connected.
Run:  python3 make_wiring_svg.py  ->  wiring.svg
"""
from xml.sax.saxutils import escape

W, H = 1500, 1120
NET = {  # net colours
    '12V': '#d62d20', '5V': '#e8820c', '3V3': '#b0359a', 'GND': '#1d1d1d', 'AC': '#7a5c2e',
}
SIG = '#12786a'
out = []


def color(net):
    return NET.get(net, SIG)


def text(x, y, s, size=13, weight=400, anchor='start', fill='#1b2a27', family='Inter,Arial,sans-serif'):
    out.append(f'<text x="{x}" y="{y}" font-size="{size}" font-weight="{weight}" text-anchor="{anchor}" '
               f'fill="{fill}" font-family="{family}">{escape(s)}</text>')


def mono(x, y, s, size=12, weight=600, anchor='start', fill='#1b2a27'):
    text(x, y, s, size, weight, anchor, fill, 'JetBrains Mono,Consolas,monospace')


def line(x1, y1, x2, y2, c='#1b2a27', w=2):
    out.append(f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" stroke="{c}" stroke-width="{w}" stroke-linecap="round"/>')


def dot(x, y, c='#1b2a27'):
    out.append(f'<circle cx="{x}" cy="{y}" r="3.5" fill="{c}"/>')


def label(x, y, net, side='right'):
    """Net flag. side='right' means the flag extends to the right of (x,y)."""
    c = color(net)
    wbox = 10 + 7.6 * len(net)
    hx = x if side == 'right' else x - wbox
    out.append(f'<rect x="{hx}" y="{y-10}" width="{wbox}" height="20" rx="4" fill="{c}"/>')
    mono(hx + wbox / 2, y + 4.5, net, 11.5, 700, 'middle', '#ffffff')
    return wbox


def block(x, y, w, h, title, sub=None, fill='#ffffff'):
    out.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="10" fill="{fill}" stroke="#1b2a27" stroke-width="2"/>')
    text(x + 12, y + 22, title, 14.5, 700)
    if sub:
        text(x + 12, y + 39, sub, 11.5, 400, fill='#55706a')


def pin(x, y, name, net, side, stub=46):
    """A pin on a block edge at (x,y): name inside the block, stub + net label outside."""
    c = color(net)
    if side == 'right':
        mono(x - 8, y + 4, name, 11.5, 600, 'end')
        line(x, y, x + stub, y, c)
        label(x + stub, y, net, 'right')
    else:
        mono(x + 8, y + 4, name, 11.5, 600, 'start')
        line(x - stub, y, x, y, c)
        label(x - stub, y, net, 'left')


def resistor(x1, y1, x2, y2, val, c='#1b2a27'):
    """Zig-zag resistor between two points (horizontal or vertical)."""
    import math
    L = math.hypot(x2 - x1, y2 - y1)
    ux, uy = (x2 - x1) / L, (y2 - y1) / L
    px, py = -uy, ux
    body = 36
    a = (L - body) / 2
    pts = [(x1, y1), (x1 + ux * a, y1 + uy * a)]
    n = 6
    for i in range(1, n + 1):
        t = a + body * i / (n + 1)
        s = 7 if i % 2 else -7
        pts.append((x1 + ux * t + px * s, y1 + uy * t + py * s))
    pts.append((x1 + ux * (a + body), y1 + uy * (a + body)))
    pts.append((x2, y2))
    d = ' '.join(f'{px_:.1f},{py_:.1f}' for px_, py_ in pts)
    out.append(f'<polyline points="{d}" fill="none" stroke="{c}" stroke-width="2" stroke-linejoin="round"/>')
    mx, my = (x1 + x2) / 2, (y1 + y2) / 2
    if abs(ux) > abs(uy):
        mono(mx, my - 12, val, 11, 700, 'middle', '#7a3b00')
    else:
        mono(mx + 13, my + 4, val, 11, 700, 'start', '#7a3b00')


# ---------------------------------------------------------------------------
out.append(f'<rect width="{W}" height="{H}" fill="#fbfcf9"/>')
# faint grid like schematic paper
for gx in range(0, W, 20):
    out.append(f'<line x1="{gx}" y1="0" x2="{gx}" y2="{H}" stroke="#eef2ee" stroke-width="1"/>')
for gy in range(0, H, 20):
    out.append(f'<line x1="0" y1="{gy}" x2="{W}" y2="{gy}" stroke="#eef2ee" stroke-width="1"/>')

text(30, 44, 'BottleKonek wiring diagram', 24, 800)
text(30, 66, 'Pins with the same coloured label are wired together. All GND labels connect to one common ground.', 13, 400, fill='#55706a')

# ---------------- POWER (top) ----------------
block(30, 90, 250, 150, '12 V switching power supply', 'screw terminals')
pin(30, 150, 'L', 'AC', 'left', 0) if False else None
for i, (nm, net) in enumerate([('L', 'AC'), ('N', 'AC'), ('⏚', 'AC')]):
    y = 140 + i * 28
    mono(44, y + 4, nm, 12)
mono(76, 172, '← 220 V plug', 11, 500, 'start', '#7a5c2e')
pin(280, 150, 'V+', '12V', 'right', 20)
pin(280, 200, 'V−', 'GND', 'right', 20)

block(470, 90, 250, 150, 'LM2596 buck converter', 'turn screw until display reads 5.0 V')
pin(470, 150, 'IN+', '12V', 'left', 20)
pin(470, 200, 'IN−', 'GND', 'left', 20)
pin(720, 150, 'OUT+', '5V', 'right', 20)
pin(720, 200, 'OUT−', 'GND', 'right', 20)

block(910, 90, 270, 150, 'ESP32 board power', 'DC jack 6.5–16 V, plug 5.5×2.1 mm')
pin(910, 150, 'DC jack +', '12V', 'left', 20)
pin(910, 200, 'DC jack −', 'GND', 'left', 20)
mono(1168, 150, 'centre pin = +', 11, 500, 'end', '#55706a')
mono(1168, 172, 'makes 3V3 on board', 11, 500, 'end', '#55706a')

out.append('<rect x="1210" y="90" width="270" height="150" rx="10" fill="#fff6df" stroke="#e3c46a" stroke-width="1.5"/>')
text(1224, 114, 'Safety', 14, 700, fill='#6b4a00')
for i, s in enumerate(['Set buck to 5.0 V BEFORE connecting', 'servos or LCD. Unplug 220 V while', 'wiring. Never feed 12 V or 5 V into', 'a GPIO pin. Servos take 5 V from the', 'buck, not from the ESP32.']):
    text(1224, 136 + i * 19, s, 12, 400, fill='#6b4a00')

# ---------------- ESP32 (centre) ----------------
ex, ey, ew, eh = 590, 300, 320, 560
block(ex, ey, ew, eh, 'ESP32 DevKit (on expansion board)', 'pin names as printed: D25 = GPIO25', '#eef7f4')
left = [('3V3', '3V3'), ('GND', 'GND'), ('D15', 'BUTTON'), ('D4', 'LCD_D7'), ('D16', 'LCD_D6'), ('D17', 'LCD_D5'),
        ('D18', 'LCD_D4'), ('D19', 'LCD_RS'), ('D23', 'LCD_E'), ('D21', 'LED_R'), ('D22', 'LED_Y')]
right = [('GND', 'GND'), ('D13', 'LED_G'), ('D14', 'BUZZER'), ('D27', 'IR'), ('D25', 'SERVO_GATE'),
         ('D26', 'SERVO_TRAY'), ('D33', 'US_TRIG'), ('D32', 'US_ECHO'), ('D35', 'METAL_BY'), ('D34', 'METAL_BX')]
for i, (nm, net) in enumerate(left):
    pin(ex, ey + 80 + i * 44, nm, net, 'left', 40)
for i, (nm, net) in enumerate(right):
    pin(ex + ew, ey + 80 + i * 48, nm, net, 'right', 40)

# ---------------- LEFT COLUMN ----------------
# LCD 16x2
lx, ly = 30, 290
block(lx, ly, 330, 470, '16×2 LCD (1602), wired directly', 'pins on top edge, left to right')
lcd = [('1 GND', 'GND'), ('2 VDD', '5V'), ('3 VO', 'POT_MID'), ('4 RS', 'LCD_RS'), ('5 RW', 'GND'), ('6 E', 'LCD_E'),
       ('7–10 D0–D3', None), ('11 D4', 'LCD_D4'), ('12 D5', 'LCD_D5'), ('13 D6', 'LCD_D6'), ('14 D7', 'LCD_D7'),
       ('15 A (LA)', '5V'), ('16 K (LK)', 'GND')]
for i, (nm, net) in enumerate(lcd):
    y = ly + 66 + i * 30
    if net:
        pin(lx + 330, y, nm, net, 'right', 30)
    else:
        mono(lx + 322, y + 4, nm, 11.5, 600, 'end')
        mono(lx + 340, y + 4, 'not connected', 11, 500, 'start', '#8a9a96')

# Potentiometer
block(30, 780, 330, 120, '10 kΩ potentiometer (contrast)', 'no pot? 1 kΩ from VO to GND works too')
pin(360, 830, 'left leg', '5V', 'right', 30)
pin(360, 856, 'middle', 'POT_MID', 'right', 30)
pin(360, 882, 'right leg', 'GND', 'right', 30)

# Traffic light + button
block(30, 920, 200, 170, 'Traffic light module', None)
pin(230, 966, 'R', 'LED_R', 'right', 26)
pin(230, 996, 'Y', 'LED_Y', 'right', 26)
pin(230, 1026, 'G', 'LED_G', 'right', 26)
pin(230, 1060, 'GND', 'GND', 'right', 26)

block(390, 920, 200, 120, 'Push button', 'use two diagonal legs')
pin(590, 970, 'leg 1', 'BUTTON', 'right', 26)
pin(590, 1004, 'leg 2', 'GND', 'right', 26)

block(390, 1050, 200, 60, 'Active buzzer', None)
pin(590, 1072, 'red +', 'BUZZER', 'right', 26)
pin(590, 1098, 'black', 'GND', 'right', 26)

# ---------------- RIGHT COLUMN ----------------
rx = 1110
# Servos
block(rx, 290, 360, 116, '2 × MG90S servo', '#1 gate · #2 tray')
mono(rx + 14, 364, 'orange #1', 11.5); label(rx + 96, 360, 'SERVO_GATE')
mono(rx + 14, 390, 'orange #2', 11.5); label(rx + 96, 386, 'SERVO_TRAY')
mono(rx + 230, 364, 'red', 11.5); label(rx + 290, 360, '5V')
mono(rx + 230, 390, 'brown', 11.5); label(rx + 290, 386, 'GND')

# IR
block(rx, 418, 360, 98, 'IR obstacle module', 'aim across the tray; screw sets range')
mono(rx + 14, 494, 'OUT', 11.5); label(rx + 56, 490, 'IR')
mono(rx + 130, 494, 'VCC', 11.5); label(rx + 168, 490, '3V3')
mono(rx + 240, 494, 'GND', 11.5); label(rx + 278, 490, 'GND')

# HC-SR04 with divider
block(rx, 530, 360, 156, 'HC-SR04 ultrasonic (bin level)', 'ECHO is 5 V → divide down to 3.3 V')
mono(rx + 14, 634, 'VCC', 11.5); label(rx + 56, 630, '5V')
mono(rx + 14, 664, 'GND', 11.5); label(rx + 56, 660, 'GND')
mono(rx + 14, 604, 'TRIG', 11.5); label(rx + 56, 600, 'US_TRIG')
mono(rx + 134, 612, 'ECHO', 11.5)
line(rx + 172, 608, rx + 180, 608)
resistor(rx + 180, 608, rx + 262, 608, '1 kΩ')
dot(rx + 262, 608)
line(rx + 262, 608, rx + 290, 608, SIG); label(rx + 290, 608, 'US_ECHO')
resistor(rx + 262, 608, rx + 262, 670, '2.2 kΩ')
label(rx + 262 + 50, 670, 'GND')
line(rx + 262, 670, rx + 312, 670, NET['GND'])

# Metal sensor BX (NPN)
block(rx, 694, 360, 196, 'Metal sensor LJ18A3-8-Z/BX (NPN)', 'output goes LOW when metal is near')
mono(rx + 14, 748, 'brown', 11.5); label(rx + 66, 744, '12V')
mono(rx + 14, 774, 'blue', 11.5);  label(rx + 66, 770, 'GND')
mono(rx + 14, 812, 'black', 11.5)
line(rx + 60, 808, rx + 150, 808)
dot(rx + 150, 808)
resistor(rx + 150, 808, rx + 150, 752, '1 kΩ')
line(rx + 150, 752, rx + 180, 752, NET['12V']); label(rx + 180, 752, '12V')
resistor(rx + 150, 808, rx + 250, 808, '10 kΩ')
dot(rx + 250, 808)
line(rx + 250, 808, rx + 280, 808, SIG); label(rx + 280, 808, 'METAL_BX')
resistor(rx + 250, 808, rx + 250, 866, '3.3 kΩ')
line(rx + 250, 866, rx + 290, 866, NET['GND']); label(rx + 290, 866, 'GND')

# Metal sensor BY (PNP)
block(rx, 904, 360, 170, 'Metal sensor LJ18A3-8-Z/BY (PNP)', 'output gives 12 V when metal is near')
mono(rx + 14, 958, 'brown', 11.5); label(rx + 66, 954, '12V')
mono(rx + 14, 982, 'blue', 11.5);  label(rx + 66, 978, 'GND')
mono(rx + 14, 1016, 'black', 11.5)
line(rx + 60, 1012, rx + 150, 1012)
resistor(rx + 150, 1012, rx + 250, 1012, '10 kΩ')
dot(rx + 250, 1012)
line(rx + 250, 1012, rx + 280, 1012, SIG); label(rx + 280, 1012, 'METAL_BY')
resistor(rx + 250, 1012, rx + 250, 1060, '3.3 kΩ')
line(rx + 250, 1060, rx + 290, 1060, NET['GND']); label(rx + 290, 1060, 'GND')

# legend
lgx, lgy = 700, 900
out.append(f'<rect x="{lgx}" y="{lgy}" width="370" height="190" rx="10" fill="#ffffff" stroke="#c9d6d2"/>')
text(lgx + 14, lgy + 26, 'Wire colours', 14, 700)
items = [('12V', '12 V from power supply (22 AWG)'), ('5V', '5 V from buck converter'), ('3V3', '3.3 V from ESP32 3V3 pin'),
         ('GND', 'common ground, all joined'), ('IR', 'signal to an ESP32 pin')]
for i, (n, d) in enumerate(items):
    label(lgx + 14, lgy + 52 + i * 27, n)
    text(lgx + 70, lgy + 56 + i * 27, d, 12.5, 400)

svg = f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {W} {H}" width="{W}" height="{H}" role="img" ' \
      f'aria-label="BottleKonek wiring diagram">' + ''.join(o for o in out if o) + '</svg>'
open('wiring.svg', 'w').write(svg)
print('wrote wiring.svg', len(svg))
