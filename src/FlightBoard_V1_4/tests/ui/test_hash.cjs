// Exercise the vendored browser implementation, never its native Node shortcut.
const fs=require('node:fs'),path=require('node:path'),vm=require('node:vm');
const assert=require('node:assert/strict'),crypto=require('node:crypto'),zlib=require('node:zlib');
const os=require('node:os'),childProcess=require('node:child_process');
const root=path.resolve(__dirname,'../..');
const vendor=path.join(root,'assets/vendor/js-sha256-0.11.1');
const metadata=JSON.parse(fs.readFileSync(path.join(vendor,'manifest.json'),'utf8'));
let assertions=0;
const equal=(a,b,message)=>{++assertions;assert.equal(a,b,message);};
const ok=(value,message)=>{++assertions;assert.ok(value,message);};
const nativeHash=bytes=>crypto.createHash('sha256').update(bytes).digest('hex');

equal(metadata.version,'0.11.1');
equal(metadata.commit,'7912494a5ae277660131557401b91f11f47c96d2');
for(const file of metadata.files){
  const bytes=fs.readFileSync(path.join(vendor,file.file));
  equal(bytes.length,file.bytes,file.file+' byte length');
  equal(nativeHash(bytes),file.sha256,file.file+' source checksum');
  ok(file.url.startsWith('https://raw.githubusercontent.com/emn178/js-sha256/'+metadata.commit+'/'));
}
const license=fs.readFileSync(path.join(vendor,'LICENSE.txt'));
const source=fs.readFileSync(path.join(vendor,'sha256.js'),'utf8');
const minified=fs.readFileSync(path.join(vendor,'sha256.min.js'),'utf8');
const expected=Buffer.concat([Buffer.from('/*\n'),Buffer.from(license.toString().trimEnd()),Buffer.from('\n*/\n'),Buffer.from(minified)]);
const generated=fs.readFileSync(path.join(root,'FlightBoardHashGzip.h'),'utf8');
ok(generated.includes('const uint8_t FLIGHTBOARD_HASH_GZIP[] PROGMEM'));
const compressed=Buffer.from([...generated.matchAll(/0x([A-F0-9]{2})/g)].map(match=>parseInt(match[1],16)));
const served=zlib.gunzipSync(compressed);
equal(served.toString(),expected.toString(),'The shipped gzip must equal the verified browser code plus its full license');
equal(compressed.readUInt32LE(4),0,'Gzip must not contain a build timestamp');
equal(compressed[9],255,'Gzip must not depend on the build OS');
ok(served.includes(license.toString().trimEnd()),'The firmware-distributed asset must include the complete MIT notice');

function browser(script){
  const context=vm.createContext({isSecureContext:false,crypto:{},
    fetch(){throw new Error('Browser SHA-256 must not use the network');},
    require(){throw new Error('Browser SHA-256 must not use Node crypto');},
  });
  vm.runInContext('window=globalThis;self=globalThis;',context);
  vm.runInContext(script,context,{timeout:1000});
  equal(vm.runInContext('typeof sha256',context),'function');
  equal(vm.runInContext('typeof crypto.subtle',context),'undefined');
  equal(vm.runInContext('typeof process',context),'undefined');
  equal(vm.runInContext('typeof module',context),'undefined');
  return context;
}
function hashBytes(context,bytes,kind='buffer'){
  // Copy into the VM's own ArrayBuffer realm, as a real browser fetch would.
  context.inputBytes=bytes;
  const expression=kind==='slice'
    ? 'var padded=new Uint8Array(inputBytes.length+17);padded.set(inputBytes,9);sha256(padded.subarray(9,9+inputBytes.length))'
    : 'var bytes=new Uint8Array(inputBytes);sha256(bytes.buffer)';
  return vm.runInContext(expression,context,{timeout:10000});
}
function bytesOfLength(length){
  const bytes=Buffer.alloc(length);
  let seed=0x12345678;
  for(let i=0;i<length;++i){seed=(Math.imul(seed,1664525)+1013904223)>>>0;bytes[i]=seed>>>24;}
  return bytes;
}
const vectors=[
  ['', 'e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855'],
  ['abc','ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad'],
  ['abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq','248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1'],
  ['a'.repeat(1000000),'cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0'],
];
const fixtures=[0,1,2,3,7,31,32,55,56,57,63,64,65,119,120,127,128,129,255,256,257,511,512,513,4095,4096,4097,65535,65536,131072,1310720]
  .map(length=>({name:length+' deterministic binary bytes',bytes:bytesOfLength(length)}));
