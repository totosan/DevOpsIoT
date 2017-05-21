#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <SPI.h>

int a;
int del = 1000; // duration between temperature readings
float temperature;
int B = 3975; //B value of the thermistor
float resistance;

void initSerial()
{
    // Start serial and initialize stdout
    Serial.begin(115200);
    Serial.setDebugOutput(true);
}

void setup()
{
    initSerial();
    pinMode(A0, INPUT);
}

void loop()
{
    a = analogRead(0);
    resistance = (float)(1023 - a) * 10000 / a;
    temperature = 1 / (log(resistance / 10000) / B + 1 / 298.15) - 273.15;
    Serial.println(temperature);
    delay(del);
}