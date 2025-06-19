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


<#---------------------------------------------------------------------------------------------#>
<#---------------------------------------------------------------------------------------------#>
function COMPILATION
{
    #param([string] $CompilationPath) 

    Write-Host "[COMPILATION] ... " #-NoNewline

    $ExecutablePath="Execs"
    if (Test-Path  $ExecutablePath){Remove-Item -Recurse -Force  $ExecutablePath}

    $CompilationPath="Build"
    if (Test-Path  $CompilationPath){Remove-Item -Recurse -Force  $CompilationPath}
    #New-Item -ItemType Directory -Path ${FolderName}  
    #Set-Location -Path ${FolderName} 
    #Get-ChildItem

    $VTK_PATH="F:\z2025_1\Dicom\DecafPV560\Compilation\Build\VTK"
    cmake.exe -S . `
    -G "Ninja" `
    -B  $CompilationPath `
    -DVTK_DIR="$VTK_PATH" `
    -DCMAKE_INSTALL_PREFIX="$ExecutablePath" `
    
    cmake.exe --build  $CompilationPath --config Release 

    cmake.exe --build  $CompilationPath --target test --config Release

    cmake.exe --install  $CompilationPath --config Release

    & ".\${ExecutablePath}\bin\HelloVTK.exe"

    Set-Location -Path ${EXECUTION_PATH} 
} 


<#---------------------------------------------------------------------------------------------#>
$EXECUTION_PATH=(Get-Location).Path 
Write-Host "[EXECUTION_PATH]:'${EXECUTION_PATH}'" 

try {cl.exe} catch{CL_SETUP}
COMPILATION #-FolderName "Build"


Set-Location -Path ${EXECUTION_PATH} 
Write-Host "'$($MyInvocation.MyCommand.Name)' !!" 



<#---------------------------------------------------------------------------------------------#>