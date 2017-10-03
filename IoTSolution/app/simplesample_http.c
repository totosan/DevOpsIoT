// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include <Arduino.h>

#include <time.h>
#include <sys/time.h>

#include "config.h"
#include "version.h"

#include "sdk/schemaserializer.h"

#ifdef ARDUINO
#include "AzureIoTHub.h"
#else
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/platform.h"
#include "serializer.h"
#include "iothub_client_ll.h"
#include "iothubtransportmqtt.h"
#endif

/* 
   !!!  connectionString is injected by config.h
*/

static bool g_continueRunning;
char *m_updateUrl;
static const short B = 3975; //B value of the thermistor*/
static int m_uploadInterval = 10000;
static int LED = 5;

typedef struct EVENT_INSTANCE_TAG
{
    IOTHUB_MESSAGE_HANDLE messageHandle;
    size_t messageTrackingId; // For tracking the messages within the user callback.
} EVENT_INSTANCE;

// Define the Model
BEGIN_NAMESPACE(TestDataNS);

/*DECLARE_STRUCT(DeviceProperties,
               ascii_char_ptr, DeviceId,
               _Bool, HubEnabledState);*/

DECLARE_MODEL(TestOMeter,
              /* Event data */
              WITH_DATA(ascii_char_ptr, DeviceId),
              WITH_DATA(int, BatteryLevel),
              WITH_DATA(int, Temperature),
              WITH_DATA(ascii_char_ptr, dtime),
              WITH_DATA(ascii_char_ptr, Version),

              /* Device Meta*/
              /*              WITH_DATA(DeviceProperties, DevProps),
              WITH_DATA(ascii_char_ptr, ObjectType),
              WITH_DATA(_Bool, IsSimulatedDevice),
              WITH_DATA(ascii_char_ptr, Version),
              WITH_DATA(ascii_char_ptr_no_quotes, Commands),*/

              /* commands, triggered by exteranl*/
              WITH_ACTION(TurnFanOn, int, ID),
              WITH_ACTION(TurnFanOff, int, ID),
              WITH_ACTION(UpdateFirmware, ascii_char_ptr, url, ascii_char_ptr, version)
            );

END_NAMESPACE(TestDataNS);

//******************************
//turning Fan On
//******************************
EXECUTE_COMMAND_RESULT TurnFanOn(TestOMeter *device, int ID)
{
    (void)device;
    (void)printf("Turning fan on.\r\n");
    return EXECUTE_COMMAND_SUCCESS;
}

//******************************
//turning Fan Off
//******************************
EXECUTE_COMMAND_RESULT TurnFanOff(TestOMeter *device, int ID)
{
    (void)device;
    (void)printf("Turning fan off.\r\n");
    return EXECUTE_COMMAND_SUCCESS;
}

char *getFileNameFromPath(char *path)
{
    for (size_t i = strlen(path) - 1; i >= 0; i--)
    {
        if (path[i] == '/')
        {
            return &path[i + 1];
        }
    }
    return path;
}

//******************************
//setting air resistance position
//******************************
EXECUTE_COMMAND_RESULT UpdateFirmware(TestOMeter *device, char *url, char *version)
{
    (void)device;
    (void)printf("Receive command for updating FW from URL %s with version %s.\r\n", url, version);
    char *path = url;
    char *file = getFileNameFromPath(path);

    int compareResult = strcmp(version, device->Version);
    printf("compare restult between device->Version (%s) and version from command (%s) is %d \r\n", device->Version, version, compareResult);
    if (compareResult > 0)
    {
        /* Updater *updater_instance = Updater_create();
        Updater_do(&updater_instance, url);*/

        g_continueRunning = false;
        m_updateUrl = strdup(url);
    }
    else
    {
        printf("Firmware is up to date with version %s \r\n", version);
    }
    return EXECUTE_COMMAND_SUCCESS;
}

METHODRETURN_HANDLE UpdateFirmware_Method(TestOMeter *device){
    (void)device;
    (void)printf("Updating firmware by DirectMethod.\r\n");

    METHODRETURN_HANDLE result = MethodReturn_Create(1, "{\"Message\":\"Updating firmware by DirectMethod\"}");
    return result;
}

//******************************
//callback method for receiving messages
//******************************
void sendCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void *userContextCallback)
{
    unsigned int messageTrackingId = (unsigned int)(uintptr_t)userContextCallback;
    digitalWrite(LED, HIGH);
    delay(1000);
    digitalWrite(LED, LOW);

    // (void)printf("Message Id: %u Received.\r\n", messageTrackingId);

    // (void)printf("Result Call Back Called! Result is: %s \r\n", ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
}

