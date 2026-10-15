#!/usr/bin/env python3
"""Original, deterministic 64x32 pixel scenes for the downloadable data-only store."""
import hashlib,json,math
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
BASE='https://raw.githubusercontent.com/kmsha19/flightboard-updates/main/store/'
def rgb(r,g,b):return ((r&248)<<8)|((g&252)<<3)|(b>>3)
COLORS=[(0,0,0),(12,15,35),(29,31,73),(50,42,105),(105,67,160),(220,65,150),(255,142,119),(255,217,150),(137,234,223),(43,178,201),(35,92,158),(56,112,80),(133,187,97),(230,242,219),(226,94,45),(124,48,32)]
P=[rgb(*c) for c in COLORS]; items=[]
def publish(id,title,desc,type,**data):
 p={'id':id,'title':title,'version':1,'type':type,**data}
 raw=json.dumps(p,separators=(',',':'),ensure_ascii=False).encode();path=ROOT/'store/packs'/f'{id}-1.json';path.write_bytes(raw)
 items.append({'id':id,'title':title,'description':desc,'type':type,'version':1,'url':BASE+'packs/'+path.name,'size':len(raw),'sha256':hashlib.sha256(raw).hexdigest(),'minFirmware':'4.1.0'})
def scene(kind):
 out=[]
 for f in range(12):
  a=[[0]*64 for _ in range(32)]
  def rect(x,y,w,h,c):
   for py in range(max(0,int(y)),min(32,int(y+h))):
    for px in range(max(0,int(x)),min(64,int(x+w))):a[py][px]=c
  def dot(x,y,c):
   if 0<=x<64 and 0<=y<32:a[int(y)][int(x)]=c
  def circle(cx,cy,r,c):
   for y in range(int(cy-r),int(cy+r+1)):
    for x in range(int(cx-r),int(cx+r+1)):
     if (x-cx)**2+(y-cy)**2<=r*r:dot(x,y,c)
  if kind=='neon-city':
   rect(0,0,64,32,1);circle(49,8,5,4);circle(51,6,5,1)
   for x,y in [(3,3),(17,7),(29,4),(38,12),(58,2)]:dot(x,y,13 if f%6<3 else 10)
   for i,(x,w,h) in enumerate([(0,9,14),(11,10,20),(23,8,12),(34,12,17),(49,14,11)]):
    rect(x,32-h,w,h,2);rect(x,32-h,w,1,5 if i%2 else 9)
    for xx in range(x+2,x+w-1,3):
     for yy in range(35-h,30,4):rect(xx,yy,1,2,7 if (xx+yy+f//4)%5 else 3)
   for x in range(64):dot(x,31,9 if (x+f*3)%17<5 else 2)
  elif kind=='cosmic-flight':
   rect(0,0,64,32,1)
   for i in range(25):dot((i*19-f*(1+i%3))%64,(i*13+3)%32,13 if i%4==0 else 10)
   circle(51,22,8,3);circle(49,20,6,4);rect(39,23,24,1,9)
   y=12+round(math.sin(f*math.pi/6)*2)
   rect(21,y,14,7,13);rect(35,y+2,3,3,8);rect(25,y-3,5,3,9);rect(25,y+7,5,3,9);rect(30,y+2,3,3,10)
   rect(17-f%3,y+2,4+f%3,3,6);rect(15-f%3,y+3,5,1,7)
  elif kind=='rain-window':
   rect(0,0,64,32,1)
   for x in range(64):
    for y in range(32):
     if y>20 and (x//7)%2==0:a[y][x]=2
   for i in range(19):
    x=(i*17+5)%64;y=(i*7+f*2)%38-5
    rect(x,y,1,3,9);dot(x,y+3,8)
   rect(0,0,64,1,4);rect(0,30,64,2,4);rect(31,0,2,32,4);rect(0,0,2,32,4);rect(62,0,2,32,4)
   rect(0,16,64,1,4)
  elif kind=='campfire':
   rect(0,0,64,32,1)
   for x,y in [(5,5),(23,3),(42,7),(57,4),(11,13)]:dot(x,y,8 if f%4 else 13)
   for x in range(64):rect(x,28+(x%5==0),1,4,11)
   for y in range(12,29):
    half=int((y-11)/2)+1
    for x in range(32-half,33+half):
     if abs(x-32)<half-1+(int(math.sin(f*math.pi/6+y)*2)):dot(x,y,14)
   for y in range(18+f%3,28):
    for x in range(32-(y-17)//3,33+(y-17)//3):dot(x,y,7 if y>22 else 6)
   rect(22,29,22,2,15);rect(25,28,15,2,6)
   dot(30+f%4,11-f%3,7)
  elif kind=='pixel-cat':
   rect(0,0,64,32,1);rect(0,29,64,3,3)
   rect(22,15,23,14,6);rect(20,10,18,14,6);rect(20,7,5,5,6);rect(33,7,5,5,6)
   rect(22,9,2,3,5);rect(34,9,2,3,5)
   rect(23,16,2,1 if f in (5,6) else 3,1);rect(32,16,2,1 if f in (5,6) else 3,1);dot(28,20,5)
   rect(22,26,7,3,7);rect(35,26,7,3,7)
   rect(44,20,6,4,6);rect(47,14+f%3,4,9-f%3,6);rect(24,23,12,3,7)
   for x,y in [(9,7),(53,5),(57,22)]:dot(x,y,8);dot(x+1,y+1,8)
  elif kind=='arcade-orbit':
   rect(0,0,64,32,1)
   for i in range(16):dot((i*17-f)%64,(i*11)%32,10)
   for row in range(2):
    for col in range(5):
     x=4+col*12+round(math.sin(f*math.pi/6)*2);y=4+row*8
     rect(x+1,y,5,1,9 if row else 5);rect(x,y+1,7,3,9 if row else 5);dot(x+1,y+2,0);dot(x+5,y+2,0)
     dot(x+(0 if f%2 else 1),y+4,8);dot(x+(6 if f%2 else 5),y+4,8)
   x=27+round(math.sin(f*math.pi/6)*13);rect(x,28,11,2,13);rect(x+4,25,3,4,8);rect(x+5,24-f*2,1,3,7)
  out.extend(''.join(format(c,'x') for c in row) for row in a)
 return ''.join(out)
for id,title,desc in [
 ('neon-city','Ville néon','Une ville nocturne, ses fenêtres et ses lumières.'),
 ('cosmic-flight','Voyage cosmique','Un petit vaisseau traverse un ciel étoilé.'),
 ('rain-window','Pluie à la fenêtre','Des gouttes lumineuses pour une ambiance calme.'),
 ('campfire','Feu de camp','Un feu pixelisé sous les étoiles.'),
 ('pixel-cat','Chat pixel','Un compagnon rétro qui cligne des yeux.'),
 ('arcade-orbit','Orbite arcade','Une scène spatiale inspirée des jeux rétro.')]:
 publish(id,title,desc,'pixel',width=64,height=32,frameCount=12,frameMs=160,palette=P,pixels=scene(id))
publish('welcome','Bienvenue','Un message lumineux pour accueillir vos invités.','message',text='BIENVENUE A LA MAISON',color=rgb(137,234,223))
publish('focus-25','Focus 25 minutes','Un minuteur prêt pour une session de concentration.','timer',durationSeconds=1500)
(ROOT/'store/catalog.json').write_text(json.dumps({'schema':1,'items':items},ensure_ascii=False,indent=2)+'\n')
print(f'{len(items)} packs; '+str(sum(i['size'] for i in items))+' bytes')
