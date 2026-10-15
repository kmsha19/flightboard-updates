#pragma once
// Included after the sketch's settings and HTTP helpers. Raw pack bytes never execute.
struct ZcsStoreReader {
  const char* id; bool pending; uint32_t offset=0, length=0;
  uint8_t buffer[512]; size_t cursor=0, buffered=0;
  int read() {
    if (cursor == buffered) {
      buffered=std::min(size_t(512),size_t(length-offset));cursor=0;
      if(!buffered || !(pending?contentStore.readPending(offset,buffer,buffered):contentStore.read(id,offset,buffer,buffered)))return -1;
      offset+=buffered;
    }
    return buffer[cursor++];
  }
  size_t readBytes(char* out,size_t count){size_t n=0;for(;n<count;++n){int c=read();if(c<0)break;out[n]=char(c);}return n;}
};

bool installedAssetMatches(const char* id,const char* expected,uint32_t size){
  const auto* entry=contentStore.find(id);if(!entry||entry->length!=size)return false;
  mbedtls_sha256_context hash;mbedtls_sha256_init(&hash);mbedtls_sha256_starts(&hash,0);
  uint8_t buffer[512],digest[32];bool ok=true;
  for(uint32_t at=0;at<size;at+=sizeof(buffer)){size_t n=std::min(sizeof(buffer),size_t(size-at));if(!contentStore.read(id,at,buffer,n)){ok=false;break;}mbedtls_sha256_update(&hash,buffer,n);}
  mbedtls_sha256_finish(&hash,digest);mbedtls_sha256_free(&hash);char hex[65];for(size_t i=0;i<32;++i)snprintf(hex+i*2,3,"%02x",digest[i]);
  return ok&&!strcmp(hex,expected);
}
void sendStoredAsset(const char* id,const char* mime){
  const auto* e=contentStore.find(id);if(!e){sendError(404,"Ressource absente.");return;}
  webServer.setContentLength(e->length);webServer.send(200,mime,"");uint8_t buffer[1024];
  for(uint32_t at=0;at<e->length;at+=sizeof(buffer)){size_t n=std::min(sizeof(buffer),size_t(e->length-at));if(!contentStore.read(id,at,buffer,n))break;webServer.sendContent((const char*)buffer,n);}
}
bool packText(JsonVariantConst value,size_t max,bool ascii=false){
  if(!value.is<JsonString>())return false;const auto s=value.as<JsonString>();
  return s.size()>0&&s.size()<=max&&strlen(s.c_str())==s.size()&&(!ascii||zcsValidAscii(s.c_str(),max,1));
}
int packNibble(char c){return c>='0'&&c<='9'?c-'0':c>='a'&&c<='f'?c-'a'+10:c>='A'&&c<='F'?c-'A'+10:-1;}
bool validPack(JsonDocument& doc,const String& id){
  if(!doc.is<JsonObject>()||!packText(doc["id"],32,true)||id!=(doc["id"]|"")||
      !packText(doc["title"],64)||!packText(doc["type"],12,true)||!doc["version"].is<uint16_t>()||doc["version"].as<uint16_t>()<1)return false;
  const String type=doc["type"]|"";
  if(type=="message")return packText(doc["text"],120,true)&&doc["color"].is<uint16_t>();
  if(type=="timer")return doc["durationSeconds"].is<uint16_t>()&&ZcsTimer::validDuration(doc["durationSeconds"].as<uint16_t>());
  if(type!="pixel")return false;
  for(const char* key:{"width","height","frameCount","frameMs"})if(!doc[key].is<uint16_t>())return false;
  const unsigned w=doc["width"],h=doc["height"],frames=doc["frameCount"],ms=doc["frameMs"];
  const auto palette=doc["palette"].as<JsonArrayConst>();const auto pixels=doc["pixels"].as<JsonString>();
  if(w<1||w>64||h<1||h>64||frames<1||frames>32||ms<80||ms>2000||
      palette.size()<1||palette.size()>16||!pixels||pixels.size()!=size_t(w)*h*frames||pixels.size()>65536)return false;
  for(JsonVariantConst color:palette)if(!color.is<uint16_t>())return false;
  for(size_t i=0;i<pixels.size();++i){int n=packNibble(pixels.c_str()[i]);if(n<0||unsigned(n)>=palette.size())return false;}
  return true;
}
bool readPack(const char* id,JsonDocument& doc,bool metadata){
  const auto* entry=contentStore.find(id);if(!entry||!strncmp(id,"__",2)||entry->length>49152)return false;
  ZcsStoreReader reader{id,false,0,entry->length};
  if(metadata){JsonDocument filter;for(const char* key:{"id","title","type","version","text","color","durationSeconds","width","height","frameCount","frameMs"})filter[key]=true;
    return !deserializeJson(doc,reader,DeserializationOption::Filter(filter),DeserializationOption::NestingLimit(4));}
  return !deserializeJson(doc,reader,DeserializationOption::NestingLimit(4))&&validPack(doc,id);
}
JsonDocument preparedPack;
std::unique_ptr<uint8_t[]> preparedPixels;
size_t preparedPixelBytes=0;
uint16_t preparedPalette[16]={};
void discardPreparedPack(){preparedPack.clear();preparedPixels.reset();preparedPixelBytes=0;}
bool preparePack(const String& id){
  discardPreparedPack();
  if(id.isEmpty())return true;
  if(!readPack(id.c_str(),preparedPack))return false;
  if(preparedPack["type"]=="pixel"){
    const JsonString hex=preparedPack["pixels"].as<JsonString>();
    preparedPixelBytes=(hex.size()+1)/2;
    preparedPixels.reset(new(std::nothrow) uint8_t[preparedPixelBytes]());
    if(!preparedPixels){discardPreparedPack();return false;}
    for(size_t i=0;i<hex.size();++i)preparedPixels[i/2]|=uint8_t(packNibble(hex.c_str()[i]))<<((i&1)?0:4);
    size_t n=0;for(JsonVariantConst color:preparedPack["palette"].as<JsonArrayConst>())preparedPalette[n++]=color.as<uint16_t>();
    preparedPack.remove("pixels");
  }
  return true;
}
void commitPreparedPack(){
  activePack=std::move(preparedPack);activePixels.swap(preparedPixels);activePixelBytes=preparedPixelBytes;
  memcpy(activePalette,preparedPalette,sizeof(activePalette));discardPreparedPack();
}
bool activatePack(const String& id){if(!preparePack(id))return false;commitPreparedPack();return true;}