//******************************
//sending messages
//******************************
static void sendMessage(IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle, const unsigned char *buffer, size_t size)
{
    static unsigned int messageTrackingId;
    IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromByteArray(buffer, size);
    if (messageHandle == NULL)
    {
        printf("unable to create a new IoTHubMessage\r\n");
    }
    else
    {
        if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messageHandle, sendCallback, (void *)(uintptr_t)messageTrackingId) != IOTHUB_CLIENT_OK)
        {
            printf("failed to hand over the message to IoTHubClient");
        }
        else
        {
            printf("IoTHubClient accepted the message for delivery\r\n");
        }
        IoTHubMessage_Destroy(messageHandle);
    }
    free((void *)buffer);
    messageTrackingId++;
}

/*********************************
this function "links" IoTHub to the serialization library
***********************************/
static IOTHUBMESSAGE_DISPOSITION_RESULT IoTHubMessage(IOTHUB_MESSAGE_HANDLE message, void *userContextCallback)
{
    IOTHUBMESSAGE_DISPOSITION_RESULT result;
    const unsigned char *buffer;
    size_t size;
    if (IoTHubMessage_GetByteArray(message, &buffer, &size) != IOTHUB_MESSAGE_OK)
    {
        printf("unable to IoTHubMessage_GetByteArray\r\n");
        result = EXECUTE_COMMAND_ERROR;
    }
    else
    {

        /*buffer is not zero terminated*/
        char *temp = malloc(size + 1);
        if (temp == NULL)
        {
            printf("failed to malloc\r\n");
            result = EXECUTE_COMMAND_ERROR;
        }
        else
        {
            EXECUTE_COMMAND_RESULT executeCommandResult;
            memcpy(temp, buffer, size);
            temp[size] = '\0';
            printf("message received: %s \r\n", temp);

            executeCommandResult = EXECUTE_COMMAND(userContextCallback, temp);
            result = IOTHUBMESSAGE_ACCEPTED;
            /*            result =
                (executeCommandResult == EXECUTE_COMMAND_ERROR) ?
                IOTHUBMESSAGE_ABANDONED : (executeCommandResult == EXECUTE_COMMAND_SUCCESS) ? 
                    IOTHUBMESSAGE_ACCEPTED : IOTHUBMESSAGE_REJECTED;*/

            /*printf("result of execution is %d\r\n", result);*/
            free(temp);
        }
    }
    return result;
}

//******************************
// getting Temperature from device
//******************************
static float getTemperatureFromSensor(int pin)
{
    int read = (unsigned int)analogRead(pin);

    float resistance = (float)((1023 - read) * 10000 / read);
    float temperature = 1 / (log(resistance / 10000) / B + 1 / 298.15) - 273.15;
    return temperature;
}

/**********************************
// Device call back method
**********************************/
/*static int DeviceMethodCallback(const char *method_name, const unsigned char *payload, size_t size, unsigned char **response, size_t *resp_size, void *userContextCallback)
{
    (void)userContextCallback;

    printf("\r\nDevice Method called\r\n");
    printf("Device Method name:    %s\r\n", method_name);
    printf("Device Method payload: %.*s\r\n", (int)size, (const char *)payload);

    int status = 200;
    char *RESPONSE_STRING = "{ \"Response\": \"This is the response from the device\" }";
    printf("\r\nResponse status: %d\r\n", status);
    printf("Response payload: %s\r\n\r\n", RESPONSE_STRING);

    *resp_size = strlen(RESPONSE_STRING);
    if ((*response = malloc(*resp_size)) == NULL)
    {
        status = -1;
    }
    else
    {
        memcpy(*response, RESPONSE_STRING, *resp_size);
    }
    g_continueRunning = false;
    return status;
}*/

static int DeviceMethodCallback(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* resp_size, void* userContextCallback)
{
    int result;

    /*this is step  3: receive the method and push that payload into serializer (from below)*/
    char* payloadZeroTerminated = (char*)malloc(size + 1);
    if (payloadZeroTerminated == 0)
    {
        printf("failed to malloc\r\n");
        *resp_size = 0;
        *response = NULL;
        result = -1;
    }
    else
    {
        (void)memcpy(payloadZeroTerminated, payload, size);
        payloadZeroTerminated[size] = '\0';

        /*execute method - userContextCallback is of type deviceModel*/
        METHODRETURN_HANDLE methodResult = EXECUTE_METHOD(userContextCallback, method_name, payloadZeroTerminated);
        free(payloadZeroTerminated);

        if (methodResult == NULL)
        {
            printf("failed to EXECUTE_METHOD\r\n");
            *resp_size = 0;
            *response = NULL;
            result = -1;
        }
        else
        {
            /* get the serializer answer and push it in the networking stack*/
            const METHODRETURN_DATA* data = MethodReturn_GetReturn(methodResult);
            if (data == NULL)
            {
                printf("failed to MethodReturn_GetReturn\r\n");
                *resp_size = 0;
                *response = NULL;
                result = -1;
            }
            else
            {
                result = data->statusCode;
                if (data->jsonValue == NULL)
                {
                    char* resp = "{}";
                    *resp_size = strlen(resp);
                    *response = (unsigned char*)malloc(*resp_size);
                    (void)memcpy(*response, resp, *resp_size);
                }
                else
                {
                    *resp_size = strlen(data->jsonValue);
                    *response = (unsigned char*)malloc(*resp_size);
                    (void)memcpy(*response, data->jsonValue, *resp_size);
                }
            }
            MethodReturn_Destroy(methodResult);
        }
    }
    return result;
}

