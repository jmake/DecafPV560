## ImportError: DLL load failed while importing _LibraryName: The specified module could not be found.
import shutil
shutil.copy("F:/z2025_1/Dicom/zlib/Execs/bin/zd.dll",".")

import sys; 
sys.path.append(r"F:\z2025_1\Dicom\DecafPV560\Omniverse\Build")

"""
cp "F:/z2025_1/Dicom/zlib/Execs/bin/zd.dll" .
"""
import SpicyTech 


import numpy as np

vtkTest = SpicyTech.VtkTest()

arr = np.array([1.0, 2.0, 3.0, 4.0]) #, dtype=np.float64)
#print("[runme] :", arr)
vtkTest.NumpyPrint(arr); 

data = np.array( vtkTest.GetData() )
print("[runme] received:", data )

print("ok!")