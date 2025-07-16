Clear-Host

<#---------------------------------------------------------------------------------------------#>
function CL_SETUP
{
  echo "CL_SETUP ..."
   
  $VSWHERE="C:\ProgramData\Chocolatey\bin\vswhere.exe"

  $VSTOOLS = &($VSWHERE) -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
  Write-Output "[VSTOOLS]:'$VSTOOLS' "

  if($VSTOOLS) 
  {
    $VSTOOLS = join-path $VSTOOLS 'Common7\Tools\vsdevcmd.bat'
    if (test-path $VSTOOLS) 
    {
      cmd /s /c " ""$VSTOOLS""  -arch=x64 -host_arch=x64 $args && set" | where { $_ -match '(\w+)=(.*)' } | 
      foreach{$null = new-item -force -path "Env:\$($Matches[1])" -value $Matches[2] }
    }
  }
  
  cl.exe 
  cmake.exe --version 
  ninja.exe --version 
}


function SWIG_SETUP 
{
    try {
        swig.exe -version 
        if ($LASTEXITCODE -ne 0) {throw "'swig.exe -version' failed with exit code $LASTEXITCODE"} 
    } 
    catch{$env:PATH="$env:PATH;F:\z2025_1\PhysX\Swig\Swig431;"}    
}


<#---------------------------------------------------------------------------------------------#>
function PYTHON_SETUP 
{
    # Download -> https://www.python.org/ftp/python/3.13.5/python-3.13.5-amd64.exe 
    # then, install with "debug symbols" and "debug binaries" ... 
    # find . | grep python313 -> F:\z2025_1\Python3135\libs\python3_d.lib | python313.lib 

    F:\z2025_1\Python3135\python.exe --version 

    F:\z2025_1\Python3135\python.exe -m venv ENV1a1

    .\ENV1a1\Scripts\Activate.ps1 

    python.exe -m pip install numpy

    #python.exe -c "import numpy" 
    python.exe -c "import numpy; print( numpy.get_include() )"

    Invoke-WebRequest -Uri "https://raw.githubusercontent.com/numpy/numpy/main/tools/swig/numpy.i" -OutFile "numpy.i"

}


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

$SOURCES_DIR="F:/z2025_1/Dicom/DecafPV560/Actions/Tests/Dicom"
$ZLIB_ROOT="F:/z2025_1/Dicom/zlib/Execs"

  Invoke-WebRequest -Uri "https://raw.githubusercontent.com/numpy/numpy/main/tools/swig/numpy.i" -OutFile "numpy.i"

  cmake.exe -S "$SCRIPT_PATH/Sources" -Wno-dev `
  -G "Ninja" `
  -B . `
  -DCMAKE_INSTALL_PREFIX="$ExecutablePath" `
  -DParaView_DIR="$PV_DIR" `
  -DSOURCES_DIR="$SOURCES_DIR" `
  -DZLIB_ROOT="$ZLIB_ROOT" `

#  -DBUILD_PYTHON_BINDINGS=ON

#Set-Location -Path ${EXECUTION_PATH}; exit  

  cmake.exe --build . #--config Release #--verbose 

  #RunmeTest -Path $AssestPath

  Set-Location -Path ${EXECUTION_PATH} 
} 


<#---------------------------------------------------------------------------------------------#>
$SCRIPT_PATH=$PSScriptRoot
Write-Host "[SCRIPT_PATH]:'${SCRIPT_PATH}'" 

$EXECUTION_PATH=(Get-Location).Path 
Write-Host "[EXECUTION_PATH]:'${EXECUTION_PATH}'" 

try {cl.exe} catch{CL_SETUP}
SWIG_SETUP


#$PV_DIR="${EXECUTION_PATH}\Compilation\Build" # :) 
$PV_DIR="F:\Download\DecafPV560_SharedRelease\Compilation\Build"
$PV_DIR="F:\Download\DecafPV560_Release\Compilation\Build"

COMPILATION #-RootPath $SCRIPT_PATH


Set-Location -Path ${EXECUTION_PATH} 
Write-Host "'$($MyInvocation.MyCommand.Name)' !!" 



<#---------------------------------------------------------------------------------------------#>