// The student portal page (served at http://192.168.4.1/).
// Everything is inside this one file: no internet is needed to show it.
#pragma once

static const char PORTAL_HTML[] PROGMEM = R"BKHTML(<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<title>BottleKonek</title>
<style>
:root{--bg:#f4f8f6;--card:#fff;--ink:#10302a;--muted:#5f7771;--line:#dfe8e5;--teal:#12786a;--teal2:#0c5a50;--sun:#f2b705;--water:#7cc7e8;--ok:#1d8a4f;--bad:#c63c2a}
*{box-sizing:border-box}html,body{margin:0;background:var(--bg);color:var(--ink);font:400 15px/1.45 system-ui,-apple-system,"Segoe UI",Roboto,sans-serif}
.app{max-width:440px;margin:0 auto;min-height:100vh;display:flex;flex-direction:column;padding:16px 16px 90px}
.mono{font-family:ui-monospace,Menlo,Consolas,monospace}
.brand{display:flex;align-items:center;gap:10px;margin-bottom:14px}.brand img{width:40px;height:40px;border-radius:50%;background:#fff}
.nm{font-weight:800;font-size:19px;letter-spacing:-.01em}.nm span{color:var(--teal)}.sub{font-size:12px;color:var(--muted)}
.who{margin-left:auto;width:36px;height:36px;border-radius:50%;background:var(--teal);color:#fff;font-weight:700;display:grid;place-items:center;font-size:14px}
.stack{display:grid;gap:12px}
.hero{border-radius:22px;background:linear-gradient(160deg,var(--teal),var(--teal2));color:#fff;padding:18px;display:grid;grid-template-columns:1fr auto;gap:8px;align-items:center;overflow:hidden}
.k{font:600 11px/1 ui-monospace,Menlo,monospace;letter-spacing:.1em;text-transform:uppercase;opacity:.85}
.v{font:700 46px/1 ui-monospace,Menlo,monospace;letter-spacing:-.03em;margin-top:8px}.v small{font-size:16px;font-weight:500;opacity:.85;letter-spacing:0}
.hs{font-size:13px;opacity:.9;margin-top:8px}
.btn{display:block;width:100%;border:0;border-radius:15px;padding:15px;font-weight:700;font-size:16px;line-height:1.1;font-family:inherit;text-align:center;cursor:pointer}
.btn:disabled{opacity:.5}.btn:focus-visible{outline:3px solid var(--sun);outline-offset:2px}
.p{background:var(--sun);color:#2b2100}.t{background:var(--teal);color:#fff}.o{background:transparent;color:var(--teal);box-shadow:inset 0 0 0 2px var(--teal)}.g{background:var(--card);color:var(--ink);box-shadow:inset 0 0 0 1px var(--line)}
.row{display:grid;grid-template-columns:1fr 1fr;gap:10px}
.card{background:var(--card);border-radius:18px;padding:14px;box-shadow:inset 0 0 0 1px var(--line)}
.lbl{font:600 11px/1 ui-monospace,Menlo,monospace;letter-spacing:.1em;text-transform:uppercase;color:var(--muted)}
.kpi{font:700 26px/1 ui-monospace,Menlo,monospace;margin-top:8px}
.pill{display:inline-flex;align-items:center;gap:6px;font-size:12px;font-weight:600;padding:6px 10px;border-radius:999px;background:#e6f4ee;color:#17704a}
.pill.warn{background:#fff3d6;color:#8a5a00}.pill.bad{background:#fde6e1;color:#a5321f}
.dot{width:7px;height:7px;border-radius:50%;background:currentColor}
.note{font-size:13px;color:var(--muted);text-align:center}
.warnbox{border-radius:14px;padding:12px 14px;background:#fff6df;color:#6b4a00;font-size:13.5px;box-shadow:inset 0 0 0 1px #f3dfa8}
.step{display:grid;grid-template-columns:24px 1fr;gap:10px;font-size:14px}.step i{font:700 12px/24px ui-monospace,monospace;font-style:normal;width:24px;height:24px;text-align:center;border-radius:50%;background:var(--teal);color:#fff}
.field{display:grid;gap:6px}.field span{font-size:13px;font-weight:600;color:var(--muted)}
input[type=text],input[type=password]{width:100%;border:0;border-radius:13px;padding:14px;font:inherit;font-size:16px;background:var(--card);box-shadow:inset 0 0 0 1px var(--line);color:var(--ink)}
input:focus{outline:none;box-shadow:inset 0 0 0 2px var(--teal)}
.check{display:flex;gap:10px;align-items:center;font-size:14px}.check input{width:18px;height:18px;accent-color:var(--teal)}
.tabs{display:grid;grid-template-columns:1fr 1fr;background:#e4ece9;border-radius:13px;padding:3px}
.tabs button{border:0;background:none;padding:11px;border-radius:11px;font-weight:600;font-size:14px;line-height:1.1;font-family:inherit;color:var(--muted);cursor:pointer}
.tabs button.on{background:var(--card);color:var(--ink);box-shadow:0 1px 2px rgba(0,0,0,.08)}
.big{font:700 72px/1 ui-monospace,Menlo,monospace;letter-spacing:-.04em;text-align:center}
.fi{display:grid;grid-template-columns:24px 1fr;gap:10px;align-items:center;font-size:14px;padding:10px 12px;border-radius:13px;background:var(--card);box-shadow:inset 0 0 0 1px var(--line)}
.s{width:24px;height:24px;border-radius:50%;display:grid;place-items:center;color:#fff;font-weight:700;font-size:13px}.s.ok{background:var(--ok)}.s.no{background:var(--bad)}
.hrow{display:grid;grid-template-columns:34px 1fr auto;gap:10px;align-items:center;padding:10px 0;border-bottom:1px solid var(--line);font-size:14px}.hrow:last-child{border-bottom:0}
.ic{width:34px;height:34px;border-radius:11px;background:#e3f2ee;display:grid;place-items:center;color:var(--teal)}
.ht{font-weight:600}.hd{font-size:12px;color:var(--muted)}.hm{font:700 14px/1 ui-monospace,monospace;color:var(--teal)}.hm.use{color:var(--muted)}
.day{font:600 11px/1 ui-monospace,monospace;letter-spacing:.1em;text-transform:uppercase;color:var(--muted);padding:12px 0 2px}
.nav{position:fixed;left:0;right:0;bottom:0;display:grid;grid-template-columns:repeat(3,1fr);background:var(--card);border-top:1px solid var(--line);padding:8px 0 calc(10px + env(safe-area-inset-bottom))}
.nav button{border:0;background:none;display:grid;justify-items:center;gap:3px;font-weight:600;font-size:11px;line-height:1.1;font-family:inherit;color:var(--muted);cursor:pointer}.nav button.on{color:var(--teal)}
.nav svg{width:22px;height:22px}
.toast{position:fixed;left:16px;right:16px;bottom:84px;max-width:408px;margin:0 auto;background:#10302a;color:#fff;border-radius:14px;padding:12px 14px;font-size:14px;transition:opacity .3s;opacity:0;pointer-events:none}
.toast.show{opacity:1}
.bottle{width:70px;height:130px}.water{transition:all .8s ease}
@media (prefers-reduced-motion:reduce){.water,.toast{transition:none}}
</style></head><body>
<svg width="0" height="0" style="position:absolute" aria-hidden="true"><defs><clipPath id="bc"><path d="M24 4h16v12c0 4 10 8 12 18v76c0 6-4 9-10 9H22c-6 0-10-3-10-9V34c2-10 12-14 12-18z"/></clipPath></defs></svg>
<div class="app" id="app"><p class="note" style="margin-top:40vh">Loading BottleKonek...</p></div>
<div class="toast" id="toast" role="status"></div>
<script>
var S=null,view='main',tab='login',T=null,tick=null,shownSecs=0;
function $(id){return document.getElementById(id)}
function esc(s){return String(s).replace(/[&<>"]/g,function(c){return{'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[c]})}
function toast(m){var t=$('toast');t.textContent=m;t.classList.add('show');clearTimeout(t._h);t._h=setTimeout(function(){t.classList.remove('show')},3200)}
function api(path,data){
  var o=data?{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams(data).toString()}:{cache:'no-store'};
  return fetch(path,o).then(function(r){return r.json().then(function(j){if(!r.ok)throw new Error(j.error||'Something went wrong');return j})})
}
function refresh(){api('/api/state').then(function(j){S=j;shownSecs=j.secs;render()}).catch(function(){}).then(schedule)}
function schedule(){clearTimeout(T);T=setTimeout(refresh,S&&S.machine==='mine'?1000:4000)}
function act(path,data,okMsg){return api(path,data||{}).then(function(j){if(okMsg)toast(okMsg);S=j.state||S;if(j.state)shownSecs=j.state.secs;render();refresh();return j}).catch(function(e){toast(e.message)})}
function mmss(s){s=Math.max(0,s|0);var h=Math.floor(s/3600),m=Math.floor(s%3600/60),x=s%60;return(h?h+':'+(m<10?'0':''):'')+m+':'+(x<10?'0':'')+x}
function bottle(secs){var pct=Math.min(1,secs/3600),h=Math.round(6+pct*104);return '<svg class="bottle" viewBox="0 0 64 120" aria-hidden="true"><g clip-path="url(#bc)"><rect width="64" height="120" fill="rgba(255,255,255,.12)"/><rect class="water" x="0" y="'+(120-h)+'" width="64" height="'+h+'" fill="var(--water)"/></g><path d="M24 4h16v12c0 4 10 8 12 18v76c0 6-4 9-10 9H22c-6 0-10-3-10-9V34c2-10 12-14 12-18z" fill="none" stroke="#fff" stroke-width="2.5"/><rect x="22" y="1" width="20" height="7" rx="2" fill="var(--sun)"/></svg>'}
function brand(sub){var u=S.user;return '<div class="brand"><img src="/logo.jpg" alt=""><div><div class="nm">'+(u?'Hi, '+esc(u):'Bottle<span>Konek</span>')+'</div><div class="sub">'+esc(sub)+'</div></div>'+(u?'<div class="who">'+esc(u.slice(0,2).toUpperCase())+'</div>':'')+'</div>'}
function machinePill(){if(S.binFull)return '<span class="pill bad"><span class="dot"></span>Bin full</span>';if(S.machine==='busy')return '<span class="pill warn"><span class="dot"></span>Machine in use</span>';return '<span class="pill"><span class="dot"></span>Machine ready</span>'}
function dropBtn(cls,label){var dis=S.binFull||S.machine==='busy';return '<button class="btn '+cls+'" onclick="startDrop()"'+(dis?' disabled':'')+'>'+(S.binFull?'Bin full, please tell the admin':S.machine==='busy'?'Machine in use, wait a moment':label)+'</button>'}
var ICON={home:'<path d="M3 9l7-6 7 6v8H3z"/>',hist:'<circle cx="10" cy="10" r="7"/><path d="M10 6v4l3 2"/>',out:'<path d="M8 4H4v12h4M13 6l4 4-4 4M17 10H8"/>'};
function nav(on){function b(id,lbl,ic,fn){return '<button class="'+(on===id?'on':'')+'" onclick="'+fn+'"><svg viewBox="0 0 20 20" fill="none" stroke="currentColor" stroke-width="1.8">'+ICON[ic]+'</svg>'+lbl+'</button>'}
  return '<nav class="nav">'+b('home','Home','home',"go('main')")+b('hist','History','hist',"go('history')")+b('out','Log out','out','logout()')+'</nav>'}
function go(v){view=v;render();if(v==='history')loadHistory()}

function render(){
  if(!S)return;var a=$('app'),h='';
  if(S.machine==='mine')h=vDrop();
  else if(view==='auth'&&!S.user)h=vAuth();
  else if(view==='history'&&S.user)h=vHistory();
  else if(S.user)h=vAccount();
  else if(S.secs>0)h=vGuestOnline();
  else h=vWelcome();
  a.innerHTML=h;
  clearInterval(tick);
  if(S.online){tick=setInterval(function(){shownSecs--;var el=$('cd');if(el)el.textContent=mmss(shownSecs);if(shownSecs<=0)refresh()},1000)}
}
function vWelcome(){return brand('Bongabon Senior High School')+'<div class="stack">'+
 '<div class="hero"><div><div class="k">Your internet time</div><div class="v">0<small> min</small></div><div class="hs">1 empty bottle = '+S.mpb+' minutes</div></div>'+bottle(0)+'</div>'+
 dropBtn('p','Drop bottles')+
 '<div class="row"><button class="btn o" onclick="tab=\'login\';go(\'auth\')">Log in</button><button class="btn g" onclick="tab=\'signup\';go(\'auth\')">Sign up</button></div>'+
 '<div class="card stack" style="gap:10px"><div class="lbl">How it works</div>'+
 '<div class="step"><i>1</i><span>Tap <b>Drop bottles</b>, then insert empty bottles one at a time.</span></div>'+
 '<div class="step"><i>2</i><span>Each accepted bottle adds '+S.mpb+' minutes.</span></div>'+
 '<div class="step"><i>3</i><span>Log in to keep unused minutes for next time.</span></div></div>'+
 '<div style="display:flex;justify-content:space-between;align-items:center">'+machinePill()+'<span class="note">Bin '+S.bin+'% full</span></div></div>'}
function vGuestOnline(){var on=S.online;return brand('Guest')+'<div class="stack">'+
 '<div class="hero"><div><div class="k">'+(on?'Online · guest':'Guest balance')+'</div><div class="v mono" id="cd">'+mmss(shownSecs)+'</div><div class="hs">'+(on?'left':'ready')+'</div></div>'+bottle(S.secs)+'</div>'+
 '<div class="warnbox"><b>Guest time is not saved.</b> If you leave the WiFi for more than '+(S.grace>=60?Math.round(S.grace/60)+' minute'+(S.grace>=120?'s':''):S.grace+' seconds')+', your time goes back to 0.</div>'+
 (on?'':'<button class="btn t" onclick="act(\'/api/connect\',{},\'Connected\')">Start internet</button>')+
 '<button class="btn p" onclick="tab=\'signup\';go(\'auth\')">Save my '+Math.floor(S.secs/60)+' min · create account</button>'+
 dropBtn('g','Drop more bottles')+
 '<div class="card stack" style="gap:8px"><div class="lbl">This visit</div><div style="display:flex;justify-content:space-between"><span>Bottles accepted</span><b class="mono">'+S.visit.b+'</b></div><div style="display:flex;justify-content:space-between"><span>Minutes earned</span><b class="mono">'+S.visit.m+'</b></div></div>'+
 (on?'<p class="note">You can close this window. The internet keeps working.</p>':'')+'</div>'}
function vAccount(){var on=S.online,m=Math.floor(S.secs/60);return brand(on?'Online now':'Welcome back')+'<div class="stack">'+
 '<div class="hero"><div><div class="k">'+(on?'Online':'Saved balance')+'</div>'+(on?'<div class="v mono" id="cd">'+mmss(shownSecs)+'</div><div class="hs">left · pauses if you leave</div>':'<div class="v">'+m+'<small> min</small></div><div class="hs">'+(S.secs>0?'Paused · ready when you are':'Drop bottles to earn minutes')+'</div>')+'</div>'+bottle(S.secs)+'</div>'+
 (on?'<button class="btn o" onclick="act(\'/api/pause\',{},\'Paused. Your minutes are saved.\')">Pause · save '+m+' min</button>':
   (S.secs>0?'<button class="btn t" onclick="act(\'/api/connect\',{},\'Connected. You can close this window.\')">Connect · use '+m+' min</button>':''))+
 dropBtn('p','Drop bottles')+
 '<div class="row"><div class="card"><div class="lbl">Bottles</div><div class="kpi">'+S.stats.b+'</div><div class="note" style="text-align:left">all time</div></div><div class="card"><div class="lbl">Earned</div><div class="kpi">'+S.stats.m+'<small style="font-size:12px"> min</small></div><div class="note" style="text-align:left">all time</div></div></div>'+
 (on?'<div class="card" style="font-size:14px"><div class="lbl" style="margin-bottom:6px">If you walk away</div>Your timer stops when this phone leaves the WiFi. Come back and tap <b>Connect</b> to use the rest.</div>':'')+
 '<div style="display:flex;justify-content:space-between;align-items:center">'+machinePill()+'<span class="note">This phone '+esc(S.mac)+'</span></div></div>'+nav('home')}
function vDrop(){var d=S.drop,f='';
 for(var i=0;i<d.ev.length;i++)f+='<div class="fi"><span class="s '+(d.ev[i][0]?'ok':'no')+'">'+(d.ev[i][0]?'✓':'✕')+'</span><span>'+esc(d.ev[i][1])+'</span></div>';
 return '<div class="stack"><div style="display:flex;justify-content:space-between;align-items:center"><span class="lbl">Drop session · Machine '+esc(S.code)+'</span><span class="pill"><span class="dot"></span>Live</span></div>'+
 '<div class="card" style="display:grid;gap:8px;justify-items:center;padding:20px"><div class="lbl">Bottles accepted</div><div class="big">'+d.acc+'</div><div class="mono" style="font-weight:700;color:var(--teal);font-size:18px">+'+d.min+' min</div></div>'+
 '<div class="card" style="text-align:center;background:#e9f5f1"><div style="font-weight:700;font-size:17px">Insert the next bottle</div><div class="note" style="margin-top:4px">One at a time. Empty it first.</div></div>'+
 '<div class="stack" style="gap:8px">'+f+'</div>'+
 '<button class="btn t" onclick="act(\'/api/drop/end\',{},\'Done\')">Done'+(S.user?'':' · start internet')+'</button>'+
 '<p class="note">Ends by itself after '+d.left+' s with no bottle</p></div>'}
function vAuth(){var su=tab==='signup',g=!S.user&&S.secs>0;
 return '<div class="stack"><div style="display:grid;justify-items:center;gap:8px;padding-top:8px"><img src="/logo.jpg" alt="" style="width:80px;height:80px;border-radius:50%;background:#fff"><div class="nm" style="font-size:22px">Bottle<span>Konek</span></div><div class="note">Excellence begins with character</div></div>'+
 '<div class="tabs"><button class="'+(su?'':'on')+'" onclick="tab=\'login\';render()">Log in</button><button class="'+(su?'on':'')+'" onclick="tab=\'signup\';render()">Sign up</button></div>'+
 '<form class="stack" onsubmit="return doAuth(event)">'+
 '<label class="field"><span>Username</span><input type="text" id="u" autocomplete="username" autocapitalize="none" spellcheck="false" maxlength="15" required></label>'+
 '<label class="field"><span>Password</span><input type="password" id="p" autocomplete="'+(su?'new-password':'current-password')+'" required minlength="4"></label>'+
 (su?'<label class="field"><span>Repeat password</span><input type="password" id="p2" autocomplete="new-password" required minlength="4"></label>':'')+
 '<label class="check"><input type="checkbox" id="rem" checked>Remember this phone for 30 days</label>'+
 (g?'<div class="warnbox">Your '+Math.floor(S.secs/60)+' guest minutes will move into this account.</div>':'')+
 '<button class="btn t" type="submit">'+(su?'Create account':'Log in')+'</button></form>'+
 (su?'<p class="note">Username: 3-15 letters, numbers, dot or underscore. Password: at least 4 characters.</p>':'<p class="note">Forgot your password? Ask the BottleKonek admin to reset it.</p>')+
 '<button class="btn g" onclick="go(\'main\')">Back</button></div>'}
function doAuth(e){e.preventDefault();var u=$('u').value.trim().toLowerCase(),p=$('p').value,su=tab==='signup';
 if(su&&p!==$('p2').value){toast('Passwords do not match');return false}
 act(su?'/api/signup':'/api/login',{u:u,p:p,remember:$('rem').checked?1:0},su?'Account created':'Logged in').then(function(j){if(j)view='main';render()});return false}
function startDrop(){act('/api/drop/start',{},'Insert your bottles now')}
function logout(){act('/api/logout',{},'Logged out').then(function(){view='main';render()})}
var H=null;
function loadHistory(){api('/api/history').then(function(j){H=j;render()}).catch(function(e){toast(e.message)})}
function vHistory(){var b='<div class="nm" style="font-size:24px;margin-bottom:12px">History</div>';
 if(!H)return b+'<p class="note">Loading...</p>'+nav('hist');
 var now=Date.now()/1000,wk=0,mo=0,rows='',last='';
 H.items.forEach(function(x){if(x.t==='D'&&x.e){if(now-x.e<7*86400)wk+=x.a;if(now-x.e<30*86400)mo+=x.a}
  var d=x.e?new Date(x.e*1000):null,day=d?d.toLocaleDateString(undefined,{weekday:'short',month:'short',day:'numeric'}):'Earlier',tm=d?d.toLocaleTimeString(undefined,{hour:'numeric',minute:'2-digit'}):'';
  if(day!==last){rows+='<div class="day">'+esc(day)+'</div>';last=day}
  if(x.t==='D')rows+='<div class="hrow"><span class="ic">●</span><div><div class="ht">'+x.a+' accepted'+(x.r?' · '+x.r+' rejected':'')+'</div><div class="hd">'+tm+' · Machine '+esc(S.code)+'</div></div><span class="hm">+'+x.m+'</span></div>';
  else rows+='<div class="hrow"><span class="ic">◷</span><div><div class="ht">Used '+x.m+' min online</div><div class="hd">'+tm+'</div></div><span class="hm use">−'+x.m+'</span></div>'});
 return b+'<div class="card" style="display:grid;grid-template-columns:repeat(3,1fr);text-align:center;gap:6px;margin-bottom:8px"><div><div class="lbl">Week</div><div class="kpi" style="font-size:20px">'+wk+'</div></div><div><div class="lbl">Month</div><div class="kpi" style="font-size:20px">'+mo+'</div></div><div><div class="lbl">All time</div><div class="kpi" style="font-size:20px">'+S.stats.b+'</div></div></div>'+
 (rows||'<p class="note">No bottles yet. Tap Home, then Drop bottles.</p>')+nav('hist')}
refresh();
</script></body></html>)BKHTML";
