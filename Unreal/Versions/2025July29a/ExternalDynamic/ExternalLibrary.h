#pragma once


// Allow generating '.exp' and '.lib' in other case only '.dll' is generated! 
#ifdef EXTERNALMATH_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

class DLL_API ExternalLibrary { // -> '.dll', '.exp' and '.lib' 
//class  ExternalLibrary {      // -> '.dll' only!!
public:
    ExternalLibrary();
    float Add(float a, float b);
    float Sqrt(float x);
};
