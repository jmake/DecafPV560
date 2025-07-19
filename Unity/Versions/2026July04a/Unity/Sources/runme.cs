using System;
using System.Linq;
using System.Runtime.InteropServices;
using System.Diagnostics;

using System.IO;
using System.Security.Cryptography;
using System.Text;

using System.Collections.Generic;

using SpicyTech;

public class wrapper 
{
    [DllImport("kernel32.dll", SetLastError = true)]
    static extern bool SetDllDirectory(string lpPathName);  

    static double maxCells = 2 * Math.Pow(2,20); // 2 * (1 048 576)


    static string GetFileHashSHA256(string filePath)
    {
        using (FileStream stream = File.OpenRead(filePath))
        using (SHA256 sha256 = SHA256.Create())
        {
            byte[] hashBytes = sha256.ComputeHash(stream);
            StringBuilder sb = new StringBuilder();
            foreach (byte b in hashBytes)
                sb.Append(b.ToString("x2"));
            return sb.ToString();
        }
    }


    static void Main() 
    {
        // LibraryName.dll location relative to the .exe executable 
        SetDllDirectory("Plugins"); // Unable to load DLL 'LibraryName'

        WrappingTest();
        DicomTest(); 
        NiftiTests(); 

        Console.WriteLine("[Tester] Finish");
    } // Main 


    static void WrappingTest() 
    {
        using (DataHolder holder = new DataHolder(9_999))
        {
            unsafe {
                ulong address = holder.GetDataAddress(); 
                IntPtr ptr = new IntPtr((long)address);  

                ulong size = holder.GetDataSize();

                float* data = (float*)ptr.ToPointer();

                for (ulong i = 0; i < size; i++)
                {
                    //Console.WriteLine(data[i]);
                }
                Console.WriteLine($"[WrappingTest] size : {size}, data[size-1] : {data[size-1]}");

                if (data[size-1] != 9998)
                    throw new InvalidOperationException($"[Tester] data[size-1] ({data[size-1]}) != 9998 ");

            } // unsafe 
        }
    }


    static void DicomTest() 
    {
        /*
        return; 

        string directory; 
        
        Dicom dicom = new Dicom();

        directory= @"F:\z2025_1\Dicom\DicomTestImages1\matlab\examples\sample_data\DICOM\digest_article";
        dicom.LoadFile(directory);

        Console.WriteLine("[Tester] DicomTest Done!!");
        */
    } // DicomTest


