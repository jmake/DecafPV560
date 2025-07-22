import sys
import pathlib
path_root = pathlib.Path(__file__)
sys.path.append(str(path_root.parent))
print("\nLoading:'%s' ... " % path_root.name) 
#=======================================================================||====#
#=======================================================================||====#

import sys,os
sys.path.append( os.path.dirname(__file__) )

#from pymxs import runtime as mxs  
from pxr import Sdf, Usd, UsdGeom, UsdShade
from pxr import UsdUtils
#from pxr import UsdGeomTokens

#import usd_suzanne 
import PIL.Image

import numpy as np 
import colorsys
from pxr import Usd, UsdGeom, Gf, Vt


#========================================================================||====#
def create_bounding_box_wireframe(stage, path, bounds):
    xmin, xmax, ymin, ymax, zmin, zmax = bounds

    points = [
        (xmin, ymin, zmin), (xmax, ymin, zmin),
        (xmax, ymax, zmin), (xmin, ymax, zmin),
        (xmin, ymin, zmax), (xmax, ymin, zmax),
        (xmax, ymax, zmax), (xmin, ymax, zmax),
    ]

    indices = [
        (0, 1), (1, 2), (2, 3), (3, 0),  # bottom square
        (4, 5), (5, 6), (6, 7), (7, 4),  # top square
        (0, 4), (1, 5), (2, 6), (3, 7),  # vertical lines
    ]

    curve = UsdGeom.BasisCurves.Define(stage, path)
    curve.CreatePointsAttr(Vt.Vec3fArray([Gf.Vec3f(*p) for p in points]))
    curve.CreateCurveVertexCountsAttr(Vt.IntArray([2] * len(indices)))
    curve.CreateWidthsAttr(Vt.FloatArray([1.0] * len(indices)))
    curve.CreateTypeAttr().Set(UsdGeom.Tokens.linear)
    curve.CreateBasisAttr().Set(UsdGeom.Tokens.bezier)
    curve.CreateWrapAttr().Set(UsdGeom.Tokens.nonperiodic)
    #curve.CreateCurveIndicesAttr(Vt.IntArray([i for line in indices for i in line]))
    return curve 


#========================================================================||====#
def map_unique_values_to_colors(data) :
    flat_data = data.flatten()

    unique_vals = np.unique(flat_data)
    num_classes = len(unique_vals)
    hsv_values = np.linspace(0, 1, num_classes, endpoint=False)

    rgb_colors = np.array([colorsys.hsv_to_rgb(h, 1.0, 1.0) for h in hsv_values])
    val_to_rgb = {v: rgb_colors[i] for i, v in enumerate(unique_vals)}

    return val_to_rgb 
    #mapped_colors = np.array([val_to_rgb[v] for v in flat_data])
    #return mapped_colors


def map_to_tricolor_gradient(data, vmin=None, vmax=None):
    InverseLerp = lambda val,valMin,valMax : (val - min_val) / (max_val - min_val + 1e-8)  # -> t
    Lerp        = lambda t,colorMin,colorMax : (1 - t) * colorMin + t * colorMax  # -> 

    flat = data.flatten().astype(np.float32)
    if(not vmin) : min_val = flat.min()
    if(not vmin) : max_val = flat.max()
    mid_val = (min_val + max_val) / 2.0

    blue  = np.array([0.0, 0.0, 1.0])
    green = np.array([0.0, 1.0, 0.0])
    red   = np.array([1.0, 0.0, 0.0])

    rgb = np.zeros((flat.shape[0], 3), dtype=np.float32)
    for i, v in enumerate(flat):
        t = InverseLerp(v,mid_val,max_val) 
        if v <= mid_val:
            rgb[i] = Lerp(t,blue,green) 
        else:
            rgb[i] = Lerp(t,green,red) 
    
    return rgb


def create_or_update_texture2(data, func, texture_path) : 
    height, width = data.shape

    #mapped_colors = np.array([val_to_rgb[v] for v in data.flatten()])
    mapped_colors = func(data) 
    colors = mapped_colors.reshape((height, width, 3))
    colors_8bit = (colors * 255).astype(np.uint8)

    img = PIL.Image.fromarray(colors_8bit, mode='RGB')
    img.save(texture_path) 
    return img



