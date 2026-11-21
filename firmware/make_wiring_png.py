#!/usr/bin/env python3
"""BottleKonek picture-style wiring diagram (real wires, part drawings).

Writes wiring.svg; render to PNG with any browser (see README).
Pin rows follow the ESP32 expansion board: left row D15..D23, right row D13..EN.
"""
from xml.sax.saxutils import escape

W, H = 2400, 1720
o = []

C = dict(v12='#e0281b', v5='#f08a00', gnd='#151515', v33='#9b30c8')
SIGCOL = ['#1f77d0', '#14a37f', '#d6338a', '#7a4fd1', '#c99a00', '#0aa5c2', '#e8590c', '#2f9e44',
          '#5c7cfa', '#a61e4d', '#087f5b', '#b45309', '#6741d9', '#0b7285']


def T(x, y, s, size=15, w=500, anchor='start', fill='#13211e', fam='Inter,Arial,sans-serif'):
    o.append(f'<text x="{x}" y="{y}" font-size="{size}" font-weight="{w}" text-anchor="{anchor}" fill="{fill}" font-family="{fam}">{escape(s)}</text>')


def M(x, y, s, size=13, w=700, anchor='start', fill='#13211e'):
    T(x, y, s, size, w, anchor, fill, 'JetBrains Mono,Consolas,monospace')


def R(x, y, w, h, fill, stroke='#13211e', rx=8, sw=2, extra=''):
    o.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{rx}" fill="{fill}" stroke="{stroke}" stroke-width="{sw}" {extra}/>')


def Cc(x, y, r, fill, stroke='#13211e', sw=2):
    o.append(f'<circle cx="{x}" cy="{y}" r="{r}" fill="{fill}" stroke="{stroke}" stroke-width="{sw}"/>')


def wire(pts, col, w=5):
    d = ' '.join(f'{x},{y}' for x, y in pts)
    o.append(f'<polyline points="{d}" fill="none" stroke="#ffffff" stroke-width="{w+4}" stroke-linejoin="round" stroke-linecap="round"/>')
    o.append(f'<polyline points="{d}" fill="none" stroke="{col}" stroke-width="{w}" stroke-linejoin="round" stroke-linecap="round"/>')


def dot(x, y, col):
    Cc(x, y, 7, col, '#ffffff', 2)


def pinhead(x, y):
    R(x - 7, y - 7, 14, 14, '#2b2b2b', '#000', 2, 1)
    Cc(x, y, 3.5, '#d9b44a', '#8a6d1f', 1)


def resistor(x, y, vertical, val):
    """Resistor body centred at x,y. Wire must be drawn through it first."""
    if vertical:
        R(x - 11, y - 30, 22, 60, '#e9d3a5', '#7a5a2a', 9, 2)
        for i, c in enumerate(['#8a4b14', '#111', '#e0281b']):
            o.append(f'<rect x="{x-11}" y="{y-20+i*11}" width="22" height="5" fill="{c}"/>')
        wv = 16 + 9 * len(val)
        R(x - 18 - wv, y - 13, wv, 26, '#fff7e6', '#c9a464', 6, 1.5)
        M(x - 18 - wv / 2, y + 5, val, 13, 700, 'middle', '#7a3b00')
    else:
        R(x - 30, y - 11, 60, 22, '#e9d3a5', '#7a5a2a', 9, 2)
        for i, c in enumerate(['#8a4b14', '#111', '#e0281b']):
            o.append(f'<rect x="{x-20+i*11}" y="{y-11}" width="5" height="22" fill="{c}"/>')
        wv = 16 + 9 * len(val)
        R(x - wv / 2, y + 18, wv, 26, '#fff7e6', '#c9a464', 6, 1.5)
        M(x, y + 36, val, 13, 700, 'middle', '#7a3b00')


def tag(x, y, s, col, anchor='start'):
    wv = 14 + 8.2 * len(s)
    xx = x if anchor == 'start' else x - wv
    R(xx, y - 12, wv, 24, col, 'none', 6, 0)
    M(xx + wv / 2, y + 5, s, 12.5, 700, 'middle', '#ffffff')


# ---------------------------------------------------------------- background
o.append(f'<rect width="{W}" height="{H}" fill="#f6f8f5"/>')
T(1260, 70, 'BottleKonek wiring diagram', 34, 800)
T(1260, 104, 'Follow each coloured wire from a part to the ESP32 pin with the same name.', 17, 400, fill='#4c635d')
T(1260, 128, 'Thick red / orange / black / purple lines are the power rails.', 17, 400, fill='#4c635d')

