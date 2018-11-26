#pragma once
#include <M5Stack.h>
#include "utility.h"
#include "config.h"

class BP35A1{
private:
    String b_root_pass_;
    String b_root_auth_ID_;
    String b_root_ipv6_addr_;
public:
    const uint32_t WAIT_TIME_ = 10 * 1000; /* 10s */

    BP35A1(void){
        b_root_pass_ = B_ROOT_PASS;
        b_root_auth_ID_ = B_ROOT_AUTH_ID;
        b_root_ipv6_addr_ = "";
    };

public:
    bool waitExpectedRes(uint32_t wait_time, String expected_str, String *res_str){
        uint32_t start_time = millis();
        uint32_t cur_time = millis();
        
        delay(100);
        while(!isOverWaitTime(start_time, cur_time, wait_time)){
            *res_str = Serial2.readStringUntil('\0');
            if(res_str->length() == 0) continue;

            Serial.println(res_str->c_str());
            //Serial.println();

            if(res_str->indexOf(expected_str) != -1) {
                return true;
            }
            if(res_str->indexOf("EVENT 22") != -1) break;
            if(res_str->indexOf("EVENT 24") != -1) break;
            //FAILでbreakしたくないケースがあれば関数を分離する
            if(res_str->indexOf("FAIL") != -1) break; 

            cur_time = millis();
        }
        return false;
    };

public:
    bool waitExpectedRes(uint32_t wait_time, String expected_str){
        String res_str;
        return waitExpectedRes(wait_time, expected_str, &res_str);
    };

public:
    bool testComm(void){
        Serial2.println("SKVER");
        delay(100);
        bool is_received_res = waitExpectedRes(WAIT_TIME_, "OK");
        if(!is_received_res){
            Serial.println("testComm() nores err");
            return false;
        }
        Serial.println("testComm() OK");
        return true;
    };

private:
    bool setPass(void){
        Serial2.println("SKSETPWD C " + b_root_pass_);
        delay(100);
        bool is_received_res = waitExpectedRes(WAIT_TIME_, "OK");
        if(!is_received_res){
            Serial.println("setPass() nores err");
            return false;
        }
        Serial.println("setPass() OK");
        return true;
    };
  
 private:
    bool setAuthID(void){
        Serial2.println("SKSETRBID " + b_root_auth_ID_);
        delay(100);
        bool is_received_res = waitExpectedRes(WAIT_TIME_, "OK");
        if(!is_received_res){
            Serial.println("setAuthID() nores err");
            return false;
        }
        Serial.println("setAuthID() OK");
        return true;
    };

 private:
    bool tryRejoin(void){
        Serial2.println("SKREJOIN");
        delay(100);
        String res_str;
        bool is_received_res = waitExpectedRes(WAIT_TIME_, "EVENT 25", &res_str);
        if(!is_received_res){
            Serial.println("tryRejoin() nores err");
            return false;
        }
        b_root_ipv6_addr_ = res_str.substring(res_str.indexOf(":") - 4, 
                                              res_str.indexOf(":") + (5 * 7));
        Serial.println("tryRejoin() OK, ipv6:" + b_root_ipv6_addr_);
        return true;
    };

public:
    bool initComm(void){
        if(setPass() == false) return false;
        if(setAuthID() == false) return false;
        if(tryRejoin() == false) return false;
        return true;        
    };
};

