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
function ExecutionTime {
    param (
        [ScriptBlock]$ScriptToMeasure
    )
    $stopwatch = [System.Diagnostics.Stopwatch]::new()
    $stopwatch.Start()
    & $ScriptToMeasure
    $stopwatch.Stop()
    Write-Host "Execution Time: $($stopwatch.Elapsed)"
    return $stopwatch.Elapsed
}


<#---------------------------------------------------------------------------------------------#>
function CMAKE()
{
    $FolderName="Build"

    ## warning C4530: C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc 
    ## -G Ninja
    ## -G "Visual Studio 17 2022"
    ## -DVTK_USE_SYSTEM_NETCDF=OFF 
    ## 
    ## - vtknetcdf\config.h(439): note: see previous definition of 'snprintf' -> 
    ##   in 'SRC/VTK/ThirdParty/netcdf/vtknetcdf/config.h.in' 
    ##   modify '#define snprintf _snprintf'
    ## 
    ## -DCMAKE_CXX_FLAGS="/EHsc /D HAVE_SNPRINTF"
    ## C4996 -> -DCMAKE_CXX_FLAGS="/D_CRT_SECURE_NO_WARNINGS"
    ## 
    cmake.exe -S "${EXECUTION_PATH}/SRC" `
            -G "Ninja" `
            -B $FolderName `
            -DCMAKE_INSTALL_PREFIX=Execs `
            -DPARAVIEW_BUILD_QT_GUI=OFF `
            -DCMAKE_CXX_FLAGS="/EHsc " `
            -DCMAKE_CXX_COMPILER=cl  `
            -DCMAKE_C_COMPILER=cl `
            -DPARAVIEW_USE_ICE_T=OFF `
            -DPARAVIEW_USE_MPI=OFF `
            -DBUILD_SHARED_LIBS=OFF `
            -DVTK_Group_ParaViewRendering=OFF `
            -DVTK_USE_X=OFF `
            -DVTK_OPENGL_HAS_OSMESA=OFF `
            -DVTK_OPENGL_HAS_EGL=OFF `
            -DModule_vtkIOExport=OFF `
            -DVTK_BUILD_ALL_MODULES_FOR_TESTS=OFF `
            -DVTK_Group_Rendering=OFF `
            -DVTK_Group_StandAlone=OFF `
            -DVTK_Group_MPI=OFF `
            -DModule_vtkCommonCore=ON `
            -DModule_vtkFiltersGeneral=ON `
            -DVTK_RENDERING_BACKEND=None `
            -DPARAVIEW_ENABLE_VTK_MODULES_AS_NEEDED=FALSE `
            -DModule_vtkVTKm=ON `
            -DModule_vtkAcceleratorsVTKm=ON `
            -DModule_vtkPVVTKExtensionsDefault=ON `
            -DPARAVIEW_ENABLE_COMMANDLINE_TOOLS=OFF `
            -DPARAVIEW_CURRENT_CS_MODULES= 

    ##cmake -S . -B build -DCMAKE_INSTALL_PREFIX="C:/my/install/path"
    ##cmake --build build --config Release -- /m:4
    ##cmake --install build --config Release

    #cmake.exe --build $FolderName --config Release #-- /m:4
    cd $FolderName
    ninja.exe -j4 

    ## cmake.exe --install $FolderName --config Release ## NO 'vtk*.lib's found in $FolderName

}


<#---------------------------------------------------------------------------------------------#>
function COMPILATION
{
    param([string]$FolderName) 

    Write-Host "[COMPILATION] ... " #-NoNewline

    if (Test-Path $FolderName){Remove-Item -Recurse -Force $FolderName}
    New-Item -ItemType Directory -Path ${FolderName}  
    Set-Location -Path ${FolderName} 
    Get-ChildItem

    ExecutionTime { CMAKE } 

    Set-Location -Path ${EXECUTION_PATH} 
} 


<#---------------------------------------------------------------------------------------------#>
function CreateFile 
{
    param([string]$Path, [string]$FileName, [string]$Content)

    $file = Join-Path $Path $FileName 
    Set-Content -Path $file -Value $Content -Encoding UTF8
    Write-Host "'${file}' created"
}



<#---------------------------------------------------------------------------------------------#>
$EXECUTION_PATH=(Get-Location).Path 
Write-Host "[EXECUTION_PATH]:'${EXECUTION_PATH}'" 

try {cl.exe} catch{CL_SETUP}
COMPILATION -FolderName "Compilation"


Set-Location -Path ${EXECUTION_PATH} 
Write-Host "'$($MyInvocation.MyCommand.Name)' !!" 



<#---------------------------------------------------------------------------------------------#>
<#---------------------------------------------------------------------------------------------#>
<#

try {deactivate} catch {Get-Location}
& .\Env1a1\Scripts\Activate.ps1

wsl --shutdown

python.exe -m pip install vtk

python.exe -c "import vtk; print(vtk.__version__)"

python -m pip install pydicom

#$URL_1="https://raw.githubusercontent.com/Kitware/vtk-examples/gh-pages/src/SupplementaryData/Cxx/IO/DicomTestImages.zip"
$URL_1="https://dataverse.harvard.edu/api/access/datafile/:persistentId?persistentId=doi:10.7910/DVN/AI2OXS/KCRFZB"
$OUTPUT_1="DicomTestImages2.zip"

if (Test-Path $OUTPUT_1){Remove-Item -Recurse -Force $OUTPUT_1}
Invoke-WebRequest -Uri $URL_1 -OutFile $OUTPUT_1
# Expand-Archive -Path $OUTPUT_1 




#>