    static void NiftiTests() 
    {
        NifftiLoaderTest1(); 

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
        VtkLoaderTest2("nifti0.vti"); 
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
            bool succes = loader.LoadFile(fname, "voxel"); 
            if (!succes)
                throw new InvalidOperationException($"[VtkLoaderTest1]  '{fname}' fail!!");

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

    static void VtkLoaderTest2(string fname) 
    {
        using (VtkLoader loader = new VtkLoader())
        {
            loader.Init(); 
            bool succes = loader.LoadFile(fname, "voxel"); 
            if (!succes)
                throw new InvalidOperationException($"[VtkLoaderTest2]  '{fname}' fail!!");

            float[] range = {0, 0}; 
            loader.RangeGet(range); 
            Console.WriteLine($"[VtkLoaderTest2] range:[{range[0]}, {range[1]}]"); 

            int[] dimensions = {0, 0, 0}; 
            loader.DimensionsGet(dimensions); 

            int n_indices = dimensions[0] / 3; 
            int n_vertices = dimensions[1] / 3; 
            int n_property = dimensions[2]; 
            Console.WriteLine($"[VtkLoaderTest2] n_indices:{n_indices} n_vertices:{n_vertices} n_property:{n_property} "); 

            if (n_indices != 8928000)
                throw new InvalidOperationException($"[VtkLoaderTest2] n_indices ({n_indices}) != 8928000");

            if (n_vertices != 0)
                throw new InvalidOperationException($"[VtkLoaderTest2] n_vertices ({n_vertices}) != 0");

            if (n_property != 8928000)
                throw new InvalidOperationException($"[VtkLoaderTest2] n_property ({n_property}) != 8928000");

            if (n_property != n_indices)
                throw new InvalidOperationException($"[VtkLoaderTest2] n_vertices != n_indices");

            int[] indices = new int[dimensions[0]]; 
            float[] vertices = new float[dimensions[1]]; 
            float[] property = new float[dimensions[2]]; 
            loader.GeometryGet(indices, vertices, property); 

            int id = 1817440; 
            Console.WriteLine($"[VtkLoaderTest2] id:{id} property:{property[id]} "); 
            if (property[id] != 3)
                throw new InvalidOperationException($"[VtkLoaderTest2] id:{id} property:{property[id]} != 3");

            //int idx = x + y * dims[0] + z * dims[0] * dims[1] -> (160, 132, 31) 
            Console.WriteLine($"[VtkLoaderTest2] id:{id} x:{indices[id*3+0]}, y:{indices[id*3+1]}, z:{indices[id*3+2]} "); 
/*
            return ; 
            if ( 
                    Math.Abs(indices[id*3+0] - 160) > 1e-3 || 
                    Math.Abs(indices[id*3+1] - 132) > 1e-3 || 
                    Math.Abs(indices[id*3+2] +  31) > 1e-3 
                )
                throw new InvalidOperationException($"[VtkLoaderTest2] id:{id} vertices ERROR!!");
*/
            loader.Finish(); 
        }
    }


    static void NifftiLoaderTest1() 
    {
        string key = "voxel"; 
        NifftiLoader nifti = new NifftiLoader(); 

        string directory = "F:\\z2025_1\\Dicom\\NII\\1.2.826.0.1.3680043.10633.nii.gz"; 
        bool succes = nifti.LoadFile(directory, key); 
        if (!succes)
            throw new InvalidOperationException($"[NifftiLoaderTest1]  '{directory}' fail!!");

        nifti.Save("NifftiLoaderTest1"); 

        string hash, expected0;  
        hash = GetFileHashSHA256("NifftiLoaderTest1.vti");
        expected0 = "2059cb1d1959d97c647945315288dcf3bf403ab625be5346d85a947bb921b32d"; 
        if(hash != expected0) throw new InvalidOperationException($"[NifftiLoaderTest1] invalid test ERROR!!");

        CutterManager manager = new CutterManager(); 
        manager.VtiSet( nifti.GetObj() , key); 

        List<string> hashs = new  List<string>(); 
        hashs.Add("ac9fc1b9771a51e5082c67def34a0b8674e5fe909f6ac5c595ec48976ce972f3"); 
        hashs.Add("c7289d8dfb963dfa559e92280886719029e977610b3d208647a3becce2386453"); 

        List<float[]> normals = new List<float[]>();
        normals.Add(new float[] { 1.0f, 0.0f, 0.0f });
        normals.Add(new float[] { 1.0f, 1.0f, 1.0f });

        foreach (var (normal,expected) in normals.Zip(hashs, (i,j) => (i,j)))
        {
            manager.Update(normal[0],normal[1],normal[2]); 
            manager.Save("NifftiLoaderTest1a"); 

            int[] dimensions = {0, 0, 0}; 
            manager.DimensionsGet(dimensions); 

            int n_indices = dimensions[0] / 3; 
            int n_vertices = dimensions[1] / 3; 
            int n_property = dimensions[2]; 
            Console.WriteLine($"[NifftiLoaderTest1] n_indices:{n_indices} n_vertices:{n_vertices} n_property:{n_property} "); 

if (n_property <= 0)
    throw new InvalidOperationException($"[NifftiLoaderTest1]  n_property -> {n_property} fail!!");


            int[] indices = new int[dimensions[0]]; 
            float[] vertices = new float[dimensions[1]]; 
            float[] property = new float[dimensions[2]]; 
            manager.GeometryGet(indices, vertices, property); 

            hash = GetFileHashSHA256("NifftiLoaderTest1a.vtp");
            if(hash == expected) 
                Console.WriteLine($"[NifftiLoaderTest1] hash : '{hash}' correct!!");
            else
                throw new InvalidOperationException($"[NifftiLoaderTest1] invalid test ERROR!!");
        }

        manager.Finish(); 

        nifti.Finish(); 
    }

} // wrapper 