param([string]$fromPath, [string]$dropFolder)
$userProfile=$ENV:USERPROFILE

. .\Deployments\Libs.ps1

$targetPathGettingStarted = Join-Path $userProfile '.iot-hub-getting-started'
write-Host "modifiing preferences.txt"

$arduinoPath = Join-Path $targetPathGettingStarted '\arduino-1.6.11\lib'
#$arduino15Path = Join-Path $env:APPDATA '..\local\Arduino15'
$sourcePathPref = Join-Path $fromPath '.\Deployments\preferences_local.txt'
#$sourcePathPref15 = Join-Path $fromPath '.\Deployments\preferences_15.txt'
$targetFilename = 'preferences.txt'

#ReplaceToken $dropFolder $sourcePathPref15
ReplaceToken $dropFolder $sourcePathPref

CreateFolderIfNotExists $dropFolder

#$dest15 = Join-Path $arduino15Path $targetFilename
$dest=Join-Path $arduinoPath $targetFilename
#Copy-Item $sourcePathPref15 -Destination $dest15 -Force
Copy-Item $sourcePathPref -Destination $dest -Force