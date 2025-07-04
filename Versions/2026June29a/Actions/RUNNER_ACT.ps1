<# 
FROM : 
    F:\Github\runner_act.ps1 
SEE : 
    https://medium.com/@jmake/how-to-build-and-package-pixars-usdview-application-4b20aa4e8537
    https://dev.to/rgomezp/how-to-set-up-an-eas-local-build-on-github-actions-1l0i

VmmemWSL is running as 'act'?
#>
Clear-Host
try {deactivate} catch {Get-Location}


<#---------------------------------------------------------------------------------------------#> 
<#---------------------------------------------------------------------------------------------#> 
## 1) 
$XYZ="ghp"
$TOKEN4="${XYZ}_W5bUgXWKzrW72fUmVQKLxbLBYyLZzB2bNWPc"

## 2) 
## https://expo.dev/accounts/spicytech/settings/access-tokens
# ExpoTest1 
$EXPO_TOKEN="V4csoE9Yues8WhpPLVOBwpAhtXEfR0InBSEfOjA9"

## 3) 
## https://github.com/settings/tokens 
$URL="https://jmake:$TOKEN4@github.com/jmake/Expo1a1.git"

## 4) 
#$WORKFLOW_NAME="testing_libraries.yaml" # app :) 
#$WORKFLOW_NAME="concept2v1.yaml"            # app_2025May06b :) 
$WORKFLOW_NAME="concept2v1.yaml"             # app_2025May12a :) 


## 5) (if memory fails, try 'wsl --shutdown' )
docker info


<#---------------------------------------------------------------------------------------------#> 
<#---------------------------------------------------------------------------------------------#> 


function INSTALL_ACT 
{
    choco.exe --version ## v2.3.0
    ##choco.exe install act-cli ## As administrator 
    act.exe --version ## 0.2.68 

    ## act.exe --action-cache-path 
}


function INSTALL_EXECUTE  
{
    param(
        [string]$FolderName
    )     

    if (Test-Path $FolderName){Remove-Item -Recurse -Force $FolderName}

    git.exe clone $URL $FolderName

    Set-Location $FolderName

    act.exe `
    --secret "EXPO_TOKEN=$EXPO_TOKEN" `
    --secret "ACTIONS_RUNTIME_TOKEN=$TOKEN4" `
    --secret "TOKEN4=$TOKEN4" `
    --artifact-server-path $EXECUTION_PATH `
    --platform windows-2019=-self-hosted `
    --workflows .github\workflows\$WORKFLOW_NAME  `
    -e ..\event.json

    Set-Location -Path $EXECUTION_PATH

    #Get-ChildItem -Path .\1\ArtifactName*
}



function RELEASE_WITH_ARTIFACT 
{
    param(
        [string]$Artifact 
    )   

    ## choco install gh -y 
    ## gh.exe auth login
    $TAG_NAME="V0.0.0"

    ##gh.exe release list --json tagName | ConvertFrom-Json | ForEach-Object {
    ##    $tagName = $_.tagName
    ##    gh release delete $tagName --yes
    ##}
    gh.exe release delete $TAG_NAME --yes

    git.exe fetch --tags
    git.exe push --delete origin $TAG_NAME
    git.exe tag --delete $TAG_NAME
    ##git.exe tag | ForEach-Object { git push --delete origin $_ }
    ##git.exe tag | ForEach-Object { git tag --delete $_ }

    $REPO=$(gh repo view --json owner,name --template "{{.owner.login}}/{{.name}}")

    $ME = "$($env:USERNAME)@$($env:COMPUTERNAME)"
    
    gh.exe release create $TAG_NAME `
    --repo $REPO `
    --title "Releasing ${TAG_NAME}" `
    --notes "Released at '$(Get-Date)' by '$ME' "

    #git.exe config user.name "${{ github.actor }}"
    #git.exe config user.email "${{ github.actor_id }}+${{ github.actor }}@users.noreply.github.com"
    gh.exe release upload $TAG_NAME $Artifact
}


function CREATE_RELEASE  
{
    param(
        [string]$FolderName, 
        [string]$Artifact
    )  

    Set-Location $FolderName

    git.exe status 
    RELEASE_WITH_ARTIFACT -Artifact $Artifact 

    Set-Location -Path $EXECUTION_PATH
}


$SCRIPT_PATH = Split-Path ($MyInvocation.MyCommand.Path)
Write-Host "[SCRIPT_PATH]:'${SCRIPT_PATH}'" 

$EXECUTION_PATH=(Get-Location).Path 
Write-Host "[EXECUTION_PATH]:'${EXECUTION_PATH}'" 

$REPO_NAME="XYZ"
INSTALL_EXECUTE -FolderName $REPO_NAME
##CREATE_RELEASE -FolderName $REPO_NAME -Artifact "..\_1\ArtifactName\ArtifactName.zip"

Write-Host "'" ($MyInvocation.MyCommand.Name) "' ..." 


#&"C:\Program Files\Google\Chrome\Application\chrome.exe" https://github.com/jmake/USDSpicyTech/releases

##
## act.exe --quiet --platform windows-2019=-self-hosted --workflows .github\workflows\usd_compilation.yaml workflow_dispatch
##
## act.exe --platform windows-latest=-self-hosted 
## act --workflows 
## act --list
## act.exe --dryrun # 
##  C:\Users\zvl_2\AppData\Local\act\actrc 
## 
