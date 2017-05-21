using System.Net;
using System;
using System.Threading.Tasks;
using Microsoft.Azure.Devices;
using Microsoft.Azure.Devices.Common.Exceptions;

static RegistryManager registryManager;

public static async Task<HttpResponseMessage> Run(HttpRequestMessage req, TraceWriter log)
{
    log.Info("C# HTTP trigger function processed a request.");

    // parse query parameter
    string mac = req.GetQueryNameValuePairs()
        .FirstOrDefault(q => string.Compare(q.Key, "mac", true) == 0)
        .Value;

    // Get request body
    dynamic data = await req.Content.ReadAsAsync<object>();

    mac = mac ?? data?.mac;

    var connectionString = GetEnvironmentVariable("devopsIoTHubRegistryConnection").Split(':')[1].Trim();
    var firmwareUrl = GetEnvironmentVariable("firmwareUrl").Replace(": ", ";").Split(';')[1].Trim();

    if (mac != null)
    {
        var deviceName = "ESP" + mac;
        log.Info("Creating device instance for " + deviceName);
        registryManager = RegistryManager.CreateFromConnectionString(connectionString);
        log.Info("firmwareUrl: " + firmwareUrl);

        var cnp = connectionString.Split(';');
        var deviceConnectionString = $"{cnp[0]};DeviceId={deviceName};SharedAccessKey=";

        var device = AddDeviceAsync(deviceName, connectionString, log).GetAwaiter().GetResult();

        deviceConnectionString = $"{deviceConnectionString}{device.Authentication.SymmetricKey.PrimaryKey}&{firmwareUrl}";
        log.Info("sending device connection string: " + deviceConnectionString);
        return req.CreateResponse(HttpStatusCode.OK, deviceConnectionString);
    }
    return req.CreateResponse(HttpStatusCode.BadRequest, "Please pass a name on the query string or in the request body");
}

static async Task<Device> AddDeviceAsync(String deviceId, string connectionString, TraceWriter log)
{
    Device device;

    try
    {
        device = await registryManager.AddDeviceAsync(new Device(deviceId));
    }
    catch (DeviceAlreadyExistsException)
    {
        device = await registryManager.GetDeviceAsync(deviceId);
    }

    Console.WriteLine("Generated device key: {0}", device.Authentication.SymmetricKey.PrimaryKey);
    return device;
}


public static string GetEnvironmentVariable(string name)
{
    return name + ": " +
        System.Environment.GetEnvironmentVariable(
            name, EnvironmentVariableTarget.Process
            );
}