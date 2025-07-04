using System;
using System.Linq;
using System.Runtime.InteropServices;
using System.Diagnostics;

using SpicyTech;

public class wrapper 
{
    [DllImport("kernel32.dll", SetLastError = true)]
    static extern bool SetDllDirectory(string lpPathName);  

    static double maxCells = 2 * Math.Pow(2,20); // 2 * (1 048 576)

    static void Main() 
    {
        // LibraryName.dll location relative to the .exe executable 
        SetDllDirectory("Plugins"); // Unable to load DLL 'LibraryName'

        DicomTest(); 
        NiftiTests(); 

        Console.WriteLine("[Tester] Finish");
    } // Main 


    static void DicomTest() 
    {
        return; 

        string directory; 
        
        Dicom dicom = new Dicom();

        directory= @"F:\z2025_1\Dicom\DicomTestImages1\matlab\examples\sample_data\DICOM\digest_article";
        dicom.LoadFile(directory);

        Console.WriteLine("[Tester] DicomTest Done!!");
    } // DicomTest


    static void NiftiTests() 
    {
        NiftiTest1(); 
        NiftiTest2();
    }


    static void NiftiTest1() 
    {        
        using (Nifti nifti = new Nifti())
        {
            string directory = "F:\\z2025_1\\Dicom\\NII\\BRATS_001.nii.gz";
            nifti.LoadFile( directory, true ); 
            nifti.VtiSave("nifti0"); 

            nifti.CutCreate("voxel"); 

            nifti.CutUpdate(0.0f,0.0f,0.0f, 0.0f,1.0f,0.0f); 
            nifti.CutSave("cutter0"); // -> cutter0.vtp
        }

        VtkLoaderTest1("cutter0.vtp"); 
    }


    static void NiftiTest2() 
    {
        using (Nifti nifti = new Nifti())
        {
            string directory = "F:\\z2025_1\\Dicom\\NII\\1.2.826.0.1.3680043.10633.nii"; 
            nifti.LoadFile(directory, false); 
 
            // Contour 
            nifti.ContourCreate("voxel", (float)maxCells); 
            nifti.ContourUpdate(55.0f, 10.0f, 100.0f); 
            nifti.ContourSave("contour_a"); 
            NiftiTest2_CountourVerify(nifti); 

            // Slicer  
            nifti.CutCreate("voxel"); 
            nifti.CutSave("cutter_yz"); 

            nifti.CutUpdate(0.0f,0.0f,0.0f, 0.0f,0.0f,1.0f); 
            nifti.CutSave("cutter_xy"); 
            NiftiTest2_SlicerVerify(nifti); 

            nifti.VtiSave("nifti"); 
        }

        Console.WriteLine("[Tester] NiftiTest Done!!");
    } // NiftiTest 


    static void NiftiTest2_SlicerVerify(Nifti nifti) 
    {
        int[] dimensions = {0, 0, 0}; 
        nifti.CutDimensions(dimensions); 

        int n_indices = dimensions[0] / 3; 
        int n_vertices = dimensions[1] / 3; 
        int n_property = dimensions[2]; 
        Console.WriteLine($"[Tester] n_indices:{n_indices} n_vertices:{n_vertices} n_property:{n_property} "); 

        if (n_indices != 522242)
            throw new InvalidOperationException($"[Tester] n_indices ({n_indices}) != 764550");

        if (n_vertices != 262144)
            throw new InvalidOperationException($"[Tester] n_vertices ({n_vertices}) != 382824");

        if (n_property != n_vertices)
            throw new InvalidOperationException($"[Tester] n_vertices != n_property");

        int[] indices = new int[dimensions[0]]; 
        float[] vertices = new float[dimensions[1]]; 
        float[] property = new float[dimensions[2]]; 
        nifti.CutGeometry(indices, vertices, property); 

        int id = 148209; 
        Console.WriteLine($"[Tester] id:{id} property:{property[id]} "); 
        if (property[id] != 3)
            throw new InvalidOperationException($"[Tester] id:{id} property:{property[id]} != 3");

        Console.WriteLine($"[Tester] id:{id} x:{vertices[id*3+0]}, x:{vertices[id*3+1]}, x:{vertices[id*3+2]} "); 
        if ( 
                Math.Abs(vertices[id*3+0] - 68.72266) > 1e-3 || 
                Math.Abs(vertices[id*3+1] - 82.41016) > 1e-3 || 
                Math.Abs(vertices[id*3+2] - 107) > 1e-3 
            )
            throw new InvalidOperationException($"[Tester] id:{id} vertices ERROR!!");
    }


