function ReplaceToken ($dropFolder, $fileName) {
    write-Host "using file $($fileName)"
    Write-Host "replacing token for drop folder $($dropFolder)"

    $fileContent = Get-Content $fileName
    $fileContent= $fileContent.Replace('__drop__',$dropFolder)
    
    Write-Host "saving changes to $($fileName)"
    Set-Content $fileName $fileContent
}

function CreateFolderIfNotExists ($folder) {
    if($(Test-Path $folder) -eq $false){
    New-Item $folder -ItemType Directory
}
}