$includeSetup = Get-Content .\.vscode\c_cpp_properties.json
$includeSetup = $includeSetup -replace "__path__", $env:USERPROFILE
$includeSetup = $includeSetup -replace '\\','/'
Set-Content -Path .\.vscode\c_cpp_properties.json -Value $includeSetup

.\Deploy.ps1
.\Deployments\DeployBiSettings.ps1 -fromPath .
.\Deployments\DeployPrefs.ps1 -fromPath . -dropFolder .\Temp