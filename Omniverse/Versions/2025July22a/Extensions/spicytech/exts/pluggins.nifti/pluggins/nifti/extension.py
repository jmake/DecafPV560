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
## 1) extra libraries 
import omni.kit.pipapi

omni.kit.pipapi.install("vtk")
import vtk 
print("[pluggins.nifti] vtk : ", vtk.vtkVersion().GetVTKVersion())

omni.kit.pipapi.install("SimpleITK")
import SimpleITK 
print("[pluggins.nifti] SimpleITK : ", vtk.vtkVersion().GetVTKVersion())
print(SimpleITK.__version__)


import PIL 
print("[pluggins.nifti] Pillow : ", PIL.__version__)


## 2) SpicyTech library 
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

import itktools
importlib.reload(itktools)


## 3) omniverse libraries 
import omni.ext
import omni.ui as ui

import PIL.Image
import numpy as np 
from pxr import Usd, UsdGeom, UsdShade, Sdf, Gf


#=======================================================================||====#
#=============================================================| Plugging |====#
fname1 = r"F:\z2025_1\Dicom\DecafPV560\Omniverse\MR.nii.gz"
fname2 = r"F:\z2025_1\Dicom\DecafPV560\Omniverse\T2.nii.gz"
fname3 = r"F:\z2025_1\Dicom\DecafPV560\Omniverse\iguana.nii.gz"
fname4 = r"F:\\z2025_1\\Dicom\\NII\\1.2.826.0.1.3680043.10633.nii.gz"
#NIFTI_PATH = fname4 

imagePathX = r"F:\z2025_1\Dicom\DecafPV560\Omniverse\sliceX.png"
imagePathY = r"F:\z2025_1\Dicom\DecafPV560\Omniverse\sliceY.png"
imagePathZ = r"F:\z2025_1\Dicom\DecafPV560\Omniverse\sliceZ.png"

options = [] 
options.append(fname4) 
options.append(fname1) 
options.append(fname2) 
options.append(fname3) 


def UsdCubeCreator(stage=None, bounds=None, fname=None, thickness=None) : 
    stage = omni.usd.get_context().get_stage() 

    usdManager = usdtools.UsdManager()

    usdManager.Init(stage)
    prim = usdManager.CubeCreate("/Bbox", bounds, thickness) 

    usdManager.MaterialCreate("/Bbox/Material", (0,1,0) ) # :) 

    if fname : 
        usdManager.StageSave(fname) ## usd, binary | usda, ascii 

    return prim


def UsdPlaneCreator(stage, path, bounds, fname=None, color=None, imagePath=None) : 
    usdManager = usdtools.UsdManager()

    usdManager.Init(stage)
    prim = usdManager.PlaneCreate(f"{path}", bounds) 

    if color : 
        usdManager.MaterialCreate(f"{path}/Material", (0,1,1) ) # :) 

    if imagePath: 
        usdManager.TextureCreate(f"{path}/Texture", imagePath)

    if fname : 
        usdManager.StageSave(fname) ## usd, binary | usda, ascii 

    return prim


def UsdCreator(stage, vertices, indices, displayColor, fname=None) :  
    usdManager = usdtools.UsdManager()

    usdManager.Init(stage)

    points = vertices
    vertexCounts = indices[:, 0].flatten() 
    vertexIndices = indices[:, 1:].flatten()
    primMesh = usdManager.MeshAdd("/Surface", points, vertexCounts, vertexIndices, displayColor)

    if fname : 
        usdManager.StageSave(fname) ## usd, binary | usda, ascii 

    usdManager.MaterialCreate("/Surface/Material", (0,1,1) ) # :) 

    return primMesh