fixtures.push({name:'all octets including zero',bytes:Buffer.from(Array.from({length:256},(_,i)=>i))});
fixtures.push({name:'UTF-8 airport/callsign labels',bytes:Buffer.from('Flight Board ✈️ · Abidjan — Île-de-France\u0000fin')});
// Optional real firmware paths are read locally; neither content nor hashes are logged.
for(const filename of process.argv.slice(2))fixtures.push({name:path.basename(filename),bytes:fs.readFileSync(filename)});
for(const [name,script] of [['readable source',source],['official minified build',minified],['firmware gzip',served.toString()]]){
  const context=browser(script);
  for(const [input,hash] of vectors){
    context.inputString=input;
    equal(vm.runInContext('sha256(inputString)',context,{timeout:10000}),hash,name+' published vector');
    equal(hashBytes(context,Buffer.from(input)),hash,name+' ArrayBuffer vector');
  }
  for(const fixture of fixtures){
    const expectedHash=nativeHash(fixture.bytes);
    equal(hashBytes(context,fixture.bytes),expectedHash,name+': '+fixture.name);
    equal(hashBytes(context,fixture.bytes,'slice'),expectedHash,name+': subarray '+fixture.name);
  }
  const changed=Buffer.from(fixtures[fixtures.length-1].bytes);
  if(changed.length){changed[Math.floor(changed.length/2)]^=1;ok(hashBytes(context,changed)!==nativeHash(fixtures[fixtures.length-1].bytes),'One changed byte must change the checksum');}
  console.log('PASS SHA-256 browser mode without Web Crypto: '+name);
}

// Generate only the hash artifact into temporary files: no Pillow, network or UI changes.
const temp=fs.mkdtempSync(path.join(os.tmpdir(),'flightboard-hash-test-'));
try{
  const script=String.raw`import importlib.util, pathlib, sys
spec=importlib.util.spec_from_file_location('assets_build',sys.argv[1])
module=importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
target=pathlib.Path(sys.argv[2])
module.build_hash(target/'one.h')
module.build_hash(target/'two.h')
vendor=module.ASSETS/'vendor'/'js-sha256-0.11.1'
original_read=pathlib.Path.read_bytes
def modified_read(self):
 data=original_read(self)
 return data+b'changed' if self==vendor/'sha256.min.js' else data
pathlib.Path.read_bytes=modified_read
try:
 module.build_hash(target/'corrupt.h')
except ValueError as error:
 assert 'checksum mismatch' in str(error)
else:
 raise AssertionError('Changed vendored source was accepted')
assert not (target/'corrupt.h').exists()
`;
  childProcess.execFileSync('python3',['-B','-c',script,path.join(root,'build_assets.py'),temp],{stdio:'pipe'});
  equal(fs.readFileSync(path.join(temp,'one.h'),'utf8'),generated,'Fresh hash build must match committed header');
  equal(fs.readFileSync(path.join(temp,'two.h'),'utf8'),generated,'Repeated generation must be deterministic');
  ok(!fs.existsSync(path.join(temp,'corrupt.h')),'Changed source must fail before writing output');
}finally{fs.rmSync(temp,{recursive:true,force:true});}
console.log(`SHA-256 offline asset: ${assertions} assertions passed; ${compressed.length} bytes gzip`);
