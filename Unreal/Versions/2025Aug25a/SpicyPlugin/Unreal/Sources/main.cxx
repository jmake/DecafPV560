#include <iostream>

#include "SpicyTechDicom2.hpp"


int main(int argc, char* argv[])
{

    // Test1 
    auto* dicom = new SpicyTech2::VtkTest();
    std::cout << "[SpicyTech]  GetVersion: "<< dicom->GetVersion() << std::endl;
    delete dicom;

/*
    // Nifti 
    std::string fname = "F:\\z2025_1\\Dicom\\NII\\1.2.826.0.1.3680043.10633.nii"; 
    auto* nifti = new SpicyTech3::Nifti(); 
    nifti->LoadFile(fname); 

    std::vector<float> spacing = nifti->GetSpacing(); 
    std::vector<int> dimensions = nifti->GetDimensions(); 

    int size = nifti->GetBufferSize();

    float* buffer = nifti->GetBuffer();

    int nx = dimensions[0]; 
    int ny = dimensions[1]; 
    int nz = dimensions[2]; 
    int nt = dimensions[3]; 

    int i = 1; 
    int j = 1; 
    int k = 1; 
    int l = 0; 
    int idx = 0; 
    float value = 0; 

    idx = 0; 
    value = buffer[idx]; 
    std::cout <<"[SpicyTech] value("<< idx << "): "<< value << " \n";

    idx = size-1; 
    value = buffer[idx]; 
    std::cout <<"[SpicyTech] value("<< idx << "): "<< value << " \n";

    idx = i + nx * (j + ny * (k + nz * l));
    value = buffer[idx]; 
    std::cout <<"[SpicyTech] value("<< idx << "): "<< value << " \n";

    //nifti->SaveVti("domain"); // F:\z2025_1\ParaView600\bin\paraview.exe Build\domain.vti
*/
/*
    // Contour 
    auto* contour = new SpicyTech2::Contour3(); 

    double maxCells = 0; 
    std::vector<float> range; 
    contour->SetVti(nifti->GetVti(), nifti->GetKeyName(), nifti->GetRange(), maxCells); 

    contour->Update(0.4, false); 

    contour->Update(0.5, false); 

    int n_rows, n_cols;
    float* vertices = contour->GetVertices(n_rows, n_cols); 
    std::cout <<"[SpicyTech] n_rows : "<< n_rows << " n_cols : "<< n_cols << " \n";

    int n_triangles; // -> 764550
    long long n_raw; // -> 3058200
    long long* triangles = contour->GetTriangles(n_triangles, n_raw);
    std::cout <<"[SpicyTech] n_triangles : "<< n_triangles <<" n_raw : "<< n_raw << "\n";

    idx = 539; // -> 3 1878 275 1890
    std::cout <<"[SpicyTech] cell("<< idx <<") : "<< triangles[idx*4+0] <<" "<< triangles[idx*4+1] <<" "<< triangles[idx*4+2] <<" "<< triangles[idx*4+3] <<" \n";


    contour->SaveVtp("contour0.5");
    contour->Finish(); 
    delete contour; 

    // nifti 
    nifti->Finish();     
    delete nifti;
*/
    std::cout << "[SpicyTech] Done!!" << std::endl;
    return 0;    
}