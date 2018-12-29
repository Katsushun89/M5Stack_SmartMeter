#pragma once
#include <M5Stack.h>
#include "utility.h"
#include "config.h"

typedef uint8_t accumulation_collection_date_t;

typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t sec;
    uint32_t power_consumpution;
} integral_power_consumpution_t;


class BP35A1{
private:
    String pass_;
    String auth_ID_;
    String ipv6_addr_;
    String channel_;
    String pan_ID_;
    float integral_power_unit_;
    float coefficient_;
    uint8_t accumulation_collection_date_;
    integral_power_consumpution_t integral_power_consumpution_;

    float convertPowerUnit(uint32_t int_power_unit){
        switch(int_power_unit){
        case 0x0:
            return 1;
        case 0x1:
            return 0.1;
        case 0x2:
            return 0.01;
        case 0x3:
            return 0.001;
        case 0x4:
            return 0.0001;
        case 0xA:
            return 10;
        case 0xB:
            return 100;
        case 0xC:
            return 1000;
        case 0xD:
            return 10000;
        default:
            Serial.println("convert error" + String(int_power_unit, DEC));
            return 0;
        }
    };

public:
    const uint32_t WAIT_TIME = 10 * 1000;
    const uint32_t WAIT_SCAN_TIME = 180 * 1000;
    const uint32_t WAIT_MEASURE_TIME = 30 * 1000;
    const uint32_t SCAN_LIMIT = 15;
    const uint32_t JOIN_LIMIT = 10;
    const uint32_t MEASURE_LIMIT = 15;

    BP35A1(void){
        pass_ = B_ROOT_PASS;
        auth_ID_ = B_ROOT_AUTH_ID;
        ipv6_addr_ = "";
        channel_ = "";
        pan_ID_ = "";
    };

public:
    bool waitExpectedRes(uint32_t wait_time, String expected_str, String *res_str){
        uint32_t start_time = millis();
        uint32_t cur_time = millis();
        
        delay(100);
        while(!isOverWaitTime(start_time, cur_time, wait_time)){
            *res_str = Serial2.readStringUntil('\0');
            cur_time = millis();
            //Serial.println("cur_time:" + String(cur_time, DEC) + ", start_time" + String(start_time, DEC));
            if(res_str->length() == 0) continue;

            Serial.println(*res_str);
            Serial.println();
            delay(1);
            if(res_str->indexOf(expected_str) != -1) {
                return true;
            }
            if(res_str->indexOf("EVENT 22") != -1) break;
            if(res_str->indexOf("EVENT 24") != -1) break;
            //FAILでbreakしたくないケースがあれば関数を分離する
            if(res_str->indexOf("FAIL") != -1) break; 
            
        }
        return false;
    };

public:
    bool waitExpectedRes(uint32_t wait_time, String expected_str){
        String res_str;
        return waitExpectedRes(wait_time, expected_str, &res_str);
    };

public:
    //データ部の表示形式（00：バイナリ表示、01：ASCII表示）
    //一度だけ設定すれば後は不要
    bool setWOPT(void){
        Serial2.println("WOPT 01");
        delay(100);
        bool is_received_res = waitExpectedRes(WAIT_TIME, "OK");
        if(!is_received_res){
            Serial.println("setWOPT()) nores err");
            return false;
        }
        Serial.println("setWOPT() OK");
        return true;
    };

public:
    bool testComm(void){
        Serial2.println("SKVER");
        delay(100);
        bool is_received_res = waitExpectedRes(WAIT_TIME, "OK");
        if(!is_received_res){
            Serial.println("testcomm() nores err");
            return false;
        }
        Serial.println("testcomm() ok");
        return true;
    };

private:
    bool setPass(void){
        Serial2.println("SKSETPWD C " + pass_);
        delay(100);
        bool is_received_res = waitExpectedRes(WAIT_TIME, "OK");
        if(!is_received_res){
            Serial.println("setPass() nores err");
            return false;
        }
        Serial.println("setPass() OK");
        return true;
    };
  
