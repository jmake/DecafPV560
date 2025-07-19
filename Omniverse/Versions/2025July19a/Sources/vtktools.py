import ctypes
import numpy as np

import vtk 
from vtkmodules.vtkCommonCore import vtkDataArray
from vtkmodules.util.numpy_support import vtk_to_numpy
from vtkmodules.util.numpy_support import numpy_to_vtk

from vtkmodules.vtkCommonDataModel import vtkImageData


import sys
import pathlib
path_root = pathlib.Path(__file__)
sys.path.append(str(path_root.parent))
print("\nLoading:'%s' ... " % path_root.name) 
#=======================================================================||====#
#=======================================================================||====#


#=======================================================================||====#
#from vtkmodules.vtkFiltersCore import vtkContourFilter

def get_contour(input_data, key, threshold, scalars) :
    contour_filter = vtk.vtkContourFilter()
    contour_filter.SetInputData(input_data)
    contour_filter.SetValue(0, threshold)
    contour_filter.SetInputArrayToProcess(0, 0, 0, 0, key)

    contour_filter.GenerateTrianglesOn()
    if not scalars:
        contour_filter.ComputeScalarsOff()
    contour_filter.ComputeNormalsOff()
    contour_filter.Update()

    return contour_filter.GetOutputDataObject(0)


def write_vtp_file(polydata, filename) :
    if not filename.lower().endswith(".vtp"):
        filename += ".vtp"

    writer = vtk.vtkXMLPolyDataWriter()
    writer.SetFileName(filename)
    writer.SetInputData(polydata)
    writer.Write()

    print(f"[write_vtp_file] '{filename}' saved!")



#=======================================================================||====#
def vtk_from_ptr(c_ptr, dtype, n_tuples, n_components) :
    np_array = np.ctypeslib.as_array(c_ptr, shape=(n_tuples * n_components,))
    
    vtk_array = numpy_to_vtk(np_array, deep=False)
    vtk_array.SetNumberOfComponents(n_components)
    return vtk_array


def vtk_from_ptr_test() : 
    n_tuples = 2
    n_components = 3
    data = (ctypes.c_float * (n_tuples * n_components))(1.0, 2.0, 3.0, 4.0, 5.0, 6.0)
    float_ptr = ctypes.cast(data, ctypes.POINTER(ctypes.c_float))

    vtk_array = vtk_from_ptr(float_ptr, np.float32, n_tuples, n_components)

    print(vtk_array.GetNumberOfTuples())      # 2
    print(vtk_array.GetNumberOfComponents())  # 3
    print(vtk_array.GetTuple(1))              # (4.0, 5.0, 6.0)


#=======================================================================||====#
def create_vtk_image_data(nx, ny, nz, dX, dY, dZ, vtk_data) :
    image = vtk.vtkImageData()
    image.SetDimensions(nx, ny, nz)
    image.SetSpacing(dX, dY, dZ)
    image.GetPointData().SetScalars(vtk_data)
    return image


def write_vti_file(image, filename) :
    if not filename.lower().endswith(".vti"):
        filename += ".vti"
    
    writer = vtk.vtkXMLImageDataWriter()
    writer.SetFileName(filename)
    writer.SetInputData(image)
    writer.Write()

    print(f"[write_vti_file] '{filename}' saved! ") 
    return 


#=======================================================================||====#
import vtk

