[Parameter(Mandatory=$True)]
[string]$pathToZipFile
[string]$apiUrl
[string]$username
[string]$password

$base64AuthInfo = [Convert]::ToBase64String([Text.Encoding]::ASCII.GetBytes(("{0}:{1}" -f $username,$password)))
$userAgent = "powershell/1.0"

#$apiUrl = "https://<yourfunction>.scm.azurewebsites.net/api/zip/site/wwwroot"
invoke-RestMethod -Uri $apiUrl -Headers @{Authorization=("Basic {0}" -f $base64AuthInfo)} -Method PUT -filePath $pathToZipFile -UserAgent $userAgent