#=======================================================================||====#
class Manager() : 

    nifti2Vtk = vtktools.Nifti2Vtk()

    def __init__(self) : 
        return 


    def LoadFile(self, directory): 
        #voxels, spacing, dimensions = niftitools.NiftiLoadFile(directory) 
        voxels, spacing, dimensions = itktools.GetVTI2(directory) 

        vti = self.nifti2Vtk.LoadFile(voxels, spacing, dimensions)
        bbox = UsdCubeCreator(bounds=vti.GetBounds(), thickness=0.25) 

        unique = self.nifti2Vtk.unique 
        self.val_to_rgb = usdtools.map_unique_values_to_colors(unique) 
        return 


    def CreateCutterX(self, texture_path, r0) : 
        Coronal = [1,0,0, 0,0,-1, 0,1,0]
        rotation = Gf.Vec3f(90, 0, -90)
        self.CreateCutter("/planeX", texture_path, r0, Coronal, rotation)
        return  


    def CreateCutterY(self, texture_path, r0) : 
        Sagittal = [0, 0, -1, 1, 0, 0, 0, 1, 0]
        rotation = Gf.Vec3f(0, 90, -90)
        self.CreateCutter("/planeY", texture_path, r0, Sagittal, rotation)
        return  


    def CreateCutterZ(self, texture_path, r0) : 
        Axial = [1, 0, 0, 0, 1, 0, 0, 0, 1]
        rotation = Gf.Vec3f(180, 0, 0)
        self.CreateCutter("/planeZ", texture_path, r0, Axial, rotation)
        return  


    def CreateCutter(self, path, texture_path, r0, anatomicalView, rotation) : 
        ## Anatomical Views
        ## - Axial = [1, 0, 0, 0, 1, 0, 0, 0, 1]
        ## - Coronal = [1, 0, 0, 0, 0, -1, 0, 1, 0]
        ## - Sagittal = [0, 0, -1, 1, 0, 0, 0, 1, 0]
        ## 
        cutter,center = self.nifti2Vtk.CreateCutter(r0[0], r0[1], r0[2], anatomicalView) 

        data = self.nifti2Vtk.ExtractCutter() 

        ## height, width = data.shape # <- From c++ wrapper  
        ## width, height = data.shape # <- From itk 
        (width, height) = data.shape
        print("[CreateCutter] shape", data.shape, "->", (width, height))
        usdtools.create_or_update_texture(data, texture_path, width, height, self.val_to_rgb)

        bounds = cutter.GetBounds()
        self.CreatePlane(path, bounds, center, rotation, texture_path) 
        return cutter 


    def CreatePlane(self, path, bounds, center, rotation, imagePath): 
        stage = omni.usd.get_context().get_stage() 
        planePrim = UsdPlaneCreator(stage, path, bounds, imagePath=imagePath) 

        center = Gf.Vec3f(center[0],center[1],center[2])
        planePrim.GetAttribute('xformOp:translate').Set( center ) 

        planePrim.GetAttribute('xformOp:rotateXYZ').Set( rotation ) 
        return 


    def CreateContour(self, threshold) : 
        self.nifti2Vtk.CreateContour(threshold)

        (indices,vertices,displayColor) = self.nifti2Vtk.ExtractContour() 

        stage = omni.usd.get_context().get_stage() 
        self.primContour = UsdCreator(stage, vertices, indices, displayColor)
        return 

#==================================================================| GUI |====#
manager = Manager() 
#manager.LoadFile(NIFTI_PATH) 
#manager.CreateCutter(imagePath, 0.15) 
#manager.CreateContour(0.25) 


def cutter_func_x(model, r0) :
    value = model.get_value_as_float() 
    print(f"[cutter_func_x] value : {value} done!")

    r0[0] = value
    #manager.CreateCutterX(imagePathX, r0) # :( 
    return 


def cutter_func_y(model, r0) :
    value = model.get_value_as_float() 
    print(f"[cutter_func_y] value : {value} done!")

    r0[1] = value 
    manager.CreateCutterY(imagePathY, r0) 
    return 


def cutter_func_z(model, r0) :
    value = model.get_value_as_float() 
    print(f"[cutter_func_z] value : {value} done!")

    r0[2] = value 
    manager.CreateCutterZ(imagePathZ, r0) 
    return 


def surfaces_func_1(model):
    print(f"[surfaces_func_1] value : {model.get_value_as_float()}")


def surfaces_click_1(model) : 
    value = model.get_value_as_float() 
    manager.CreateContour(value) 
    print(f"[surfaces_click_1] value:{value}")
    return 


def surfaces_click_2( args ) : 
    args["active"] = not args["active"] 
    print(f"[surfaces_click_2] value:{args}")

    print( manager.primContour )
    manager.primContour.SetActive( args["active"] )
    return 


def path_click(model, rx0, ry0, rz0) : 
    value = model.get_value_as_string() 
    print(f"[path_click] value:{value}")

    SceneModifications() 

    manager.LoadFile(value) 

    #manager.CreateCutterX(imagePathX, rx0) # :(
    manager.CreateCutterY(imagePathY, ry0) 
    manager.CreateCutterZ(imagePathZ, rz0) 

    manager.CreateContour(0.1) 
    return 


def path_func(model):
    value = model.get_value_as_string() 
    print(f"[path_func] text : '{value}'")


def files_func(model):
    selected_index = model.get_value_as_int()
    selected_text = options[selected_index]
    print("Selected:", selected_text)


def combo_func(model,item,model_string): 
    all_options = [
        model.get_item_value_model(child).as_string
        for child in model.get_item_children()
    ]

    current_index = model.get_item_value_model().as_int
    selected = all_options[current_index]

    model_string.set_value(selected)
    print("Selected:", selected)
    return 


