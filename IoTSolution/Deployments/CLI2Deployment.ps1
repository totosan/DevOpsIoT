# Parameter help description
Param([string]$rg_name,[string]$region = 'westeurope',[string]$firmware_name,[string]$firmware_path)
$firmware_path = Join-Path $firmware_path $firmware_name

#--- create resource group
Write-Host 'create Resource Group'
az group create -n $rg_name -l $region

### Storage 
#--- create storage account
Write-Host 'create Storage'
$storage_account_name = $rg_name + 'storage'
az storage account create -n $storage_account_name -l $region --sku Standard_LRS -g $rg_name
#> storage-key output
az storage account keys list -g $rg_name -n $storage_account_name --query "[1].value" | Select-Object -OutVariable storage-key
#> blob_uri output
az storage account show -g $rg_name -n $storage_account_name --query "primaryEndpoints.blob" | Select-Object -OutVariable blob_uri

#--- creat  blob container
Write-Host 'create blob container'
$container_name = 'devops-iot-firmware'
az storage container create -n $container_name --account-name $storage_account_name --account-key ${storage-key}

#--- create blob
Write-Host 'create blob and upload ' $firmware_name 
az storage blob upload -f $firmware_path -n $firmware_name -c $container_name --account-name $storage_account_name --account-key ${storage-key}

### DB
#--- create documentDB
#Write-Host 'create documentDB'
#az documentDB create -n $rg_name -l $region

### IoT Hub
#--- create iot iothub
Write-Host 'create IoTHub'
$iothub_name = $rg_name + 'hub'
az iot hub create -g $rg_name -n $iothub_name -l $region --sku S1
#> iothub_connectionstring output
az iot hub show-connection-string -g $rg_name -n $iothub_name --query "connectionString" | Select-Object -OutVariable iothub_connectionstring

### Web
#--- create app service plan
Write-Host 'create App ServicePlan'
$appserviceplan_name =  $rg_name + 'asp'
az appservice plan create -n $appserviceplan_name -l $region -g $rg_name --sku D1

#--- create website/Function
Write-Host 'create function'
$function_name = $rg_name + 'function'
az resource list --query "[?contains(id,`'$appserviceplan_name`')].id | [0]" | Select-Object -OutVariable aspResId
az resource list --query "[?contains(id,`'$storage_account_name`')].id | [0]" | Select-Object -OutVariable storageResId
az group deployment create --template-file ".\FunctionTemplate.json" --parameters "{\`"resourceGroupName\`":{\`"value\`":\`"$rg_name\`"},\`"innerAppServPlanName\`":{\`"value\`":\`"$appserviceplan_name\`"},\`"functionName\`":{\`"value\`":\`"$function_name\`"},\`"storageAccountName\`":{\`"value\`":\`"$storage_account_name\`"},\`"storageAccountResId\`":{\`"value\`":\`"$storageResId\`"},\`"innerAppServPlanNameResId\`":{\`"value\`":\`"$aspResId\`"}}" -g $rg_name

Write-Host 'appsettings config...'
az storage account show-connection-string -g $rg_name -n $storage_account_name --query "connectionString" | Select-Object -OutVariable AzureWebJobsStorage
az appservice web config appsettings update -g $rg_name -n $function_name --settings "AzureWebJobsStorage=$AzureWebJobsStorage"
az appservice web config appsettings update -g $rg_name -n $function_name --settings "AzureWebJobsDashboard=$AzureWebJobsStorage"
az appservice web config appsettings update -g $rg_name -n $function_name --settings "WEBSITE_NODE_DEFAULT_VERSION": "6.5.0"
az appservice web config appsettings update -g $rg_name -n $function_name --settings "FUNCTIONS_EXTENSION_VERSION": "~1"

az appservice web config appsettings update -g $rg_name -n $function_name --settings ''
az appservice web config appsettings update -g $rg_name -n $function_name --settings ''
az appservice web config appsettings update -g $rg_name -n $function_name --settings ''
az appservice web config appsettings update -g $rg_name -n $function_name --settings "devopsIoTHubRegistryConnection=${iothub_connectionstring}"
az appservice web config appsettings update -g $rg_name -n $function_name --settings "firmwareUrl=${blob_uri}"

#--- output variables
Write-Host "##vso[task.setvariable variable=functionName]$function_name"