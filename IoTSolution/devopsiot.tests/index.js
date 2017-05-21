'use strict';

var Registry = require('azure-iothub').Registry;
var Client = require('azure-iothub').Client;

var connectionString = 'HostName=DevOpsTestHub.azure-devices.net;SharedAccessKeyName=iothubowner;SharedAccessKey=ZV8GDiP1GbnFupM2gCbkPHf/1CG3/KKgfkneF4/VzkA=';
var registry = Registry.fromConnectionString(connectionString);
var client = Client.fromConnectionString(connectionString);
var deviceToTrigger = 'ESP1';

var startTurningOnFan = function (twin) {

    var methodName = "TurnFanOn";

    var methodParams = {
        methodName: methodName,
        payload: null,
        timeoutInSeconds: 30
    };

    client.invokeDeviceMethod(deviceToTrigger, methodParams, function (err, result) {
        if (err) {
            console.error("Direct method error: " + err.message);
        } else {
            console.log("Successfully invoked the device to trigger.");
        }
    });
};

 var queryTwinLastReboot = function() {

     registry.getTwin(deviceToTrigger, function(err, twin){

         if (twin.properties.reported.iothubDM != null)
         {
             if (err) {
                 console.error('Could not query twins: ' + err.constructor.name + ': ' + err.message);
             } else {
                 var lastRebootTime = twin.properties.reported.iothubDM.reboot.lastReboot;
                 console.log('Last reboot time: ' + JSON.stringify(lastRebootTime, null, 2));
             }
         } else 
             console.log('Waiting for device to report last reboot time.');
     });
 };

startTurningOnFan();
setInterval(queryTwinLastReboot, 2000);