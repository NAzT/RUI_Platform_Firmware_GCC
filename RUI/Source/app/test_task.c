#include "board_basic.h"
#include "boards.h"
#include "nrf_delay.h"
#include "nrf_drv_rtc.h"
#include "nrf_rtc.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include "sensor.h"
#include "itracker.h"
#include "nrf_log.h"

#ifdef LORA_TEST
extern uint8_t JOIN_FLAG;
extern lora_cfg_t g_lora_cfg_t;
int lora_send_ok = 0;
#endif

#if defined(BC95G_TEST) || defined(M35_TEST) || defined(BG96_TEST)
extern uint8_t cmd[128];
extern xSemaphoreHandle xBinarySemaphore_iot;
#endif

extern double gps_lat;
extern double gps_lon;  

#ifndef ACCESS_NET_TEST
void test_task(void * pvParameter)
{
    uint8_t gps_rsp[128] = {0};
    uint8_t lora_data[128] = {0};
    uint8_t lora_len_acc = 0;
    uint8_t lora_len_t_h = 0;
    uint8_t lora_len_gps = 0;
    double temp = 0;
    double humidity = 0;
    double pressure = 0;
    int x = 0;
    int y = 0;
    int z = 0;
    float magnetic_x = 0;
    float magnetic_y = 0;
    float magnetic_z = 0;
    float light = 0;
    double lat = 0;
    double lon = 0;
#ifdef LORA_TEST
    if(g_lora_cfg_t.sof == LORA_CONFIG_MAGIC)
    {
       region_init();
    }

#endif
    while(1)
    {
        NRF_LOG_INFO("++++++++++++++++test begin++++++++++++++++\r\n");
        power_save_close();
#ifdef BEM280_TEST
        itracker_function.temperature_get(&temp);
        NRF_LOG_INFO("temperature = %d\r\n",temp);
        itracker_function.humidity_get(&humidity);
        NRF_LOG_INFO("humidity = %d\r\n",humidity);
        itracker_function.pressure_get(&pressure);
        NRF_LOG_INFO("pressure = %d\r\n",pressure);
#endif

#ifdef LPS22HB_TEST
	itracker_function.pressure_get(&pressure);
        NRF_LOG_INFO("pressure = %d hPa\r\n",pressure);	
#endif
#ifdef LIS3DH_TEST
        itracker_function.acceleration_get(&x,&y,&z);
        NRF_LOG_INFO("acceleration x,y,z = %d mg,%d mg,%d mg",x,y,z);

#endif
#ifdef LIS2MDL_TEST
        itracker_function.magnetic_get(&magnetic_x,&magnetic_y,&magnetic_z);
        NRF_LOG_INFO("magnetic x,y,z = %d,%d,%d\r\n",magnetic_x,magnetic_y,magnetic_z);
#endif
#ifdef OPT3001_TEST
        itracker_function.light_strength_get(&light);
        NRF_LOG_INFO("light strength = %d\r\n",light);
#endif

#if defined(L70R_TEST) ||  defined(BG96_TEST) || defined(MAX7_TEST)

        memset(gps_rsp,0,128);
        itracker_function.gps_get(gps_rsp,128);
        vTaskDelay(2000);
        NRF_LOG_INFO("gps info :%s;",gps_rsp);

#endif

#if defined(SHT31_TEST) || defined(SHTC3_TEST)
           itracker_function.temperature_get(&temp);
           NRF_LOG_INFO("temperature = %d\r\n",temp);
           itracker_function.humidity_get(&humidity);
           NRF_LOG_INFO("humidity = %d\r\n",humidity);
#endif

#ifdef LORA_TEST
        if(JOIN_FLAG==1)
        {
            memset(lora_data,0,128);
            lora_len_acc = 0;
            lora_len_t_h = 0;
            lora_len_gps = 0;
            lora_len_acc = sprintf(lora_data,"A:%d,%d,%d;",x,y,z);
            lora_len_t_h = sprintf(lora_data+lora_len_acc,"T:%d;H:%d;",(int)temp,(int)humidity);
            lora_len_gps = sprintf(lora_data+lora_len_acc+lora_len_t_h,"G:%lf,%lf;",gps_lat,gps_lon);            
            itracker_function.communicate_send(lora_data);
            lora_send_ok = 1; 
        }
        
#endif
#if defined(SLEEP_MODE) && !defined(LORA_TEST)
        power_save_open();
#endif
        NRF_LOG_INFO("++++++++++++++++test end++++++++++++++++\r\n");
        vTaskDelay(5000);
    }
}