# ---------------------------------------------------------------- rails
Y12, Y5, YG, Y33 = 250, 285, 320, 352
XG_L, X5_L = 40, 76
X33_R, XG_R, X5_R, X12_R = 2246, 2284, 2322, 2360
BOT = 1660
wire([(600, Y12), (X12_R, Y12), (X12_R, BOT)], C['v12'], 9)
wire([(X5_L, BOT), (X5_L, Y5), (X5_R, Y5), (X5_R, BOT)], C['v5'], 9)
wire([(XG_L, BOT), (XG_L, YG), (XG_R, YG), (XG_R, BOT)], C['gnd'], 9)
for x, s, col in [(XG_L, 'GND', C['gnd']), (X5_L, '5V', C['v5'])]:
    tag(x - 18, BOT + 30, s, col)
for x, s, col in [(X33_R, '3V3', C['v33']), (XG_R, 'GND', C['gnd']), (X5_R, '5V', C['v5']), (X12_R, '12V', C['v12'])]:
    tag(x - 18, BOT + 30, s, col)
M(1660, Y12 - 10, '12 V rail', 13, 700, 'start', C['v12'])
M(1660, Y5 - 10, '5 V rail', 13, 700, 'start', C['v5'])
M(1660, YG - 10, 'GND rail (common ground)', 13, 700, 'start', C['gnd'])

# ---------------------------------------------------------------- power supply
R(300, 110, 340, 90, '#d4d8db', '#5b6166', 6)
for i in range(9):
    for j in range(3):
        Cc(330 + i * 34, 128 + j * 24, 7, '#aab0b5', '#8b9196', 1)
R(300, 196, 340, 34, '#c96a1a', '#6b3a0c', 4)
labels = ['L', 'N', '⏚', 'V−', 'V+']
xs = [330, 390, 450, 530, 600]
for x, l in zip(xs, labels):
    Cc(x, 213, 11, '#d9b44a', '#8a6d1f', 2)
    M(x, 190, l, 14, 800, 'middle', '#13211e') if l in ('L', 'N', '⏚') else None
T(310, 104, '12 V switching power supply', 17, 700)
wire([(330, 213), (330, 230), (250, 230)], '#7a5c2e', 5)
wire([(390, 213), (390, 238), (250, 238)], '#2b6cb0', 5)
wire([(450, 213), (450, 246), (250, 246)], '#2f9e44', 5)
T(80, 236, '220 V plug →', 15, 700, fill='#7a5c2e')
T(60, 196, 'L brown · N blue · ⏚ green', 12, 600, fill='#7a5c2e')
wire([(600, 213), (600, Y12)], C['v12'], 7); dot(600, Y12, C['v12'])
wire([(530, 213), (530, YG)], C['gnd'], 7); dot(530, YG, C['gnd'])
M(540, 244, 'V−', 13); M(610, 244, 'V+', 13)

# ---------------------------------------------------------------- buck converter
R(860, 110, 320, 110, '#1f5fae', '#0d2f5c', 8)
R(890, 128, 70, 46, '#3b0d0d', '#000', 3); M(925, 160, '5.0', 22, 800, 'middle', '#ff3b2f')
Cc(1010, 150, 18, '#cfcfcf', '#555'); R(1060, 126, 30, 40, '#2f6fd1', '#0d2f5c', 3)
Cc(1075, 136, 4, '#e8c547', '#7a5a00', 1)
T(870, 104, 'LM2596 buck (set to 5.0 V first!)', 17, 700)
for x, l in [(885, 'IN+'), (935, 'IN−'), (1105, 'OUT+'), (1155, 'OUT−')]:
    Cc(x, 205, 9, '#d9b44a', '#8a6d1f', 2)
    M(x, 196 - 6, l, 11, 700, 'middle', '#ffffff')
wire([(885, 205), (885, Y12)], C['v12'], 6); dot(885, Y12, C['v12'])
wire([(935, 205), (935, YG)], C['gnd'], 6); dot(935, YG, C['gnd'])
wire([(1105, 205), (1105, Y5)], C['v5'], 6); dot(1105, Y5, C['v5'])
wire([(1155, 205), (1155, YG)], C['gnd'], 6); dot(1155, YG, C['gnd'])

