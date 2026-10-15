#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>
using esp_err_t = int;
constexpr int ESP_OK=0,ESP_FAIL=-1,ESP_PARTITION_TYPE_DATA=1,ESP_PARTITION_SUBTYPE_DATA_SPIFFS=0x82;
struct esp_partition_t { uint32_t size; };
namespace FlashMock {
inline esp_partition_t part{0x160000};
inline std::vector<uint8_t> bytes(part.size,255);
inline bool present=true;
inline long cut=-1;
inline int calls=0, failCall=-1, corruptCall=-1;
inline size_t mutations=0;
struct PowerLoss{};
inline void reset(){part.size=0x160000;bytes.assign(part.size,255);present=true;cut=-1;calls=0;failCall=corruptCall=-1;mutations=0;}
inline bool bounds(size_t off,size_t length){return off<=bytes.size()&&length<=bytes.size()-off&&off<=part.size&&length<=part.size-off;}
inline void mutate(size_t off,uint8_t value){if(cut==0)throw PowerLoss{};if(cut>0)--cut;bytes[off]=value;++mutations;}
}
inline const esp_partition_t* esp_partition_find_first(int type,int subtype,const char* label){return FlashMock::present&&type==ESP_PARTITION_TYPE_DATA&&subtype==ESP_PARTITION_SUBTYPE_DATA_SPIFFS&&!strcmp(label,"spiffs")?&FlashMock::part:nullptr;}
inline esp_err_t esp_partition_read(const esp_partition_t*,size_t off,void* out,size_t len){using namespace FlashMock;int call=++calls;if(call==failCall||!bounds(off,len))return ESP_FAIL;if(len)memcpy(out,bytes.data()+off,len);return ESP_OK;}
inline esp_err_t esp_partition_erase_range(const esp_partition_t*,size_t off,size_t len){using namespace FlashMock;int call=++calls;if(call==failCall||off%4096||len%4096||!bounds(off,len))return ESP_FAIL;for(size_t i=0;i<len;++i)mutate(off+i,255);return ESP_OK;}
inline esp_err_t esp_partition_write(const esp_partition_t*,size_t off,const void* data,size_t len){using namespace FlashMock;int call=++calls;if(call==failCall||!bounds(off,len))return ESP_FAIL;auto b=static_cast<const uint8_t*>(data);for(size_t i=0;i<len;++i){if((bytes[off+i]&b[i])!=b[i])throw std::runtime_error("NOR write attempted 0 to 1");mutate(off+i,b[i]);}if(call==corruptCall&&len)bytes[off+len-1]^=1;return ESP_OK;}