def visualize_vtk_image(image):
    scalars = image.GetPointData().GetScalars()
    if scalars is None:
        print("No scalar data in vtkImageData. Aborting.")
        return

    scalar_range = image.GetScalarRange()

    dims = image.GetDimensions()
    is_2d = dims[2] == 1

    mapper = vtk.vtkSmartVolumeMapper()
    mapper.SetBlendModeToComposite()
    mapper.SetInputData(image)

    volume_property = vtk.vtkVolumeProperty()
    volume_property.ShadeOff()
    volume_property.SetInterpolationTypeToLinear()

    opacity = vtk.vtkPiecewiseFunction()
    opacity.AddPoint(scalar_range[0], 0.0)
    opacity.AddPoint(scalar_range[1], 1.0)
    volume_property.SetScalarOpacity(opacity)

    color = vtk.vtkColorTransferFunction()
    color.AddRGBPoint(scalar_range[0], 0.0, 0.0, 0.0)
    color.AddRGBPoint(scalar_range[1], 1.0, 1.0, 1.0)
    volume_property.SetColor(color)

    volume = vtk.vtkVolume()
    volume.SetMapper(mapper)
    volume.SetProperty(volume_property)

    outline = vtk.vtkOutlineFilter()
    outline.SetInputData(image)

    outline_mapper = vtk.vtkPolyDataMapper()
    outline_mapper.SetInputConnection(outline.GetOutputPort())

    outline_actor = vtk.vtkActor()
    outline_actor.SetMapper(outline_mapper)
    outline_actor.GetProperty().SetColor(1.0, 1.0, 1.0)

    renderer = vtk.vtkRenderer()
    renderer.SetBackground(0.1, 0.1, 0.2)
    renderer.AddActor(outline_actor)

    if is_2d:
        # 2D slice rendering using image actor
        slice_mapper = vtk.vtkImageSliceMapper()
        slice_mapper.SetInputData(image)
        slice_mapper.SetSliceNumber(0)
        slice_mapper.SliceAtFocalPointOn()
        slice_mapper.SliceFacesCameraOn()

        slice_actor = vtk.vtkImageSlice()
        slice_actor.SetMapper(slice_mapper)

        color_map = vtk.vtkImageMapToWindowLevelColors()
        color_map.SetInputData(image)
        color_map.SetWindow(scalar_range[1] - scalar_range[0])
        color_map.SetLevel(0.5 * (scalar_range[1] + scalar_range[0]))

        renderer.AddViewProp(slice_actor)
    else:
        renderer.AddVolume(volume)

    render_window = vtk.vtkRenderWindow()
    render_window.AddRenderer(renderer)
    render_window.SetSize(800, 600)

    interactor = vtk.vtkRenderWindowInteractor()
    interactor.SetRenderWindow(render_window)
    interactor.Initialize()

    renderer.ResetCamera()
    render_window.Render()
    interactor.Start()


#=======================================================================||====#
def get_flat_cell_indices_direct(polydata):
    cell_array = polydata.GetPolys()  # or GetLines(), GetStrips(), etc.
    if cell_array is None or cell_array.GetNumberOfCells() == 0:
        return np.array([], dtype=np.int32), 0

    # Extract raw connectivity array directly
    vtk_id_type_array = cell_array.GetData()
    n_cells = polydata.GetNumberOfCells()

    flat_data = vtk_to_numpy(vtk_id_type_array)
    assert np.all(flat_data[::4] == 3), "Not all cells are triangles!"

    reshaped = flat_data.reshape(-1, 4)  # shape = (n_cells, 4) -> [N_i, vertice_i1, vertice_i2, ..., vertice_iN]
    return reshaped 
    #indices_only = reshaped[:, 1:]  # drop the leading '3'
    #flat_indices = indices_only.flatten()
    ##return indices_only


