# Parameter help description
Param([string]$rg_name,[string]$region = 'westeurope',[string]$firmware_name,[string]$firmware_path, [string]$functionTamplate, [string]$user, [string]$pwd, [string]$tenant)
if($rg_name -eq ""){
    Write-Host "Please specify parameters."
    exit
}
#--- create resource group

if((Get-AzureRmResourceGroup -ResourceGroupName $rg_name -ErrorAction Ignore) -eq $null){
    Write-Host 'create Resource Group'
    New-AzureRmResourceGroup -Name $rg_name -Location $region -ErrorAction Stop
}
### Storage 
#--- create storage account
$storage_account_name = $rg_name + 'storage'
if((Get-AzureRmStorageAccount -ResourceGroupName $rg_name -Name $storage_account_name -ErrorAction Ignore) -eq $null ){
    Write-Host 'create Storage'
    New-AzureRmStorageAccount -ResourceGroupName $rg_name -Name $storage_account_name -SkuName "Standard_LRS" -Location $region -ErrorAction Stop
}
#> storage-key output
Get-AzureRmStorageAccountKey -ResourceGroupName $rg_name -Name $storage_account_name -OutVariable keys -ErrorAction Stop
$storage_key =  $keys[0][0].Value

#> blob_uri output
Get-AzureRmStorageAccount -ResourceGroupName $rg_name -Name $storage_account_name -OutVariable account -ErrorAction Stop
$blob_uri = $account.PrimaryEndpoints.Blob

#--- creat  blob container
$container_name = 'devops-iot-firmware'
$stoCtx = New-AzureStorageContext -StorageAccountName $storage_account_name -StorageAccountKey $storage_key -ErrorAction Stop
try {
    Write-Host 'create blob container'
    New-AzureStorageContainer -Name $container_name -Context $stoCtx -Permission Blob -ErrorAction Stop    
}Catch{
    Write-Host "storage conainer already exists."
}


### IoT Hub
#--- create iot iothub
Write-Host 'create IoTHub'
$iothub_name = $rg_name + 'hub'
if((Get-AzureRmIotHubConnectionString -ResourceGroupName $rg_name -Name $iothub_name -KeyName iothubowner -ErrorAction Ignore -OutVariable iothub_connectionstring) -eq $null ){
    New-AzureRmIotHub -ResourceGroupName $rg_name -Name $iothub_name -SkuName S1 -Location $region -Units 1 -ErrorAction Stop
    $iothub_connectionstring = Get-AzureRmIotHubConnectionString -ResourceGroupName $rg_name -Name $iothub_name -KeyName iothubowner -ErrorAction Ignore
}

### Web
#--- create app service plan
Write-Host 'create App ServicePlan'
$appserviceplan_name =  $rg_name + 'asp'
if((Get-AzureRmAppServicePlan -ResourceGroupName $rg_name -Name $appserviceplan_name -ErrorAction Ignore)-eq $null){
    New-AzureRmAppServicePlan -Location $region -WorkerSize Small -Tier Shared -Name $appserviceplan_name -ResourceGroupName $rg_name -ErrorAction Stop
}
#--- create website/Function
Write-Host 'create function'
$function_name = $rg_name + 'function'
$aspResId = $(Get-AzureRmAppServicePlan -ResourceGroupName $rg_name -Name $appserviceplan_name -ErrorAction Stop).Id
$storageResId = $(Get-AzureRmStorageAccount -ResourceGroupName $rg_name -StorageAccountName $storage_account_name  -ErrorAction Stop).Id
$params = @{innerAppServPlanName=$appserviceplan_name;functionName=$function_name;innerAppServPlanNameResId=$aspResId; } 
New-AzureRmResourceGroupDeployment -ResourceGroupName $rg_name -Name $rg_name -TemplateFile $functionTamplate -TemplateParameterObject $params -ErrorAction Stop

Write-Host 'appsettings config...'
$AzureWebJobsStorage = "DefaultEndpointsProtocol=https;EndpointSuffix=core.windows.net;AccountName=${storage_account_name};AccountKey=${storage_key}"
$functionApp = Get-AzureRmWebAppSlot -ResourceGroupName $rg_name -Name $function_name -Slot production -ErrorAction Stop
$blob_uri = $blob_uri -replace "https", "http"
$iothub_cnnStr_fixed = ($iothub_connectionstring.PrimaryConnectionString -replace "ShareAccessKeyName","SharedAccessKeyName" )
$settings = @{
    FUNCTIONS_EXTENSION_VERSION="~1";
    WEBSITE_NODE_DEFAULT_VERSION="6.5.0";
    AzureWebJobsStorage=$AzureWebJobsStorage;
    AzureWebJobsDashboard=$AzureWebJobsStorage;
    devopstesthub="";
    devopstesthubRead="";
    devopsiot_internal="";
    devopsIoTHubRegistryConnection=$iothub_cnnStr_fixed ;
    firmwareUrl="${blob_uri}devops-iot-firmware/app.ino.bin"
}
Set-AzureRmWebAppSlot -ResourceGroupName $rg_name -Name $function_name -Slot production -AppSettings $settings  -ErrorAction Stop

#--- create blob
Write-Host 'create blob and upload ' $firmware_name 
$firmware_path = Join-Path $firmware_path $firmware_name
Set-AzureStorageBlobContent -File $firmware_path -Blob $firmware_name -Container $container_name -Context $stoCtx -Force -ErrorAction Stop

Write-Host "##vso[task.setvariable variable=functionName]$function_name"
Write-Host "##vso[task.setvariable variable=iothubcnnstr]$iothub_cnnStr_fixed"