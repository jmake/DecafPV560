
## rsnaCervicalSpineFractureDetection.vti 

<img width="1585" height="918" alt="image" src="https://github.com/user-attachments/assets/4e3ee7e8-38de-4003-974f-774c84983d94" />


## mr 160, 232, 160
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/5aef89b5-e98b-426e-9e61-e34f62fe5cc1" />


## iguana 308, 535, 380 
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/f8f7b5fa-98be-4fe9-9c70-b819f30f9830" />

## t2 140, 160, 124
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/14e41bef-04d7-4c0f-bdd9-746bd76df61c" />



## Code 
```
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
if __name__ == "__main__" :
    """
    F:\z2025_1\ParaView600\bin\paraview.exe 
    """
    fname1 = r"F:\z2025_1\Dicom\DecafPV560\Omniverse\MR.nii.gz"
    fname2 = r"F:\z2025_1\Dicom\DecafPV560\Omniverse\T2.nii.gz"
    fname3 = r"F:\z2025_1\Dicom\DecafPV560\Omniverse\iguana.nii.gz"
    fname4 = r"F:\\z2025_1\\Dicom\\NII\\1.2.826.0.1.3680043.10633.nii.gz"

    #voxels, spacing, dimensions = GetVTI2(fname3, "iguana") # 308, 535, 380 
    #voxels, spacing, dimensions = GetVTI2(fname1, "mr") # 160, 232, 160
    voxels, spacing, dimensions = GetVTI2(fname2, "t2") # 140, 160, 124


    print(voxels)
    print(spacing)
    print(dimensions)






print("Leaving:'%s' ..." % path_root.name)
#=======================================================================||====#
r"""
python.exe -m pip install SimpleITKUtilities


"""
```
