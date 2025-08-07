clear

cd E:\MyProject5 

rm E:\MyProject5\Saved\*.png

rm E:\MyProject5\Plugins\SpicyWrapper\Binaries\Win64\LibrarySnippet.*
rm E:\MyProject5\Plugins\SpicyWrapper\Source\ThirdParty\LibrarySnippet.*
rm E:\MyProject5\Plugins\SpicyWrapper\Source\ThirdParty\Include\SpicyTechDicom2.hpp

rm E:\MyProject5\Plugins\SpicyWrapper\Source\ThirdParty\zd.dll 

<# 
    The game module 'MyProject5' could not be loaded. 
    There may be an operating system error, the module may not be properly set up, or 
    a plugin which has been included into the build has not been turned on.
#>
cp F:\z2025_1\Dicom\DecafPV560\Unreal\Build\LibrarySnippet.* `
E:\MyProject5\Plugins\SpicyWrapper\Binaries\Win64

cp F:\z2025_1\Dicom\DecafPV560\Unreal\Build\LibrarySnippet.* `
E:\MyProject5\Plugins\SpicyWrapper\Source\ThirdParty

cp F:\z2025_1\Dicom\zlib\Execs\bin\zd.dll `
E:\MyProject5\Plugins\SpicyWrapper\Source\ThirdParty

cp F:\z2025_1\Dicom\DecafPV560\Unreal\Sources\SpicyTechDicom2.hpp `
E:\MyProject5\Plugins\SpicyWrapper\Source\ThirdParty\Include\

#& "E:\UE_5.5\Engine\Build\BatchFiles\Build.bat" MyProject5 Win64 Development -Project="E:\MyProject5\MyProject5.uproject" -Clean 

#& "E:\UE_5.5\Engine\Build\BatchFiles\Build.bat" MyProject5Editor Win64 Development -Project="E:\MyProject5\MyProject5.uproject" -Clean 

#& "E:\UE_5.5\Engine\Build\BatchFiles\Build.bat" MyProject5 Win64 Development -Project="E:\MyProject5\MyProject5.uproject"

& "E:\UE_5.5\Engine\Build\BatchFiles\Build.bat" MyProject5Editor Win64 Development -Project="E:\MyProject5\MyProject5.uproject"

& "E:\UE_5.5\Engine\Binaries\Win64\UnrealEditor.exe" "E:\MyProject5\MyProject5.uproject"  #-game -log

