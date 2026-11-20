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
 '<div class="card"><div style="display:flex;justify-content:space-between"><span class="lbl">Collection bin</span><b>'+s.bin+'%</b></div><div class="bar"><i style="width:'+s.bin+'%;background:'+(s.bin>=c.binpct?'var(--bad)':s.bin>=70?'var(--sun)':'var(--teal)')+'"></i></div><div style="font-size:13px;color:var(--muted)">Stops accepting at '+c.binpct+'%.</div></div>'+
 '<div class="grid"><div class="card"><div class="lbl">Bottles today</div><div class="kpi">'+s.today+'</div></div><div class="card"><div class="lbl">Rejected today</div><div class="kpi">'+s.rej+'</div></div><div class="card"><div class="lbl">All bottles</div><div class="kpi">'+s.total+'</div></div><div class="card"><div class="lbl">Online now</div><div class="kpi">'+s.online+'<small style="font-size:13px">/'+s.max+'</small></div></div><div class="card"><div class="lbl">Accounts</div><div class="kpi">'+s.accounts+'</div></div></div>'+
 '<form class="card" onsubmit="return saveCfg(event)"><h2>Settings</h2><div class="form"><label>Minutes per bottle<input id="mpb" type="number" min="1" max="120" value="'+c.mpb+'"></label><label>Guest grace (seconds)<input id="grace" type="number" min="0" max="3600" value="'+c.grace+'"></label><label>Bin full at (%)<input id="binpct" type="number" min="50" max="100" value="'+c.binpct+'"></label><label>Bin empty distance (cm)<input id="bempty" type="number" min="10" max="300" value="'+c.bempty+'"></label><label>Bin full distance (cm)<input id="bfull" type="number" min="2" max="100" value="'+c.bfull+'"></label><button>Save settings</button></div></form>'+
 '<div class="card"><h2>Phones on BottleKonek WiFi</h2><div class="tbl"><table><thead><tr><th>Phone</th><th>Account</th><th>Status</th><th style="text-align:right">Balance</th></tr></thead><tbody>'+(ph||'<tr><td colspan="4">No phones connected</td></tr>')+'</tbody></table></div></div>'+
 '<div class="card"><h2>Accounts</h2><div class="tbl"><table><thead><tr><th>Username</th><th style="text-align:right">Minutes</th><th style="text-align:right">Bottles</th><th>Actions</th></tr></thead><tbody>'+(us||'<tr><td colspan="4">No accounts yet</td></tr>')+'</tbody></table></div></div>'+
 '<div><button class="g" onclick="load()">Refresh</button></div>'}
function saveCfg(e){e.preventDefault();var d={};['mpb','grace','binpct','bempty','bfull'].forEach(function(k){d[k]=$(k).value});api('/api/admin/settings',d).then(function(){msg('Settings saved');load()}).catch(function(x){msg(x.message)});return false}
function userAct(d,ok){api('/api/admin/user',d).then(function(){msg(ok);load()}).catch(function(x){msg(x.message)})}
function addMin(u){var row=event.target.closest('tr'),cell=row.lastChild;cell.innerHTML='<div class="acts"><input id="am" type="number" min="1" max="600" placeholder="min" style="width:80px"><button onclick="userAct({u:\''+u+'\',a:\'add\',m:$(\'am\').value},\'Minutes added\')">Add</button><button class="g" onclick="load()">Cancel</button></div>'}
function resetPw(u){var row=event.target.closest('tr'),cell=row.lastChild;cell.innerHTML='<div class="acts"><input id="np" type="text" placeholder="new password" style="width:130px"><button onclick="userAct({u:\''+u+'\',a:\'reset\',p:$(\'np\').value},\'Password changed\')">Save</button><button class="g" onclick="load()">Cancel</button></div>'}
function delUser(u){var row=event.target.closest('tr'),cell=row.lastChild;cell.innerHTML='<div class="acts">Delete '+esc(u)+'? <button class="r" onclick="userAct({u:\''+u+'\',a:\'delete\'},\'Account deleted\')">Yes, delete</button><button class="g" onclick="load()">Cancel</button></div>'}
load();setInterval(function(){if(!document.querySelector('input:focus'))load()},10000);
</script></body></html>)BKHTML";
