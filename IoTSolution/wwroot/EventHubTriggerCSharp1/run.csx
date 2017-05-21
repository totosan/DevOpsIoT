#r "Newtonsoft.Json"

using System;
using System.Text;
using Microsoft.Azure.Devices.Client;
using Microsoft.ServiceBus.Messaging;

public static void Run(string iotHubMessage,out string outputEventHubMessage, TraceWriter log)
{
    double B = 3975.0;
    log.Info($"C# Event Hub trigger function processed a message: {iotHubMessage}");
    var msg = Newtonsoft.Json.JsonConvert.DeserializeObject<MessageTransformed>(iotHubMessage);

    /*double resistance = (float)((1023.0 - msg.Temperature) * 10000.0 / (double)msg.Temperature);
    double temperature = 1.0 / (Math.Log(resistance / 10000.0) / B + 1 / 298.15) - 273.15;
    msg.Temperature = (float)temperature;*/
    msg.Temperature = (float)Thermister(msg.Temperature);
    msg.convTime = DateTime.Now;
    var msgSerialized = Newtonsoft.Json.JsonConvert.SerializeObject(msg);

    log.Info($"C# Event Hub trigger function processed a message: {msgSerialized}");
    outputEventHubMessage = msgSerialized;
}

public static double Thermister(float adc){
  double Temp;
 Temp = Math.Log(((10240000/adc) - 10000));
 Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp ))* Temp );
 Temp = Temp - 273.15;              // Convert Kelvin to Celsius
 //Temp = (Temp * 9.0)/ 5.0 + 32.0; // Celsius to Fahrenheit - comment out this line if you need Celsius
 return Temp;   
}

public static void SendMessageBack(string ms){
    var iothubUrl = CreateConnectionString();
    var client = DeviceClient.CreateFromConnectionString(iothubUrl);
    var commandMessage = new Message(Encoding.ASCII.GetBytes(ms));
    client.SendEventAsync(commandMessage);
}

public static string CreateConnectionString(){
    var eventhubConn = GetEnvironmentVariable("devopstesthubRead");
    var parts = eventhubConn.Split(';');
    var iothubname=parts[1].Split('=')[1];
    var newConnectionString = $"HostName={iothubname};{parts[2]};{parts[3]}";
    return newConnectionString;
}

public static string GetEnvironmentVariable(string name)
{
    return name + ": " + 
        System.Environment.GetEnvironmentVariable(
            name, EnvironmentVariableTarget.Process
            );
}

public class MessageTransformed{
    public string DeviceId;
    public int BatteryLevel;
    public float Temperature;
    public string dtime;
    public DateTime convTime;
}