#=======================================================================||====#
def SceneModifications(): 
    stage = omni.usd.get_context().get_stage() 
    if (stage is None) : return 

    #stage.GetPrimAtPath("/Environment/ground").SetActive(True)    
    stage.GetPrimAtPath("/Environment/ground").GetAttribute("visibility").Set("invisible")
    stage.GetPrimAtPath("/Environment/groundCollider").GetAttribute("visibility").Set("invisible")

    new_color = Gf.Vec3f(0.18988335, 0.288056, 0.38613862)
    stage.GetPrimAtPath("/Environment/Sky").GetAttribute("inputs:color").Set(new_color)
    return 


def Remove() : 
    stage = omni.usd.get_context().get_stage() 
    if (stage is None) : return 

    usdtools.remove_prim_if_exists2(stage, "/Bbox")
    usdtools.remove_prim_if_exists2(stage, "/PlaneY")
    usdtools.remove_prim_if_exists2(stage, "/PlaneZ")
    usdtools.remove_prim_if_exists2(stage, "/Surface")
    return 


class PlugginsNiftiExtension(omni.ext.IExt) :
    ## Loader 
    path_model = ui.SimpleStringModel(f"{options[0]}")
    path_model.add_value_changed_fn(path_func)

    ## Cutter 
    rx0 = [0.0, 0.5, 0.5]
    ry0 = [0.5, 0.0, 0.5]
    rz0 = [0.5, 0.5, 0.0]
    cutterx_model = ui.SimpleFloatModel(0.5)
    cuttery_model = ui.SimpleFloatModel(ry0[1])
    cutterz_model = ui.SimpleFloatModel(rz0[2])

    ## Surfaces 
    toggle_state = {"active": True}
    surfaces_model_1 = ui.SimpleFloatModel(0.5)
    surfaces_model_1.add_value_changed_fn(surfaces_func_1)

    ## Main 
    def on_shutdown(self):
        Remove()
        print("[pluggins.nifti] pluggins nifti shutdown")


    def on_startup(self, ext_id) :
        Remove()

        print("[pluggins.nifti] pluggins nifti startup")

        self._window = ui.Window("SpicyViewer") 
        #self._window.width = int(256*2)  
        #self._window.height = int(256*1.5)  
        #self._window.setPosition(32, 256*1.5)
        with self._window.frame : 

            ui.Separator()
            ui.Label("Neuroimaging Informatics Technology Initiative", height=20, alignment=ui.Alignment.CENTER)

            with ui.ZStack(): 
                with ui.Frame(height=0, alignment=ui.Alignment.CENTER) : 
                    with ui.VStack(alignment=ui.Alignment.CENTER) : 
                        ## Surface 
                        ui.Separator()
                        with ui.VStack(style={"margin": 5}) :
                            with ui.HStack() :
                                label_1 = ui.Label("Surface") 
                                ui.FloatField(model=self.surfaces_model_1)
                                ui.Button("Calculate", clicked_fn=lambda : surfaces_click_1(self.surfaces_model_1))
                                activate = ui.Button("On/Off",  clicked_fn=lambda : surfaces_click_2(self.toggle_state))
                                #activate.text = "ON" if self.toggle_state["active"] else "OFF"

                        ## Cutter 
                        ui.Separator()
                        with ui.VStack(style={"margin":0}) :
                            with ui.HStack(style={"margin": 10}) :
                                self.cutterx_model.add_value_changed_fn(lambda model : cutter_func_x(model,self.rx0))
                                self.cuttery_model.add_value_changed_fn(lambda model : cutter_func_y(model,self.ry0))
                                self.cutterz_model.add_value_changed_fn(lambda model : cutter_func_z(model,self.rz0))

                                label_2 = ui.Label("Slicer", style={"color": ui.color("#FF0000")}) 

                                with ui.VStack(spacing=0, alignment=ui.Alignment.CENTER) :
                                    #ui.FloatSlider(model=self.cutterx_model, min=0.0, max=1.0, step=0.05)
                                    ui.FloatSlider(model=self.cuttery_model, min=0.0, max=1.0, step=0.05)
                                    ui.FloatSlider(model=self.cutterz_model, min=0.0, max=1.0, step=0.05)

                        ## Loader  
                        ui.Separator()
                        with ui.VStack(style={"margin": 5}) :
                            with ui.HStack() :
                                label_3 = ui.Label("Models") 
                                cb = ui.ComboBox(0, *options) 
                                cb.model.add_item_changed_fn(lambda model,item: combo_func(model,item,self.path_model) )
                                
                                ui.StringField(model=self.path_model)
                                ui.Button("Load", clicked_fn=lambda : path_click(self.path_model,self.rx0,self.ry0,self.rz0))


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