def get_cpp_array3(vtk_array) :
    ## fails with  - > print( vertices[ shape[0]-1] )  :( 
    n_tuples = vtk_array.GetNumberOfTuples()
    n_components = vtk_array.GetNumberOfComponents()
    n_elements = n_tuples * n_components

    vtk_type = vtk_array.GetDataType() 
    vtk_to_numpy_map = {
        vtk.VTK_CHAR:           (ctypes.c_char,   np.int8),
        vtk.VTK_UNSIGNED_CHAR:  (ctypes.c_ubyte,  np.uint8),
        vtk.VTK_INT:            (ctypes.c_int,    np.int32),
        vtk.VTK_UNSIGNED_INT:   (ctypes.c_uint,   np.uint32),
        vtk.VTK_FLOAT:          (ctypes.c_float,  np.float32),
        vtk.VTK_DOUBLE:         (ctypes.c_double, np.float64),
        vtk.VTK_ID_TYPE:        (ctypes.c_long,   np.int64),  # usually long on 64-bit
    }


    if vtk_type not in vtk_to_numpy_map:
        raise TypeError(f"Unsupported VTK data type code: {vtk_type}")

    c_type, np_type = vtk_to_numpy_map[vtk_type]

    if n_tuples == 0 or n_components == 0:
        raise ValueError("VTK array has zero dimensions")

    void_ptr = vtk_array.GetVoidPointer(0)
    if void_ptr is None:
        raise RuntimeError("GetVoidPointer returned NULL")

    data_ptr = ctypes.cast(void_ptr, ctypes.POINTER(c_type))

    expected_bytes = ctypes.sizeof(c_type) * n_elements
    #actual_bytes = vtk_array.GetActualMemorySize() * 1024
    actual_bytes = vtk_array.GetDataSize() * vtk_array.GetElementComponentSize()

    print(f"[get_cpp_array3] Tuples: {n_tuples}, Components: {n_components}")
    if int(actual_bytes) != int(expected_bytes):
        print(f"[get_cpp_array3] Expected bytes: {expected_bytes}")
        print(f"[get_cpp_array3] Actual bytes from VTK: {actual_bytes}")
        raise MemoryError("Underlying VTK array does not have enough allocated memory")

    flat_array = np.ctypeslib.as_array(data_ptr, shape=(n_elements,))
    np_array = flat_array.reshape((n_tuples, n_components))
    return np_array


#=======================================================================||====#
def center_image_at_origin(image):
    dims = image.GetDimensions()
    spacing = image.GetSpacing()

    origin = [
        -0.5 * spacing[0] * (dims[0] - 1),
        -0.5 * spacing[1] * (dims[1] - 1),
        -0.5 * spacing[2] * (dims[2] - 1)
    ]

    return image.SetOrigin(origin)



#=======================================================================||====#
#=======================================================================||====#
def CutterPlane3( volumeData, center ) : 
    resliceFilter = vtk.vtkImageReslice()
    resliceFilter.SetInputData(volumeData)

    resliceFilter.SetOutputDimensionality(2)
    resliceFilter.SetResliceAxesDirectionCosines(1, 0, 0, 0, 1, 0, 0, 0, 1) # Identity matrix for axes
    resliceFilter.SetResliceAxesOrigin(center) # Slice through the center

    resliceFilter.Update()
    sliceImage = resliceFilter.GetOutput()

    return resliceFilter.GetOutputDataObject(0)




#=======================================================================||====#
#=======================================================================||====#
def PaletteCreate(palette, xint, xmin, xmax) : 
    x = np.linspace(xmin, xmax, palette.shape[0]) 
    rgb = np.array([np.interp(xint, x, palette[:,i]) for i in range(palette.shape[1])]).T
    return rgb.tolist()  



#=======================================================================||====#
def normalize_array_to_range(x, new_min, new_max, arr_min=0.0, arr_max=1.0) :
    scaled = (x - arr_min) / (arr_max - arr_min)
    mapped = scaled * (new_max - new_min) + new_min
    return mapped


def DisplayColorCreate(pointData, key): 
    palette = np.array([[0.0,0.0,1.0], [0.0,1.0,0.0], [1.0,0.0,0.0]])

    displayColor = None 
    if( pointData.HasArray(key) ) :
        data = vtk_to_numpy( pointData.GetArray(key) )  
        print(f"[DisplayColorCreate] data({key}): ", data.shape) 
        displayColor = PaletteCreate(palette, data, data.min(), data.max()) 
    return displayColor 


