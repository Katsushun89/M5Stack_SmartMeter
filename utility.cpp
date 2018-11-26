#include <stdint.h>
#include "utility.h"

static bool isOverUint32Limit(uint32_t num)
{
    if(num > 0xFFFFFFFF){
        return true;
    } 
    return false;
}

bool isOverWaitTime(uint32_t start_time, uint32_t cur_time, uint32_t wait_time)
{
    uint32_t diff_time = 0;
    if(isOverUint32Limit(start_time + cur_time)){
        diff_time = (0xFFFFFFFF - start_time) + cur_time;
    }else{
        diff_time = cur_time - start_time;
    } 

   if(diff_time >= wait_time){
       return true;
   }
   return false;
}

