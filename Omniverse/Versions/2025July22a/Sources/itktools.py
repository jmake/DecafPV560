import sys
import pathlib
path_root = pathlib.Path(__file__)
sys.path.append(str(path_root.parent))
print("\nLoading:'%s' ... " % path_root.name) 

import sys,os
sys.path.append( os.path.dirname(__file__) )

#=======================================================================||====#
#=======================================================================||====#
import numpy as np 
import SimpleITK as sitk
#print(sitk.__version__)


def GetVTI1(fin, fout=None): 
    from SimpleITK.utilities.vtk import sitk2vtk

    reader = sitk.ImageFileReader()
    reader.SetImageIO("NiftiImageIO")

    reader.SetFileName(fin)
    image = reader.Execute()

    vti = sitk2vtk(image)
    print("[GetVTI] dimensions : ", vti.GetDimensions() )

    if fout :
        writer = sitk.ImageFileWriter()
        writer.SetImageIO("VTKImageIO") 
        writer.SetFileName(f"{fout}.vtk")
        writer.Execute(image)
    return vti 


def GetVTI2(fin, fout=None): 
    reader = sitk.ImageFileReader()
    reader.SetImageIO("NiftiImageIO")

    reader.SetFileName(fin)
    image = reader.Execute()

    if fout :
        writer = sitk.ImageFileWriter()
        writer.SetImageIO("VTKImageIO") 
        writer.SetFileName(f"{fout}.vtk")
        writer.Execute(image)

    dimensions = np.ones(4).astype(np.int32) 
    dimensions[:3] = image.GetSize()                # (x, y, z)
    print("Dimensions:", dimensions)

    spacing        = image.GetSpacing()             # (x_spacing, y_spacing, z_spacing)
    print("Spacing:", spacing)

    voxels         = sitk.GetArrayFromImage(image)  # shape: (z, y, x)
    print("Voxel array shape (x,y,z):", voxels.shape)
    #voxels         = voxels.flatten() 

    voxels = np.transpose(voxels, (2, 1, 0)) # :( 
    voxels = voxels.ravel(order='F')  # zyx → flat
    return voxels, spacing, dimensions


#=======================================================================||====#
#=======================================================================||====#








print("Leaving:'%s' ..." % path_root.name)
#=======================================================================||====#
r"""
python.exe -m pip install SimpleITKUtilities


"""