using System;
using System.Linq;
using System.Runtime.InteropServices;
using System.Diagnostics;

using SpicyTech;

public class wrapper 
{
    [DllImport("kernel32.dll", SetLastError = true)]
    static extern bool SetDllDirectory(string lpPathName);  


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
            nifti.LoadFile( directory ); 
            nifti.VtiSave("nifti"); 

            nifti.CutCreate("voxel"); 
        }
    }


    static void NiftiTest2() 
    {
        using (Nifti nifti = new Nifti())
        {
            string directory = "F:\\z2025_1\\Dicom\\NII\\1.2.826.0.1.3680043.10633.nii"; 
            nifti.LoadFile(directory); 
 
            nifti.CutCreate("voxel"); 
            nifti.CutSave("cutter_yz"); 

            nifti.ContourCreate("voxel"); 
            nifti.ContourUpdate(55.0f, 10.0f, 100.0f); 

            nifti.ContourSave("contour_a"); 

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

            nifti.CutUpdate(0.0f,0.0f,0.0f, 0.0f,0.0f,1.0f); 
            nifti.CutSave("cutter_xy"); 

            nifti.VtiSave("nifti"); 
        }

        Console.WriteLine("[Tester] NiftiTest Done!!");
    } // NiftiTest 


} // wrapper 