# ---------------------------------------------------------------- ESP32 expansion board
BX0, BY0, BW, BH = 960, 400, 480, 1060
R(BX0, BY0, BW, BH, '#1c1f22', '#000', 16, 3)
R(BX0 + 140, BY0 + 520, 200, 260, '#b9bfc4', '#6f767b', 6)
R(BX0 + 160, BY0 + 540, 160, 150, '#d3d8dc', '#8a9196', 4)
T(BX0 + 240, BY0 + 625, 'ESP32', 26, 800, 'middle', '#3b4246')
T(BX0 + 240, BY0 + 735, 'WROOM-32', 15, 700, 'middle', '#3b4246')
R(BX0 + 30, BY0 + 18, 70, 60, '#111', '#444', 6); T(BX0 + 65, BY0 + 96, 'DC jack', 13, 600, 'middle', '#cfd6d4')
R(BX0 + 200, BY0 + 18, 60, 34, '#bfc4c8', '#666', 6); T(BX0 + 230, BY0 + 70, 'USB-C', 12, 600, 'middle', '#cfd6d4')
T(BX0 + 240, BY0 + 1010, 'ESP32 expansion board', 18, 700, 'middle', '#e9efed')
T(BX0 + 240, BY0 + 1036, 'use the S (signal) pin of each G·V·S row', 13, 400, 'middle', '#b9c4c1')
# DC jack feed
wire([(BX0 + 65, Y12), (BX0 + 65, BY0 + 18)], C['v12'], 7); dot(BX0 + 65, Y12, C['v12'])
T(BX0 + 82, BY0 - 14, 'barrel plug 5.5×2.1 (centre +)', 13, 600, 'start', C['v12'])
# top power header
P33 = (BX0 + 380, BY0 + 60); PG = (BX0 + 430, BY0 + 60)
pinhead(*P33); pinhead(*PG)
M(P33[0], P33[1] + 30, '3V3', 12, 700, 'middle', '#e9efed'); M(PG[0], PG[1] + 30, 'GND', 12, 700, 'middle', '#e9efed')
wire([PG, (PG[0], YG)], C['gnd'], 6); dot(PG[0], YG, C['gnd'])
wire([P33, (P33[0], Y33), (X33_R, Y33), (X33_R, BOT)], C['v33'], 7)
M(1660, Y33 - 10, '3.3 V from ESP32', 13, 700, 'start', C['v33'])

LROWS = ['D15', 'D2', 'D4', 'D16', 'D17', 'D5', 'D18', 'D19', 'D21', 'RX0', 'TX0', 'D22', 'D23']
RROWS = ['D13', 'D12', 'D14', 'D27', 'D26', 'D25', 'D33', 'D32', 'D35', 'D34', 'VN', 'VP', 'EN']
ROW0, PITCH = 540, 52
LX, RXp = BX0 + 22, BX0 + BW - 22


def ry(i):
    return ROW0 + i * PITCH


used_l = {'D15', 'D4', 'D16', 'D17', 'D21', 'D22'}
used_r = {'D14', 'D27', 'D26', 'D25', 'D33', 'D32', 'D35', 'D34'}
for i, n in enumerate(LROWS):
    pinhead(LX, ry(i))
    M(LX + 20, ry(i) + 5, n, 15, 800, 'start', '#ffe066' if n in used_l else '#7d8784')
for i, n in enumerate(RROWS):
    pinhead(RXp, ry(i))
    M(RXp - 20, ry(i) + 5, n, 15, 800, 'end', '#ffe066' if n in used_r else '#7d8784')


def to_left_pin(name, pts, col):
    y = ry(LROWS.index(name))
    wire(pts + [(pts[-1][0], y), (LX, y)], col)


def to_right_pin(name, pts, col):
    y = ry(RROWS.index(name))
    wire(pts + [(pts[-1][0], y), (RXp, y)], col)


# ---------------------------------------------------------------- LEFT: push button (D15)
R(640, 512, 70, 70, '#2a2a2a', '#000', 6); Cc(675, 547, 20, '#111', '#555')
T(600, 498, 'Push button', 15, 700)
wire([(710, 527), (760, 527)], '#888', 4)
to_left_pin('D15', [(710, 540), (900, 540)], SIGCOL[0])
wire([(640, 560), (XG_L, 560)], C['gnd'], 5); dot(XG_L, 560, C['gnd'])
M(720, 512, 'leg 1 → D15', 12, 700, 'start', SIGCOL[0]); M(560, 552, 'leg 2 → GND', 12, 700, 'end')

