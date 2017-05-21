param([string]$fromPath='.\Deployments', [string]$toPath='.\node_modules\gulp-common\')

$source = Join-Path $fromPath 'arduino-esp8266-nodemcuv2.js'
$target = Join-Path $toPath 'arduino-esp8266-nodemcuv2.js'

Copy-Item $source -Destination $target
