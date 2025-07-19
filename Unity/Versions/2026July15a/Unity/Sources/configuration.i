%module SpicyTech

%{
#include "SpicyTechDicom.hpp"
%}

%include "std_vector.i"
%template(VectorInt) std::vector<int>;
%template(VectorFloat) std::vector<float>;

%include "typemaps.i"
%include "arrays_csharp.i"

%apply double INOUT[] { double* arrayDouble1 };
%apply double INOUT[] { double* arrayDouble2 };
%apply double INOUT[] { double* arrayDouble3 };
%apply double INOUT[] { double* arrayDouble4 };
%apply  float INOUT[] {  float*  arrayFloat1 };
%apply  float INOUT[] {  float*  arrayFloat2 };
%apply    int INOUT[] {    int*    arrayInt1 };
%apply    int INOUT[] {    int*    arrayInt2 };


%include "std_string.i"

%include "typemaps.i"
%include "cpointer.i"

/*
%typemap(csout) void* "new System.IntPtr($imcall)"
%typemap(cstype) void* "System.IntPtr"
%typemap(imtype) void* "System.IntPtr"
*/
/*
%typemap(csout) float* GetDataPtr { return new System.IntPtr($imcall); }
%typemap(imtype) float* "System.IntPtr"
%typemap(cstype) float* "System.IntPtr"
*/
/*
%typemap(csout) float* SpicyTech::DataHolder::GetDataPtr { return new System.IntPtr($imcall); }
%typemap(imtype) float* SpicyTech::DataHolder::GetDataPtr "System.IntPtr"
%typemap(cstype) float* SpicyTech::DataHolder::GetDataPtr "System.IntPtr"
*/

%include "stdint.i"  // for uintptr_t
%include "SpicyTechDicom.hpp"

/*
%include "arrays_csharp.i"

%apply int  INPUT[] { int* sourceArray }
%apply int OUTPUT[] { int* targetArray }

%apply int  INOUT[] { int* array1 }
%apply int  INOUT[] { int* array2 }
%apply int  INOUT[] { int* array3 }
%apply int  INOUT[] { int* array4 }


%include "spicytech.hpp"
*/
/*
%clear int* sourceArray;
%clear int* targetArray;

%clear int* array1;
%clear int* array2;
%clear int* array3;
%clear int* array4;
*/
// Below replicates the above array handling but this time using the pinned (fixed) array typemaps
//%csmethodmodifiers "public unsafe";
/*
%apply int FIXED[] { int* sourceArray }
%apply int FIXED[] { int* targetArray }

%inline %{
void myArrayCopyUsingFixedArrays( int *sourceArray, int* targetArray, int nitems ) {
  myArrayCopy(sourceArray, targetArray, nitems);
}
%}

%apply int FIXED[] { int* array1 }
%apply int FIXED[] { int* array2 }
*/

/*
%inline %{
class MyClass2 {
  public:
    MyClass2(int x) : value(x) {}
    int GetValue() const { return value; }
    void SetValue(int x) { value = x; }
  private:
    int value;
};
%}
*/
//%warnfilter(SWIGWARN_TYPEMAP_SWIGTYPELEAK); // Silence typemap warnings