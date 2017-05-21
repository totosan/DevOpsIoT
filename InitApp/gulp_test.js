var Tests = function () { };
var sentMessageCount =0;

Tests.prototype.UpdateFwTest = function (config) {
    var Message = require('azure-iot-common').Message;
    
    var client = require('azure-iothub').Client.fromConnectionString(config.iot_hub_connection_string);

    // Get device id from IoT device connection string
    var getDeviceId = function (connectionString) {
        var elements = connectionString.split(';');
        var dict = {};
        for (var i = 0; i < elements.length; i++) {
            var kvp = elements[i].split('=');
            dict[kvp[0]] = kvp[1];
        }
        return dict.DeviceId;
    };
    var targetDevice = getDeviceId(config.iot_device_connection_string);

    // Build cloud-to-device message with message Id
    var buildMessage = function (messageId) {
        return new Message(JSON.stringify({ 'Name': 'UpdateFirmware', 
        'Parameters': { 'url': 'http://functioncaca969abc71.blob.core.windows.net/devops-iot-firmware/app.ino.bin', 'version':'1.9' } }));
    };

    // Construct and send cloud-to-device message to IoT Hub
    var sendMessage = function () {
        sentMessageCount++;
        var message = buildMessage(sentMessageCount);
        console.log('[IoT Hub] Sending message #' + sentMessageCount + ': ' + message.getData() + '\n');
        client.send(targetDevice, message, sendMessageCallback);
    };

    // Start another run after message is sent out
    var sendMessageCallback = function (err) {
        if (err) {
            console.log(err);
            console.error('[IoT Hub] Sending message error: ' + err.message);
        } else {
            console.log('Update firmware commanded');
            client.close(closeConnectionCallback);
        }
    };

    var run = function () {
        setTimeout(sendMessage, 1000);
    };

    // Log information to console when closing connection to IoT Hub
    var closeConnectionCallback = function (err) {
        if (err) {
            console.error('[IoT Hub] Close connection error: ' + err.message + '\n');
        } else {
            console.log('[IoT Hub] Connection closed\n');
        }
    };

    // Start running this sample after getting connected to IoT Hub.
    // If there is any error, log the error message to console.
    var connectCallback = function (err) {
        if (err) {
            console.error('[IoT Hub] Fail to connect: ' + err.message + '\n');
        } else {
            console.log('[IoT Hub] Client connected\n');
            // Wait for 5 seconds so that device gets connected to IoT Hub.
            setTimeout(run, 5000);
        }
    };

    client.open(connectCallback);
}

exports.Tests = new Tests();