def create_or_update_texture(data, texture_path, width, height, val_to_rgb=[]) : 

    #height, width = data.shape # <- original 
    #width, height = data.shape # <- using itk 

    if len(val_to_rgb) > 0 : 
        pass
    else :
        val_to_rgb = map_unique_values_to_colors(data) 

    mapped_colors = np.array([val_to_rgb[v] for v in data.flatten()])
    colors = mapped_colors.reshape((height, width, 3))
    colors_8bit = (colors * 255).astype(np.uint8)

    img = PIL.Image.fromarray(colors_8bit, mode='RGB')
    #texture_path = "F:/z2025_1/Dicom/DecafPV560/Omniverse/generated_texture.png"
    img.save(texture_path) 
    return img


def remove_prim_if_exists2(stage, prim_path):
    prim = stage.GetPrimAtPath(prim_path)
    if prim.IsValid():
        stage.RemovePrim(prim_path)
        print(f"[remove_prim_if_exists] '{prim_path}' removed!")


def remove_prim_if_exists1(stage, prim):
    if isinstance(prim, Usd.Prim):
        path = prim.GetPath()
    else:
        path = Usd.Prim(prim).GetPath() if isinstance(prim, str) else None

    print(f"[remove_prim_if_exists] '{path}' removed!")
    if path:
        target = stage.GetPrimAtPath(path)
        if target.IsValid():
            stage.RemovePrim(path)
            print(f"[remove_prim_if_exists] '{path}' removed!")


def create_operations(prim) :
    xform = UsdGeom.Xformable(prim)

    ops = [] 
    ops.append( xform.AddTranslateOp() )
    ops.append( xform.AddRotateXYZOp() )
    ops.append( xform.AddScaleOp() )
    xform.SetXformOpOrder(ops)
    return 


#========================================================================||====#
#========================================================================||====#
def Mesh2Dic(points, extent, vertexCounts, vertexIndices, texCoord2) : 
    import itertools 
    cumsum = lambda l : list(itertools.accumulate([0]+l)) 
    accumulate = cumsum(vertexCounts) 
    minmax = [(n,x) for (n,x) in zip(accumulate[:-1],accumulate[1:])]

    geometry = {} 
    for j,(n,x) in enumerate(minmax) :   
        indices = [vertexIndices[e] for e in range(n,x)] 
        geometry[(j,0)] = vertexCounts[j]  
        geometry[(j,1)] = indices
        geometry[(j,2)] = [texCoord2[i] for i in indices]
        geometry[(j,3)] = [points[i] for i in indices] 
    return len(geometry),geometry  

#========================================================================||====#
# Utility function to export the contents of a stage to a string.
StageShow = lambda stage : stage.GetRootLayer().ExportToString()


def StageCreate() : 
    """Create and return a new in-memory USD stage.""" 
    return Usd.Stage.CreateInMemory() #layer.identifier)


def StageIdGet(stage) : 
    """Get a unique identifier for a given stage using the USD stage cache.
    
    Args:
        stage (Usd.Stage): The USD stage to identify.
    
    Returns:
        int: A long integer representing the stage's unique identifier.
    """
    cache = UsdUtils.StageCache.Get()
    return cache.Insert(stage).ToLongInt()


def MeshCreate(stage, path, points, vertexCounts, vertexIndices, extent, texCoord, displayColor) : 
    """Create a mesh on the provided stage with specified attributes.
    
    Args:
        stage (Usd.Stage): The USD stage where the mesh will be defined.
        path (str): The path where the mesh will be created within the stage.
        points (list of tuples): Vertex positions of the mesh.
        vertexCounts (list of int): Number of vertices for each face.
        vertexIndices (list of int): Indexes of vertices that make up the mesh faces.
        extent (list of tuples): The bounding box of the mesh.
        texCoord (list of tuples): Texture coordinates for each vertex.
        displayColor : ...
    
    Returns:
        UsdGeom.Mesh: The created mesh object.
    """
    mesh = UsdGeom.Mesh.Define(stage, path)
    mesh.CreatePointsAttr(points)
    mesh.CreateFaceVertexCountsAttr(vertexCounts)
    mesh.CreateFaceVertexIndicesAttr(vertexIndices)
    mesh.CreateExtentAttr(extent)

    primvarsAPI = UsdGeom.PrimvarsAPI(mesh)  
    if not texCoord is None : 
        st = primvarsAPI.CreatePrimvar("st", Sdf.ValueTypeNames.TexCoord2fArray, UsdGeom.Tokens.varying)
        st.Set(texCoord)

    if not displayColor is None : 
        dc = primvarsAPI.CreatePrimvar("displayColor", Sdf.ValueTypeNames.Vector3fArray, UsdGeom.Tokens.vertex)
        dc.Set(displayColor)
    return mesh 


