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

// Converting from Hex to Decimal:
//
// NOTE: This function can handle a positive hex value from 0 - 65,535 (a four digit hex string).
//       For larger/longer values, change "unsigned int" to "long" in both places.
unsigned int hexToDec(String hex_string) 
{
  unsigned int dec_value = 0;
  int next_int;
  
  for (uint32_t i = 0; i < hex_string.length(); i++) {
    
    next_int = int(hex_string.charAt(i));
    if (next_int >= 48 && next_int <= 57) next_int = map(next_int, 48, 57, 0, 9);		//0-9
    if (next_int >= 65 && next_int <= 70) next_int = map(next_int, 65, 70, 10, 15);		//A-F
    if (next_int >= 97 && next_int <= 102) next_int = map(next_int, 97, 102, 10, 15);	//a-f
    next_int = constrain(next_int, 0, 15);
    dec_value = (dec_value * 16) + next_int;
    //Serial.println(dec_value);
  }
  
  return dec_value;
}