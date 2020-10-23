PAAKAT -- Insitu analysis 
=====================================


Motivation
----------

Scalable, Data analysis, Visualization 

# Table of contents
1. [Compilation](#Compilation)
    1. [VTK / PARAVIEW / CATALYST](#subparagraph1)
2. [PAAKAT](#PAAKAT)


------------
## Compilation <a name="Compilation"></a>
Compilation is divided in three main steps : 

1. **VTK** / **PARAVIEW** / **CATALYST**  
2. **PAAKAT** 

The Visualization Tookit [(VTK)][VTK] is used to manipulation and displaying of scientific data.
 
[Paraview][PARAVIEW] is a data analysis and visualization application.  

ParaView Catalyst [(Catalyst)][CATALYST] is an *in situ* use case library

[NEK5K][NEK5K] is a fast and scalable high-order solver for computational fluid dynamics

PAAKAT is an *adaptador*.  

[VTK]: https://vtk.org/   
[PARAVIEW]: https://www.paraview.org/
[CATALYST]: https://www.paraview.org/in-situ/
[NEK5K]: https://nek5000.mcs.anl.gov/  


## 1. VTK / PARAVIEW / CATALYST <a name="subparagraph1"></a>

CATALYST uses the scaling capabilities of VTK and ParaView to suppor the *in situ* use case. 

Three levels of compilation are here considered : 

I. **Full Paraview**  

II. **Partial Paraview** 

III. **Decaf Paraview**


*Full Paraview* 
manipulates and displays scientific data through the use of a python or C++ VTK API 

*Partial Paraview*
can manipulate and/or display scientific data 
through the use of the C++ VTK API    
The use of python is optional. 

*Decaf Paraview*
manipulates scientific data
through the use of the C++ VTK API. 
 
This last option has been chosen 
in order to encourage 
scalability and portability of the code. 


#### Decaf Compilation   

1.III. **Decaf Paraview** 

a. Download ParaView Source Code
```
git clone https://github.com/jmake/DecafPV560.git 
```
[PARAVIEW560]:https://www.paraview.org/download/


b. CMake configuration
```c++
>> cmake ../SRC  \
    -DCMAKE_INSTALL_PREFIX=EXECS01 \
    -DPARAVIEW_BUILD_QT_GUI=OFF \
    -DCMAKE_CXX_FLAGS=-fpermissive \
    -DCMAKE_CXX_COMPILER=CC  \
    -DCMAKE_C_COMPILER=cc \
    -DCMAKE_Fortran_COMPILER=ftn \
    -DPARAVIEW_USE_ICE_T=OFF \
    -DPARAVIEW_USE_MPI=ON \
    -DBUILD_SHARED_LIBS=OFF \
    -DVTK_Group_ParaViewRendering=OFF \
    -DVTK_USE_X=OFF \
    -DVTK_OPENGL_HAS_OSMESA=OFF \
    -DVTK_OPENGL_HAS_EGL=OFF \
    -DModule_vtkIOExport=OFF \
    -DVTK_BUILD_ALL_MODULES_FOR_TESTS=OFF \
    -DVTK_Group_Rendering=OFF \
    -DVTK_Group_StandAlone=OFF \
    -DVTK_Group_MPI=ON \
    -DModule_vtkCommonCore=ON \
    -DModule_vtkFiltersGeneral=ON \
    -DVTK_RENDERING_BACKEND=None \
    -DPARAVIEW_ENABLE_VTK_MODULES_AS_NEEDED=FALSE \
    -DModule_vtkVTKm=ON \
    -DModule_vtkAcceleratorsVTKm=ON \
    -DModule_vtkPVVTKExtensionsDefault=ON \
    -DPARAVIEW_ENABLE_COMMANDLINE_TOOLS=OFF \
    -DPARAVIEW_CURRENT_CS_MODULES= 

``` 

c. Checking
```
>> ccmake .

 CMAKE_BUILD_TYPE                 Debug
 MACOSX_APP_INSTALL_PREFIX        WHERE/WILL/BE/INSTALLED
 ...
 PARAVIEW_ENABLE_CATALYST         ON
 PARAVIEW_ENABLE_PYTHON           OFF  
 PARAVIEW_ENABLE_QT_SUPPORT       OFF
 ...
 MPIEXEC_EXECUTABLE               mpiexec
 ...
 PARAVIEW_USE_MPI                 ON 

``` 

d. Compilation  
```
-- The C compiler identification is GNU 7.3.0
-- The CXX compiler identification is GNU 7.3.0
-- Cray Programming Environment 2.6.0 C
...
-- Cray Programming Environment 2.6.0 CXX
...
-- Could not use git to determine source version, using version 5.6.0
-- The Fortran compiler identification is GNU 7.3.0
 ... 
-- Found MPI_C: /opt/cray/pe/craype/2.6.0/bin/cc  
-- Found MPI_CXX: /opt/cray/pe/craype/2.6.0/bin/CC  
-- Found MPI_Fortran: /opt/cray/pe/craype/2.6.0/bin/ftn  
...
-- Configuring done
-- Generating done
-- Build files have been written to: ... 

```

```
[  0%] Built target ParaViewData
[  0%] Built target H5make_libsettings
[  1%] Built target verdict
[  1%] Built target VTKData
...
[ 88%] Built target vtkIOMPIParallel
...
[100%] Built target vtkPVCatalyst
...
[100%] Built target vtkPVVTKExtensionsDefault
```

Beskow, 16cores, ~5 min  
Nord, 16cores, ~7 min


