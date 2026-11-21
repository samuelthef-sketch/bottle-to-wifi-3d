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
wire([(1105, Y5), (X5_R, Y5), (X5_R, BOT)], C['v5'], 9)
wire([(530, YG), (XG_R, YG), (XG_R, BOT)], C['gnd'], 9)
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
# USB phone charger powers the ESP32 (DC jack not used)
CHX, CHY = 1240, 146
R(CHX, CHY, 120, 80, '#f4f4f2', '#8a8f93', 12)
R(CHX + 118, CHY + 18, 28, 12, '#c9cdd0', '#777', 2); R(CHX + 118, CHY + 50, 28, 12, '#c9cdd0', '#777', 2)
T(CHX + 60, CHY + 36, 'Phone', 14, 700, 'middle', '#3b4246'); T(CHX + 60, CHY + 54, 'charger', 14, 700, 'middle', '#3b4246')
M(CHX + 60, CHY + 72, '5V 1-2A', 11, 700, 'middle', '#6b7378')
T(CHX + 156, CHY + 46, '← plug into 220 V outlet', 13, 700, 'start', '#7a5c2e')
o.append(f'<polyline points="{CHX+60},{CHY+80} {CHX+60},{BY0-40} {BX0+230},{BY0-40} {BX0+230},{BY0+18}" fill="none" stroke="#ffffff" stroke-width="14" stroke-linejoin="round"/>')
o.append(f'<polyline points="{CHX+60},{CHY+80} {CHX+60},{BY0-40} {BX0+230},{BY0-40} {BX0+230},{BY0+18}" fill="none" stroke="#cfd3d6" stroke-width="10" stroke-linejoin="round"/>')
R(BX0 + 214, BY0 + 4, 32, 20, '#e9ecee', '#777', 4)
T(BX0 + 214, BY0 - 14, 'USB-C cable from charger', 13, 700, 'end', '#4c635d')
M(BX0 + 65, BY0 + 114, '(not used)', 11, 600, 'middle', '#8a9a96')
T(BX0 + 456, BY0 + 116, 'GND to rail: required', 12, 700, 'end', '#9fe0c0')
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


def left_g(name):
    y = ry(LROWS.index(name)) + 22
    pinhead(LX, y); M(LX + 20, y + 5, 'G', 12, 800, 'start', '#9fe0c0')
    return y


def left_v(name):
    y = ry(LROWS.index(name)) - 22
    pinhead(LX, y); M(LX + 20, y + 5, 'V (5V)', 12, 800, 'start', '#ffb86b')
    return y


# ---------------------------------------------------------------- LEFT: push button (D15)
R(640, 512, 70, 70, '#2a2a2a', '#000', 6); Cc(675, 547, 20, '#111', '#555')
T(600, 498, 'Push button', 15, 700)
wire([(710, 527), (760, 527)], '#888', 4)
to_left_pin('D15', [(710, 540), (900, 540)], SIGCOL[0])
gb = left_g('D15')
wire([(710, gb), (LX, gb)], C['gnd'], 5)
M(720, 512, 'leg 1 → D15', 12, 700, 'start', SIGCOL[0]); M(720, gb + 22, 'leg 2 → G', 12, 700, 'start')

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
gt = left_g('D17')
wire([(TLX + 100, gnd_y), (900, gnd_y), (900, gt), (LX, gt)], C['gnd'], 5)
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
to_left_pin('D21', [(PX, lp['SDA']), (800, lp['SDA'])], SIGCOL[1])
to_left_pin('D22', [(PX, lp['SCL']), (820, lp['SCL'])], SIGCOL[3])
M(PX + 14, lp['SDA'] - 8, 'SDA', 11, 700, 'start', SIGCOL[1])
M(PX + 14, lp['SCL'] + 18, 'SCL', 11, 700, 'start', SIGCOL[3])
lg = left_g('D18')
wire([(PX, lp['GND']), (760, lp['GND']), (760, lg), (LX, lg)], C['gnd'], 5)
lv = left_v('D21')
wire([(PX, lp['VCC']), (780, lp['VCC']), (780, lv), (LX, lv)], C['v5'], 5)

