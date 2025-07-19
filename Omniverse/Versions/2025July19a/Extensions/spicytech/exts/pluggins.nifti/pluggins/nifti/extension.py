r"""
USEFUL : 
    C:\Users\zvl_2\AppData\Local\ov\pkg\create-2023.2.5\omni.create.bat

BUILD : 
    1) 
        cd C:\Users\zvl_2\AppData\Local\ov\pkg\create-2023.2.5\kit

        python.bat --version  # <- Same version (3.10.13)

        python.bat -m pip list 

    2) 
        cd F:\z2025_1\Dicom\DecafPV560
        
        C:\Users\zvl_2\AppData\Local\ov\pkg\create-2023.2.5\kit\python.bat -m venv Omniverse\Env3a1  

        .\Omniverse\Env3a1\Scripts\Activate.ps1

        python.exe --version # <- Same version (3.10.13)

    3) 
        omni.create.bat -> Window 
                        -> Script Editor 
                        -> import numpy; print(numpy.get_include()) 
                        -> .../create-2023.2.5/.../numpy/core/include

         ## then vim .\Omniverse\RUNNER.ps1 
        $NUMPY_INCLUDE_DIR="...\create-2023.2.5\...\numpy\core\include"
     
        .\Omniverse\RUNNER.ps1 

SEE : 
    https://github.com/jmake/VarjoOmniverse/blob/Omniverse/Scripts/__Extensions/__vespa/extension.py

"""
## 1) SpicyTech library 
import shutil ## ImportError: DLL load failed while importing _LibraryName: ...
shutil.copy("F:/z2025_1/Dicom/zlib/Execs/bin/zd.dll",".")

import sys; 
sys.path.append(r"F:\z2025_1\Dicom\DecafPV560\Omniverse\Build") # 
import SpicyTech 


import importlib ##  reload module after modifying 
sys.path.append(r"F:\z2025_1\Dicom\DecafPV560\Omniverse\Sources")
import vtktools 
importlib.reload(vtktools)

import usdtools 
importlib.reload(usdtools)

import niftitools 
importlib.reload(niftitools)


## 2) extra libraries 
import omni.kit.pipapi
omni.kit.pipapi.install("vtk")
import vtk 
print("[pluggins.nifti] vtk : ", vtk.vtkVersion().GetVTKVersion())

import PIL 
print("[pluggins.nifti] Pillow : ", PIL.__version__)


## 3) omniverse libraries 
import omni.ext
import omni.ui as ui


import PIL.Image
import numpy as np 
from pxr import Usd, UsdGeom, UsdShade, Sdf, Gf


#=======================================================================||====#
#=======================================================================||====#
NIFTI_PATH = "F:\\z2025_1\\Dicom\\NII\\1.2.826.0.1.3680043.10633.nii.gz"; 


def UsdPlaneCreator(stage, bounds, fname=None, color=None, imagePath=None) : 
    usdManager = usdtools.UsdManager()

    usdManager.Init(stage)
    planePrim = usdManager.PlaneCreate("/Plane", bounds) 

    if color : 
        usdManager.MaterialCreate("/Materials", (0,1,1) ) # :) 

    if imagePath: 
        usdManager.TextureCreate("/Texture", imagePath)

    if fname : 
        usdManager.StageSave(fname) ## usd, binary | usda, ascii 

    return planePrim


