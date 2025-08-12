Clear-Host


<#---------------------------------------------------------------------------------------------#>
<#---------------------------------------------------------------------------------------------#>
function CL_SETUP2 {
    param (
        [string]$msvcVersion = "" 
    )

    Write-Output "CL_SETUP ..."

    $VSWHERE = "C:\ProgramData\Chocolatey\bin\vswhere.exe"

    # Get all VS installations that include VC tools
    $allInstalls = & $VSWHERE -all -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -format json | ConvertFrom-Json

    $match = $null

    foreach ($inst in $allInstalls) {
        $toolPath = Join-Path $inst.installationPath "VC\Tools\MSVC\$msvcVersion"
        if (Test-Path $toolPath) {
            $match = $inst
            break
        }
    }

    if (-not $match) {
        Write-Error "MSVC version $msvcVersion not found in any Visual Studio installation."
        return
    }

    $VSTOOLS = Join-Path $match.installationPath 'Common7\Tools\vsdevcmd.bat'

    Write-Output "[Using Visual Studio]: $($match.installationPath)"
    Write-Output "[Using MSVC Toolset] : $msvcVersion"

    if (Test-Path $VSTOOLS) {
        cmd /s /c " `"$VSTOOLS`" -arch=x64 -host_arch=x64 && set" | 
            Where-Object { $_ -match '(\w+)=(.*)' } | 
            ForEach-Object {
                $null = New-Item -Force -Path "Env:\$($Matches[1])" -Value $Matches[2]
            }
    }

    cl.exe
    cmake.exe --version
    ninja.exe --version
}


function REMOVER {
    param (
        [string]$Path
    )

    if (Test-Path $Path) {
        Remove-Item -Recurse -Force $Path
        echo "Removed : '${Path}' "
    }
}


<#---------------------------------------------------------------------------------------------#>
<#---------------------------------------------------------------------------------------------#>
function COMPILATION_SIMPLET
{
    param([string]$RootPath) 

    REMOVER Build 
    REMOVER Release

    # | ------ | ------------------------------------------ |
    # | `.dll` | The dynamic library itself                 |
    # | `.lib` | Import library for linking at compile time |
    # | `.exp` | Intermediate file listing exported symbols |
    Get-ChildItem -Path . -Filter *.dll -File -ErrorAction SilentlyContinue | Remove-Item -Force
    Get-ChildItem -Path . -Filter *.exp -File -ErrorAction SilentlyContinue | Remove-Item -Force
    Get-ChildItem -Path . -Filter *.lib -File -ErrorAction SilentlyContinue | Remove-Item -Force

    mkdir Release 
    cd Release/ 

    # /LD -> dynamic-link library 
    cl.exe /LD /EHsc  `
    /D EXTERNALMATH_EXPORTS `
    ..\ExternalLibrary.cpp `
    /IMPLIB:DynamicLibrary.lib `
    /Fe:DynamicLibrary.dll `
    /Fo:DynamicLibrary.obj

    cd .. 

    ## IMPORTANT : Always restart UE after rebuilding your DLL when using __declspec(dllimport) import.
    Get-ChildItem -Path . -Filter *.obj -File -ErrorAction SilentlyContinue | Remove-Item -Force
} 

<#---------------------------------------------------------------------------------------------#>
function COMPILATION_CMAKE  
{
    param([string]$RootPath) 

    REMOVER Build 
    REMOVER Release

    cmake . -G "Visual Studio 17 2022" -A x64 -B Build 
    cmake --build Build --config Release

    ls Release 

    REMOVER DynamicLibrary.dll 
    cp Release\DynamicLibrary.dll E:\MyProject5\Binaries\Win64
    ls E:\MyProject5\Binaries\Win64\DynamicLibrary.dll

}


function MAIN
{
    #COMPILATION_SIMPLET
    COMPILATION_CMAKE 
} 


<#---------------------------------------------------------------------------------------------#>
<#---------------------------------------------------------------------------------------------#>
$SCRIPT_PATH=$PSScriptRoot
Write-Host "[SCRIPT_PATH]:'${SCRIPT_PATH}'" 

$EXECUTION_PATH=(Get-Location).Path 
Write-Host "[EXECUTION_PATH]:'${EXECUTION_PATH}'" 

try {cl.exe} catch{CL_SETUP2 -msvcVersion "14.38.33130"}

MAIN 

Set-Location -Path ${EXECUTION_PATH} 
Write-Host "'$($MyInvocation.MyCommand.Name)' !!" 
<#---------------------------------------------------------------------------------------------#>