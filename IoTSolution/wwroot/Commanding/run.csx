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
    int number;
    int.TryParse(req.GetQueryNameValuePairs()
        .FirstOrDefault(q => string.Compare(q.Key, "number", true) == 0)
        .Value,out number);

    log.Info($"parameter number was {number}");
    // Get request body
    
    //dynamic data = await req.Content.ReadAsAsync<object>();

    //mac = mac ?? data?.mac;

    var connectionString = GetEnvironmentVariable("devopsIoTHubRegistryConnection").Split(':')[1].Trim();
    var firmwareUrl = GetEnvironmentVariable("firmwareUrl").Replace(": ", ";").Split(';')[1].Trim();


    registryManager = RegistryManager.CreateFromConnectionString(connectionString);
    client = ServiceClient.CreateFromConnectionString(connectionString);

    var devices = (await registryManager.GetDevicesAsync(100)).ToArray();
    int i=0;
        for(i=0;i<devices.Count();i++){
    var message = new Message(System.Text.ASCIIEncoding.ASCII.GetBytes(@"{""Name"":""UpdateFirmware"",""Parameters"":{""url"":""http://devops005storage.blob.core.windows.net/devops-iot-firmware/app.ino.bin"",""version"":""2.10""}}"));
        if(i==number)
            break;
        if(devices[i].Status == DeviceStatus.Enabled){
            log.Info($"Device Id: {devices[i].Id}");
            await client.SendAsync(devices[i].Id,message);
        }
    }
    
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