// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../Sources/vtktools4.hpp"
#include "../PixelTextureDisplay.h"

#include "CoreMinimal.h"


//----------------------------------------------------------------------------------|  |--//
//----------------------------------------------------------------------------------|  |--//
//template<typename RETURN_TYPE> void SetAsyncFuncs(TFunction<RETURN_TYPE()> Task, TFunction<void(RETURN_TYPE)> After); 


//----------------------------------------------------------------------------------|  |--//
//----------------------------------------------------------------------------------|  |--//
void LoadVtiFile(
	const std::string& fname, 
	vtkImageData*& vti_ptr, 
	std::vector<double>& range
);  


//----------------------------------------------------------------------------------|  |--//
//----------------------------------------------------------------------------------|  |--//
struct SurfaceProperties
{
    int n_rows = 0;
    int n_cols = 0;
    float* points = nullptr;
    int n_triangles = 0;
    long long n_raw = 0;
    long long* triangles = nullptr;

    void DeleteAll()
    {
        if (points)
        {
            delete[] points;
            points = nullptr;
        }
        if (triangles)
        {
            delete[] triangles;
            triangles = nullptr;
        }

        n_rows = 0;
        n_cols = 0;
        n_triangles = 0;
        n_raw = 0;
    }

    // Destructor to automatically clean up -> ~SurfaceProperties() { DeleteAll(); }
};



void Vti2SurfaceBuffer( 
						vtkImageData* domain, 
						float value, 
						float valueMin, 
						float valueMax, 
						vtkPolyData*& vtp, 
						float*& points, 
						int& n_rows, 
						int& n_cols, 
						long long*& triangles, 
						int& n_triangles
					); 


//----------------------------------------------------------------------------------|  |--//
//----------------------------------------------------------------------------------|  |--//
struct AnatomicalProperties
{
    int32 width = 0;
    int32 height = 0;
    float spacingX = 0.f;
    float spacingY = 0.f;

    float* data = nullptr;
	vtkImageData* vti_ptr = nullptr;

    void Finish()
    {
        if (vti_ptr)
        {
            vti_ptr->Delete();
            vti_ptr = nullptr;
        }
        if (data)
        {
            delete[] data;
            data = nullptr;
        }

        width = 0;
        height = 0;
        spacingX = 0.f;
        spacingY = 0.f;
    }

	// Destructor to automatically clean up -> ~AnatomicalProperties() { DeleteAll(); }
};


float* Vti2AnatomicalBuffer(
							vtkImageData* domain, 
							double anatomicalView[9], 
							float x0, float y0, float z0, 
							vtkImageData*& cutter, 
							int& width, int& height, 
							float& sizeX, float& sizeY 
						); 

void Vti2AnatomicalBufferAsync(
                                vtkImageData* domain, 
                                double anatomicalView[9], 
                                float x0, 
                                float y0, 
                                float z0,
								std::function<void(AnatomicalProperties)> PostProcess
); 


UTexture2D* AnatomicalBuffer2UTexture2D(AnatomicalProperties result, UWorld* world, APixelTextureDisplay*& actor); 

static double AXIAL[9] = {1, 0,  0, 0, 1,  0, 0, 0, 1}; 
static double CORONAL[9] = {1, 0,  0, 0, 0, -1, 0, 1, 0};
static double SAGITTAL[9] = {0, 0, -1, 1, 0,  0, 0, 1, 0}; 


//----------------------------------------------------------------------------------|  |--//
//----------------------------------------------------------------------------------|  |--//
class MYPROJECT5_API VtkInterface
{
public:
	VtkInterface();
	~VtkInterface();
};
//----------------------------------------------------------------------------------|  |--//
//----------------------------------------------------------------------------------|  |--//