class Manager() : 

    nifti2Vtk = vtktools.Nifti2Vtk()

    def __init__(self) : 
        return 


    def LoadFile(self, directory): 
        voxels, spacing, dimensions = niftitools.NiftiLoadFile(directory) 
        self.nifti2Vtk.LoadFile(voxels, spacing, dimensions)

        unique = self.nifti2Vtk.unique 
        self.val_to_rgb = usdtools.map_unique_values_to_colors(unique) 
        return 


    def CreateCutter(self, texture_path, z0): 
        cutter,center = self.nifti2Vtk.CreateCutter(z0) 

        data = self.nifti2Vtk.ExtractCutter() 
        usdtools.create_or_update_texture(data, texture_path, self.val_to_rgb)

        bounds = cutter.GetBounds()
        self.CreatePlane(bounds, center, texture_path) 
        return cutter 


    def CreatePlane(self, bounds, center, imagePath): 
        stage = omni.usd.get_context().get_stage() 
        planePrim = UsdPlaneCreator(stage, bounds, imagePath=imagePath) 

        center = Gf.Vec3f(center[0],center[1],center[2])
        planePrim.GetAttribute('xformOp:translate').Set( center ) 
        return 


imagePath = r"F:\z2025_1\Dicom\DecafPV560\Omniverse\slice1.png"

manager = Manager() 
manager.LoadFile(NIFTI_PATH) 
manager.CreateCutter(imagePath, 0.15) 


#=======================================================================||====#
#=======================================================================||====#
def threshold_func(model, label_callback) :
    value = model.get_value_as_float() 

    msg = str(round(value,3) )
    label_callback.text = msg 

    manager.CreateCutter(imagePath, value) 

    print(f"[threshold_func] value : {msg} done!")
    return 


def path_func(model):
    value = model.get_value_as_string() 
    print(f"[path_func] text : '{value}'")


#=======================================================================||====#
class PlugginsNiftiExtension(omni.ext.IExt) :

    path_model = ui.SimpleStringModel(f"{NIFTI_PATH}")
    path_model.add_value_changed_fn(path_func)

    threshold_model = ui.SimpleFloatModel(0.5)

    def on_click(self) : 
        value = self.path_model.get_value_as_string() 
        print(f"[on_click] value:{value}")
        return 


    def on_startup(self, ext_id) :
        print("[pluggins.nifti] pluggins nifti startup")

        self._count = 0
        self._window = ui.Window("SpicyViewer", width=512, height=256)

        with self._window.frame:
            with ui.VStack() :

                with ui.VStack() :
                    label = ui.Label( f"{self.threshold_model.get_value_as_float()}" ) 
                    self.threshold_model.add_value_changed_fn(lambda model : threshold_func(model,label))

                    ui.FloatSlider(model=self.threshold_model, min=0.0, max=1.0, step=0.05)


                with ui.HStack() :
                    ui.StringField(model=self.path_model)
                    ui.Button("Load", clicked_fn=lambda : self.on_click())
                    #ui.Button("Reset", clicked_fn=self.on_reset) 

    def on_shutdown(self):
        print("[pluggins.nifti] pluggins nifti shutdown")


#=======================================================================||====#
#=======================================================================||====#
r"""
def Testing1() : 
    import numpy  
    import SpicyTech 

    vtkTest = SpicyTech.VtkTest()

    arr = numpy.array([1.0, 2.0, 3.0, 4.0]) #, dtype=np.float64)
    vtkTest.NumpyPrint(arr); 

    data = numpy.array( vtkTest.GetData() )
    print("[runme] received:", data )


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


def Testing3() : 
    import omni.volume
    import numpy as np
    from pxr import Usd, UsdVol, Sdf

    scale = 1.0  
    grid_name = "density"
    voxel_size = 0.05
    origin_array = np.array([0.0, 0.0, 0.0])

    density_array = Testing2() 

    vol = omni.volume.get_volume_interface()

    buffer = density_array[...,0].astype(np.float32)
    print( type(buffer), buffer.dtype )
    
    ## https://docs.omniverse.nvidia.com/kit/docs/omni.volume/0.5.2/omni.volume/omni.volume.IVolume.html
    #grid_data = vol.create_from_dense(
    #    voxel_size,
    #    buffer,
    #    scale,
    #    origin_array,
    #    grid_name
    #)

    import os   
    print(os.getcwd())

    #fname = "xyz.vdb"
    #vol.save_volume(grid_data, fname) 
    #print( fname )
    print("[Testing3] Done!!")
    return 

"""