def CubeCreate(stage, path, bounds, thickness) :
    """Create a standard cube mesh with predefined vertex and face information.
    
    Args:
        stage (Usd.Stage): The USD stage where the cube will be created.
        path (str): The path where the cube will be created within the stage.
    
    Returns:
        UsdGeom.Mesh: The created cube mesh.
    """
    #extent = [(-0.5, -0.5, -0.5), (0.5, 0.5, 0.5)]
    #points = [ (-0.5,-0.5, 0.5), (0.5,-0.5, 0.5), (-0.5, 0.5, 0.5), (0.5, 0.5, 0.5),  (-0.5, 0.5,-0.5), (0.5, 0.5,-0.5),  (-0.5,-0.5,-0.5), (0.5,-0.5,-0.5) ]

    (xmin, xmax, ymin, ymax, zmin, zmax) = bounds

    extent = [(xmin, ymin, zmin), (xmax, ymax, zmax)]

    points = [
        (xmin, ymin, zmax),  # 0 front-bottom-left
        (xmax, ymin, zmax),  # 1
        (xmax, ymin, zmax),  # repeated start of new segment
        (xmax, ymax, zmax),  # 3
        (xmax, ymax, zmax),  # repeat
        (xmin, ymax, zmax),  # 5
        (xmin, ymax, zmax),  # repeat
        (xmin, ymin, zmax),  # 7 – front face loop
    
        (xmin, ymax, zmin),  # 8 back-top-left
        (xmax, ymax, zmin),  # 9
        (xmax, ymax, zmin),  # 10
        (xmax, ymin, zmin),  # 11
        (xmax, ymin, zmin),  # 12
        (xmin, ymin, zmin),  # 13
        (xmin, ymin, zmin),  # 14
        (xmin, ymax, zmin),  # 15 – back face loop

        (xmin, ymin, zmax),  # 16
        (xmin, ymin, zmin),  # 17 – side edge
        (xmax, ymin, zmax),  # 18
        (xmax, ymin, zmin),  # 19
        (xmin, ymax, zmax),  # 20
        (xmin, ymax, zmin),  # 21
        (xmax, ymax, zmax),  # 22
        (xmax, ymax, zmin),  # 23
    ]
    curve_counts = [2] * 12  

    curves = UsdGeom.BasisCurves.Define(stage, path)
    curves.CreatePointsAttr(points)
    curves.CreateCurveVertexCountsAttr(curve_counts)
    curves.CreateTypeAttr("linear")
    
    #curves.CreateWidthsAttr([1.0])
    curves.CreateWidthsAttr([thickness])
    curves.SetWidthsInterpolation(UsdGeom.Tokens.constant)


    #vertexCounts = [4, 4, 4, 4, 4, 4]
    #vertexIndices = [ 0,1,3,2,  2,3,5,4,  4,5,7,6,  6,7,1,0,  1,7,5,3,  6,0,2,4 ]
    #texCoord2 = [ (0, 0),(0, 1),(1, 0),(1, 1),  (0, 0),(0, 1),(1, 0),(1, 1),  (0, 0),(0, 1),(1, 0), (1, 1),  (0, 0),(0, 1),(1, 0),(1, 1)]
    #return MeshCreate(stage, path,  points, vertexCounts, vertexIndices, extent, texCoord2, None)
    return curves 


