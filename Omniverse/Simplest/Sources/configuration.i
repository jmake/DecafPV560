%module SpicyTech

// 1) Exchanges
%include "std_vector.i"
%include "std_string.i"
%template(VectorInt) std::vector<int>;
%template(VectorFloat) std::vector<float>;
%template(VectorDouble) std::vector<double>;
%template(VectorString) std::vector<std::string>;


// 2) Numpy Exchanges 
%{
    #define SWIG_FILE_WITH_INIT
    #include "SpicyTechDicom.hpp"
%}

%include "numpy.i"
%init 
%{
    import_array(); // <- SWIG_FILE_WITH_INIT
%}


/*
// 3) others... 
%include "std_string.i"
%include "typemaps.i"
%include "cpointer.i"
%include "stdint.i"  // for uintptr_t
*/

// SEE : void NumpyPrint(double* data, int size, ...); 
%apply (double* INPLACE_ARRAY1, int DIM1) { (double* data, int size) };

// Final...
%include "SpicyTechDicom.hpp"