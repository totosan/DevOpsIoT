using System.Net;
using System;
using System.Threading.Tasks;
using Microsoft.Azure.Devices;
using Microsoft.Azure.Devices.Common.Exceptions;

static RegistryManager registryManager;
static ServiceClient client;
static JobClient jobClient;

public static async Task<HttpResponseMessage> Run(HttpRequestMessage req, TraceWriter log)
{
   log.Info("C# HTTP trigger function processed a request.");

    // parse query parameter
    
    string action = req.GetQueryNameValuePairs()
        .FirstOrDefault(q => string.Compare(q.Key, "Action", true) == 0)
        .Value;
    string deviceId = req.GetQueryNameValuePairs()
        .FirstOrDefault(q => string.Compare(q.Key, "DeviceId", true) == 0)
        .Value;

    log.Info($"parameter was {action} for device {deviceId}");

    var connectionString = GetEnvironmentVariable("devopsIoTHubRegistryConnection").Split(':')[1].Trim();
    
    registryManager = RegistryManager.CreateFromConnectionString(connectionString);
    client = ServiceClient.CreateFromConnectionString(connectionString);
    var method = action == "On" ? 
    @"{""Name"":""TurnFanOn"",""Parameters"":{""ID"":0}}":
    @"{""Name"":""TurnFanOff"",""Parameters"":{""ID"":0}}";
        var message = new Message(System.Text.ASCIIEncoding.ASCII.GetBytes(method));
        await client.SendAsync(deviceId,message);
    
    
    log.Info("job done");
    return req.CreateResponse(HttpStatusCode.OK, "Command sent!");
}

public static string GetEnvironmentVariable(string name)
{
    return name + ": " +
        System.Environment.GetEnvironmentVariable(
            name, EnvironmentVariableTarget.Process
            );
}