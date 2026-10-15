#include "FlightBoardUpdate.h"
#include <fstream>
#include <stdexcept>
static int checks=0;
#define CHECK(x) do{++checks;if(!(x))throw std::runtime_error(std::string(__func__)+":"+std::to_string(__LINE__)+" " #x);}while(0)
std::vector<uint8_t> image(){
 esp_image_header_t head{};head.magic=ESP_IMAGE_HEADER_MAGIC;head.segment_count=2;head.chip_id=ESP_CHIP_ID_ESP32S3;head.hash_appended=1;
 esp_image_segment_header_t first{0x3c000020,256},second{0x3fc00000,512};
 esp_app_desc_t desc{};desc.magic_word=ESP_APP_DESC_MAGIC_WORD;
 std::vector<uint8_t> b(848,0);memcpy(b.data(),&head,24);memcpy(b.data()+24,&first,8);memcpy(b.data()+32,&desc,256);memcpy(b.data()+288,&second,8);
 const char*family="FlightBoard.ESP32S3.Application.v2";memcpy(b.data()+315,family,strlen(family));return b;
}
struct Fixture{
 WebServer s;FlightBoardUpdate f{s};
 Fixture(){testNow=1000;ESP=EspMock();Update=UpdateMock();noNext=noOtaData=samePartition=false;s.collected.push_back("X-Existing");f.begin();s.request("/api/system");JsonDocument d;CHECK(!deserializeJson(d,s.responseBody));s.requestHeaders["X-Update-Token"]=d["updateToken"].as<String>();CHECK(s.collected.front()=="X-Existing");}
 void start(size_t size,const char*name="program.ino.bin"){s.requestHeaders["X-Firmware-Size"]=std::to_string(size);s.currentUpload.name="firmware";s.currentUpload.filename=name;s.event(UPLOAD_FILE_START);}
 void write(const std::vector<uint8_t>&b,size_t chunk=1024){for(size_t i=0;i<b.size();i+=chunk){size_t n=std::min(chunk,b.size()-i);memcpy(s.currentUpload.buf,b.data()+i,n);s.currentUpload.currentSize=n;s.event(UPLOAD_FILE_WRITE);}}
 void end(size_t size){s.currentUpload.totalSize=size;s.event(UPLOAD_FILE_END);}
 void finish(){s.request("/api/update",HTTP_POST);}
 void all(const std::vector<uint8_t>&b,size_t chunk=1024){start(b.size());write(b,chunk);end(b.size());finish();}
};
void success(){for(size_t chunk:{1,7,31,287,288,289,333,1024}){Fixture t;auto b=image();t.start(b.size());CHECK(t.f.busy());t.write(b,chunk);CHECK(Update.written==b);t.end(b.size());CHECK(!Update.bootActivated);CHECK(Update.ends==0);t.finish();CHECK(t.s.responseCode==200);CHECK(Update.bootActivated);CHECK(t.f.busy());t.f.update();CHECK(ESP.restarts==0);testNow+=1799;t.f.update();CHECK(ESP.restarts==0);++testNow;t.f.update();CHECK(ESP.restarts==1);}}
void invalid(){for(int variant=0;variant<13;++variant){Fixture t;auto b=image();switch(variant){case 0:b[0]=0;break;case 1:b[12]=0;break;case 2:b[32]=0;break;case 3:b[315]='X';break;case 4:b.push_back(0);break;case 5:b.resize(800);break;case 6:b[1]=17;break;case 7:b[23]=2;break;case 8:b[28]=0;b[29]=0;break;case 9:b[292]=0xff;b[293]=0xff;break;case 10:t.s.requestHeaders["X-Update-Token"]="wrong";break;case 11:noNext=true;break;case 12:noOtaData=true;break;}t.all(b,53);CHECK(t.s.responseCode>=400);CHECK(!Update.bootActivated);CHECK(!t.f.busy());CHECK(ESP.restarts==0);}}
void length_and_extension(){for(const char*size:{"0","-12","9999999999","848.0","848x","849","847"}){Fixture t;auto b=image();t.start(b.size());t.s.event(UPLOAD_FILE_ABORTED);t.s.requestHeaders["X-Firmware-Size"]=size;t.s.event(UPLOAD_FILE_START);t.write(b);t.end(b.size());t.finish();CHECK(t.s.responseCode>=400);CHECK(!Update.bootActivated);}Fixture t;auto b=image();t.start(b.size(),"program.txt");t.write(b);t.end(b.size());t.finish();CHECK(t.s.responseCode==400);CHECK(Update.begins==0);}
void failures_and_retry(){for(int variant=0;variant<7;++variant){Fixture t;auto b=image();t.start(b.size());if(variant==0)Update.failBegin=true;if(variant==1)Update.failWrite=true;if(variant==2)Update.failEnd=true;t.write(b,77);t.end(b.size());if(variant==3)t.s.event(UPLOAD_FILE_START);if(variant==4)t.s.argsCount=1;if(variant==5){t.s.event(UPLOAD_FILE_ABORTED);CHECK(!t.f.busy());}if(variant==6){testNow+=30001;t.f.update();CHECK(!t.f.busy());}t.finish();CHECK(t.s.responseCode>=400);CHECK(!Update.bootActivated);CHECK(!t.f.busy());Update.failBegin=Update.failWrite=Update.failEnd=false;t.s.argsCount=0;t.all(b);CHECK(t.s.responseCode==200);CHECK(Update.bootActivated);}}
void abort_midstream(){Fixture t;auto b=image();t.start(b.size());std::vector<uint8_t> partial(b.begin(),b.begin()+400);t.write(partial);CHECK(Update.begins==1);t.s.event(UPLOAD_FILE_ABORTED);CHECK(Update.aborts==1);CHECK(!Update.bootActivated);CHECK(!t.f.busy());t.all(b);CHECK(t.s.responseCode==200);}
void no_body(){Fixture t;t.finish();CHECK(t.s.responseCode==400);CHECK(!Update.bootActivated);CHECK(Update.begins==0);}
void preparation_guard(){Fixture t;auto b=image();t.f.setPrepareCheck([](){return false;});t.all(b);CHECK(t.s.responseCode==503);CHECK(Update.begins==0);CHECK(!Update.bootActivated);CHECK(!t.f.busy());t.f.setPrepareCheck([](){return true;});t.all(b);CHECK(t.s.responseCode==200);CHECK(Update.bootActivated);}
void real_image(const char*path,bool accept){Fixture t;std::ifstream in(path,std::ios::binary);CHECK(in.good());std::vector<uint8_t>b((std::istreambuf_iterator<char>(in)),{});t.all(b,1024);CHECK((t.s.responseCode==200)==accept);CHECK(Update.bootActivated==accept);}
int main(int argc,char**argv){try{success();invalid();length_and_extension();failures_and_retry();abort_midstream();no_body();preparation_guard();if(argc>1)real_image(argv[1],true);if(argc>2)real_image(argv[2],false);if(argc>3)real_image(argv[3],false);std::cout<<"PASS OTA "<<checks<<" assertions\n";return 0;}catch(const std::exception&e){std::cerr<<"FAIL "<<e.what()<<"\n";return 1;}}