def PlaneCreate(stage, path, bounds) : 
    """Create a flat plane mesh with texture mapping.
    
    Args:
        stage (Usd.Stage): The USD stage where the plane will be created.
        path (str): The path where the plane will be created within the stage.
    
    Returns:
        UsdGeom.Mesh: The created plane mesh.
    """

    (xmin, xmax, ymin, ymax, zmin, zmax) = bounds

    points = [
        (xmin, ymin, zmin),
        (xmax, ymin, zmin),
        (xmax, ymax, zmin),
        (xmin, ymax, zmin),
    ]

    extent = [
        (xmin, ymin, zmin),
        (xmax, ymax, zmin),
    ]

    vertexCounts = [4]
    vertexIndices = [0,1,2,3] 
    texCoord2 = [(0, 0), (1, 0), (1,1), (0, 1)] 

    return MeshCreate(stage, path,  points, vertexCounts, vertexIndices, extent, texCoord2, None) 


def MaterialCreate(stage, path, diffuseColor, **opts) :
    """Create a material for use in shading a mesh.
    
    Args:
        stage (Usd.Stage): The USD stage where the material will be created.
        path (str): The path where the material will be created within the stage.
        diffuseColor (tuple): The RGB color values for the diffuse color of the material.
        opts (dict): Additional options such as shader name.
    
    Returns:
        UsdShade.Material: The created material.
    """
    materialPath = Sdf.Path(path)

    shaderName = opts.get("shaderName","Shader")
    shader = UsdShade.Shader.Define(stage, materialPath.AppendChild(shaderName))
    shader.CreateIdAttr("UsdPreviewSurface")
    shader.CreateInput("diffuseColor", Sdf.ValueTypeNames.Color3f).Set(diffuseColor)

    material = UsdShade.Material.Define(stage, materialPath)
    material.CreateSurfaceOutput().ConnectToSource(shader.ConnectableAPI(), "surface")
    return material


def TextureCreate(stage, path, filePath) : 
    """Create a texture material with an associated PBR shader.
    
    Args:
        stage (Usd.Stage): The USD stage where the texture material will be created.
        path (str): The path where the texture material will be created within the stage.
        filePath (str): Path to the texture file.
    
    Returns:
        UsdShade.Material: The created texture material with the PBR shader.
    """
    texturePath = Sdf.Path(path)

    pbrShader = UsdShade.Shader.Define(stage, texturePath.AppendChild('PBRShader'))
    pbrShader.CreateIdAttr("UsdPreviewSurface")
    pbrShader.CreateInput("emissiveColor", Sdf.ValueTypeNames.Color3f).Set((0.0, 0.1, 0.5))

    stReader = UsdShade.Shader.Define(stage, texturePath.AppendChild('stReader'))
    stReader.CreateIdAttr('UsdPrimvarReader_float2')

    diffuseTextureSampler = UsdShade.Shader.Define(stage, texturePath.AppendChild('diffuseTexture'))
    diffuseTextureSampler.CreateIdAttr('UsdUVTexture')
    diffuseTextureSampler.CreateInput('file', Sdf.ValueTypeNames.Asset).Set(filePath)
    diffuseTextureSampler.CreateInput("st", Sdf.ValueTypeNames.Float2).ConnectToSource(stReader.ConnectableAPI(), 'result')
    diffuseTextureSampler.CreateOutput('rgb', Sdf.ValueTypeNames.Float3)
    pbrShader.CreateInput("diffuseColor", Sdf.ValueTypeNames.Color3f).ConnectToSource(diffuseTextureSampler.ConnectableAPI(), 'rgb')

    material = UsdShade.Material.Define(stage, texturePath)
    material.CreateSurfaceOutput().ConnectToSource(pbrShader.ConnectableAPI(), "surface")

    stInput = material.CreateInput('frame:stPrimvarName', Sdf.ValueTypeNames.String)
    stInput.Set('st')

    stReader.CreateInput('varname',Sdf.ValueTypeNames.Token).ConnectToSource(stInput)
    return material 