# ---------------------------------------------------------------- LEFT: traffic light (standing, pins on right)
TLX, TLY = 560, 610
R(TLX, TLY, 80, 196, '#222', '#000', 12)
tl = [('G', 'D4', '#2f9e44'), ('Y', 'D16', '#f2c200'), ('R', 'D17', '#e03131')]
for n, pin, col in tl:
    y = ry(LROWS.index(pin))
    Cc(TLX + 40, y, 21, col, '#000')
    pinhead(TLX + 100, y); M(TLX + 116, y - 12, n, 12, 700)
    to_left_pin(pin, [(TLX + 100, y), (TLX + 100, y)], col)
gnd_y = ry(LROWS.index('D17')) + 52
pinhead(TLX + 100, gnd_y); M(TLX + 116, gnd_y - 12, 'GND', 12, 700)
wire([(TLX + 100, gnd_y), (TLX + 120, gnd_y), (TLX + 120, gnd_y + 42), (XG_L, gnd_y + 42)], C['gnd'], 5); dot(XG_L, gnd_y + 42, C['gnd'])
T(TLX - 10, TLY - 14, 'Traffic light module', 15, 700)

# ---------------------------------------------------------------- LEFT: 16x2 LCD with I2C board (back view)
LCDX, LCDY = 170, 930
R(LCDX, LCDY, 440, 180, '#2e7d32', '#14451a', 10)
for x, y in [(LCDX + 16, LCDY + 16), (LCDX + 424, LCDY + 16), (LCDX + 16, LCDY + 164), (LCDX + 424, LCDY + 164)]:
    Cc(x, y, 8, '#c9a227', '#7a5a00', 2)
Cc(LCDX + 150, LCDY + 100, 28, '#111', '#000')
# the black I2C board on the back
R(LCDX + 230, LCDY + 30, 210, 110, '#141414', '#000', 6)
R(LCDX + 290, LCDY + 60, 50, 50, '#2b2b2b', '#555', 3)
R(LCDX + 360, LCDY + 46, 36, 36, '#1f5fae', '#0d2f5c', 4); Cc(LCDX + 378, LCDY + 64, 10, '#e7e7e7', '#555')
o.append(f'<line x1="{LCDX+371}" y1="{LCDY+64}" x2="{LCDX+385}" y2="{LCDY+64}" stroke="#555" stroke-width="3"/>')
M(LCDX + 378, LCDY + 100, 'contrast', 11, 700, 'middle', '#9fc2ff')
M(LCDX + 378, LCDY + 114, 'screw', 11, 700, 'middle', '#9fc2ff')
R(LCDX + 240, LCDY + 36, 22, 12, '#f2c200', '#8a6d00', 2)
M(LCDX + 270, LCDY + 47, 'LED jumper (keep on)', 10, 600, 'start', '#f2c200')
T(LCDX, LCDY - 16, '16×2 LCD, back side (I2C board)', 16, 700)
lp = {}
for i, n in enumerate(['GND', 'VCC', 'SDA', 'SCL']):
    y = LCDY + 50 + i * 26
    lp[n] = y
    pinhead(LCDX + 452, y); M(LCDX + 432, y + 5, n, 11.5, 700, 'end', '#e8f5e9')
PX = LCDX + 452
to_left_pin('D21', [(PX, lp['SDA']), (790, lp['SDA'])], SIGCOL[1])
to_left_pin('D22', [(PX, lp['SCL']), (810, lp['SCL'])], SIGCOL[3])
M(PX + 14, lp['SDA'] - 8, 'SDA', 11, 700, 'start', SIGCOL[1])
M(PX + 14, lp['SCL'] + 18, 'SCL', 11, 700, 'start', SIGCOL[3])
wire([(PX, lp['GND']), (PX + 16, lp['GND']), (PX + 16, LCDY - 40), (XG_L, LCDY - 40)], C['gnd'], 5); dot(XG_L, LCDY - 40, C['gnd'])
wire([(PX, lp['VCC']), (PX + 34, lp['VCC']), (PX + 34, LCDY - 54), (X5_L, LCDY - 54)], C['v5'], 5); dot(X5_L, LCDY - 54, C['v5'])

# ---------------------------------------------------------------- RIGHT side parts
CX = 1760            # left edge of right-side parts
PW = 1820            # x where signal pins sit (left edge)

