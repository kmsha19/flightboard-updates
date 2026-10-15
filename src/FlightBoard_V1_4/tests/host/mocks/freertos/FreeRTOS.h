#pragma once
#include "MockPlatform.hpp"
#include <stdexcept>
#define PI 3.14159265358979323846
template<class T>T constrain(T a,T low,T high){return std::min(high,std::max(low,a));}
constexpr int pdTRUE=1,pdPASS=1;
constexpr unsigned long portMAX_DELAY=0xffffffff;
struct FakeQueue {size_t itemSize;std::deque<std::vector<uint8_t>> items;};
using QueueHandle_t=FakeQueue*;
using TaskHandle_t=void*;
inline std::vector<FakeQueue*> fakeQueues;
inline int queueFailuresAfter=-1,queueDeletes=0;
inline bool taskCreateSuccess=true,stopWorkerAfterSend=false;
inline void(*workerFunction)(void*)=nullptr;
inline void* workerContext=nullptr;
struct WorkerStopped:std::exception{};
inline QueueHandle_t xQueueCreate(int,size_t size){if(queueFailuresAfter==0)return nullptr;if(queueFailuresAfter>0)--queueFailuresAfter;auto q=new FakeQueue{size,{}};fakeQueues.push_back(q);return q;}
inline void vQueueDelete(QueueHandle_t q){++queueDeletes;fakeQueues.erase(std::remove(fakeQueues.begin(),fakeQueues.end(),q),fakeQueues.end());delete q;}
inline int xQueueSend(QueueHandle_t q,const void* data,unsigned long wait){if(!q||!q->items.empty())return 0;std::vector<uint8_t> item(q->itemSize);std::memcpy(item.data(),data,q->itemSize);q->items.push_back(item);if(stopWorkerAfterSend&&wait==portMAX_DELAY)throw WorkerStopped();return pdTRUE;}
inline int xQueueReceive(QueueHandle_t q,void* data,unsigned long){if(!q||q->items.empty())return 0;std::memcpy(data,q->items.front().data(),q->itemSize);q->items.pop_front();return pdTRUE;}
inline int xTaskCreate(void(*fn)(void*),const char*,unsigned int,void* context,unsigned int,TaskHandle_t* handle){if(!taskCreateSuccess)return 0;workerFunction=fn;workerContext=context;*handle=context;return pdPASS;}
inline int taskDeletes=0;
inline void vTaskDelete(TaskHandle_t){++taskDeletes;}
