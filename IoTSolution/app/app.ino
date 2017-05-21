// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Use Arduino IDE 1.6.8 or later.

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <SPI.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <WiFiClientSecure.h>
#include <WiFiUdp.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#include <AzureIoTHub.h>
#include <AzureIoTUtility.h>
#include <AzureIoTProtocol_HTTP.h>

#include "simplesample_http.h"
#include "config.h"

#include <EEPROM.h>

#define USE_SERIAL Serial

/*
    !!! ssid, pass are injected by config.h
*/
ESP8266WiFiMulti WiFiMulti;

static int LED = LED_BUILTIN;
static int LED_SIGNAL = 5;

static WiFiClientSecure sslClient; // for ESP8266

static AzureIoTHubClient iotHubClient;
char *m_cnnStr, *m_ssid, *m_pass;
uint8_t MAC_array[6];
char MAC_char[18];

char *updateUrl;
bool updateNeeded = false;
int eeprom_used_len = 0;

void setup()
{

    pinMode(LED, OUTPUT);
    pinMode(LED_SIGNAL, OUTPUT);
    pinMode(A0, INPUT);

    initSerial();

    eeprom_used_len = LoadConfig(512);
Serial.print("Check for Update flag!");
    if (LoadUpdateFlag())
    {
        digitalWrite(LED_SIGNAL, HIGH);
        SaveUpdateFlag(false);
        UpdateIt();
    }else
    {
        digitalWrite(LED_SIGNAL, LOW);
    }

    initWifi();
    initTime();

    iotHubClient.begin(sslClient);

    Serial.print("free memory:");
    Serial.println(ESP.getFreeSketchSpace());
}