class Nifti2Vtk : 

    def __init__(self) : 
        self.key = "voxels" 
        self.maxCells = 2e6 
        return 


    def LoadFile(self, voxels, spacing, dimensions) :
        #voxels, spacing, dimensions = NiftiLoadFile(directory)         
        self.domain = self.CreateVti(voxels, spacing, dimensions) 
        center_image_at_origin(self.domain) 
        print("Origin:", self.domain.GetOrigin())

        self.range = (voxels.min(), voxels.max())
        self.bounds = self.domain.GetBounds() 

        self.unique = np.unique( voxels[:] )
        print("[Nifti2Vtk] values:", self.unique )
        return 


    def CreateVti(self, voxels, spacing, dimensions) : 
        (dX, dY, dZ) = spacing
        (nx, ny, nz, nt) = dimensions
        assert nt > 0, "[Nifti2Vtk] time series !!"

        vtk_array = numpy_to_vtk(voxels[:], deep=False)
        vtk_array.SetName(self.key) 
        vtk_array.SetNumberOfComponents(1)
        vtk_array.SetNumberOfTuples(nx * ny * nz)
        image = create_vtk_image_data(nx, ny, nz, dX, dY, dZ, vtk_array) 
        return image


    def CreateContour(self, threshold) :
        assert not self.domain is None, "self.domain is None"

        threshold = normalize_array_to_range(threshold, self.range[0], self.range[1], 0.0, 1.0) 
        print(f"[CreateContour] threshold : {threshold} ")

        self.contour = get_contour(self.domain, self.key, threshold, scalars=True)   
        return self.contour


    def ExtractContour(self) : 
        assert not self.contour is None, "self.contour is None"

        indices = get_flat_cell_indices_direct( self.contour ) 
        print("[ExtractContour] indices : ", indices.shape)

        indices_sizes = indices[:, 0]
        assert np.unique(indices_sizes)[0] == 3, "Not all cells are triangles!"
        ##print( indices[676297] ) # -> [337877 337849 337850]
        ##for id in indices[676297] : print( vertices[id] ) 

        pts = self.contour.GetPoints(); 
        vertices = vtk_to_numpy( pts.GetData() ) 
        print("[ExtractContour] vertices : ", vertices.shape)

        displayColor = DisplayColorCreate(self.contour.GetPointData(), self.key) 
        return indices,vertices,displayColor


    def ExtractCutter(self): 
        assert not self.cutter is None, "self.cutter is None"

        key = "ImageScalars" 
        pd = self.cutter.GetPointData()

        data = None
        if( pd.HasArray(key) ) :
            data = vtk_to_numpy( pd.GetArray(key) )  
            #print("[ExtractCutter] unique : ", np.unique(data) )

        (dimx, dimy, dimz) = self.cutter.GetDimensions()
        data = data.reshape((dimx,dimy))
        print("[ExtractCutter] shape : ", data.shape )

        #displayColor = DisplayColorCreate(pd, key) 
        return data 


    def SaveDomain(self, filename): 
        assert not self.domain is None, "self.domain is None"
        write_vti_file(self.domain, filename)
        return 

    def SaveContour(self, filename): 
        assert not self.contour is None, "self.contour is None"
        write_vtp_file(self.contour, filename) 
        return 


    def CreateCutter(self, z0) :
        assert not self.domain is None, "self.domain is None"

        (xmin,xmax, ymin,ymax, zmin,zmax) = self.bounds
        z = normalize_array_to_range(z0, zmin,zmax, 0.0, 1.0) 
        print(f"[CreateContour] z0 : {z0} -> {z}")

        center = self.domain.GetCenter() + np.array([0.0,0.0,z])
        self.cutter = CutterPlane3(self.domain, center)   
        return self.cutter,center


    def SaveCutter(self, filename): 
        assert not self.cutter is None, "self.cutter is None"
        write_vti_file(self.cutter, filename) 
        return 


print("Leaving:'%s' ..." % path_root.name)
#=======================================================================||====#
"""

def Testing3() :
    import vtk
    arr = vtk.vtkFloatArray()
    ## Returns a list of valid attributes and methods for an object.
    for x in dir(arr) : print(x) 
    ## void_ptr = vtk_array.GetVoidPointer(0)
    return 

"""