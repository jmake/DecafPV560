## 1. 
SETUP_SYSTEM() 
{
  apt-get --yes -qq update 
  DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC apt-get -y install tzdata
  apt-get --yes -qq update 
  apt-get --yes -qq install build-essential  
  apt-get --yes -qq install gfortran gcc g++  
  apt-get --yes -qq install openmpi-bin libopenmpi-dev  
  apt-get --yes -qq install cmake 
  apt-get --yes -qq clean
  rm -rf /var/lib/apt/lists/*
}

## 2. 
CMAKE()
{  
  cmake ../SRC  \
        -DCMAKE_INSTALL_PREFIX=EXECS01 \
        -DPARAVIEW_BUILD_QT_GUI=OFF \
        -DCMAKE_CXX_FLAGS=-fpermissive \
        -DCMAKE_CXX_COMPILER=mpicxx  \
        -DCMAKE_C_COMPILER=mpicc \
        -DCMAKE_Fortran_COMPILER=mpif90 \
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
        
}

## 1. 
SETUP_SYSTEM 

## 2. 
mkdir BUILD 
cd BUILD 
CMAKE

## 3. 
make -j4 