#else

void hologram_cmd_packet(uint8_t *key, uint8_t *data)
{
    uint8_t i = 0;
    uint8_t j = 0;
    cmd[0]= '{';
    cmd[1]= '\"';
    cmd[2]= 'k';
    cmd[3]= '\"';
    cmd[4]= ':';  
    cmd[5]= '\"';
    for (i = 0; i < 8; i++)
    {
        cmd[6+i] = key[j++];
    }
    cmd[14] = '\"';
    cmd[15] = ',';
    cmd[16]= '\"';
    cmd[17]= 'd';
    cmd[18]= '\"';
    cmd[19]= ':';  
    cmd[20]= '\"';
    j = 0;
    for (i = 0; i < 256; i++)
    {
        if (data[j] != 0)
        {
            cmd[21+i] = data[j++];
        }
        else
        {
            break;
        }
    }    
    cmd[21+j]='\"';
    cmd[22+j]=',';
    cmd[23+j]='\"'; 
    cmd[24+j]='t';       
    cmd[25+j]='\"';
    cmd[26+j]=':';
    cmd[27+j]='\"';  
    cmd[28+j]='T'; 
    cmd[29+j]='O'; 
    cmd[30+j]='P'; 
    cmd[31+j]='I'; 
    cmd[32+j]='C';
    cmd[33+j]='1'; 
    cmd[34+j]='\"';
    cmd[35+j]='}';                
}
void nb_iot_task(void * pvParameter)
{
    uint8_t rsp[500] = {0};
    uint8_t device_key[9] = {0};
    uint8_t test_data[256] = {0};
	uint8_t gps_data[128] = {0};
    uint8_t len[20] = {0}; 
    uint8_t sensor_len = 0;
    double temp = 0;
    double humidity = 0;
    double pressure = 0;
    int x = 0;
    int y = 0;
    int z = 0;
    float magnetic_x = 0;
    float magnetic_y = 0;
    float magnetic_z = 0;
    float light = 0;
    double lat = 0;
    double lon = 0;
    uint8_t i =0;
    uint8_t j =0;

    while(1)
    {
        if( xSemaphoreTake( xBinarySemaphore_iot, portMAX_DELAY ) == pdTRUE && cmd[0] != 0)
        {
            if(strstr(cmd,"SEND")!= NULL)
            {
                for (i = 5; i < 13; i++)
                {
                    device_key[j++] = cmd[i];
                }
                j = 0;
                for (i = 14; i < 256; i++)
                {
                    if (cmd[i] != 0)
                    {
                        test_data[j++] = cmd[i];
                    }
                    else
                    {
                        break;
                    }
                }
                memset(cmd,0,128);
                hologram_cmd_packet(device_key,test_data);
                NRF_LOG_INFO("device_key = %s\r\n",device_key);
                NRF_LOG_INFO("test_data = %s\r\n",test_data); 
                NRF_LOG_INFO("send packet = %s\r\n",cmd);  
                itracker_function.communicate_send("AT+QIOPEN=1,0,\"TCP\",\"cloudsocket.hologram.io\",9999,0,1");
                memset(rsp, 0, 500);
                itracker_function.communicate_response(rsp, 500, 500 * 60, GSM_TYPE_CHAR);
                memset(rsp, 0, 500);
                itracker_function.communicate_response(rsp, 500, 500 * 20, GSM_TYPE_CHAR);
                vTaskDelay(500);
                memset(len,0,20);
                sprintf(len,"AT+QISEND=0,%d",36+j+1);
                itracker_function.communicate_send(len);
                vTaskDelay(500);                              
                itracker_function.communicate_send(cmd);
                memset(rsp, 0, 500);
                itracker_function.communicate_response(rsp, 500, 500 * 60, GSM_TYPE_CHAR);
                memset(rsp, 0, 500);
                itracker_function.communicate_response(rsp, 500, 500 * 80, GSM_TYPE_CHAR);
                itracker_function.communicate_send("AT+QICLOSE=0,30000");
                memset(rsp, 0, 500);
                itracker_function.communicate_response(rsp, 500, 500 * 60, GSM_TYPE_CHAR);
            }
            else if (strstr(cmd,"SENSOR")!= NULL)
            {
                for (i = 7; i < 15; i++)
                {
                    device_key[j++] = cmd[i];
                }
                j = 0;

#ifdef BEM280_TEST
                itracker_function.temperature_get(&temp);
                NRF_LOG_INFO("temperature = %d\r\n",temp);
                itracker_function.humidity_get(&humidity);
                NRF_LOG_INFO("humidity = %d\r\n",humidity);
                itracker_function.pressure_get(&pressure);
                NRF_LOG_INFO("pressure = %d\r\n",pressure);
#endif

#ifdef LPS22HB_TEST
                itracker_function.pressure_get(&pressure);
                NRF_LOG_INFO("pressure = %d hPa\r\n",pressure); 
#endif
#ifdef LIS3DH_TEST
                itracker_function.acceleration_get(&x,&y,&z);
                NRF_LOG_INFO("acceleration x,y,z = %d mg,%d mg,%d mg",x,y,z);

#endif
#ifdef LIS2MDL_TEST
                itracker_function.magnetic_get(&magnetic_x,&magnetic_y,&magnetic_z);
                NRF_LOG_INFO("magnetic x,y,z = %d,%d,%d\r\n",magnetic_x,magnetic_y,magnetic_z);
#endif
#ifdef OPT3001_TEST
                itracker_function.light_strength_get(&light);
                NRF_LOG_INFO("light strength = %d\r\n",light);
#endif

#if defined(SHT31_TEST) || defined(SHTC3_TEST)
                itracker_function.temperature_get(&temp);
                NRF_LOG_INFO("temperature = %d\r\n",temp);
                itracker_function.humidity_get(&humidity);
                NRF_LOG_INFO("humidity = %d\r\n",humidity);
#endif

                itracker_function.gps_get(gps_data,128);
                vTaskDelay(500);
				NRF_LOG_INFO("GPS = %s\r\n",gps_data);
                memset(test_data,0,256);
                sensor_len = sprintf(test_data,"Acc:%d,%d,%d;Tem:%d;Hum:%d;Pre:%d;Mag:%d,%d,%d;Lig:%d;Gps:%s;",x,y,z,(int)temp,(int)humidity,(int)pressure,(int)magnetic_x,(int)magnetic_y,(int)magnetic_z,(int)light,gps_data);
                memset(cmd,0,128);
                hologram_cmd_packet(device_key,test_data);
                NRF_LOG_INFO("device_key = %s\r\n",device_key);
                NRF_LOG_INFO("test_data = %s\r\n",test_data);
                NRF_LOG_INFO("test_data len = %d\r\n",sensor_len);                 
                NRF_LOG_INFO("send packet = %s\r\n",cmd);  
                itracker_function.communicate_send("AT+QIOPEN=1,0,\"TCP\",\"cloudsocket.hologram.io\",9999,0,1");
                memset(rsp, 0, 500);
                itracker_function.communicate_response(rsp, 500, 500 * 60, GSM_TYPE_CHAR);
                memset(rsp, 0, 500);
                itracker_function.communicate_response(rsp, 500, 500 * 20, GSM_TYPE_CHAR);
                vTaskDelay(500);
                memset(len,0,20);
                sprintf(len,"AT+QISEND=0,%d",36+sensor_len+1);
                itracker_function.communicate_send(len);
                vTaskDelay(500);                              
                itracker_function.communicate_send(cmd);
                memset(rsp, 0, 500);
                itracker_function.communicate_response(rsp, 500, 500 * 60, GSM_TYPE_CHAR);
                memset(rsp, 0, 500);
                itracker_function.communicate_response(rsp, 500, 500 * 80, GSM_TYPE_CHAR);
                itracker_function.communicate_send("AT+QICLOSE=0,30000");
                memset(rsp, 0, 500);
                itracker_function.communicate_response(rsp, 500, 500 * 60, GSM_TYPE_CHAR);
            }
            else
            {
                itracker_function.communicate_send(cmd);
                memset(rsp, 0, 500);
                itracker_function.communicate_response(rsp, 500, 500 * 60, GSM_TYPE_CHAR);
            }

            memset(cmd,0,128);
            memset(device_key,0,9);
            memset(test_data,0,256);
			memset(gps_data,0,256);
            memset(len,0,20);
            sensor_len = 0;
            i = 0;
            j = 0;
        }
        vTaskDelay(10000);
    }
}
#endif