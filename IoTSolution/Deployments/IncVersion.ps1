$versioninfo = Get-Content("..\app\version.h")
$strVersion = [double][regex]::Match($versioninfo,'"(.*?)"').Groups[1].Value + 0.01
$cultUS = New-Object System.Globalization.CultureInfo("en-US")
$newVersion = '"'+$strVersion.ToString($cultUS)+'"'
$versioninfo = $versioninfo -replace '"(.*?)"',$newVersion
Set-Content "..\app\version.h" $versioninfo