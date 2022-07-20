#include <Arduino.h>
#include <Wire.h>

#include "driver/twai.h"


union float_byte
{
    float f;
    uint8_t float_bytes[4];
};

#define ID_SLAVE_X_ANGLE 0x0B0
#define ID_SLAVE_Y_ANGLE 0x0B1
#define ID_SLAVE_Z_ANGLE 0x0B2

static  twai_message_t x_angle = {.identifier = ID_SLAVE_X_ANGLE, .data_length_code = 4,
                                        .data = {0, 0 , 0 , 0 ,0 ,0 ,0 ,0}};
static  twai_message_t y_angle = {.identifier = ID_SLAVE_Y_ANGLE, .data_length_code = 4,
                                        .data = {0, 0 , 0 , 0 ,0 ,0 ,0 ,0}};
static  twai_message_t z_angle = {.identifier = ID_SLAVE_Z_ANGLE, .data_length_code = 4,
                                        .data = {0, 0 , 0 , 0 ,0 ,0 ,0 ,0}};

extern "C" {
    const char *TAG = "main";
    const int MPU_addr=0x68;
    int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

    int minVal=265;
    int maxVal=402;

    float x;
    float y;
    float z;

    void app_main(void){
        initArduino();
        Wire.begin(6, 7);
        Wire.beginTransmission(MPU_addr);
        Wire.write(0x6B);
        Wire.write(0);
        Wire.endTransmission(true);

    pinMode(10, OUTPUT);
        digitalWrite(10, LOW);
        twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_4, GPIO_NUM_5, TWAI_MODE_NORMAL);
        twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
        twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
        if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        ESP_LOGI(TAG, "CAN driver installed");
        } else {
        ESP_LOGI(TAG, "unable to install CAN driver");
        while(1) delay(100);
        }
        if (twai_start() == ESP_OK) {
            ESP_LOGI(TAG, "Driver started\n");
        } else {
            ESP_LOGE(TAG, "Failed to start driver\n");
            while(1) delay(100);
        }

        while(1){
            Wire.beginTransmission(MPU_addr);
        Wire.write(0x3B);
        Wire.endTransmission(false);
        Wire.requestFrom(MPU_addr,14,true);
        AcX=Wire.read()<<8|Wire.read();
        AcY=Wire.read()<<8|Wire.read();
        AcZ=Wire.read()<<8|Wire.read();
        int xAng = map(AcX,minVal,maxVal,-90,90);
        int yAng = map(AcY,minVal,maxVal,-90,90);
        int zAng = map(AcZ,minVal,maxVal,-90,90);

        x= RAD_TO_DEG * (atan2(-yAng, -zAng)+PI);
        y= RAD_TO_DEG * (atan2(-xAng, -zAng)+PI);
        z= RAD_TO_DEG * (atan2(-yAng, -xAng)+PI);

        ESP_LOGI(TAG, "Logging ", "------------");
        ESP_LOGI(TAG, "AngleX = %f", x);
        ESP_LOGI(TAG, "AngleY = %f", y);
        ESP_LOGI(TAG, "AngleZ = %f", z);

    float_byte fbx, fby, fbz;
        fbx.f = x;
        fby.f = y;
        fbz.f = z;

        for(int i = 0; i < 4; i++) {
        x_angle.data[i] = fbx.float_bytes[i];
        y_angle.data[i] = fby.float_bytes[i];
        z_angle.data[i] = fbz.float_bytes[i];
        }

    if (twai_transmit(&x_angle, pdMS_TO_TICKS(1000)) == ESP_OK) {
        ESP_LOGI(TAG, "x angleMessage queued for transmission");
        ESP_LOGI(TAG, "value: %x %x %x", fbx.float_bytes[3], fbx.float_bytes[2], fbx.float_bytes[1], fbx.float_bytes[0]);
        } else {
        ESP_LOGI(TAG, "x angle Failed to queue message for transmission");

        }
        if (twai_transmit(&y_angle, pdMS_TO_TICKS(1000)) == ESP_OK) {
        ESP_LOGI(TAG, "y angleMessage queued for transmission");
        ESP_LOGI(TAG, "value: %x %x %x", fby.float_bytes[3], fby.float_bytes[2], fby.float_bytes[1], fby.float_bytes[0]);
        } else {
        ESP_LOGI(TAG, "y angle Failed to queue message for transmission");
        }
        if (twai_transmit(&z_angle, pdMS_TO_TICKS(1000)) == ESP_OK) {
        ESP_LOGI(TAG, "z angleMessage queued for transmission");
        ESP_LOGI(TAG, "value: %x %x %x", fbz.float_bytes[3], fbz.float_bytes[2], fbz.float_bytes[1], fbz.float_bytes[0]);
        } else {
        ESP_LOGI(TAG, "z angle Failed to queue message for transmission");
        } 
        delay(1000);
        }
    }
}
