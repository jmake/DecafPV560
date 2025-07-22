import omni.usd
from pxr import Usd, UsdVol, Sdf
from pxr import UsdShade


vdb_path = "F:\z2025_1\Dicom\DecafPV560\Omniverse\rsnaCervicalSpineFractureDetection.vdb"
vdb_path = "F:\z2025_1\Dicom\DecafPV560\Omniverse\smoke.vdb"


def Init() : 
    stage = omni.usd.get_context().get_stage()

    volume_path = "/MyVDBVolume"
    volume_prim = stage.DefinePrim(volume_path, "Volume")

    field_path = f"{volume_path}/density"
    field_prim = stage.DefinePrim(field_path, "OpenVDBAsset")

    field_prim.GetAttribute("filePath").Set( Sdf.AssetPath(vdb_path) )
    field_prim.GetAttribute("fieldName").Set("density")  # Replace if grid name differs

    vol_schema = UsdVol.Volume(volume_prim)
    vol_schema.CreateFieldRelationship("density", field_prim.GetPath())
    return volume_prim


def CreateMaterial(prim) : 
    material_path = Sdf.Path("/VolumeMaterial")
    material = UsdShade.Material.Define(stage, material_path)

    shader = UsdShade.Shader.Define(stage, material_path.AppendPath("OmniVolume"))
    shader.CreateIdAttr("OmniVolumeSurface")
    shader.CreateInput("density", Sdf.ValueTypeNames.Float).Set(1.0)

    shader_output = shader.CreateOutput("volume", Sdf.ValueTypeNames.Token)
    material_output = material.CreateOutput("volume", Sdf.ValueTypeNames.Token)
    material_output.ConnectToSource(shader_output)

    UsdShade.MaterialBindingAPI(prim).Bind(material)
    return 


volume_prim = Init() 
CreateMaterial(volume_prim) 


r"""
SEE : 
'- OmniVolumeDensity
    https://docs.omniverse.nvidia.com/kit/docs/omni.vdb/latest/source/extensions/omni.vdb.neuralvdb/docs/tutorials/basics.html

    **In USD Composer:**
    4\. Switch the renderer to **RTX - Interactive (Path Tracing)**.
    5\. Under Render Settings, enable **Non-uniform Volumes**.
    6\. Create a cube and an OmniVolumeDensity material in a new stage.
    7\. Select the cube, then enable **primvars\:isVolume** under **Extra Properties** in the Property tab.
    8\. Select the OmniVolumeDensity material from the Stage tab.
    9\. In the Property tab, under **Base > Volume Density Texture**, pick your NeuralVDB file.
    10\. Wait a few seconds for the volume to decode (time depends on the size).
    11\. The decoded VDB volume will appear in the viewport.


"""