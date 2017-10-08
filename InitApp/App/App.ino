/**
 * BasicHTTPClient.ino
 *
 *  Created on: 24.05.2015
 *
 */
#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#include <EEPROM.h>
#include <FS.h>
#include "config.h"

#define USE_SERIAL Serial

ESP8266WiFiMulti WiFiMulti;

uint8_t MAC_array[6];
char MAC_char[18];

static int LED = 2;

void setup()
{

    USE_SERIAL.begin(115200);
    //USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    pinMode(LED,OUTPUT);
    
    EraseEEPROM();

    for (uint8_t t = 4; t > 0; t--)
    {
        USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }
  
    WiFiMulti.addAP(ssid, pass);

    WiFi.macAddress(MAC_array);
    for (int i = 0; i < sizeof(MAC_array); ++i)
    {
        sprintf(MAC_char, "%s%02x", MAC_char, MAC_array[i]);
    }
    Serial.print("MAC: ");
    Serial.println(MAC_char);
/*     Serial.println(ssid);
    Serial.println(pass);
 */}

void loop()
{
    Serial.println("Connecting...");
    // wait for WiFi connection
    if ((WiFiMulti.run() == WL_CONNECTED))
    {
        Serial.println("Connected!");
        
        HTTPClient http;

        Serial.println("5sec before starting update...!");
        delay(5000);

        USE_SERIAL.print("[HTTP] begin...\n");
        // configure traged server and url
        char *url1 = "http://devops005function.azurewebsites.net/api/BootStrapDevice?mac=";
        int sizeUrls = strlen(url1) + strlen(MAC_char);
        char url[sizeUrls];
        strcpy(url, url1);
        strcat(url, MAC_char);
        puts(url);
        Serial.printf("URL: %s\r\n", url);
        http.begin(url); //HTTP

        USE_SERIAL.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();

        // httpCode will be negative on error
        if (httpCode > 0)
        {
            // HTTP header has been send and Server response header has been handled
            USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

            if (httpCode == HTTP_CODE_OK)
            {
                String payload = http.getString();
                //USE_SERIAL.println(payload);
                String trimmedPayload = payload.substring(1, payload.length() - 1);

                String payloadCnnStr = getValue(trimmedPayload, '&', 0);
                String payloadFirmware = getValue(trimmedPayload, '&', 1);
                USE_SERIAL.printf("[ConnectionString] %s\r\n", payloadCnnStr.c_str());
                USE_SERIAL.printf("[Firmware-Endpoint] %s\r\n", payloadFirmware.c_str());
                if (SaveConfig(payloadCnnStr, ssid, pass))
                {
                    LoadConfig(512);
                    EEPROM.end();
                    delay(2000);
                    String deviceName;
                    String deviceNameContainer;
                    deviceNameContainer = getValue(payloadCnnStr, ';', 1);
                    deviceName = getValue(deviceNameContainer, '=', 1);
                    USE_SERIAL.printf("\r\nDevices Name:%s\r\n", deviceName.c_str());
                    // don't forget to free the string after finished using it
                    USE_SERIAL.printf("starting firmware update....\r\n");
                    
                    SaveUpdateFlag(true);
                    t_httpUpdate_return ret = ESPhttpUpdate.update(payloadFirmware);
                    //t_httpUpdate_return  ret = ESPhttpUpdate.update("https://server/file.bin");

                    switch (ret)
                    {
                    case HTTP_UPDATE_FAILED:
                        USE_SERIAL.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
                        break;

                    case HTTP_UPDATE_NO_UPDATES:
                        USE_SERIAL.println("HTTP_UPDATE_NO_UPDATES");
                        break;

                    case HTTP_UPDATE_OK:
                        USE_SERIAL.println("HTTP_UPDATE_OK");
                        delay(10000);
                        break;
                    }
                }
            }
        }
        else
        {
            USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    }

    delay(5000);
}

void SaveUpdateFlag(bool isUpdate)
{
    EEPROM.begin(512);
    EEPROM.write(250, (int)isUpdate);
    if (isUpdate)
        Serial.print("Update flag written");
    EEPROM.commit();
    EEPROM.end();
}

bool SaveConfig(String connectionStr, const char *ssid, const char *pwd)
{
    int size = strlen(connectionStr.c_str()) + strlen(ssid) + strlen(pwd) + 2;
    char all[size];
    strcpy(all, connectionStr.c_str());
    strcat(all, "|");
    strcat(all, ssid);
    strcat(all, "|");
    strcat(all, pwd);
    puts(all);

    Serial.printf("all to EEPROM: %s\r\n", all);
    EEPROM.begin(size + 2);
    EEPROM.write(0, size);
    Serial.printf("%d Bytes to write to EEPROM:\r\n", size + 2);
    int i = 1;
    for (i = 1; i < size + 1; i++)
    {
        EEPROM.write(i, all[i - 1]);
    }
    Serial.printf("counter after writing to EEPROM: %d\r\n", i);

    EEPROM.write(i + 1, '\0');

    if (EEPROM.commit())
    {
        Serial.printf("Config saved to EEPROM\r\n");
        return true;
    }
    return false;
}

void EraseEEPROM(){
    Serial.print("Erasing EEPROM...");
    EEPROM.begin(512);
    // write a 0 to all 512 bytes of the EEPROM
    for (int i = 0; i < 512; i++)
      EEPROM.write(i, 0);
  
    // turn the LED on when we're done
    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);
    EEPROM.end();
    Serial.print("EEPROM erased!");
}

void LoadConfig(int sizeEEPROM)
{
    int c = 0;
    int sizeFromBuffer = EEPROM.read(0);
    Serial.printf("EEPROM-Size:%d", sizeFromBuffer);
    char buffer[sizeFromBuffer + 1];
    do
    {
        buffer[c] = EEPROM.read(c + 1);
/*         Serial.print(buffer[c]);
 */        c++;
    } while (c < sizeFromBuffer);
    buffer[sizeFromBuffer] = '\0';
    /* USE_SERIAL.printf("\r\n[Checked config:%s]\r\n", buffer); */
}

bool SaveFile(char *cnn, char *ssid, char *pwd)
{
    // Open config file for writing.
    File configFile = SPIFFS.open("/cl_conf.txt", "w");
    if (!configFile)
    {
        Serial.println("Failed to open cl_conf.txt for writing");

        return false;
    }

    // Save SSID and PSK.
    configFile.println(cnn);
    configFile.println(ssid);
    configFile.println(pass);

    configFile.close();

    return true;
} // saveConfig

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++)
    {
        if (data.charAt(i) == separator || i == maxIndex)
        {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }

    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