 private:
    bool setAuthID(void){
        Serial2.println("SKSETRBID " + auth_ID_);
        delay(100);
        bool is_received_res = waitExpectedRes(WAIT_TIME, "OK");
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
        bool is_received_res = waitExpectedRes(WAIT_TIME, "EVENT 25", &res_str);
        if(!is_received_res){
            Serial.println("tryRejoin() nores err");
            return false;
        }
        uint32_t offset = res_str.indexOf(":");
        ipv6_addr_ = res_str.substring(offset - 4, offset + (5 * 7));
        Serial.println("tryRejoin() OK, ipv6:" + ipv6_addr_);
        return true;
    };

public:
    bool initComm(void){
        if(setPass() == false) return false;
        if(setAuthID() == false) return false;
        if(tryRejoin() == false) return false;
        return true;        
    };

private:
    bool scan(uint32_t *loop_cnt, bool *should_retry, String *addr){
        //PairIDは、B ROOT IDの末尾8バイトが自動的に設定される
        Serial2.println("SKSCAN 2 FFFFFFFF 3");
        delay(100);
        String res_str;
        bool is_received_res = waitExpectedRes(WAIT_SCAN_TIME, "EPANDESC", &res_str);
        if(!is_received_res){
            //もし拡張ビーコン応答より先に"EVENT 22"（スキャン終了）が返ってきたらリトライする
            if(res_str.startsWith("EVENT 22")){
                (*loop_cnt)++;
                *should_retry = true;
                Serial.println("retry scan");
            }else{
                Serial.println("scan res is not EVENT 22");
                Serial.println(res_str.c_str());
            }
            return false;
        }
        uint32_t offset = 0;

        res_str = res_str.substring(res_str.indexOf("Channel"));
        offset = res_str.indexOf(":") + 1;
        channel_ = res_str.substring(offset, offset + 2);

        res_str = res_str.substring(res_str.indexOf("Pan ID"));
        offset = res_str.indexOf(":") + 1;
        pan_ID_ = res_str.substring(offset, offset + 4);

        res_str = res_str.substring(res_str.indexOf("Addr"));
        offset = res_str.indexOf(":") + 1;
        *addr = res_str.substring(offset, offset + 16);

        Serial.println("scan() OK, addr:" + *addr);
        return true;
    };

private:
    bool setChannel(void){
        Serial2.println("SKSREG S2 " + channel_);
        delay(100);
        bool is_received_res = waitExpectedRes(WAIT_TIME, "OK");
        if(!is_received_res){
            Serial.println("setChannel() nores err");
            return false;
        }
        Serial.println("setChannel() OK");
        return true;
    };
  
    bool setPanID(void){
        Serial2.println("SKSREG S3 " + pan_ID_);
        delay(100);
        bool is_received_res = waitExpectedRes(WAIT_TIME, "OK");
        if(!is_received_res){
            Serial.println("setPanID() nores err");
            return false;
        }
        Serial.println("setPanID() OK");
        return true;
    };
 
