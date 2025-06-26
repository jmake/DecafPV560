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
function Verify-FileHash 
{
  ## Get-FileHash  -Algorithm SHA256 -Path .\Actions\Tests\Dicom\nifti.vti 
  param (
      [Parameter(Mandatory = $true)][string]$FilePath, 
      [Parameter(Mandatory = $true)][string]$ExpectedHash
  )

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

  csc.exe /platform:x64 /target:exe /unsafe /out:runme.exe `
  $SCRIPT_PATH\Assets\Plugins\*.cs  `
  $SCRIPT_PATH\Sources\runme.cs 

  .\runme.exe
  
  ## Get-FileHash -Algorithm SHA256 -Path .\Unity\Assets\nifti.vti
  Verify-FileHash `
  -FilePath "nifti.vti" `
  -ExpectedHash "2059CB1D1959D97C647945315288DCF3BF403AB625BE5346D85A947BB921B32D" 

  ## Get-FileHash -Algorithm SHA256 -Path .\Unity\Assets\cutter_yz.vtp
  Verify-FileHash `
  -FilePath "cutter_yz.vtp" `
  -ExpectedHash "AC9FC1B9771A51E5082C67DEF34A0B8674E5FE909F6AC5C595EC48976CE972F3" 

  ## Get-FileHash -Algorithm SHA256 -Path .\Unity\Assets\contour_a.vtp
  Verify-FileHash `
  -FilePath "contour_a.vtp" `
  -ExpectedHash "A933EAEFBC8891C2E74D3F6D25BDA19CEEAE4C9837BBDA49F5C8F0123E10219E" 

  echo "F:\z2025_1\ParaView600\bin\paraview.exe .\Unity\Assets\nifti.vti"

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

  cmake.exe -S "$SCRIPT_PATH/Sources" -Wno-dev `
  -G "Ninja" `
  -B . `
  -DCMAKE_INSTALL_PREFIX="$ExecutablePath" `
  -DParaView_DIR="$PV_DIR" `
  -DSOURCES_DIR="$SOURCES_DIR" `
  -DZLIB_ROOT="$ZLIB_ROOT"

#Set-Location -Path ${EXECUTION_PATH}; exit  

  cmake.exe --build . --config Release #--verbose 

  RunmeTest -Path $AssestPath

  Set-Location -Path ${EXECUTION_PATH} 
} 


<#---------------------------------------------------------------------------------------------#>
$SCRIPT_PATH=$PSScriptRoot
Write-Host "[SCRIPT_PATH]:'${SCRIPT_PATH}'" 

$EXECUTION_PATH=(Get-Location).Path 
Write-Host "[EXECUTION_PATH]:'${EXECUTION_PATH}'" 

try {cl.exe} catch{CL_SETUP}
SWIG_SETUP


$PV_DIR="${EXECUTION_PATH}\Compilation\Build"

COMPILATION #-RootPath $SCRIPT_PATH


Set-Location -Path ${EXECUTION_PATH} 
Write-Host "'$($MyInvocation.MyCommand.Name)' !!" 



<#---------------------------------------------------------------------------------------------#>