#========================================================================||====#
class UsdManager :
    """
    A class to manage USD (Universal Scene Description) stages with functionality to create, manipulate,
    and save USD structures and associated materials.
    """

    def __init__(self) :
        """
        Initializes the UsdManager instance. Currently, this constructor does not perform any action.
        """
        return 


    def ClearCache(self) : 
        """
        Clears the USD stage cache to free up resources or prepare for a new session.
        """
        cache = UsdUtils.StageCache.Get()
        cache.Clear()   


    def Init(self, stage=None) : 
        """
        Creates a new USD stage and retrieves its unique identifier. Also initializes the material to None.
        """
        if stage : 
            self.stage = stage 
        else : 
            self.ClearCache() 
            self.stage = StageCreate()   

        self.id = StageIdGet(self.stage) 
        print(f"[UsdManager] stageId: '{self.id}'") 
        return 


    def StageSave(self, fname) :  
        """
        Saves the current USD stage to a file.

        Args:
            fname (str): Filename to which the USD stage will be exported.
        """
        print("[StageSave] '%s' saving..." % fname) 

        StageShow(self.stage) 
        self.stage.Export(r"%s" % fname)   



    def CubeCreate(self, path, bounds, thickness) :
        """
        Creates a cube mesh in the USD stage.
        """
        remove_prim_if_exists2(self.stage, path)
        self.mesh = CubeCreate(self.stage, path, bounds, thickness)

        prim = self.stage.GetPrimAtPath(path)
        create_operations(prim) 
        return prim 


    def PlaneCreate(self, path, bounds) : 
        """
        Creates a plane mesh in the USD stage.
        """
        remove_prim_if_exists2(self.stage, path)
        self.mesh = PlaneCreate(self.stage, path, bounds) 

        prim = self.stage.GetPrimAtPath(path)
        create_operations(prim) 
        return prim 


    def MeshAdd(self, path, points, vertexCounts, vertexIndices, displayColor=None) :
        #path = "/Mesh"
        extent = None  
        texCoord2 = None 
        #displayColor = None 
        self.mesh = MeshCreate(
                                self.stage, 
                                path, 
                                points, 
                                vertexCounts, 
                                vertexIndices, 
                                extent, 
                                texCoord2, 
                                displayColor
                            )
        return self.stage.GetPrimAtPath(path)


    def MeshMaterialSet(self) :  
        """
        Binds a material to the currently managed mesh in the USD stage.
        """
        #if hasattr(self, 'mesh'):
        if self.mesh : 
            self.mesh.GetPrim().ApplyAPI(UsdShade.MaterialBindingAPI)
            UsdShade.MaterialBindingAPI(self.mesh).Bind(self.material)
        else : 
            return 


    def MaterialCreate(self, path, diffuseColor, **opts) :
        """
        Creates a material with a specified diffuse color and optional properties.

        Args:
            diffuseColor (tuple): The RGB values for the diffuse color of the material.
                                  for instance, blue -> (0,0,1)
            opts (dict): Additional optional keyword arguments for material properties.
        """
        #remove_prim_if_exists(self.stage, self.material) 
        remove_prim_if_exists2(self.stage, path)
        self.material = MaterialCreate(self.stage, path, diffuseColor)
        self.MeshMaterialSet() 
        return self.stage.GetPrimAtPath(path)



    def TextureCreate(self, path, filePath, **opts) :
        """
        Creates a textured material using a specified image file.

        Args:
            filePath (str): Path to the texture image file.
            opts (dict): Additional optional keyword arguments for texture properties.
        """
        #path = "/Texture"
        #remove_prim_if_exists(self.stage, self.material) 
        remove_prim_if_exists2(self.stage, path)
        self.material = TextureCreate(self.stage, path, filePath)
        self.MeshMaterialSet() 
        return self.stage.GetPrimAtPath(path)
 

    def StageSet(self) : 
        """
        Applies cached stage settings to a USD import operation, using pre-configured import options.
        """
        #option = mxs.USDImporter.CreateOptions()
        ###opts.FileFormat = #ascii
        #mxs.USDImporter.ImportFromCache(self.id, importOptions=option)
        return 



    def StageShow(self) : 
        """
        Returns a string representation of the current USD stage for inspection or debugging.
        """
        txt = StageShow(self.stage)  
        return txt  


    def MaxShow(self, meshPath) : 
        """
        Displays detailed information about a mesh at a specified path within the 3ds Max environment.

        Args:
            meshPath (str): The path of the mesh within the USD structure to be inspected in 3ds Max.
        """
        #meshData = mxs.USDImporter.ConvertUsdMesh(self.id, meshPath) 
        #mesh = meshData.mesh
        #print("number of verts:", mesh.numVerts)
        #print("number of faces:", mesh.numFaces)
        return 


print("Leaving:'%s' ..." % path_root.name)
#=======================================================================||====#