    bool setAddr(String addr){
        String tmp_addr = "";
        Serial2.println("SKLL64 " + addr);
        delay(100);
        bool is_received_res = waitExpectedRes(WAIT_TIME, ":" + pan_ID_, &tmp_addr);
        if(!is_received_res){
            Serial.println("setAddr() nores err");
            return false;
        }
        if(tmp_addr.indexOf("SKLL64") == -1){
            Serial.println("setAddr() fail");
            return true;
        }
        uint32_t offset = tmp_addr.indexOf(":");
        ipv6_addr_ = tmp_addr.substring(offset - 4, offset + (5 * 7));

        Serial.println("setAddr() OK ipv6_addr_:" + ipv6_addr_);
        return true;
    };
  
private:
    bool join(uint32_t *loop_cnt){
        Serial2.println("SKJOIN " + ipv6_addr_);
        delay(100);
        String res_str;
        bool is_received_res = waitExpectedRes(WAIT_TIME, "EVENT 25");
        if(!is_received_res){
            (*loop_cnt)++;
            Serial.println("retry join");
            return false;
        }
        Serial.println("join() OK");
        return true;
    };

public:
    bool activeScan(void){
        Serial.println(__func__);
        uint32_t loop_cnt = 0;
        String addr = "";
        bool should_retry = false;
        while(loop_cnt <= SCAN_LIMIT){
            /* EVENT 22返ってこなくてもリトライするならshould_retry要らないかも */
            if(scan(&loop_cnt, &should_retry, &addr)){
                break;
            }
        }
        if(loop_cnt >= SCAN_LIMIT){
            Serial.println("activeScan() scan limit over");
            return false;
        }
        Serial.println("scan res recv");

        //アクティブスキャンが完全に終了するまで待つ
        bool is_received_res = waitExpectedRes(WAIT_SCAN_TIME, "EVENT 22");
        if(!is_received_res){
            Serial.println("scan not complete");
            return false;
        }
        Serial.println("scan success");

        //仮想レジスタのレジスタ番号S2に拡張ビーコン応答で得られたチャンネル（自端末が使用する周波数の論理チャンネル番号）を設定する
        if(setChannel() == false) return false;
        if(setPanID() == false) return false;
        if(setAddr(addr) == false) return false;

        loop_cnt = 0;
        while(loop_cnt < JOIN_LIMIT){
            if(join(&loop_cnt)){
                break;
            }
        }

        if(loop_cnt >= JOIN_LIMIT){
            Serial.println("activeScan() join limit over");
            return false;
        }
        Serial.println("activeScan() OK");
        return true; 
    };

public:
    bool getInstantaneousPower(uint32_t *power){
        Serial.println(__func__);
        const uint32_t DATA_STR_LEN = 14;
        char data_str[DATA_STR_LEN] = {"0"};

        //EHD
        data_str[0] = char(0x10);
        data_str[1] = char(0x81);
        //TID
        data_str[2] = char(0x00);
        data_str[3] = char(0x01);
        //SEOJ
        data_str[4] = char(0x05);
        data_str[5] = char(0xff);
        data_str[6] = char(0x01);
        //DEOJ 低圧スマート電力量メータークラス
        data_str[7] = char(0x02);
        data_str[8] = char(0x88);
        data_str[9] = char(0x01);
        //ESV(0x62:プロパティ値読み出し)
        data_str[10] = char(0x62);

        //OPC(1個)
        data_str[11] = char(0x01);

        //EPC
        //瞬時電力計測値:0xE7
        data_str[12] = char(0xe7);
        //PDC read:0
        data_str[13] = char(0x00);

        String data_str_len = String(DATA_STR_LEN, HEX);
        data_str_len.toUpperCase();
        uint32_t str_len = data_str_len.length(); 
        for(uint32_t i = 0; i < 4 - str_len; i++){
            data_str_len = "0" + data_str_len;
        }
        
        String com_str = "SKSENDTO 1 " + ipv6_addr_ + " 0E1A 1 " + data_str_len + " ";
        byte com_bytes[1024];
        com_str.getBytes(com_bytes, com_str.length() + 1);
        for(uint32_t i = 0; i < DATA_STR_LEN; i++){
            com_bytes[com_str.length() + i] = data_str[i];
        }
        
        uint32_t loop_cnt = 0;
        do{
            String measure_value;
            Serial2.write(com_bytes, com_str.length() + DATA_STR_LEN);
            Serial.write(com_bytes, com_str.length() + DATA_STR_LEN);
            Serial2.println();
            String expected_res = "1081000102880105FF01";
            bool is_received_res = waitExpectedRes(WAIT_MEASURE_TIME, expected_res, &measure_value);
            if(!is_received_res){
               Serial.println("measure nores err");
               loop_cnt++;
               continue;
            }

            uint32_t offset = measure_value.indexOf(expected_res);
            measure_value = measure_value.substring(offset + expected_res.length());
            Serial.println("EDATA = " + measure_value);
            if(!(measure_value.indexOf("72") != -1 || measure_value.indexOf("52") != -1)){
               Serial.println("measure res data err");
               loop_cnt++;
               continue;
            }

            Serial.println("measure res OK");

            offset = measure_value.indexOf("E7");
            String hex_power = measure_value.substring(offset + 4, offset + 4 + 8);
            Serial.println(hex_power);
            unsigned int hex_power_char = hexToDec(hex_power);
            Serial.println("IPMV:" + String(hex_power_char) + "[W]");
            
            *power = static_cast<uint32_t>(hex_power_char);
            return true;
        }while(loop_cnt < MEASURE_LIMIT);

        return false;
    };

