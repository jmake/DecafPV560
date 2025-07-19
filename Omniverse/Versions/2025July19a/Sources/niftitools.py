## ImportError: DLL load failed while importing _LibraryName: The specified module could not be found.
import shutil
shutil.copy("F:/z2025_1/Dicom/zlib/Execs/bin/zd.dll",".")

import sys; 
sys.path.append(r"F:\z2025_1\Dicom\DecafPV560\Omniverse\Build")
import SpicyTech 

#import warp as wp # python.exe -m pip install warp-lang

import ctypes
import numpy as np

import sys
import pathlib
path_root = pathlib.Path(__file__)
sys.path.append(str(path_root.parent))
print("\nLoading:'%s' ... " % path_root.name) 
#=======================================================================||====#
#=======================================================================||====#


#=======================================================================||====#
def Testing1() : 
    import numpy  
    import SpicyTech 

    vtkTest = SpicyTech.VtkTest()

    arr = numpy.array([1.0, 2.0, 3.0, 4.0]) #, dtype=np.float64)
    vtkTest.NumpyPrint(arr); 

    data = numpy.array( vtkTest.GetData() )
    print("[runme] received:", data )


#=======================================================================||====#
def Testing2() : 
    import ctypes
    import numpy as np 
    import SpicyTech 

    directory = "F:\\z2025_1\\Dicom\\NII\\1.2.826.0.1.3680043.10633.nii.gz"; 
    nifti = SpicyTech.Nifti()
    nifti.LoadFile( directory ) 

    n_buffer = nifti.GetBufferSize() 
    print("[runme] n_buffer:", n_buffer)

    ptr = nifti.GetBuffer() 
    addr = ctypes.c_void_p(int(ptr))
    float_ptr = ctypes.cast(addr, ctypes.POINTER(ctypes.c_float))
    voxels = np.ctypeslib.as_array(float_ptr, shape=(n_buffer,))

    (nx, ny, nz, nt) = map(int, np.array(nifti.GetDimensions()))
    print("[runme] dimension:", nx, ny, nz, nt)

    voxels_4d = voxels.reshape((nx, ny, nz, nt))
    print( voxels_4d.shape )
    return voxels_4d


#=======================================================================||====#
#=======================================================================||====#
def NiftiLoadFile(directory) : 
    nifti = SpicyTech.Nifti()
    nifti.LoadFile( directory ) 

    n_buffer = nifti.GetBufferSize() 
    print("[NiftiLoadFile] n_buffer:", n_buffer)

    ptr = nifti.GetBuffer() 
    addr = ctypes.c_void_p(int(ptr))
    float_ptr = ctypes.cast(addr, ctypes.POINTER(ctypes.c_float))
    voxels = np.ctypeslib.as_array(float_ptr, shape=(n_buffer,))

    dimensions = np.array(nifti.GetDimensions())
    (nx, ny, nz, nt) = map(int,dimensions)
    print("[NiftiLoadFile] dimension:", nx, ny, nz, nt)

    voxels_4d = voxels.reshape((nx, ny, nz, nt))
    print("[NiftiLoadFile] voxel : ", voxels_4d.shape )

    spacing = np.array(nifti.GetSpacing())
    print("[NiftiLoadFile] spacing :", spacing)

    return voxels, spacing, dimensions


#=======================================================================||====#
#=======================================================================||====#
def Tests() : 
    Testing1()
    Testing2() 


print("Leaving:'%s' ..." % path_root.name)
#=======================================================================||====#