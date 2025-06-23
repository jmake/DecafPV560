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
        SetDllDirectory("Assets\\Plugins"); // Unable to load DLL 'LibraryName'

        string directory = @"F:\z2025_1\Dicom\DicomTestImages1\matlab\examples\sample_data\DICOM\digest_article";

        Dicom dicom = new Dicom();
        dicom.LoadFile(directory);

        Console.WriteLine("[Tester] Finish");
    } // Main 

} // wrapper 