    bool setIntegralCollectDate(uint8_t collect_date){
        Serial.println(__func__);
        const uint32_t DATA_STR_LEN = 15;
        char data_str[DATA_STR_LEN] = {"0"};

        //EHD
        data_str[0] = char(0x10);
        data_str[1] = char(0x81);
        //TID
        data_str[2] = char(0x00);
        data_str[3] = char(0x01);
        //SEOJ
        data_str[4] = char(0x05);
        data_str[5] = char(0xff);
        data_str[6] = char(0x01);
        //DEOJ 低圧スマート電力量メータークラス
        data_str[7] = char(0x02);
        data_str[8] = char(0x88);
        data_str[9] = char(0x01);
        //ESV(0x61:プロパティ値書き込み)
        data_str[10] = char(0x61);

        //OPC(1個)
        data_str[11] = char(0x01);
     
        //EPC
        //積算履歴収集日１:0xE5
        data_str[12] = char(0xe5);
        //PDC write:1
        data_str[13] = char(0x01);
        //EDT 0x0:today, -1:yesterday
        data_str[14] = char(collect_date);

        String data_str_len = String(DATA_STR_LEN, HEX);
        data_str_len.toUpperCase();
        uint32_t str_len = data_str_len.length(); 
        for(uint32_t i = 0; i < 4 - str_len; i++){
            data_str_len = "0" + data_str_len;
        }
        
        String com_str = "SKSENDTO 1 " + ipv6_addr_ + " 0E1A 1 " + data_str_len + " ";
        byte com_bytes[1024];
        com_str.getBytes(com_bytes, com_str.length() + 1);
        for(uint32_t i = 0; i < DATA_STR_LEN; i++){
            com_bytes[com_str.length() + i] = data_str[i];
        }
        
        uint32_t loop_cnt = 0;
        do{
            String measure_value;
            int32_t start_data; 
            int32_t end_data; 
            String hex_PDC;
            uint32_t int_PDC;

            Serial2.write(com_bytes, com_str.length() + DATA_STR_LEN);
            Serial.write(com_bytes, com_str.length() + DATA_STR_LEN);
            Serial2.println();
            String expected_res = "1081000102880105FF01";
            bool is_received_res = waitExpectedRes(WAIT_MEASURE_TIME, expected_res, &measure_value);
            if(!is_received_res){
               Serial.println("measure nores err");
               loop_cnt++;
               continue;
            }

            uint32_t offset = measure_value.indexOf(expected_res);
            measure_value = measure_value.substring(offset + expected_res.length());
            Serial.println("EDATA = " + measure_value);
            //71:要求受付, 51:不可応答
            if(!(measure_value.indexOf("71") != -1 || measure_value.indexOf("51") != -1)){
               Serial.println("measure res data err");
               loop_cnt++;
               continue;
            }

            Serial.println("measure res OK");

            //積算履歴収集日１:0xE5
            offset = measure_value.indexOf("E5");
            start_data = offset + 2;
            end_data = start_data + 2;
            hex_PDC = measure_value.substring(start_data, end_data);
            int_PDC = hexToDec(hex_PDC);
            Serial.println("E5 PDC " + hex_PDC + ", " + String(int_PDC, DEC));
            //PDC=0:success
            if(hex_PDC == "00"){
                Serial.println("set collect date:success");
                return true;
            }else{
                Serial.println("set collect date:failure");
                return false;
            }
        }while(loop_cnt < MEASURE_LIMIT);

        return false;
 
    };