void widgetState(JsonObject o){
  o["text"]=widgetConfig["text"]|"BONJOUR";o["color"]=widgetConfig["color"]|uint16_t(0xaed8);
  o["durationSeconds"]=widgetTimer.durationSeconds();o["remainingSeconds"]=widgetTimer.remainingSeconds(millis());o["running"]=widgetTimer.running(millis());
  o["league"]=widgetConfig["league"]|"fra.1";o["eventId"]=widgetConfig["eventId"]|"";o["date"]=widgetConfig["date"]|"";
  o["contentId"]=settings.contentId;scores.toJson(o);
  if(!settings.contentId.isEmpty()&&activePack["type"]=="message"){o["text"]=activePack["text"];o["color"]=activePack["color"];}
}
void makeWidgetFrame(ZcsWidgetFrame& f){
  const bool messagePack=!settings.contentId.isEmpty()&&activePack["type"]=="message";
  f.message={messagePack?(activePack["text"]|""):(widgetConfig["text"]|"BONJOUR"),messagePack?activePack["color"].as<uint16_t>():widgetConfig["color"].as<uint16_t>()};
  f.timer={widgetTimer.remainingSeconds(millis()),widgetTimer.running(millis())};
  const auto& score=scores.data();f.score={score.home,score.away,score.status,score.homeScore,score.awayScore,scores.available(),scores.stale(millis())};
  if(activePack["type"]=="pixel")f.pixel={activePack["width"].as<uint8_t>(),activePack["height"].as<uint8_t>(),activePack["frameCount"].as<uint8_t>(),uint8_t(activePack["palette"].size()),activePack["frameMs"].as<uint16_t>(),activePalette,activePixels.get(),activePixelBytes};
}
bool persistWidget(JsonDocument& next,ZcsTimer timer){
  next["durationSeconds"]=timer.durationSeconds();next["remainingSeconds"]=timer.remainingSeconds(millis());
  String json;serializeJson(next,json);return storage.putString("widget2",json)==json.length();
}
void loadWidgets(){
  deserializeJson(widgetConfig,storage.getString("widget2","{}"));
  if(!packText(widgetConfig["text"],120,true))widgetConfig["text"]="BONJOUR";
  if(!widgetConfig["color"].is<uint16_t>())widgetConfig["color"]=uint16_t(0xaed8);
  const uint16_t duration=widgetConfig["durationSeconds"]|300;
  widgetTimer=ZcsTimer(duration);widgetTimer.restorePaused(widgetTimer.durationSeconds(),widgetConfig["remainingSeconds"]|widgetTimer.durationSeconds());
  if(!settings.contentId.isEmpty()&&!activatePack(settings.contentId)){settings.contentId="";if(settings.app=="pixel")settings.app="clock";}
  if(settings.app=="pixel"&&settings.contentId.isEmpty())settings.app="clock";
  if(activePack["type"]=="timer" && widgetTimer.durationSeconds()!=activePack["durationSeconds"].as<uint16_t>()) widgetTimer=ZcsTimer(activePack["durationSeconds"].as<uint16_t>());
  logosPackReady=installedAssetMatches("__logos",AIRLINE_IMAGES_SHA256,AIRLINE_IMAGES_SIZE);
  webPackReady=installedAssetMatches("__web",ZCS_WEB_SHA256,ZCS_WEB_SIZE);
  char token[33];snprintf(token,sizeof(token),"%08lx%08lx%08lx%08lx",(unsigned long)esp_random(),(unsigned long)esp_random(),(unsigned long)esp_random(),(unsigned long)esp_random());contentToken=token;
}
void changeWidget(){
  JsonDocument body;if(!readJsonBody(body))return;
  if(!packText(body["app"],12,true)){sendError(400,"Application invalide.");return;}
  const String app=body["app"]|"";
  JsonDocument next(widgetConfig);ZcsTimer timer=widgetTimer;
  if(app=="message"){
    if(!packText(body["text"],120,true)||!body["color"].is<uint16_t>()){sendError(400,"Message invalide (120 caracteres maximum).");return;}
    next["text"]=body["text"];next["color"]=body["color"];
  }else if(app=="timer"){
    if(!body["durationSeconds"].is<uint16_t>()||!ZcsTimer::validDuration(body["durationSeconds"].as<uint16_t>())){sendError(400,"Duree invalide.");return;}
    timer=ZcsTimer(body["durationSeconds"].as<uint16_t>());
  }else if(app=="score"){
    if(!packText(body["league"],16,true)||!packText(body["eventId"],12,true)||!packText(body["date"],8,true)||!ZcsScores::validSelection(body["league"]|"",body["eventId"]|"",body["date"]|"")){sendError(400,"Match invalide.");return;}
    for(const char* key:{"league","eventId","date"})next[key]=body[key];
  }else{sendError(400,"Application inconnue.");return;}
  if(!persistWidget(next,timer)){sendError(503,"Enregistrement impossible.");return;}
  widgetConfig=std::move(next);widgetTimer=timer;if(app=="score")scores.reset();sendRadarState();
}
void changeTimer(){
  JsonDocument body;if(!readJsonBody(body))return;
  if(!packText(body["action"],12,true)){sendError(400,"Action invalide.");return;}
  const String action=body["action"]|"";
  ZcsTimer next=widgetTimer;bool ok=action=="start"?next.start(millis()):action=="pause"?next.pause(millis()):action=="resume"?next.resume(millis()):action=="reset"?next.reset(millis()):false;
  if(!ok){sendError(409,"Action indisponible pour le minuteur.");return;}
  JsonDocument config(widgetConfig);if(!persistWidget(config,next)){sendError(503,"Enregistrement impossible.");return;}
  widgetConfig=std::move(config);widgetTimer=next;sendRadarState();
}
bool contentAuthorized(){return !contentToken.isEmpty()&&webServer.header("X-Content-Token")==contentToken;}
size_t userContentCount(){size_t count=0;for(size_t i=0;i<contentStore.count();++i)if(strncmp(contentStore.entries()[i].id,"__",2))++count;return count;}
void sendContentList(){
  JsonDocument doc;doc["capacity"]=14;doc["available"]=contentStore.available();doc["token"]=contentToken;
  auto logos=doc["logos"].to<JsonObject>(); logos["installed"]=logosPackReady;
  logos["url"]="https://raw.githubusercontent.com/kmsha19/flightboard-updates/main/store/zcs-logos.bin";
  logos["size"]=AIRLINE_IMAGES_SIZE;logos["sha256"]=AIRLINE_IMAGES_SHA256;
  auto list=doc["items"].to<JsonArray>();
  for(size_t i=0;i<contentStore.count();++i){const auto& entry=contentStore.entries()[i];JsonDocument item;if(readPack(entry.id,item,true))list.add(item.as<JsonObjectConst>());}
  doc["used"]=list.size();String out;serializeJson(doc,out);webServer.sendHeader("Cache-Control","no-store");webServer.send(200,"application/json",out);
}
void sendContentData(){
  const String id=webServer.arg("id");const auto* entry=contentStore.find(id.c_str());
  if(!entry||id.startsWith("__")){sendError(404,"Contenu absent.");return;}
  webServer.setContentLength(entry->length);webServer.sendHeader("Cache-Control","no-store");webServer.send(200,"application/json","");uint8_t buffer[1024];
  for(size_t at=0;at<entry->length;at+=sizeof(buffer)){size_t count=std::min(sizeof(buffer),size_t(entry->length-at));if(!contentStore.read(id.c_str(),at,buffer,count))break;webServer.sendContent((const char*)buffer,count);}
}
bool validatePendingContent(ZcsContentStore&,void*){
  if(contentUploadId=="__web")return contentUploadLength==ZCS_WEB_SIZE;
  if(contentUploadId=="__logos")return contentUploadLength==AIRLINE_IMAGES_SIZE; // fixed generated lossless WebP asset bank
  JsonDocument pack;ZcsStoreReader reader{nullptr,true,0,contentUploadLength};
  if(deserializeJson(pack,reader,DeserializationOption::NestingLimit(4))||!validPack(pack,contentUploadId))return false;
  int tail;while((tail=reader.read())>=0)if(tail!=' '&&tail!='\r'&&tail!='\n'&&tail!='\t')return false;
  return reader.offset==contentUploadLength;
}
void receiveContent(){
  HTTPUpload& upload=webServer.upload();contentUploadAt=millis();
  if(upload.status==UPLOAD_FILE_START){
    if(contentRequestSeen){contentUploadError="Un seul fichier par transfert.";contentUploadComplete=false;contentStore.abort();return;}
    contentRequestSeen=true;contentUploadComplete=false;contentUploadError="";contentUploadId=webServer.arg("id");
    const String size=webServer.arg("size");contentUploadLength=0;
    bool sizeValid=size.length()>0&&size.length()<7;for(size_t i=0;i<size.length();++i){if(size[i]<'0'||size[i]>'9')sizeValid=false;else contentUploadLength=contentUploadLength*10+size[i]-'0';}
    if(!contentAuthorized())contentUploadError="Acces refuse.";
    else if(boardUpdate.busy()||contentStore.busy()||radar.pending()||routes.pending()||weather.pending()||scores.pending())contentUploadError="Panneau occupe. Reessayez dans quelques secondes.";
    else if(contentUploadId.startsWith("__")&&contentUploadId!="__web"&&contentUploadId!="__logos")contentUploadError="Identifiant reserve.";
    else if(!contentUploadId.startsWith("__")&&!contentStore.find(contentUploadId.c_str())&&userContentCount()>=14)contentUploadError="Bibliotheque pleine.";
    else if(!sizeValid||!contentUploadLength||contentUploadLength>(contentUploadId=="__web"?98304U:49152U)||contentUploadId==settings.contentId)contentUploadError="Taille ou contenu actif invalide.";
    else if(contentUploadId=="__web" && (webServer.arg("sha256")!=ZCS_WEB_SHA256 || contentUploadLength!=ZCS_WEB_SIZE))contentUploadError="Version interface incompatible.";
    else if(contentUploadId=="__logos" && webServer.arg("sha256")!=AIRLINE_IMAGES_SHA256)contentUploadError="Version des logos incompatible.";
    else if(!contentStore.beginPut(contentUploadId.c_str(),contentUploadLength,webServer.arg("sha256").c_str()))contentUploadError=contentStore.error();
  }else if(upload.status==UPLOAD_FILE_WRITE){
    if(contentUploadError.isEmpty()&&!contentStore.write(upload.buf,upload.currentSize)){contentUploadError=contentStore.error();contentStore.abort();}
  }else if(upload.status==UPLOAD_FILE_END){
    contentUploadComplete=contentUploadError.isEmpty()&&contentStore.busy();
  }else if(upload.status==UPLOAD_FILE_ABORTED){contentRequestSeen=false;contentUploadComplete=false;contentStore.abort();contentUploadError="Transfert interrompu.";}
}
void finishContent(){
  const bool valid=contentRequestSeen&&contentUploadComplete&&contentUploadError.isEmpty();
  contentRequestSeen=false;contentUploadComplete=false;
  if(!valid||!contentStore.finish(validatePendingContent,nullptr)){
    if(contentUploadError.isEmpty())contentUploadError=valid?contentStore.error():"Transfert incomplet.";
    contentStore.abort();sendError(400,contentUploadError);return;
  }
  if(contentUploadId=="__web")webPackReady=true;
  if(contentUploadId=="__logos")logosPackReady=true;
  sendContentList();
}

void removeContent(){
  if(!contentAuthorized()){sendError(403,"Acces refuse.");return;}
  const String id=webServer.arg("id");
  if(boardUpdate.busy()||contentStore.busy()||id==settings.contentId||id.startsWith("__")){sendError(409,"Affiche une autre application avant de supprimer ce contenu.");return;}
  if(!contentStore.remove(id.c_str())){sendError(400,contentStore.error());return;}sendContentList();
}
void startContentApi(){
  webServer.on("/api/content",HTTP_GET,sendContentList);webServer.on("/api/content/data",HTTP_GET,sendContentData);
  webServer.on("/api/content",HTTP_DELETE,removeContent);
  webServer.on("/api/content/install",HTTP_POST,finishContent,receiveContent);
}
