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
                float* data = (float*)ptr.ToPointer();

                ulong size = holder.GetDataSize();
                for (ulong i = 0; i < size; i++)
                {
                    //Console.WriteLine(data[i]);
                }
                Console.WriteLine($"[WrappingTest] size : {size}, data[size-1] : {data[size-1]:F9}");
                Console.WriteLine( data[size-1] );

                if ( Math.Abs(data[size-1] - 9999.123456789 ) > 0.001)
                    throw new InvalidOperationException($"[Tester] data[size-1] ({data[size-1]}) != 9999 ");

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
        ExecutionTime(NifftiLoaderTest3, "[NifftiLoaderTest3]"); //
/*
        ExecutionTime(NifftiLoaderTest2, "[NifftiLoaderTest2]"); // ~ 6.5 seg

        ExecutionTime(NifftiLoaderTest1, "[NifftiLoaderTest1]"); 
        ExecutionTime(NiftiTest1, "[NiftiTest1]"); 
        ExecutionTime(NiftiTest2, "[NiftiTest2]"); 
// */
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

if (n_property <= 0) throw new InvalidOperationException($"[NifftiLoaderTest1]  n_property -> {n_property} fail!!");


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


    static void NifftiLoaderTest3() 
    {
/*
NifftiLoaderTest3 ~ 6.663 seg 
                -> 2.38 seg (CutterPlane3) 
                -> 1.178 (extract = false)
                -> 1.288 (new method 'GetProperty' and 'this->obj = cutter3', 'this->SetObj' removed)

[LoadFile]       Execution time : ~ 1.163 seg
[CutterPlane3]   Execution time : ~ 0.012 seg
[GetProperty]    Execution time : ~ 0.002 seg
[GetDataAddress] Execution time :   0.000 seg   <---- 
*/
        bool extract = false;
        bool centered = true;
        NifftiLoader nifti = new NifftiLoader(); 

        string key = "voxel"; 
        string directory = "F:\\z2025_1\\Dicom\\NII\\1.2.826.0.1.3680043.10633.nii.gz"; 
        bool success = nifti.LoadFile(directory, key, centered, extract); 
nifti.Save("domain"); 

        double[] bounds = {0,0,0, 0,0,0}; 
        nifti.BoundsGet(bounds); // xmin,xmax, ymin,ymax, zmin,zmax
        Console.WriteLine($"[NifftiLoaderTest 3] nifti bounds:[{bounds[0]}, {bounds[1]}], [{bounds[2]}, {bounds[3]}], [{bounds[4]}, {bounds[5]}]"); 

        float[] range = {0, 0}; 
        nifti.RangeGet(key, range); // Slow!!
        Console.WriteLine($"[NifftiLoaderTest 3] nifti range:[{range[0]}, {range[1]}] "); 

        if(true)
        {
            CutterManager2 manager = new CutterManager2();   // 
            manager.VtiSet( nifti.GetObj(), "ImageScalars"); // 

//            manager.Update(1.0f, 1.0f, 1.0f);                // 


double[]  axisX1 = {1, 0, 0};
double[]  axisY1 = {0, 1, 0};
double[]  axisZ1 = {0, 0, 1};
double[] center1 = {50, 100, -50}; 
manager.Update(axisX1, axisY1, axisZ1, center1); 
//manager.SetOrigin(0.0, 0.0, center1[2]); 
manager.Save("axial"); 


double[]  axisX2 = {1,  0, 0};
double[]  axisY2 = {0,  0, 1};
double[]  axisZ2 = {0, -1, 0};
double[] center2 = {-1000,-25, 1000}; 
manager.Update(axisX2, axisY2, axisZ2, center2); 
manager.Save("coronal"); // -> rotate X90 (normal -> 0,-1,0)

double[]  axisX3 = {0, 0,-1};
double[]  axisY3 = {1, 0, 0};
double[]  axisZ3 = {0,-1, 0};
double[] center3 = {0, 0, 0}; 
manager.Update(axisX3, axisY3, axisZ3, center3); 
manager.Save("sagittal"); // -> rotate Y90/Z-90  (normal -> 0,-1,0)



double[]  axisX0 = {0.707107, -0.707107, 0};
double[]  axisY0 = {-0.408248, -0.408248, 0.816497};
double[]  axisZ0 = {0.57735, 0.57735, 0.57735};
double[] center0 = {0, 0, 0}; 
manager.Update(axisX0, axisY0, axisZ0, center0); 
manager.Save("oblique"); 

            double[] spacing = {0, 0, 0}; 
            manager.GetSpacing(spacing); 
            Console.WriteLine($"[NifftiLoaderTest 3] spacing : ({spacing[0]},{spacing[1]},{spacing[2]}) "); 

            double[] origin = {0, 0, 0}; 
            manager.GetOrigin(origin); 
            Console.WriteLine($"[NifftiLoaderTest 3] origin : ({origin[0]},{origin[1]},{origin[2]}) "); 

            manager.BoundsGet(bounds); // xmin,xmax, ymin,ymax, zmin,zmax
            Console.WriteLine($"[NifftiLoaderTest 3] Cutter bounds:[{bounds[0]}, {bounds[1]}], [{bounds[2]}, {bounds[3]}], [{bounds[4]}, {bounds[5]}]"); 

            int[] pointsByAxis = {0, 0, 0}; 
            int nPoints = manager.PointsByAxis(pointsByAxis); 
            Console.WriteLine($"[NifftiLoaderTest 3] pointsbyAxis : ({pointsByAxis[0]},{pointsByAxis[1]},{pointsByAxis[2]}) -> {nPoints}"); 
            if (nPoints != 742938) throw new InvalidOperationException($"[NifftiLoaderTest 3] nPoints ({nPoints}) != 742938");


            // 
            // px = origin[0] + x * spacing[0];
            // id = x  +  y * pointsbyAxis[0]  +  z * pointsbyAxis[0] * pointsbyAxis[1] 
            //
            //int id = 318 + 485 * 722 + 0 * 722 * 1029; // = 350488 -> 3 
            int id = 432067; // -> 3, (311,598,0)
            int x = id % pointsByAxis[0];
            int y = id / pointsByAxis[0];

            if (x != 311) throw new InvalidOperationException($"[NifftiLoaderTest 3] id:{id} x:{x} wrong!!");
            if (y != 598) throw new InvalidOperationException($"[NifftiLoaderTest 3] id:{id} y:{y} wrong!!");

            // Coping memory 
            if(true) 
            {
                float[] property = new float[nPoints]; 
                manager.GetProperty(property); // ~ 0.002 seg
                Console.WriteLine($"[NifftiLoaderTest 3] id:{id}/{property.Length} property:{property[id]} ({origin[0] + x * spacing[0]},{origin[1] + y * spacing[1]}) "); 

                if (property[id] != 3) throw new InvalidOperationException($"[NifftiLoaderTest 3] id:{id} property:{property[id]} wrong!!");
            }

            // Accessing memory (crassing in Unity)
            unsafe {
                int[] Length = {0}; 
                ulong address = manager.GetDataAddress2(Length); // 0 seg
                IntPtr ptr = new IntPtr((long)address);  
                float* property = (float*)ptr.ToPointer(); // unmanaged pointer

                Console.WriteLine($"[NifftiLoaderTest 3] id:{id}/{Length[0]} property:{property[id]} ({x},{y}) "); 

                if (property[id] != 3) throw new InvalidOperationException($"[NifftiLoaderTest 3] id:{id} property:{property[id]} wrong!!");
            } // unsafe

            //manager.Save("cutter3"); 
            manager.Finish(); 
        }

        if(true)
        {
            Contour2 manager = new Contour2();                     // ~  seg  
            manager.VtiSet( nifti.GetObj(), key, range, maxCells); // ~ 1.971 seg / 764550 cells (GetContour)
            manager.Update(0.5f, true);                            // 
            //manager.Save("contour"); 

            int[] dimensions = {0, 0, 0}; 
            manager.DimensionsGet(dimensions); // ~ 0.0 seg 

            int n_indices = dimensions[0] / 3; 
            int n_vertices = dimensions[1] / 3; 
            int n_property = dimensions[2]; 
            Console.WriteLine($"[NifftiLoaderTest 3] n_indices:{n_indices} n_vertices:{n_vertices} n_property:{n_property} "); 

            if (n_indices != 764_550)
                throw new InvalidOperationException($"[NifftiLoaderTest 3] n_indices ({n_indices}) != 764550");

            if (n_vertices != 382_824)
                throw new InvalidOperationException($"[NifftiLoaderTest 3] n_vertices ({n_vertices}) != 382824");

            int[] indices = new int[dimensions[0]]; 
            float[] vertices = new float[dimensions[1]]; 
            float[] property = new float[dimensions[2]]; 
            manager.GeometryGet(indices, vertices, property); // ~ 0.002 seg 

            int id = 382805;
            Console.WriteLine($"[NifftiLoaderTest 3] id:{id} x:{vertices[id*3+0]}, x:{vertices[id*3+1]}, x:{vertices[id*3+2]} "); 
            if ( 
                    Math.Abs(vertices[id*3+0] - 22.2422) > 1e-3 || 
                    Math.Abs(vertices[id*3+1] - 22.2303) > 1e-3 || 
                    Math.Abs(vertices[id*3+2] - 43.4167) > 1e-3 
                )
                throw new InvalidOperationException($"[NifftiLoaderTest 3] id:{id} vertices ERROR!!");

            manager.Finish(); 
        } 

        nifti.Finish(); 
    }


    static void NifftiLoaderTest2() 
    {
/*
 total  | GetFlatStructuredCoordinates |             | ArrayGet | GetFlatCellIndices
        |               GeometryUpdate | GetCppArray | 
 21.035 |                       12.596 |       1.085 |
  9.612 |                        1.083 |             |
  8.717 |                        1.083 |       0.106 |      2.0 |
  6.873 |                        1.083 |       0.106 |      0.1 |

GetFlatCellIndices : 0.736 seg -> 0.095 (6.494 Total)

CutterPlane : 3.763 -> CutterPlane2 : 0.135 (6.277 -> 2.548 Total)
                    -> CutterPlane3 : 0.011  

SEE : https://examples.vtk.org/site/VTKBook/02Chapter2/
*/
        string key = "voxel"; 
        NifftiLoader nifti = new NifftiLoader(); 

        string directory = "F:\\z2025_1\\Dicom\\NII\\1.2.826.0.1.3680043.10633.nii.gz"; 
        bool succes = nifti.LoadFile(directory, key); 

        if(true)
        {
            CutterManager manager = new CutterManager(); // 0.00 / 6.118 seg
            manager.VtiSet( nifti.GetObj() , key);       // 2.29 / 6.118 seg
            manager.Update(1.0f, 1.0f, 1.0f);            // 3.95 / 6.118 seg

            int[] dimensions = {0, 0, 0}; 
            manager.DimensionsGet(dimensions); 

            int n_indices = dimensions[0] / 3; 
            int n_vertices = dimensions[1] / 3; 
            int n_property = dimensions[2]; 
            Console.WriteLine($"[NifftiLoaderTest 2] n_indices:{n_indices} n_vertices:{n_vertices} n_property:{n_property} "); 

            int[] indices = new int[dimensions[0]]; 
            float[] vertices = new float[dimensions[1]]; 
            float[] property = new float[dimensions[2]]; 
            manager.GeometryGet(indices, vertices, property); 
            manager.Finish(); 
        }

        nifti.Finish(); 
    } 


    static public 
    void ExecutionTime(Action action, string label)
    {
        Console.WriteLine($"===============================================================================");

        System.Diagnostics.Stopwatch stopwatch = System.Diagnostics.Stopwatch.StartNew();
        action();
        stopwatch.Stop();

        float dtime = stopwatch.ElapsedMilliseconds / 1000.0f; 
        Console.WriteLine($"{label} time : {dtime} seg ");

        Console.WriteLine($"===============================================================================");
    }


} // wrapper 