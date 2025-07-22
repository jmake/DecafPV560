import vtktools 
import usdtools 
import niftitools 
r"""
python.exe --version ## Python 3.12.6
python.exe -m venv .\ENV2a1  
.\ENV2a1\Scripts\Activate.ps1
python.exe -m pip list 
python.exe -m pip install vtk numpy usd-core 
"""


#=======================================================================||====#
def UsdCreator(vertices, indices, displayColor, fname): 
    usdManager = usdtools.UsdManager()
    usdManager.Clear() 
    usdManager.Create()
    usdManager.MeshMaterialSet() 

    points = vertices
    vertexCounts = indices[:, 0].flatten() 
    vertexIndices = indices[:, 1:].flatten()
    usdManager.MeshAdd(points, vertexCounts, vertexIndices, displayColor)
    usdManager.StageSave(f"{fname}.usd") ## usd, binary | usda, ascii 
    return 


#=======================================================================||====#
def UsdPlaneCreator(): 
    usdManager = usdtools.UsdManager()
    usdManager.Clear() 
    usdManager.Create()

    usdManager.PlaneCreate()     

    #usdManager.MaterialCreate( (0,1,1) )
    textureImage = r"F:\z2025_1\Dicom\DecafPV560\Omniverse\generated_texture.png"
    usdManager.TextureCreate(textureImage)
    #usdManager.MeshMaterialSet() 
    usdManager.StageSave(f"plane.usd") ## usd, binary | usda, ascii 
    return 


UsdPlaneCreator() 
#exit() 


#=======================================================================||====#
directory = "F:\\z2025_1\\Dicom\\NII\\1.2.826.0.1.3680043.10633.nii.gz"
voxels, spacing, dimensions = niftitools.NiftiLoadFile(directory) 

nifti2Vtk = vtktools.Nifti2Vtk()
nifti2Vtk.LoadFile(voxels, spacing, dimensions)
#nifti2Vtk.SaveDomain("domain") 

#exit() 
if True : 
    cutter = nifti2Vtk.CreateCutter()
    nifti2Vtk.SaveCutter("cutter") 
    data = nifti2Vtk.ExtractCutter() 
    #vtktools.visualize_vtk_image(cutter) # :( 

    usdtools.create_or_update_texture(data)


if False : 
    nifti2Vtk.CreateContour(threshold=0.5)
    #nifti2Vtk.SaveContour("contour") 
    (indices,vertices,displayColor) = nifti2Vtk.ExtractContour() 
    UsdCreator(vertices, indices, displayColor, "contour")

 


#=======================================================================||====#



print("ok!")
#=======================================================================||====#
#=======================================================================||====#
r"""
F:\z2025_1\Dicom\DecafPV560\ENV1a1\Scripts\Activate.ps1

- SEE : 
    https://nvidia.github.io/warp/ (high-performance simulation and graphics code)
    https://github.com/NVIDIA-Omniverse/ext-openvdb
 
    - Vtk in Blender 
    https://github.com/tkeskita/BVtkNodes


- Omniverse
https://docs.omniverse.nvidia.com/kit/docs/omni.vdb/latest/source/extensions/omni.vdb.tool/docs/index.html





"""

r"""
import omni.volume

vdb_path = "F:\z2025_1\Dicom\DecafPV560\Omniverse\smoke.vdb"

vol = omni.volume.get_volume_interface()
grid_data = vol.create_from_file(vdb_path)

num = vol.get_num_grids(grid_data)
print("Number of grids:", num)

data = vol.get_grid_data(grid_data, 0)
#print("Grid 0 data:", data)

print(  vol.get_index_bounding_box(grid_data,0) )

print(  vol.get_world_bounding_box(grid_data,0) )

import omni.usd
from pxr import Usd, UsdVol, Sdf

# 1. Open the current USD stage
stage = omni.usd.get_context().get_stage()

# 2. Create a Volume prim
volume_path = "/MyVDBVolume"
volume_prim = stage.DefinePrim(volume_path, "Volume")

# 3. Create an OpenVDBAsset prim
field_path = f"{volume_path}/density"
field_prim = stage.DefinePrim(field_path, "OpenVDBAsset")

field_prim.GetAttribute("filePath").Set( Sdf.AssetPath(vdb_path) )
field_prim.GetAttribute("fieldName").Set("density")  # Replace if grid name differs

# 4. Bind the field to the volume
vol_schema = UsdVol.Volume(volume_prim)
vol_schema.CreateFieldRelationship("density", field_prim.GetPath())

"""

r"""
import omni.volume
import numpy as np
from pxr import Usd, UsdVol, Sdf

vol = omni.volume.get_volume_interface()

nx, ny, nz = 32, 32, 32
density_array = np.random.rand(nx, ny, nz).astype(np.float32)

voxel_size = 0.05
scale = 1.0  # or whatever this second float parameter should be
origin_array = np.array([0.0, 0.0, 0.0])
grid_name = "density"

grid_data = vol.create_from_dense(
    voxel_size,
    density_array,
    scale,
    origin_array,
    grid_name
)


stage = omni.usd.get_context().get_stage()

vol_prim = stage.DefinePrim("/DenseVolume", "Volume")

field_prim = stage.DefinePrim("/DenseVolume/" + grid_name, "OpenVDBAsset")
field_prim.GetAttribute("gridData").Set(grid_data) ## <- 'gridData' no exist!!

UsdVol.Volume(vol_prim).CreateFieldRelationship(grid_name, field_prim.GetPath())


## Alternative :(
coords = np.argwhere(density_array > 0.5)
pts = coords * voxel_size + origin

# Create point cloud prim
stage = omni.usd.get_context().get_stage()
pc_prim = UsdGeom.Points.Define(stage, "/InMemPoints")
pc_prim.CreatePointsAttr(pts.tolist())


"""


r"""
"""