param([string]$fromPath)
$userProfile=$ENV:USERPROFILE

. .\Deployments\Libs.ps1

$targetPathGettingStarted = Join-Path $userProfile '.iot-hub-getting-started'

#find .iot-hub-getting-started path and copy biSettings.json to work arround Microsoft question
$sourcePathBiSettings = Join-Path $fromPath '.\Deployments\biSettings.json'

CreateFolderIfNotExists $targetPathGettingStarted

Copy-Item $sourcePathBiSettings -Destination $targetPathGettingStarted -Force
