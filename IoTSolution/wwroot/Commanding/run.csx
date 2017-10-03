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
    string mac = req.GetQueryNameValuePairs()
        .FirstOrDefault(q => string.Compare(q.Key, "mac", true) == 0)
        .Value;

    // Get request body
    
    //dynamic data = await req.Content.ReadAsAsync<object>();

    //mac = mac ?? data?.mac;

    var connectionString = GetEnvironmentVariable("devopsIoTHubRegistryConnection").Split(':')[1].Trim();
    var firmwareUrl = GetEnvironmentVariable("firmwareUrl").Replace(": ", ";").Split(';')[1].Trim();

    var message = new Message(System.Text.ASCIIEncoding.ASCII.GetBytes(@"{""Name"":""UpdateFirmware"",""Parameters"":{""url"":""http://devops005storage.blob.core.windows.net/devops-iot-firmware/app.ino.bin"",""version"":""2.06""}}"));

    registryManager = RegistryManager.CreateFromConnectionString(connectionString);
    client = ServiceClient.CreateFromConnectionString(connectionString);

    var devices = await registryManager.GetDevicesAsync(100);
    foreach(var device in devices){
        log.Info($"Device Id: {device.Id}");
        await client.SendAsync(device.Id,message);
    }
    
    log.Info("job done");
    return req.CreateResponse(HttpStatusCode.BadRequest, "Please pass a name on the query string or in the request body");
}

public static string GetEnvironmentVariable(string name)
{
    return name + ": " +
        System.Environment.GetEnvironmentVariable(
            name, EnvironmentVariableTarget.Process
            );
}