    bool getIntegralPowerConsumption(integral_power_consumpution_t *integral_power){
        Serial.println(__func__);
        const uint32_t DATA_STR_LEN = 20;
        char data_str[DATA_STR_LEN] = {"0"};

        //EHD
        data_str[0] = char(0x10);
        data_str[1] = char(0x81);
        //TID
        data_str[2] = char(0x00);
        data_str[3] = char(0x01);
        //SEOJ
        data_str[4] = char(0x05);
        data_str[5] = char(0xff);
        data_str[6] = char(0x01);
        //DEOJ 低圧スマート電力量メータークラス
        data_str[7] = char(0x02);
        data_str[8] = char(0x88);
        data_str[9] = char(0x01);
        //ESV(0x62:プロパティ値読み出し)
        data_str[10] = char(0x62);

        //OPC(4個)
        data_str[11] = char(0x04);
     
        //EPC
        //係数:0xD3
        data_str[12] = char(0xd3);
        //PDC read:0
        data_str[13] = char(0x00);

        //EPC
        //積算電力量単位:0xE1
        data_str[14] = char(0xe1);
        //PDC read:0
        data_str[15] = char(0x00);

        //EPC
        //積算履歴収集日１:0xE5
        data_str[16] = char(0xe5);
        //PDC read:0
        data_str[17] = char(0x00);

        //EPC
        //定時積算電力量計測値(正方向計測値):0xEA
        data_str[18] = char(0xea);
        //PDC read:0
        data_str[19] = char(0x00);

        String data_str_len = String(DATA_STR_LEN, HEX);
        data_str_len.toUpperCase();
        uint32_t str_len = data_str_len.length(); 
        for(uint32_t i = 0; i < 4 - str_len; i++){
            data_str_len = "0" + data_str_len;
        }
        
        String com_str = "SKSENDTO 1 " + ipv6_addr_ + " 0E1A 1 " + data_str_len + " ";
        byte com_bytes[1024];
        com_str.getBytes(com_bytes, com_str.length() + 1);
        for(uint32_t i = 0; i < DATA_STR_LEN; i++){
            com_bytes[com_str.length() + i] = data_str[i];
        }
        
        uint32_t loop_cnt = 0;
        do{
            String measure_value;
            int32_t start_data; 
            int32_t end_data; 
            String hex_PDC;
            uint32_t int_PDC;

            Serial2.write(com_bytes, com_str.length() + DATA_STR_LEN);
            Serial.write(com_bytes, com_str.length() + DATA_STR_LEN);
            Serial2.println();
            String expected_res = "1081000102880105FF01";
            bool is_received_res = waitExpectedRes(WAIT_MEASURE_TIME, expected_res, &measure_value);
            if(!is_received_res){
               Serial.println("measure nores err");
               loop_cnt++;
               continue;
            }

            uint32_t offset = measure_value.indexOf(expected_res);
            measure_value = measure_value.substring(offset + expected_res.length());
            Serial.println("EDATA = " + measure_value);
            if(!(measure_value.indexOf("72") != -1 || measure_value.indexOf("52") != -1)){
               Serial.println("measure res data err");
               loop_cnt++;
               continue;
            }

            Serial.println("measure res OK");

            //係数:0xD3
            offset = measure_value.indexOf("D3");
            start_data = offset + 2;
            end_data = start_data + 2;
            hex_PDC = measure_value.substring(start_data, end_data);
            int_PDC = hexToDec(hex_PDC);
            Serial.println("D3 PDC " + hex_PDC + ", " + String(int_PDC, DEC));
            start_data = end_data;
            end_data = start_data + 2 * int_PDC;
            String str_coefficient = measure_value.substring(start_data, end_data);
            Serial.println("str_coefficient:" + str_coefficient);
            uint32_t int_coef = hexToDec(str_coefficient);
            Serial.println("coefficient:" + String(int_coef));
            coefficient_ = float(int_coef);

            measure_value = measure_value.substring(end_data);

            //積算電力量単位:0xE1
            offset = measure_value.indexOf("E1");
            start_data = offset + 2;
            end_data = start_data + 2;
            hex_PDC = measure_value.substring(start_data, end_data);
            int_PDC = hexToDec(hex_PDC);
            Serial.println("E1 PDC " + hex_PDC + ", " + String(int_PDC, DEC));
            start_data = end_data; 
            end_data = start_data + 2 * int_PDC;
            String str_power_unit = measure_value.substring(start_data, end_data);
            Serial.println("str power unit:" + str_power_unit);
            uint32_t int_power_unit = hexToDec(str_power_unit);
            Serial.println("power unit:" + String(int_power_unit));
            integral_power_unit_ = convertPowerUnit(int_power_unit);

            measure_value = measure_value.substring(end_data);

            //積算履歴収集日１:0xE5
            offset = measure_value.indexOf("E5");
            start_data = offset + 2;
            end_data = start_data + 2;
            hex_PDC = measure_value.substring(start_data, end_data);
            int_PDC = hexToDec(hex_PDC);
            Serial.println("E5 PDC " + hex_PDC + ", " + String(int_PDC, DEC));
            start_data = end_data;
            end_data = start_data + 2 * int_PDC;
            String str_collect_date = measure_value.substring(start_data, end_data);
            uint32_t int_collect_date = hexToDec(str_collect_date);
            Serial.println("collect date:" + String(int_collect_date));
            accumulation_collection_date_ = int_collect_date;
            if(accumulation_collection_date_ != 0){
                Serial.println("collect date is not today!");
            }

            measure_value = measure_value.substring(end_data);

            //定時積算電力量計測値(正方向計測値):0xEA
            offset = measure_value.indexOf("EA");
            start_data = offset + 2;
            end_data = start_data + 2;
            hex_PDC = measure_value.substring(start_data, end_data);
            int_PDC = hexToDec(hex_PDC);
            Serial.println("EA PDC " + hex_PDC + ", " + String(int_PDC, DEC));
            start_data = end_data;
            end_data = start_data + 2 * int_PDC;
            String str_integral_power = measure_value.substring(start_data, end_data);
            
            Serial.println(str_integral_power);
            String str_year = str_integral_power.substring(0, 4);
            integral_power_consumpution_.year = hexToDec(str_year);
            String str_month = str_integral_power.substring(4, 6);
            integral_power_consumpution_.month= hexToDec(str_month);
            String str_day = str_integral_power.substring(6, 8);
            integral_power_consumpution_.day = hexToDec(str_day);
            String str_hour = str_integral_power.substring(8, 10);
            integral_power_consumpution_.hour = hexToDec(str_hour);
            String str_minute = str_integral_power.substring(10, 12);
            integral_power_consumpution_.minute = hexToDec(str_minute);
            String str_sec = str_integral_power.substring(12, 14);
            integral_power_consumpution_.sec = hexToDec(str_sec);
            String str_power_consumption = str_integral_power.substring(14, 22);
            integral_power_consumpution_.power_consumpution = hexToDec(str_power_consumption);

            Serial.println("Date " + String(integral_power_consumpution_.year) + ":" 
                            + String(integral_power_consumpution_.month) + ":"
                            + String(integral_power_consumpution_.day) + ":" 
                            + String(integral_power_consumpution_.hour) + ":" 
                            + String(integral_power_consumpution_.minute) + ":" 
                            + String(integral_power_consumpution_.sec)
                            );

            //係数 * 積算電力量単位 * 時積算電力量計測値 
            integral_power_consumpution_.power_consumpution = 
            (uint32_t)(integral_power_consumpution_.power_consumpution *  
                        coefficient_ * integral_power_unit_);
            Serial.println("power consumption:" + String(integral_power_consumpution_.power_consumpution) + "[kWh]");
            *integral_power = integral_power_consumpution_;
            return true;
        }while(loop_cnt < MEASURE_LIMIT);

        return false;
 
    };

};