void loop()
{
    Serial.println("Flash memory:     " + String(ESP.getFlashChipSize()) + " Bytes.");
    Serial.println("Free heap memory: " + String(ESP.getFreeHeap()) + " Bytes.");
    Serial.println("Chip speed:       " + String(ESP.getFlashChipSpeed()) + " Hz.");

    if (updateUrl != 0)
    {
        t_httpUpdate_return ret = ESPhttpUpdate.update(updateUrl);

        switch (ret)
        {
        case HTTP_UPDATE_FAILED:
            Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            break;

        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("HTTP_UPDATE_NO_UPDATES");
            break;

        case HTTP_UPDATE_OK:
            Serial.println("HTTP_UPDATE_OK");
            break;
        }
    }

    simplesample_http_run(A0, connectionString);
    updateUrl = simplesample_http_getUrl();
    if (updateUrl != 0)
    {
        SaveUpdateFlag(true);
        ESP.restart();
    }
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

bool LoadUpdateFlag()
{
    EEPROM.begin(512);
    bool bUpdate = (bool)EEPROM.read(250);
    EEPROM.end();
    if (bUpdate)
        Serial.print("Has update pending");
    return bUpdate;
}

void UpdateIt()
{
    WiFiMulti.addAP(ssid, pass);

    WiFi.macAddress(MAC_array);
    for (int i = 0; i < sizeof(MAC_array); ++i)
    {
        sprintf(MAC_char, "%s%02x", MAC_char, MAC_array[i]);
    }
    Serial.println(MAC_char);

init_connect:
    Serial.println("Connecting for update...");
    // wait for WiFi connection
    if ((WiFiMulti.run() == WL_CONNECTED))
    {

        HTTPClient http;

        USE_SERIAL.print("[HTTP] begin...\n");
        // configure traged server and url
        char *url1 = "http://devops005function.azurewebsites.net/api/BootStrapDevice?mac=";
        int sizeUrls = strlen(url1) + strlen(MAC_char);
        char url[sizeUrls];
        strcpy(url, url1);
        strcat(url, MAC_char);
        puts(url);
        Serial.printf("URL: %s", url);
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
                USE_SERIAL.printf("[Payload 1] %s\r\n", payloadCnnStr.c_str());
                USE_SERIAL.printf("[Payload 2] %s\r\n", payloadFirmware.c_str());
                if (true)
                {
                    String deviceName;
                    String deviceNameContainer;
                    deviceNameContainer = getValue(payloadCnnStr, ';', 1);
                    deviceName = getValue(deviceNameContainer, '=', 1);
                    USE_SERIAL.printf("\r\nDevices Name:%s\r\n", deviceName.c_str());
                    // don't forget to free the string after finished using it

                    t_httpUpdate_return ret = ESPhttpUpdate.update(payloadFirmware);
                    //t_httpUpdate_return  ret = ESPhttpUpdate.update("https://server/file.bin","fingerprint");

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
    else
    {
        delay(2000);
        goto init_connect;
    }
}

int LoadConfig(int sizeEEPROM)
{
    EEPROM.begin(sizeEEPROM);

    Serial.println("[EEPROM]     Reading config...");
    int sizeFromEEPROM = EEPROM.read(0);
    Serial.printf("[EEPROM]     %d bytes to read from EEPROM...\n", sizeFromEEPROM);
    char buffer[sizeFromEEPROM + 1];
    int c = 0;
    do
    {
        c++;
        buffer[c - 1] = EEPROM.read(c);
        /*        Serial.printf("c:%d ",c);
        Serial.println(buffer[c-1]);*/
    } while (c <= sizeFromEEPROM && buffer[c - 1] != '\0');

    //buffer[sizeFromEEPROM] = '\0';
    Serial.printf("[EEPROM]     after reading to buffer: %s\n", buffer);

    if (buffer[0] != '\0')
    {
        char *buffer2 = strdup(buffer);
        connectionString = strdup(getValue(buffer2, '|', 0));
        ssid = strdup(getValue(buffer2, '|', 1));
        pass = strdup(getValue(buffer2, '|', 2));
        Serial.printf("[EEPROM]     new connectionStr:[%s]\n", connectionString);
        Serial.printf("[EEPROM]     new SSID:[%s]\n", ssid);
        Serial.printf("[EEPROM]     new pass:[%s]\n", pass);
        splitString(buffer);
    }
    else
    {
        Serial.println("[EEPROM] is empty");
    }
    EEPROM.end();
    return c;
}

char *SubStr(char *string, int start, int length)
{
    char *buffer;
    strncpy(buffer, string + start, length);
    buffer += '\0';
    return buffer;
}

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
char *getValue(char *data, char separator, int index)
{
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = strlen(data) - 1;
    Serial.printf("maxIndex: %d\n", maxIndex);
    Serial.printf("strIndex[0]:%d strIndex[1]:%d\n", strIndex[0], strIndex[1]);
    for (int i = 0; i <= maxIndex && found <= index; i++)
    {
        // Serial.printf("i: %d, ", i);
        if (data[i] == separator || i == maxIndex)
        {
            found++;
            Serial.printf("found:%d\n", found);

            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
            Serial.printf("strIndex[0]:%d strIndex[1]:%d\n", strIndex[0], strIndex[1]);
        }
    }

    int length = strIndex[1] - strIndex[0];
    Serial.printf("length:%d\n", length);
    char buffer[length];
    if (found > index)
    {
        strncpy(buffer, data + strIndex[0], length);
        buffer[length] = '\0';
    }
    else
    {
        buffer[0] = '\0';
        Serial.println("No token found!");
    }
    return buffer;
}

char *splitString(char *text)
{
    char *copyText = strdup(text);
    char *ptr;
    ptr = strtok(copyText, "|");
    while (ptr != NULL)
    {
        Serial.printf("[Token %s]\n", ptr);
        ptr = strtok(NULL, "|");
    }
    return NULL;
}

void initSerial()
{
    // Start serial and initialize stdout
    Serial.begin(115200);
    Serial.setDebugOutput(false);
}

void initWifi()
{

    // check for the presence of the shield :
    if (WiFi.status() == WL_NO_SHIELD)
    {
        Serial.println("WiFi shield not present");
        // don't continue:
        while (true)
            ;
    }

    // attempt to connect to Wifi network:
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);

connect:
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    if (WiFi.status() != WL_CONNECTED)
    {                           // FIX FOR USING 2.3.0 CORE (only .begin if not connected)
        WiFi.begin(ssid, pass); // connect to the network
    }
    int count = 0;
    for (count = 0; count < 20; count++)
    {
        delay(500);
        toggleLED();
        int wifi_stat = WiFi.status();
        Serial.print(count);
        Serial.print(" WifiStat: ");
        Serial.println(wifi_stat);
        if (wifi_stat == WL_CONNECTED)
        {
            break;
        }
    }

    Serial.println("");

    if (count < 20)
    {
        Serial.println("WiFi connected");
    }
    else
    {
        Serial.println("Client !!!NOT!!! connected");
        goto connect;
    }

    Serial.println("Connected to wifi");
    digitalWrite(LED, LOW);
}

void toggleLED()
{
    digitalWrite(LED, !digitalRead(LED));
}

void initTime()
{

    time_t epochTime;

    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    while (true)
    {
        epochTime = time(NULL);

        if (epochTime == 0)
        {
            Serial.println("Fetching NTP epoch time failed! Waiting 2 seconds to retry.");
            delay(2000);
        }
        else
        {
            Serial.print("Fetched NTP epoch time is: ");
            Serial.println(epochTime);
            break;
        }
    }
}