# buzzer (D14)
Cc(1900, 545, 36, '#1a1a1a', '#000'); Cc(1900, 545, 9, '#ddd', '#888')
T(1960, 520, 'Active buzzer', 15, 700)
pinhead(PW, 530); pinhead(PW + 230, 560)
M(PW + 18, 512, 'red +', 12, 700, 'start', '#c92a2a')
to_right_pin('D14', [(PW, 530), (1660, 530)], SIGCOL[8])
wire([(PW + 230, 560), (XG_R, 560)], C['gnd'], 5); dot(XG_R, 560, C['gnd'])
M(PW + 150, 590, 'black → GND', 12, 700)

# IR module (D27)
R(1860, 630, 220, 70, '#1f5fae', '#0d2f5c', 6)
Cc(1880, 652, 10, '#111', '#000'); Cc(1880, 680, 10, '#e9f0ff', '#777')
T(1860, 622, 'IR obstacle module', 15, 700)
pinhead(PW, 665); M(PW + 18, 712, 'OUT', 12, 700)
to_right_pin('D27', [(PW, 665), (1640, 665)], SIGCOL[9])
pinhead(2100, 650); pinhead(2100, 680)
M(2116, 646, 'VCC', 11); M(2116, 696, 'GND', 11)
wire([(2100, 650), (X33_R, 650)], C['v33'], 5); dot(X33_R, 650, C['v33'])
wire([(2100, 680), (XG_R, 680)], C['gnd'], 5); dot(XG_R, 680, C['gnd'])
wire([(PW, 665), (1860, 665)], SIGCOL[9])


def servo(y, title, pin, ch, col):
    R(1880, y, 170, 80, '#1e4fa8', '#0d2f5c', 6)
    Cc(1925, y + 40, 20, '#f1f1f1', '#666'); R(1925, y + 32, 70, 16, '#f1f1f1', '#666', 8)
    T(1880, y - 8, title, 15, 700)
    pinhead(PW, y + 40); M(PW + 16, y + 74, 'orange', 12, 700, 'start', '#e8590c')
    wire([(PW, y + 40), (1880, y + 40)], '#e8590c')
    to_right_pin(pin, [(PW, y + 40), (ch, y + 40)], col)
    pinhead(2100, y + 26); pinhead(2100, y + 56)
    M(2116, y + 22, 'red', 11); M(2116, y + 72, 'brown', 11)
    wire([(2050, y + 26), (2100, y + 26), (X5_R, y + 26)], C['v5'], 5); dot(X5_R, y + 26, C['v5'])
    wire([(2050, y + 56), (2100, y + 56), (XG_R, y + 56)], C['gnd'], 5); dot(XG_R, y + 56, C['gnd'])


servo(760, 'Servo #2 TRAY (MG90S)', 'D26', 1620, SIGCOL[10])
servo(890, 'Servo #1 GATE (MG90S)', 'D25', 1600, SIGCOL[11])

# HC-SR04 (D33 trig, D32 echo with 1k / 2.2k)
R(1860, 1030, 240, 100, '#1f5fae', '#0d2f5c', 6)
for x in (1915, 2045):
    Cc(x, 1080, 34, '#c9ced2', '#555'); Cc(x, 1080, 22, '#7d858a', '#444')
T(1860, 1022, 'HC-SR04 ultrasonic (bin level)', 15, 700)
pinhead(PW, 1060); pinhead(PW, 1100)
M(PW - 10, 1052, 'TRIG', 12, 700, 'end'); M(PW - 10, 1124, 'ECHO', 12, 700, 'end')
wire([(PW, 1060), (1860, 1060)], SIGCOL[12]); wire([(PW, 1100), (1860, 1100)], SIGCOL[13])
to_right_pin('D33', [(PW, 1060), (1580, 1060)], SIGCOL[12])
# echo: PW -> 1k -> node(1690) -> channel 1560; node -> 2.2k down -> GND
to_right_pin('D32', [(PW, 1100), (1560, 1100)], SIGCOL[13])
resistor(1750, 1100, False, '1 kΩ')
wire([(1690, 1100), (1690, 1190), (XG_R, 1190)], C['gnd'], 5); dot(1690, 1100, SIGCOL[13]); dot(XG_R, 1190, C['gnd'])
resistor(1690, 1150, True, '2.2 kΩ')
pinhead(2120, 1060); pinhead(2120, 1100)
M(2136, 1056, 'VCC', 11); M(2136, 1116, 'GND', 11)
wire([(2100, 1060), (X5_R, 1060)], C['v5'], 5); dot(X5_R, 1060, C['v5'])
wire([(2100, 1100), (XG_R, 1100)], C['gnd'], 5); dot(XG_R, 1100, C['gnd'])


