using System;
using System.Linq;
using System.Runtime.InteropServices;

using SpicyTech;

public class wrapper 
{
    [DllImport("kernel32.dll", SetLastError = true)]
    static extern bool SetDllDirectory(string lpPathName);  

    static void Main() 
    {
        // LibraryName.dll location relative to the .exe executable 
        SetDllDirectory("Plugins"); // Unable to load DLL 'LibraryName'

        string directory; 
        
        directory= @"F:\z2025_1\Dicom\DicomTestImages1\matlab\examples\sample_data\DICOM\digest_article";
        Dicom dicom = new Dicom();
        //dicom.LoadDicom(directory);

        directory = "F:\\z2025_1\\Dicom\\NII\\BRATS_001.nii.gz";
        dicom.LoadNifti( directory ); 

        directory = "F:\\z2025_1\\Dicom\\NII\\1.2.826.0.1.3680043.10633.nii"; 
        dicom.LoadNifti( directory ); 

        Console.WriteLine("[Tester] Finish");
    } // Main 

} // wrapper 