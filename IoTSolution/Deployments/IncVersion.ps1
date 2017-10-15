# Specifies a path to one or more locations.
param($version)

$versioninfo = Get-Content("..\app\version.h")
<# $strVersion = [double][regex]::Match($versioninfo,'"(.*?)"').Groups[1].Value + 0.01
$cultUS = New-Object System.Globalization.CultureInfo("en-US")
 $newVersion = '"'+$strVersion.ToString($cultUS)+'"'#>
 $newVersion = '"'+$version+'"'
 $versioninfo = $versioninfo -replace '"(.*?)"',$newVersion
Set-Content "..\app\version.h" $versioninfo