char *simplesample_http_getUrl()
{
    return m_updateUrl;
}

//******************************
//Main function running http messages (sending and receiving msgs)
//
//******************************
void simplesample_http_run(int pin, const char *cnnStr, const char *deviceId)
{
    g_continueRunning = true;
    if (platform_init() != 0)
    {
        printf("Failed to initialize the platform.\r\n");
    }
    else
    {
        if (serializer_init(NULL) != SERIALIZER_OK)
        {
            (void)printf("Failed on serializer_init\r\n");
        }
        else
        {
            int receiveContext = 0;
            printf("Try to connect to IoT Hub with cnnStr %s\r\n", cnnStr);
            IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(cnnStr, MQTT_Protocol);

            int avgBatteryLevel = 10;
            srand((unsigned int)time(NULL));

            if (iotHubClientHandle == NULL)
            {
                (void)printf("Failed on IoTHubClient_LL_Create\r\n");
            }
            else
            {
                // Because it can poll "after 9 seconds" polls will happen
                // effectively at ~10 seconds.
                // Note that for scalabilty, the default value of minimumPollingTime
                // is 25 minutes. For more information, see:
                // https://azure.microsoft.com/documentation/articles/iot-hub-devguide/#messaging
                unsigned int minimumPollingTime = 9;
                TestOMeter *myTestOMeter;

                if (IoTHubClient_LL_SetOption(iotHubClientHandle, "MinimumPollingTime", &minimumPollingTime) != IOTHUB_CLIENT_OK)
                {
                    printf("failure to set option \"MinimumPollingTime\"\r\n");
                }
                printf("Creating model instance..\r\n");
                
                myTestOMeter = CREATE_MODEL_INSTANCE(TestDataNS, TestOMeter);
                if (myTestOMeter == NULL)
                {
                    (void)printf("Failed on CREATE_MODEL_INSTANCE\r\n");
                }
                else
                {
                    printf("setup direct method callback...\r\n");
                    
                     if (IoTHubClient_LL_SetDeviceMethodCallback(iotHubClientHandle, DeviceMethodCallback, myTestOMeter) != IOTHUB_CLIENT_OK)
                    {
                        (void)printf("ERROR: IoTHubClient_LL_SetDeviceMethodCallback..........FAILED!\r\n");
                    }
                    else  
                    if (IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, IoTHubMessage, myTestOMeter) != IOTHUB_CLIENT_OK)
                    {
                        printf("unable to IoTHubClient_SetMessageCallback\r\n");
                    }
                    else
                    {

                        myTestOMeter->DeviceId = (char *)deviceId;
                        myTestOMeter->Version = (char *)version;
                        printf("This is device %s \r\n Firmwareversion: %s\r\n", (char *)deviceId, (char *)version);

                        do
                        {

                            myTestOMeter->BatteryLevel = avgBatteryLevel; /* + (rand() % 4 + 2);*/
                            myTestOMeter->Temperature = analogRead(pin);
                            myTestOMeter->dtime = "";

                            {
                                unsigned char *destination;
                                size_t destinationSize;
                                if (SERIALIZE(&destination, &destinationSize,
                                              myTestOMeter->DeviceId,
                                              myTestOMeter->BatteryLevel,
                                              myTestOMeter->Temperature,
                                              myTestOMeter->dtime) != CODEFIRST_OK)
                                {
                                    (void)printf("Failed to serialize\r\n");
                                }
                                else
                                {
                                    /* char *temp = malloc(destinationSize + 1);
                                    memcpy(temp, destination, destinationSize);
                                    temp[destinationSize] = "\0";*/
                                    /*printf("Message: %s\r\n", temp);*/
                                    IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromByteArray(destination, destinationSize);
                                    if (messageHandle == NULL)
                                    {
                                        printf("unable to create a new IoTHubMessage\r\n");
                                    }
                                    else
                                    {
                                        if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messageHandle, sendCallback, (void *)1) != IOTHUB_CLIENT_OK)
                                        {
                                            printf("failed to hand over the message to IoTHubClient\r\n");
                                        }
                                        else
                                        {
                                            printf("IoTHubClient accepted the message for delivery\r\n");
                                            /*digitalWrite(5, !digitalRead(5));*/
                                        }

                                        IoTHubMessage_Destroy(messageHandle);
                                    }
                                    free(destination);
                                }
                            }

                            /* wait for commands */

                            IoTHubClient_LL_DoWork(iotHubClientHandle);
                            ThreadAPI_Sleep(m_uploadInterval);

                        } while (g_continueRunning);
                    }
                    DESTROY_MODEL_INSTANCE(myTestOMeter);
                }
                IoTHubClient_LL_Destroy(iotHubClientHandle);
            }
            serializer_deinit();
        }
        platform_deinit();
    }
}