def sensor(y, title, sub):
    R(1890, y, 210, 46, '#c9ced2', '#6f767b', 8)
    for i in range(10):
        o.append(f'<line x1="{1910+i*18}" y1="{y+4}" x2="{1910+i*18}" y2="{y+42}" stroke="#9aa1a6" stroke-width="2"/>')
    R(2100, y + 6, 40, 34, '#f0a030', '#8a5a10', 8)
    R(1840, y + 14, 52, 18, '#dfe3e6', '#888', 4)
    T(1890, y - 10, title, 15, 700)
    T(1890, y + 66, sub, 12.5, 400, fill='#4c635d')


# BY (PNP) -> D35 : 10k series, 3.3k to GND
sensor(1260, 'Metal sensor LJ18A3-8-Z/BY (PNP)', 'brown 12 V · blue GND · black signal')
pinhead(PW, 1283); M(PW + 6, 1250, 'black', 12, 700)
to_right_pin('D35', [(PW, 1283), (1540, 1283)], SIGCOL[2])
resistor(1760, 1283, False, '10 kΩ')
wire([(1680, 1283), (1680, 1370), (XG_R, 1370)], C['gnd'], 5); dot(1680, 1283, SIGCOL[2]); dot(XG_R, 1370, C['gnd'])
resistor(1680, 1330, True, '3.3 kΩ')
wire([(1840, 1283), (PW, 1283)], '#222')
wire([(2140, 1270), (X12_R, 1270)], C['v12'], 5); dot(X12_R, 1270, C['v12'])
wire([(2140, 1296), (XG_R, 1296)], C['gnd'], 5); dot(XG_R, 1296, C['gnd'])
M(2150, 1262, 'brown', 11, 700, 'start', '#8a4b14'); M(2150, 1316, 'blue', 11, 700, 'start', '#1f5fae')

# BX (NPN) -> D34 : 1k pull-up to 12V, 10k series, 3.3k to GND
sensor(1470, 'Metal sensor LJ18A3-8-Z/BX (NPN)', 'brown 12 V · blue GND · black signal')
pinhead(PW, 1493); M(PW + 6, 1460, 'black', 12, 700)
to_right_pin('D34', [(PW, 1493), (1520, 1493)], SIGCOL[3])
wire([(1800, 1493), (1800, 1420), (X12_R, 1420)], C['v12'], 5); dot(1800, 1493, SIGCOL[3]); dot(X12_R, 1420, C['v12'])
resistor(1800, 1455, True, '1 kΩ')
resistor(1720, 1493, False, '10 kΩ')
wire([(1650, 1493), (1650, 1590), (XG_R, 1590)], C['gnd'], 5); dot(1650, 1493, SIGCOL[3]); dot(XG_R, 1590, C['gnd'])
resistor(1650, 1545, True, '3.3 kΩ')
wire([(1840, 1493), (PW, 1493)], '#222')
wire([(2140, 1480), (X12_R, 1480)], C['v12'], 5); dot(X12_R, 1480, C['v12'])
wire([(2140, 1506), (XG_R, 1506)], C['gnd'], 5); dot(XG_R, 1506, C['gnd'])
M(2150, 1472, 'brown', 11, 700, 'start', '#8a4b14'); M(2150, 1526, 'blue', 11, 700, 'start', '#1f5fae')

# ---------------------------------------------------------------- notes box
R(1010, 1500, 560, 150, '#fff6df', '#e3c46a', 10, 1.5)
T(1030, 1532, 'Before you power on', 17, 700, fill='#6b4a00')
for i, s in enumerate(['1. Set the buck converter to 5.0 V with nothing connected.',
                       '2. All black GND wires join together (common ground).',
                       '3. Never connect 12 V or 5 V straight to a D-pin.',
                       '4. The resistors protect the ESP32: do not skip them.']):
    T(1030, 1560 + i * 22, s, 14, 400, fill='#6b4a00')

svg = f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {W} {H}" width="{W}" height="{H}">' + ''.join(x for x in o if x) + '</svg>'
open('wiring.svg', 'w').write(svg)
print('wrote wiring.svg')
