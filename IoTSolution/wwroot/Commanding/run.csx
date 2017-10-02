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

    jobClient = JobClient.CreateFromConnectionString(connectionString);

    log.Info("All Env-Vars retrueved");
    
    var dmethod = new CloudToDeviceMethod("UpdateFirmware");
    dmethod.SetPayloadJson(
         @"{
             Parameters : {
                 'url':'http://devops005storage.blob.core.windows.net/devops-iot-firmware/app.ino.bin',
                 'version':'2.05'
             }
         }");
    
    log.Info($"Method generated");
    
    JobResponse result = await jobClient.ScheduleDeviceMethodAsync(Guid.NewGuid().ToString(),
        "deviceId='ESP5ccf7f2cb436'",
        dmethod,
        DateTime.Now,
        10);
    
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