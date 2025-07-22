import vtktools 
import usdtools 
import itktools  

import numpy as np 


def WorkFlow(fname, expected) : 
    print("------------------------------------------------------")
    #vti = itktools.GetVTI1(fname) 
    #vtktools.write_vti_file(vti,"original")
    voxels, spacing, dimensions = itktools.GetVTI2(fname, "original") 

    nifti2Vtk = vtktools.Nifti2Vtk()
    nifti2Vtk.LoadFile(voxels, spacing, dimensions)
    #nifti2Vtk.SetDomain(vti) # :(
    nifti2Vtk.SaveDomain("domain")
    (vmin, vmax) = nifti2Vtk.range  

    cutter,center = nifti2Vtk.CreateCutter(z0=0.5) 
    nifti2Vtk.SaveCutter("cutter")

    texture_path = "texture.png"
    data = nifti2Vtk.ExtractCutter() 

    ## height, width = data.shape # <- From c++ wrapper  
    ## width, height = data.shape # <- From itk 
    (width, height) = data.shape
    print(data.shape, "->", (width, height))

    unique = nifti2Vtk.unique; #print( unique )
    val_to_rgb = usdtools.map_unique_values_to_colors(unique) 
    usdtools.create_or_update_texture(data, texture_path, width, height, val_to_rgb)

    current = vtktools.get_file_hash_sha256(texture_path)
    print("SHA-256:", current)
    if current != expected: raise ValueError("Hash mismatch! File content is not as expected.")

    ##data = np.array([ [0,1], [1,3]]) 
    ##vmin = data.min()
    ##vmax = data.max() 
    #GetColors = lambda u: usdtools.map_to_tricolor_gradient(u, vmin, vmax) 
    #colors = GetColors(data)
    #print(colors)
    #usdtools.create_or_update_texture2(data, GetColors, texture_path) 

    #bounds = cutter.GetBounds()
    #CreatePlane(bounds, center, texture_path) 

    #nifti2Vtk.CreateContour(u=0.5)
    #nifti2Vtk.SaveContour("contour")

    return 


#=======================================================================||====#

## Data From 'https://github.com/neurolabusc/VolumeRenderingData'
fname1 = r"F:\z2025_1\Dicom\DecafPV560\Omniverse\MR.nii.gz"
expected1 = "49887cadf765012c224a07b98fa37e61dd00a40ebff39202ace1dfbda89d27b7"

fname2 = r"F:\z2025_1\Dicom\DecafPV560\Omniverse\T2.nii.gz"
expected2 = "abfbdcb703369fe794ebf4b653cc2526fe1ceef09bc9cbc507fb53027d3f3aca"

fname3 = r"F:\z2025_1\Dicom\DecafPV560\Omniverse\iguana.nii.gz"
expected3 = "76318b5f15eb8150c775ea4f02509fd3604e93b4bc32efc1f7f5ec19915cb6d7"

fname4 = "F:\\z2025_1\\Dicom\\NII\\1.2.826.0.1.3680043.10633.nii.gz"
expected4 = "22aa054c3263153d2af96d0583fce83296938e71f34515748a4222a0c1904011"

WorkFlow(fname1, expected1)
WorkFlow(fname2, expected2)
WorkFlow(fname3, expected3)
WorkFlow(fname4, expected4)


print("ok!")
r"""
python.exe -m pip install SimpleITKUtilities
"""