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
<#---------------------------------------------------------------------------------------------#>
function COMPILATION
{
  param([string]$RootPath) 

  Write-Host "[COMPILATION] ... " #-NoNewline

  Set-Location -Path $SCRIPT_PATH

  #$ExecutablePath="Execs"
  #if (Test-Path  $ExecutablePath){Remove-Item -Recurse -Force  $ExecutablePath}

  $CompilationPath="Build"
  if (Test-Path  $CompilationPath){Remove-Item -Recurse -Force  $CompilationPath}
  New-Item -ItemType Directory -Path ${CompilationPath}  
  Set-Location -Path ${CompilationPath} 
  Get-ChildItem

$SOURCES_DIR="F:\z2025_1\Dicom\DecafPV560\Actions\Tests\Dicom"
$SOURCES_DIR = $SOURCES_DIR -replace '\\', '/'

  cmake.exe -S "$SCRIPT_PATH/Sources" `
  -G "Ninja" `
  -B . `
  -DCMAKE_INSTALL_PREFIX="$ExecutablePath" `
  -DParaView_DIR="$PV_DIR" `
  -DSOURCES_DIR="$SOURCES_DIR"

  cmake.exe --build . --config Release 
  cd $SCRIPT_PATH

  csc.exe /platform:x64 /target:exe /unsafe /out:Test/runme.exe `
  Assets\Plugins\*.cs  `
  Sources\runme.cs 

  .\runme.exe

  #cmake.exe --build  $CompilationPath --config Release 
  #cmake.exe --build  $CompilationPath --target test --config Release
  #cmake.exe --install  $CompilationPath --config Release
  #& ".\${ExecutablePath}\bin\SpicyTech.exe"

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