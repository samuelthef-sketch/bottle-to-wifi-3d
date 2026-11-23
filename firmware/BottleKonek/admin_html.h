// Admin page (http://192.168.4.1/admin). Password is ADMIN_PASSWORD in config.h.
#pragma once

static const char ADMIN_HTML[] PROGMEM = R"BKHTML(<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>BottleKonek Admin</title>
<style>
:root{--bg:#f4f8f6;--card:#fff;--ink:#10302a;--muted:#5f7771;--line:#dfe8e5;--teal:#12786a;--sun:#f2b705;--bad:#c63c2a}
*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--ink);font:400 15px/1.45 system-ui,-apple-system,"Segoe UI",Roboto,sans-serif}
.w{max-width:900px;margin:0 auto;padding:20px 16px 60px;display:grid;gap:16px}
h1{margin:0;font-size:26px}h2{margin:0 0 10px;font-size:17px}
.brand{display:flex;gap:12px;align-items:center}.brand img{width:44px;height:44px;border-radius:50%;background:#fff}
.card{background:var(--card);border-radius:16px;padding:16px;box-shadow:inset 0 0 0 1px var(--line);min-width:0}
.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(150px,1fr));gap:12px}
.lbl{font:600 11px/1 ui-monospace,Menlo,monospace;letter-spacing:.1em;text-transform:uppercase;color:var(--muted)}
.kpi{font:700 28px/1 ui-monospace,Menlo,monospace;margin-top:8px}
.bar{height:10px;border-radius:99px;background:#e4ece9;overflow:hidden;margin:10px 0 6px}.bar i{display:block;height:100%;background:var(--teal)}
label{display:grid;gap:4px;font-size:13px;color:var(--muted);font-weight:600}
input{border:0;border-radius:10px;padding:10px 12px;font:inherit;background:#fff;box-shadow:inset 0 0 0 1px var(--line);color:var(--ink);min-width:0}
input:focus{outline:none;box-shadow:inset 0 0 0 2px var(--teal)}
button{border:0;border-radius:10px;padding:10px 14px;font-weight:600;font-size:14px;line-height:1.1;font-family:inherit;background:var(--teal);color:#fff;cursor:pointer}
button.g{background:#fff;color:var(--ink);box-shadow:inset 0 0 0 1px var(--line)}button.r{background:var(--bad)}
.form{display:grid;grid-template-columns:repeat(auto-fit,minmax(160px,1fr));gap:12px;align-items:end}
.tbl{overflow-x:auto}table{border-collapse:collapse;width:100%;min-width:520px;font-size:14px}
th,td{text-align:left;padding:9px 10px;border-bottom:1px solid var(--line)}th{font:600 11px/1 ui-monospace,monospace;letter-spacing:.06em;text-transform:uppercase;color:var(--muted)}
td.n{font-family:ui-monospace,monospace;text-align:right}.acts{display:flex;gap:6px;flex-wrap:wrap}.acts button{padding:7px 10px;font-size:12.5px}
.msg{position:fixed;left:16px;right:16px;bottom:16px;max-width:420px;margin:0 auto;background:#10302a;color:#fff;border-radius:12px;padding:12px 14px;opacity:0;transition:opacity .3s}.msg.show{opacity:1}
.ok{color:#1d8a4f;font-weight:600}.bad{color:var(--bad);font-weight:600}
</style></head><body><div class="w" id="w"></div><div class="msg" id="msg"></div>
<script>
function $(i){return document.getElementById(i)}
function esc(s){return String(s).replace(/[&<>"]/g,function(c){return{'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[c]})}
function msg(m){var t=$('msg');t.textContent=m;t.classList.add('show');clearTimeout(t._h);t._h=setTimeout(function(){t.classList.remove('show')},3000)}
function api(p,d){var o=d?{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams(d).toString()}:{cache:'no-store'};
 return fetch(p,o).then(function(r){return r.json().then(function(j){if(r.status===401){login();throw new Error('Please log in')}if(!r.ok)throw new Error(j.error||'Error');return j})})}
function login(){$('w').innerHTML='<div class="brand"><img src="/logo.jpg" alt=""><h1>BottleKonek Admin</h1></div><form class="card form" onsubmit="return doLogin(event)"><label>Admin password<input type="password" id="ap" required></label><button>Log in</button></form>'}
function doLogin(e){e.preventDefault();api('/api/admin/login',{p:$('ap').value}).then(load).catch(function(x){msg(x.message)});return false}
function load(){Promise.all([api('/api/admin/stats'),api('/api/admin/users')]).then(function(r){draw(r[0],r[1])}).catch(function(){})}
function draw(s,u){var c=s.cfg,ph='';
 s.phones.forEach(function(p){ph+='<tr><td>'+esc(p.mac)+'</td><td>'+(p.u?esc(p.u):'<i>guest</i>')+'</td><td>'+(p.on?'<span class="ok">online</span>':p.c?'portal':'away')+'</td><td class="n">'+Math.floor(p.s/60)+' min</td></tr>'});
 var us='';u.forEach(function(x){us+='<tr><td><b>'+esc(x.u)+'</b></td><td class="n">'+Math.floor(x.s/60)+'</td><td class="n">'+x.b+'</td><td><div class="acts"><button class="g" onclick="addMin(\''+x.u+'\')">+ minutes</button><button class="g" onclick="resetPw(\''+x.u+'\')">Reset password</button><button class="r" onclick="delUser(\''+x.u+'\')">Delete</button></div></td></tr>'});
 $('w').innerHTML='<div class="brand"><img src="/logo.jpg" alt=""><div><h1>BottleKonek Admin</h1><div style="color:var(--muted);font-size:13px">Main WiFi: '+(s.sta?'<span class="ok">connected</span> ('+esc(s.ip)+')':'<span class="bad">not connected</span>')+' · up '+Math.floor(s.up/3600)+' h</div></div></div>'+
 (function(){var pct=Math.min(100,Math.round(s.bin*100/c.bincap)),full=s.bin>=c.bincap;return '<div class="card"><div style="display:flex;justify-content:space-between;align-items:center;gap:10px;flex-wrap:wrap"><span class="lbl">Collection bin</span><b>'+s.bin+' / '+c.bincap+' bottles'+(full?' · <span class="bad">FULL</span>':'')+'</b></div><div class="bar"><i style="width:'+pct+'%;background:'+(full?'var(--bad)':pct>=75?'var(--sun)':'var(--teal)')+'"></i></div><div style="display:flex;justify-content:space-between;align-items:center;gap:10px;flex-wrap:wrap"><span style="font-size:13px;color:var(--muted)">The machine stops accepting at '+c.bincap+' bottles.</span><button onclick="binEmptied()">I emptied the bin</button></div></div>'})()+
 '<div class="grid"><div class="card"><div class="lbl">Bottles today</div><div class="kpi">'+s.today+'</div></div><div class="card"><div class="lbl">Rejected today</div><div class="kpi">'+s.rej+'</div></div><div class="card"><div class="lbl">All bottles</div><div class="kpi">'+s.total+'</div></div><div class="card"><div class="lbl">Online now</div><div class="kpi">'+s.online+'<small style="font-size:13px">/'+s.max+'</small></div></div><div class="card"><div class="lbl">Accounts</div><div class="kpi">'+s.accounts+'</div></div></div>'+
 '<form class="card" onsubmit="return saveCfg(event)"><h2>Settings</h2><div class="form"><label>Minutes per bottle<input id="mpb" type="number" min="1" max="120" value="'+c.mpb+'"></label><label>Guest grace (seconds)<input id="grace" type="number" min="0" max="3600" value="'+c.grace+'"></label><label>Bin capacity (bottles)<input id="bincap" type="number" min="5" max="1000" value="'+c.bincap+'"></label><button>Save settings</button></div></form>'+
 '<div class="card"><h2>Sensor check (live)</h2><div id="sens" style="display:grid;gap:8px"><div style="color:var(--muted)">Reading sensors...</div></div><div class="acts" style="margin-top:10px"><button class="g" onclick="recal()">Re-measure ultrasonic (keep drop path empty)</button></div></div>'+
 '<form class="card" onsubmit="return saveGate(event)"><h2>Gate settings</h2>'+
 '<div style="display:flex;justify-content:space-between;align-items:center;gap:10px;flex-wrap:wrap;padding:10px 12px;border-radius:12px;margin-bottom:12px;background:'+(c.test?'#fff3d6':'#f0f5f3')+'"><div><b>Test mode: '+(c.test?'<span style="color:#8a5a00">ON</span>':'OFF')+'</b><div style="font-size:13px;color:var(--muted)">When ON, students see a "Test drop (3 s)" button. Test drops give no minutes.</div></div><button type="button" class="'+(c.test?'r':'')+'" onclick="setTest('+(c.test?0:1)+')">'+(c.test?'Turn test mode OFF':'Turn test mode ON')+'</button></div>'+
 '<div class="acts" style="margin-bottom:12px"><button type="button" class="g" onclick="gate(\'open\')">Open gate</button><button type="button" class="g" onclick="gate(\'close\')">Close gate</button><button type="button" onclick="gate(\'cycle\')">Test cycle (open 3 s, close)</button></div>'+
 '<div class="form"><label>Open time (ms)<input id="gopen" type="number" min="50" max="3000" value="'+c.gopen+'"></label><label>Close time (ms)<input id="gclose" type="number" min="50" max="3000" value="'+c.gclose+'"></label><label>Speed (50-500)<input id="gspd" type="number" min="50" max="500" value="'+c.gspd+'"></label><label>Open direction<select id="gdir" style="border:0;border-radius:10px;padding:10px 12px;font:inherit;background:#fff;box-shadow:inset 0 0 0 1px var(--line)"><option value="1"'+(c.gdir>0?' selected':'')+'>Normal</option><option value="-1"'+(c.gdir<0?' selected':'')+'>Reversed</option></select></label><button>Save gate settings</button></div>'+
 '<div style="font-size:13px;color:var(--muted);margin-top:10px">Bigger open time = opens wider. If the gate ends up a bit open after closing, raise the close time. Close the gate by hand before powering on.</div></form>'+
 '<div class="card"><h2>Phones on BottleKonek WiFi</h2><div class="tbl"><table><thead><tr><th>Phone</th><th>Account</th><th>Status</th><th style="text-align:right">Balance</th></tr></thead><tbody>'+(ph||'<tr><td colspan="4">No phones connected</td></tr>')+'</tbody></table></div></div>'+
 '<div class="card"><h2>Accounts</h2><div class="tbl"><table><thead><tr><th>Username</th><th style="text-align:right">Minutes</th><th style="text-align:right">Bottles</th><th>Actions</th></tr></thead><tbody>'+(us||'<tr><td colspan="4">No accounts yet</td></tr>')+'</tbody></table></div></div>'+
 '<div><button class="g" onclick="load()">Refresh</button></div>';pollSens()}
function saveCfg(e){e.preventDefault();var d={};['mpb','grace','bincap','gopen','gclose','gspd','gdir'].forEach(function(k){d[k]=$(k).value});api('/api/admin/settings',d).then(function(){msg('Settings saved');load()}).catch(function(x){msg(x.message)});return false}
function saveGate(e){e.preventDefault();var d={};['mpb','grace','bincap','gopen','gclose','gspd','gdir'].forEach(function(k){d[k]=$(k).value});api('/api/admin/settings',d).then(function(){msg('Gate settings saved');load()}).catch(function(x){msg(x.message)});return false}
function gate(a){api('/api/admin/gate',{a:a}).then(function(){msg(a==='cycle'?'Gate opens for 3 s, then closes':'Gate '+(a==='open'?'opening':'closing'))}).catch(function(x){msg(x.message)})}
function setTest(on){api('/api/admin/testmode',{on:on}).then(function(){msg('Test mode '+(on?'ON':'OFF'));load()}).catch(function(x){msg(x.message)})}
function row(ok,name,txt,hint){return '<div style="display:grid;grid-template-columns:18px 1fr;gap:10px;align-items:start;padding:8px 10px;border-radius:10px;background:'+(ok?'#e6f4ee':'#f4f7f6')+'"><span style="width:14px;height:14px;margin-top:3px;border-radius:50%;background:'+(ok?'#1d8a4f':'#c3cfcb')+'"></span><div><b>'+name+':</b> '+txt+'<div style="font-size:12.5px;color:var(--muted)">'+hint+'</div></div></div>'}
function drawSens(x){var el=$('sens');if(!el)return;
 el.innerHTML=row(x.ir,'IR sensor (D27)',x.ir?'SEES SOMETHING':'nothing in front','Green only when your finger is 2-5 cm in front. Always green? Turn its blue screw or point it at empty space.')+
 (x.metalOn?row(x.metal,'Metal sensor (D35)',x.metal?'METAL DETECTED':'no metal','Hold a coin or can near the orange face (under 0.8 cm). Needs 12 V from the MT3608.'):row(false,'Metal sensor (D35)','switched OFF in config.h','Set USE_METAL_SENSOR 1 after the MT3608 is installed.'))+
 row(x.passing,'Ultrasonic (D33/D32)',x.cm>0?(x.cm.toFixed(1)+' cm'+(x.base>0?' (empty = '+x.base.toFixed(1)+' cm)':'')):'NO ECHO','Wave your hand in front of it: the distance should change and the dot turn green. VCC must be on the 5 V rail (ESP32 V pin).')}
var sensT=null;function pollSens(){clearTimeout(sensT);if(!$('sens'))return;api('/api/admin/sensors').then(drawSens).catch(function(){}).then(function(){sensT=setTimeout(pollSens,700)})}
function recal(){api('/api/admin/recal',{x:1}).then(function(x){drawSens(x);msg(x.base>0?'Empty distance: '+x.base.toFixed(1)+' cm':'Still no echo: check power and wires')}).catch(function(e){msg(e.message)})}
function binEmptied(){api('/api/admin/bin-emptied',{x:1}).then(function(){msg('Bin count reset to 0');load()}).catch(function(x){msg(x.message)})}
function userAct(d,ok){api('/api/admin/user',d).then(function(){msg(ok);load()}).catch(function(x){msg(x.message)})}
function addMin(u){var row=event.target.closest('tr'),cell=row.lastChild;cell.innerHTML='<div class="acts"><input id="am" type="number" min="1" max="600" placeholder="min" style="width:80px"><button onclick="userAct({u:\''+u+'\',a:\'add\',m:$(\'am\').value},\'Minutes added\')">Add</button><button class="g" onclick="load()">Cancel</button></div>'}
function resetPw(u){var row=event.target.closest('tr'),cell=row.lastChild;cell.innerHTML='<div class="acts"><input id="np" type="text" placeholder="new password" style="width:130px"><button onclick="userAct({u:\''+u+'\',a:\'reset\',p:$(\'np\').value},\'Password changed\')">Save</button><button class="g" onclick="load()">Cancel</button></div>'}
function delUser(u){var row=event.target.closest('tr'),cell=row.lastChild;cell.innerHTML='<div class="acts">Delete '+esc(u)+'? <button class="r" onclick="userAct({u:\''+u+'\',a:\'delete\'},\'Account deleted\')">Yes, delete</button><button class="g" onclick="load()">Cancel</button></div>'}
load();setInterval(function(){if(!document.querySelector('input:focus'))load()},15000);
</script></body></html>)BKHTML";
