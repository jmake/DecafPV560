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

<#---------------------------------------------------------------------------------------------#>
<#---------------------------------------------------------------------------------------------#>
function Verify-FileHash 
{
  ## Get-FileHash  -Algorithm SHA256 -Path .\Actions\Tests\Dicom\nifti.vti 
  param (
      [Parameter(Mandatory = $true)][string]$FilePath, 
      [Parameter(Mandatory = $true)][string]$ExpectedHash
  )

  if (-Not (Test-Path -Path $FilePath)) {
      Write-Host "[RUNNER] File '${FilePath}' does not exist. Skipping verification."
      return
  }
  
  $actualHash = (Get-FileHash -Path $FilePath -Algorithm SHA256).Hash
  if ($actualHash -ne $ExpectedHash) {
      Write-Host "[RUNNER] '${FilePath}' fails!!"
      Set-Location -Path ${EXECUTION_PATH}
      exit 1
  } else {
      Write-Host "[RUNNER] '${FilePath}' verified!!"
  }
}


function FixPath 
{
    param ([string]$path)
    return ($path -replace '\\', '/')
}

<#---------------------------------------------------------------------------------------------#>
function Extract-AllLinkLibraries {
    param (
        [string]$BuildNinjaPath = "build.ninja",
        [string]$OutputPrefix = "extracted_libs_"
    )

    if (-not (Test-Path $BuildNinjaPath)) {
        Write-Error "File not found: $BuildNinjaPath"
        return
    }

    $lines = Get-Content $BuildNinjaPath
    $count = 0
    $pattern = '^\s*LINK_LIBRARIES\s*=\s*(.+)$'

    foreach ($line in $lines) {
        if ($line -match $pattern) {
            $count++
            $libsLine = $matches[1].Trim()
            $libs = $libsLine -split '\s+' | Where-Object { $_ -ne "" }
            $outFile = "${OutputPrefix}${count}.txt"
            $libs | Set-Content $outFile
            Write-Output "Saved $($libs.Count) libraries to $outFile"
        }
    }

    if ($count -eq 0) {
        Write-Output "No LINK_LIBRARIES lines found in $BuildNinjaPath"
    }
}


<#---------------------------------------------------------------------------------------------#>
<#---------------------------------------------------------------------------------------------#>
function RunmeTest 
{
  param([string]$Path) 
  Set-Location -Path ${Path}  

  $TestPath="runme.exe"
  if (Test-Path  $TestPath){Remove-Item -Recurse -Force  $TestPath}

  ##Set-Location -Path ${EXECUTION_PATH}; exit  
}


function COMPILATION
{
  param([string]$RootPath) 

  Write-Host "[COMPILATION] ... " #-NoNewline

  Set-Location -Path $SCRIPT_PATH

  $AssestPath="$SCRIPT_PATH/Assets"
  if (Test-Path  $AssestPath){Remove-Item -Recurse -Force  $AssestPath}

  $CompilationPath="$SCRIPT_PATH/Build"
  if (Test-Path  $CompilationPath){Remove-Item -Recurse -Force  $CompilationPath}
  New-Item -ItemType Directory -Path ${CompilationPath}  
  Set-Location -Path ${CompilationPath} 
  Get-ChildItem

#Invoke-WebRequest -Uri "https://raw.githubusercontent.com/numpy/numpy/main/tools/swig/numpy.i" -OutFile "numpy.i"
#$NUMPY_INCLUDE_DIR = python -c "import numpy; print(numpy.get_include())"
#Write-Host "Numpy include directory: $NUMPY_INCLUDE_DIR"
##$NUMPY_INCLUDE_DIR="C:\users\zvl_2\appdata\local\ov\pkg\create-2023.2.5\extscache\omni.kit.pip_archive-0.0.0+ece658d9.wx64\pip_prebundle\numpy\core\include"

  $CMAKE_PATH = FixPath $CMAKE_PATH 
  $SOURCES_DIR = FixPath $SOURCES_DIR 

  cmake.exe -S "$CMAKE_PATH" -Wno-dev `
  -G "Ninja" `
  -B . `
  -DCMAKE_INSTALL_PREFIX="$ExecutablePath" `
  -DParaView_DIR="$PV_DIR" `
  -DSOURCES_DIR="$SOURCES_DIR" `
  -DZLIB_ROOT="$ZLIB_ROOT" 
  
  cmake.exe --build . # --verbose  #--config Release #--verbose 

  Extract-AllLinkLibraries -BuildNinjaPath "build.ninja" -OutputPrefix "libs_"

  cp "${ZLIB_ROOT}\bin\zd.dll" . 
  
  .\SpicyTech.exe

  Set-Location -Path ${EXECUTION_PATH} 
} 


<#---------------------------------------------------------------------------------------------#>
<#---------------------------------------------------------------------------------------------#>
$SCRIPT_PATH=$PSScriptRoot
Write-Host "[SCRIPT_PATH]:'${SCRIPT_PATH}'" 

$EXECUTION_PATH=(Get-Location).Path 
Write-Host "[EXECUTION_PATH]:'${EXECUTION_PATH}'" 

try {cl.exe} catch{CL_SETUP2 -msvcVersion "14.38.33130"}
#SWIG_SETUP

$PV_DIR="F:\Download\DecafPV560_Release\Compilation\Build"

$ZLIB_ROOT="F:/z2025_1/Dicom/zlib/Execs"

$CMAKE_PATH="${SCRIPT_PATH}\Plugging" 
$SOURCES_DIR="${SCRIPT_PATH}\Sources" 


COMPILATION #-RootPath $SCRIPT_PATH


Set-Location -Path ${EXECUTION_PATH} 
Write-Host "'$($MyInvocation.MyCommand.Name)' !!" 
<#---------------------------------------------------------------------------------------------#>