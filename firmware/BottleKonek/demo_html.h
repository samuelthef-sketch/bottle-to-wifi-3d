// Demo panel (http://192.168.4.1/paneldemo). Same password as /admin.
// While demo mode is ON, the admin decides for each bottle: Accept or Metal.
#pragma once

static const char DEMO_HTML[] PROGMEM = R"BKHTML(<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>BottleKonek Demo Panel</title>
<style>
:root{--bg:#f4f8f6;--card:#fff;--ink:#10302a;--muted:#5f7771;--line:#dfe8e5;--teal:#12786a;--sun:#f2b705;--bad:#c63c2a;--ok:#1d8a4f}
*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--ink);font:400 16px/1.45 system-ui,-apple-system,"Segoe UI",Roboto,sans-serif}
.w{max-width:520px;margin:0 auto;padding:20px 16px 60px;display:grid;gap:14px}
.brand{display:flex;gap:12px;align-items:center}.brand img{width:44px;height:44px;border-radius:50%;background:#fff}
h1{margin:0;font-size:22px}.sub{font-size:13px;color:var(--muted)}
.card{background:var(--card);border-radius:18px;padding:16px;box-shadow:inset 0 0 0 1px var(--line);display:grid;gap:10px}
.lbl{font:600 11px/1 ui-monospace,Menlo,monospace;letter-spacing:.1em;text-transform:uppercase;color:var(--muted)}
button{border:0;border-radius:16px;padding:18px;font:700 18px/1.1 inherit;font-family:inherit;cursor:pointer;width:100%}
button:disabled{opacity:.45;cursor:default}
.acc{background:var(--ok);color:#fff}.met{background:var(--bad);color:#fff}.tog{background:var(--teal);color:#fff}.off{background:#fff;color:var(--ink);box-shadow:inset 0 0 0 1px var(--line)}
.row{display:grid;grid-template-columns:1fr 1fr;gap:10px}
.big{font:700 34px/1 ui-monospace,Menlo,monospace}
.pill{display:inline-flex;gap:6px;align-items:center;font-size:13px;font-weight:600;padding:6px 10px;border-radius:999px;background:#eef3f1;color:var(--muted)}
.pill.on{background:#e6f4ee;color:#17704a}.dot{width:8px;height:8px;border-radius:50%;background:currentColor}
input{border:0;border-radius:12px;padding:14px;font:inherit;background:#fff;box-shadow:inset 0 0 0 1px var(--line);width:100%}
.msg{position:fixed;left:16px;right:16px;bottom:16px;max-width:488px;margin:0 auto;background:#10302a;color:#fff;border-radius:12px;padding:12px 14px;opacity:0;transition:opacity .3s}.msg.show{opacity:1}
</style></head><body><div class="w" id="w"></div><div class="msg" id="msg"></div>
<script>
function $(i){return document.getElementById(i)}
function esc(s){return String(s).replace(/[&<>"]/g,function(c){return{'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[c]})}
function msg(m){var t=$('msg');t.textContent=m;t.classList.add('show');clearTimeout(t._h);t._h=setTimeout(function(){t.classList.remove('show')},2500)}
function api(p,d){var o=d?{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams(d).toString()}:{cache:'no-store'};
 return fetch(p,o).then(function(r){return r.json().then(function(j){if(r.status===401){login();throw new Error('login')}if(!r.ok)throw new Error(j.error||'Error');return j})})}
var T=null,logged=false;
function login(){logged=false;clearTimeout(T);$('w').innerHTML='<div class="brand"><img src="/logo.jpg" alt=""><div><h1>Demo Panel</h1><div class="sub">BottleKonek</div></div></div><form class="card" onsubmit="return doLogin(event)"><div class="lbl">Admin password</div><input type="password" id="ap" required><button class="tog">Log in</button></form>'}
function doLogin(e){e.preventDefault();api('/api/admin/login',{p:$('ap').value}).then(function(){logged=true;poll()}).catch(function(x){if(x.message!=='login')msg(x.message)});return false}
function poll(){clearTimeout(T);api('/api/admin/demo').then(function(d){logged=true;draw(d)}).catch(function(){}).then(function(){if(logged)T=setTimeout(poll,800)})}
function draw(d){var can=d.on&&d.active&&!d.busy;
 $('w').innerHTML='<div class="brand"><img src="/logo.jpg" alt=""><div><h1>Demo Panel</h1><div class="sub">You decide for each bottle</div></div></div>'+
 '<div class="card"><div style="display:flex;justify-content:space-between;align-items:center"><span class="lbl">Demo mode</span><span class="pill'+(d.on?' on':'')+'"><span class="dot"></span>'+(d.on?'ON':'OFF')+'</span></div>'+
 '<button class="'+(d.on?'off':'tog')+'" onclick="setDemo('+(d.on?0:1)+')">'+(d.on?'Turn demo mode OFF':'Turn demo mode ON')+'</button>'+
 '<div class="sub">When ON, the machine does not decide by itself. A student taps <b>Drop bottles</b> on their phone, puts in an item, and you press a button below.</div></div>'+
 '<div class="card"><span class="lbl">Student drop session</span>'+(d.active?
   '<div style="display:flex;justify-content:space-between;align-items:end"><div><div style="font-weight:700;font-size:18px">'+esc(d.who)+'</div><div class="sub">accepted '+d.acc+' · rejected '+d.rej+'</div></div><div class="big">+'+d.min+'<span style="font-size:15px"> min</span></div></div>'
   :'<div class="sub">Waiting for a student to tap <b>Drop bottles</b> on the website...</div>')+'</div>'+
 '<div class="row"><button class="acc" onclick="act(\'accept\')"'+(can?'':' disabled')+'>✓ Accept<br><span style="font-size:13px;font-weight:600">no metal · +'+d.mpb+' min</span></button>'+
 '<button class="met" onclick="act(\'reject\')"'+(can?'':' disabled')+'>✕ Metal<br><span style="font-size:13px;font-weight:600">reject · gate closed</span></button></div>'+
 (d.last?'<div class="card"><span class="lbl">Last action</span><div>'+esc(d.last)+'</div></div>':'')+
 '<div class="sub" style="text-align:center">Minutes per bottle is set on <a href="/admin">/admin</a> (now '+d.mpb+').</div>'}
function setDemo(on){api('/api/admin/demo/mode',{on:on}).then(function(){msg('Demo mode '+(on?'ON':'OFF'));poll()}).catch(function(x){msg(x.message)})}
function act(a){api('/api/admin/demo/'+a,{x:1}).then(function(){msg('Checking for metal or paper... (2.5 s)');poll()}).catch(function(x){msg(x.message)})}
poll();
</script></body></html>)BKHTML";