# ---------------------------------------------------------------- RIGHT side parts
CX = 1760            # left edge of right-side parts
PW = 1820            # x where signal pins sit (left edge)

# buzzer (D14)
Cc(1900, 545, 36, '#1a1a1a', '#000'); Cc(1900, 545, 9, '#ddd', '#888')
T(1960, 520, 'Active buzzer', 15, 700)
pinhead(PW, 530); pinhead(PW, 552)
M(PW + 18, 512, 'red +', 12, 700, 'start', '#c92a2a')
to_right_pin('D14', [(PW, 530), (1660, 530)], SIGCOL[8])
wire([(PW, 552), (1866, 552)], C['gnd'], 5)
M(PW + 6, 576, 'black', 12, 700)

# ESP32 G pin next to a row's S pin (each row is G · V · S on the board)
def board_g(name):
    y = ry(RROWS.index(name)) + 22
    pinhead(RXp, y)
    M(RXp - 20, y + 5, 'G', 12, 800, 'end', '#9fe0c0')
    return y


def gnd_to_board(name, ys, ch):
    """GND wire from a part (left side, at height ys) to the G pin of that row, running beside the signal."""
    gy = board_g(name)
    gx = ch - 14 if ys > gy else ch + 14       # stay on the side that never crosses the signal wire
    wire([(PW, ys), (gx, ys), (gx, gy), (RXp, gy)], C['gnd'], 5)


gnd_to_board('D14', 552, 1660)   # buzzer black

# IR module (D27): OUT + GND go to the ESP32 row D27, VCC to 3V3
R(1860, 660, 220, 72, '#1f5fae', '#0d2f5c', 6)
Cc(1880, 683, 10, '#111', '#000'); Cc(1880, 711, 10, '#e9f0ff', '#777')
T(1860, 652, 'IR obstacle module', 15, 700)
pinhead(PW, 700); pinhead(PW, 722)
M(PW + 6, 690, 'OUT', 11, 700); M(PW + 6, 748, 'GND', 11, 700)
wire([(PW, 700), (1860, 700)], SIGCOL[9]); wire([(PW, 722), (1860, 722)], C['gnd'], 5)
to_right_pin('D27', [(PW, 700), (1640, 700)], SIGCOL[9])
gnd_to_board('D27', 722, 1640)
pinhead(2100, 696); M(2116, 692, 'VCC', 11)
wire([(2080, 696), (X33_R, 696)], C['v33'], 5); dot(X33_R, 696, C['v33'])


def servo(y, title, pin, ch, col):
    R(1880, y, 170, 80, '#1e4fa8', '#0d2f5c', 6)
    Cc(1925, y + 40, 20, '#f1f1f1', '#666'); R(1925, y + 32, 70, 16, '#f1f1f1', '#666', 8)
    T(1880, y - 8, title, 15, 700)
    pinhead(PW, y + 40); pinhead(PW, y + 62)
    M(PW + 6, y + 30, 'orange', 11, 700, 'start', '#e8590c'); M(PW + 6, y + 88, 'brown', 11, 700, 'start', '#6b3a0c')
    wire([(PW, y + 40), (1880, y + 40)], '#e8590c'); wire([(PW, y + 62), (1880, y + 62)], C['gnd'], 5)
    to_right_pin(pin, [(PW, y + 40), (ch, y + 40)], col)
    gnd_to_board(pin, y + 62, ch)
    pinhead(2100, y + 30); M(2116, y + 26, 'red', 11)
    wire([(2050, y + 30), (2100, y + 30), (X5_R, y + 30)], C['v5'], 5); dot(X5_R, y + 30, C['v5'])


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
to_right_pin('D33', [(PW, 1060), (1570, 1060)], SIGCOL[12])
# echo: PW -> 1k -> node(1690) -> channel 1560; node -> 2.2k down -> GND
to_right_pin('D32', [(PW, 1100), (1554, 1100)], SIGCOL[13])
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