    static void NiftiTest2_CountourVerify(Nifti nifti) 
    {
        int[] dimensions = {0, 0, 0}; 
        nifti.ContourDimensions(dimensions); 

        int n_indices = dimensions[0] / 3; 
        int n_vertices = dimensions[1] / 3; 
        int n_property = dimensions[2]; 
        Console.WriteLine($"[Tester] n_indices:{n_indices} n_vertices:{n_vertices} n_property:{n_property} "); 

        if (n_indices != 764550)
            throw new InvalidOperationException($"[Tester] n_indices ({n_indices}) != 764550");

        if (n_vertices != 382824)
            throw new InvalidOperationException($"[Tester] n_vertices ({n_vertices}) != 382824");

        if (n_property != n_vertices)
            throw new InvalidOperationException($"[Tester] n_vertices != n_property");

        int[] indices = new int[dimensions[0]]; 
        float[] vertices = new float[dimensions[1]]; 
        float[] property = new float[dimensions[2]]; 
        nifti.ContourGeometry(indices, vertices, property); 
    }


    static void VtkLoaderTest1(string fname) 
    {
        using (VtkLoader loader = new VtkLoader())
        {
            loader.Init(); 
            loader.LoadFile(fname, "voxel"); 

            float[] range = {0, 0}; 
            loader.RangeGet(range); 
            Console.WriteLine($"[VtkLoaderTest1] range:[{range[0]}, {range[1]}]"); 

            int[] dimensions = {0, 0, 0}; 
            loader.DimensionsGet(dimensions); 

            int n_indices = dimensions[0] / 3; 
            int n_vertices = dimensions[1] / 3; 
            int n_property = dimensions[2]; 
            Console.WriteLine($"[VtkLoaderTest1] n_indices:{n_indices} n_vertices:{n_vertices} n_property:{n_property} "); 

            if (n_indices != 73612)
                throw new InvalidOperationException($"[VtkLoaderTest1] n_indices ({n_indices}) != 73612");

            if (n_vertices != 37200)
                throw new InvalidOperationException($"[VtkLoaderTest1] n_vertices ({n_vertices}) != 37200");

            if (n_property != n_vertices)
                throw new InvalidOperationException($"[VtkLoaderTest1] n_vertices != n_property");

            int[] indices = new int[dimensions[0]]; 
            float[] vertices = new float[dimensions[1]]; 
            float[] property = new float[dimensions[2]]; 
            loader.GeometryGet(indices, vertices, property); 

            int id = 12640; 
            Console.WriteLine($"[VtkLoaderTest1] id:{id} property:{property[id]} "); 
            if (property[id] != 2)
                throw new InvalidOperationException($"[VtkLoaderTest1] id:{id} property:{property[id]} != 2");

            Console.WriteLine($"[VtkLoaderTest1] id:{id} x:{vertices[id*3+0]}, x:{vertices[id*3+1]}, x:{vertices[id*3+2]} "); 
            if ( 
                    Math.Abs(vertices[id*3+0] - 40.5) > 1e-3 || 
                    Math.Abs(vertices[id*3+1] - 0.0) > 1e-3 || 
                    Math.Abs(vertices[id*3+2] + 25.0) > 1e-3 
                )
                throw new InvalidOperationException($"[VtkLoaderTest1] id:{id} vertices ERROR!!");

            loader.Finish(); 
        }
    }


} // wrapper 