import sys
import os
import SimpleITK as sitk


import vtk
import numpy as np
from vtk.util import numpy_support

import matplotlib.pylab as plt


def read_dicom_tags(filepath):
    with open(filepath, 'rb') as f:
        f.seek(128)          # skip preamble
        magic = f.read(4)
        if magic != b'DICM':
            return None, None

        pos = None
        inst = None

        while True:
            tag_group = f.read(2)
            tag_elem = f.read(2)
            if len(tag_group) < 2 or len(tag_elem) < 2:
                break

            group = int.from_bytes(tag_group, 'little')
            elem = int.from_bytes(tag_elem, 'little')

            vr = f.read(2).decode('ascii')
            if vr in ('OB','OW','OF','SQ','UT','UN'):
                f.read(2)      # 2 reserved bytes
                length = int.from_bytes(f.read(4), 'little')
            else:
                length = int.from_bytes(f.read(2), 'little')

            value = f.read(length)

            if group == 0x0020 and elem == 0x0032:
                pos = value.decode('ascii').strip()
            elif group == 0x0020 and elem == 0x0013:
                inst = value.decode('ascii').strip()

            if pos and inst:
                break

        ## patient’s coordinate system (usually Right-Anterior-Superior, RAS).
        pos = tuple(map(float, pos.split('\\'))) 
        return pos, inst


def rescale_ct(ct_scans, new_min, new_max):
    old_min = np.min(ct_scans)
    old_max = np.max(ct_scans)
    print(f"old_min: {ct_scans.min()}, old_max: {ct_scans.max()}")

    if old_max == old_min:
        return np.full_like(ct_scans, new_min, dtype=np.float32)
    scaled = (ct_scans - old_min) / (old_max - old_min)
    scaled = scaled * (new_max - new_min) + new_min
    return scaled.astype(np.float32)


def threshold_vtk_image(vtk_image, lower, upper, output_filename):
    thresh = vtk.vtkImageThreshold()
    thresh.SetInputData(vtk_image)
    thresh.ThresholdBetween(lower, upper)
    thresh.SetInValue(1)
    thresh.SetOutValue(0)
    thresh.SetOutputScalarTypeToUnsignedChar()
    thresh.Update()

    writer = vtk.vtkXMLImageDataWriter()
    writer.SetFileName(output_filename)
    writer.SetInputData(thresh.GetOutput())
    writer.Write()


def save_vti_from_numpy(ct_scans: np.ndarray, filename: str, spacing=(1.0, 1.0, 1.0)):
    flipped = np.transpose(ct_scans, (2, 1, 0)).copy()
    flat_data = flipped.ravel(order='F') 

    vtk_data = numpy_support.numpy_to_vtk(num_array=flat_data, deep=True, array_type=vtk.VTK_FLOAT)

    image = vtk.vtkImageData()
    image.SetDimensions(flipped.shape)
    image.SetSpacing(spacing)
    image.GetPointData().SetScalars(vtk_data)

    #writer = vtk.vtkXMLImageDataWriter()
    #writer.SetFileName(filename)
    #writer.SetInputData(image)
    #writer.Write()
    return image



def reader_test(fname) :
    print("---------------")
    import vtk
    from vtk.util import numpy_support

    from vtkmodules.vtkIOImage import vtkDICOMImageReader
    reader = vtkDICOMImageReader()
    #reader.SetDirectoryName(fname)
    reader.SetFileName(fname)
    reader.Update()
    print( reader.GetOutputPort() )
    print("GetStudyUID:", reader.GetStudyUID()  ) 
    print("GetPatientName:", reader.GetPatientName()  ) 
    print("GetImagePositionPatient:", reader.GetImagePositionPatient()  ) 


    vti = reader.GetOutputDataObject(0)
    print(  vti )
    #Array = GetCellDataArray(a.GetOutputDataObject(0), key)

    GetCoords    = lambda _ds : numpy_support.vtk_to_numpy( _ds.GetPoints().GetData() )
    GetPointData = lambda _pd : {_pd.GetArrayName(k):numpy_support.vtk_to_numpy(_pd.GetArray(k)) for k in range(_pd.GetNumberOfArrays())}
    GetCellData  = lambda _pd : {_pd.GetArrayName(k):numpy_support.vtk_to_numpy(_pd.GetArray(k)) for k in range(_pd.GetNumberOfArrays())}

    print( GetCellData(vti.GetCellData()) )
    print( GetPointData(vti.GetPointData()) )

    print("---------------")
    return 


directory = ""
directory = r"F:\z2025_1\Dicom\DicomTestImages1\matlab\examples\sample_data\DICOM\digest_article"
directory = r"F:\z2025_1\Dicom\DicomTestImages4\series-000001"

dcm_file = r"F:\z2025_1\Dicom\DicomTestImages4\series-000001\image-000200.dcm"
#dcm_file = r"F:\z2025_1\Dicom\DicomTestImages1\matlab\examples\sample_data\DICOM\digest_article\brain_020.dcm"

reader_test(dcm_file) 

position, instance = read_dicom_tags(dcm_file) 
print("Position:", position)
print("Instance:", instance)



exit()

reader = sitk.ImageSeriesReader()
dicom_names = reader.GetGDCMSeriesFileNames(directory)
reader.SetFileNames(dicom_names)
image = reader.Execute()

size = image.GetSize()
print("Image size:", size[0], size[1], size[2])

ct_scans = sitk.GetArrayFromImage(image) ##  raw voxel intensity (Hounsfield Units)
nscans = ct_scans.shape[0] 
print( ct_scans.shape )

#rescale_ct(ct_scans, 0.0, 1.0)
vtk_image = save_vti_from_numpy(ct_scans, "output.vti", spacing=image.GetSpacing())
threshold_vtk_image(vtk_image, 700.0, 5000.0, "threshold.vti")


exit()

nrows = 4
ncols = 8
ntotal = nrows * ncols

#exit() 
plt.figure() #figsize=(20,16))
plt.gray()
plt.subplots_adjust(0,0,1,1,0.01,0.01)
for i in range(ntotal):
    plt.subplot(nrows,ncols,i+1)
    plt.imshow(ct_scans[i]) 
    plt.axis('off')
    # use plt.savefig(...) here if you want to save the images as .jpg, e.g.,
plt.show()