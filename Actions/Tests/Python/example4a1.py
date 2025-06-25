import numpy as np
import SimpleITK as sitk
import matplotlib.pyplot as plt

import vtk
from vtk.util import numpy_support


def write_vtk_xml_image(vtk_image, filename):
    writer = vtk.vtkXMLImageDataWriter()
    writer.SetFileName(filename)
    writer.SetInputData(vtk_image)
    writer.Write()


def sitk_to_vtk(image):
    # Convert SimpleITK image to numpy array
    arr = sitk.GetArrayFromImage(image)  # shape: (depth, height, width)
    
    # VTK expects data in (width, height, depth), so reorder axes
    arr = np.transpose(arr, (2, 1, 0))  # now (width, height, depth)
    
    # Convert numpy array to vtkDataArray
    vtk_data_array = numpy_support.numpy_to_vtk(num_array=arr.ravel(order='F'), deep=True, array_type=vtk.VTK_FLOAT)
    
    # Create vtkImageData object
    vtk_image = vtk.vtkImageData()
    vtk_image.SetDimensions(arr.shape)
    
    # Set spacing, origin from SimpleITK image
    vtk_image.SetSpacing(image.GetSpacing())
    vtk_image.SetOrigin(image.GetOrigin())
    
    # Set the scalar data
    vtk_image.GetPointData().SetScalars(vtk_data_array)
    
    return vtk_image


def plot_slice(image, slice_index=None, axis=2):
    array = sitk.GetArrayFromImage(image)
    # array shape: (depth, height, width)
    shape = array.shape  # (D, H, W)

    if slice_index is None:
        # Default center slice based on axis
        if axis == 0:
            slice_index = shape[0] // 2
        elif axis == 1:
            slice_index = shape[1] // 2
        else:
            slice_index = shape[2] // 2

    if axis == 0:
        slice_img = array[slice_index, :, :]
    elif axis == 1:
        slice_img = array[:, slice_index, :]
    else:  # axis == 2
        slice_img = array[:, :, slice_index]

    plt.imshow(slice_img, cmap='gray')
    plt.axis('off')
    plt.show()
    return 

def print_image_info(image):
    print("Size:", image.GetSize(), "[width, height, depth]")
    print("Spacing:", image.GetSpacing(), "[Voxel Dimensions]")
    print("Origin:", image.GetOrigin())
    print("Direction:", image.GetDirection(), "[Direction Cosines]")
    print("Pixel Type:", image.GetPixelIDTypeAsString())
    print("Number of Components:", image.GetNumberOfComponentsPerPixel())
    return 


import numpy as np
import SimpleITK as sitk

def get_physical_points(image, dtype=np.float32):
    size = image.GetSize()
    spacing = np.array(image.GetSpacing(), dtype=dtype)
    origin = np.array(image.GetOrigin(), dtype=dtype)
    direction = np.array(image.GetDirection(), dtype=dtype).reshape(3, 3)

    # Preallocate output array
    physical_points = np.zeros((size[0], size[1], size[2], 3), dtype=dtype)

    for k in range(size[2]):
        i = np.arange(size[0], dtype=dtype)
        j = np.arange(size[1], dtype=dtype)
        I, J = np.meshgrid(i, j, indexing='ij')
        K = np.full_like(I, k, dtype=dtype)

        indices = np.vstack((I.ravel(), J.ravel(), K.ravel()))
        phys = origin[:, None] + direction @ (spacing[:, None] * indices)
        phys = phys.T.reshape(size[0], size[1], 3)

        physical_points[:, :, k, :] = phys

    return physical_points




def TestNII(): 
    fin = r"F:\z2025_1\Dicom\NII\BRATS_001.nii.gz"
    fin = r"F:\z2025_1\Dicom\NII\1.2.826.0.1.3680043.10633.nii"

    reader = sitk.ImageFileReader()
    reader.SetImageIO("NiftiImageIO")
    reader.SetFileName( fin )

    image = reader.Execute()
    print_image_info(image) 

    get_physical_points(image)

    vti = sitk_to_vtk(image)
    write_vtk_xml_image(vti, "output.vti")

    plot_slice(image, axis=0)

    t1 = sitk.GetArrayFromImage(image)
    print(t1.shape)
    return 



def load_dicom_directory() :
    dicom_dir = r"F:\z2025_1\Dicom\Dicoms\1.2.826.0.1.3680043.5876"
    reader = sitk.ImageSeriesReader()
    series_ids = reader.GetGDCMSeriesIDs(dicom_dir)
    
    if not series_ids:
        raise ValueError(f"No DICOM series found in directory: {dicom_dir}")

    series_file_names = reader.GetGDCMSeriesFileNames(dicom_dir, series_ids[0])
    reader.SetFileNames(series_file_names)

    image = reader.Execute()
    return image


def TestDicom(): 
    image = load_dicom_directory()
    print_image_info(image) 

    get_physical_points(image)

    vti = sitk_to_vtk(image)
    write_vtk_xml_image(vti, "output.vti")

    plot_slice(image, axis=0)

    t1 = sitk.GetArrayFromImage(image)
    print(t1.shape)
    return 


TestNII()
TestDicom() 


print("ok!")

r"""
.\Env1a1\Scripts\Activate.ps1

DATA : 
    http://medicaldecathlon.com/
    https://www.kaggle.com/competitions/rsna-2022-cervical-spine-fracture-detection/data
    https://github.com/layerfMRI/LAYNII/tree/master/test_data

SEE :
    https://simpleitk.readthedocs.io/en/v1.2.4/Documentation/docs/source/IO.html 
    #include <itkNiftiImageIO.h>


OPEN : 
    F:\z2025_1\ParaView600\bin\paraview.exe  output.vti
"""

"""
import os 
from kaggle.api.kaggle_api_extended import KaggleApi

#$env:KAGGLE_USERNAME = "mzavala"
#$env:KAGGLE_KEY = "c61768a7e9c21b643745f107a9d96bee"
api = KaggleApi()
api.authenticate()

fname = r"1.2.826.0.1.3680043.5876/1.dcm"
api.competition_download_file('rsna-2022-cervical-spine-fracture-detection', fname)
"""

