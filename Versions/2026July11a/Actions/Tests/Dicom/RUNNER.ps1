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
function COMPILATION
{
  Write-Host "[COMPILATION] ... " #-NoNewline

  Set-Location -Path $SELF_PATH

  $ExecutablePath="Execs"
  if (Test-Path  $ExecutablePath){Remove-Item -Recurse -Force  $ExecutablePath}

  $CompilationPath="Build"
  if (Test-Path  $CompilationPath){Remove-Item -Recurse -Force  $CompilationPath}
  ##New-Item -ItemType Directory -Path ${FolderName}  
  ##Set-Location -Path ${FolderName} 
  ##Get-ChildItem

  $ZLIB_ROOT="F:/z2025_1/Dicom/zlib/Execs"

  cmake.exe -S . `
  -G "Ninja" `
  -B  $CompilationPath `
  -DCMAKE_INSTALL_PREFIX="$ExecutablePath" `
  -DParaView_DIR="$PV_DIR" `
  -DZLIB_ROOT="$ZLIB_ROOT"


  cmake.exe --build  $CompilationPath --config Release 
#Set-Location -Path ${EXECUTION_PATH}; exit  

  ##cmake.exe --build  $CompilationPath --target test --config Release

  cmake.exe --install  $CompilationPath --config Release

  & ".\${ExecutablePath}\bin\SpicyTech.exe"

  ## Get-FileHash -Algorithm SHA256 -Path .\Actions\Tests\Dicom\nifti.vti 
  Verify-FileHash `
  -FilePath "nifti.vti" `
  -ExpectedHash "5A3C264E13B887F16B69DFF91495136BB37C3CB05A7020A3B8EA5C9E7BAA7191" 

  Set-Location -Path ${EXECUTION_PATH} 
} 


<#---------------------------------------------------------------------------------------------#>
$EXECUTION_PATH=(Get-Location).Path 
Write-Host "[EXECUTION_PATH]:'${EXECUTION_PATH}'" 

try {cl.exe} catch{CL_SETUP}

$PV_DIR="${EXECUTION_PATH}\Compilation\Build"
$SELF_PATH="${EXECUTION_PATH}\Actions\Tests\Dicom" 

COMPILATION 


Set-Location -Path ${EXECUTION_PATH} 
Write-Host "'$($MyInvocation.MyCommand.Name)' !!" 



<#---------------------------------------------------------------------------------------------#>