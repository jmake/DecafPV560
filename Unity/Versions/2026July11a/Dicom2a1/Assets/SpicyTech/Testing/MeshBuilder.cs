using UnityEngine;
using Unity.Collections;
using Unity.Collections.LowLevel.Unsafe;
using Unity.Mathematics;
using UnityEngine.Rendering;
using System;


/*
error CS0227: Unsafe code may only appear if compiling with /unsafe. 
Enable "Allow 'unsafe' code" in Player Settings to fix this error.

Project Settings -> Player -> Other Settings -> Script Compilation -> Allow unsafe Code  
*/

namespace SpicyTech {

public static 
class MeshBuilder
{
    public static 
    Mesh CreateMeshFromRaw(
        ref Mesh reusableMesh,
        float[] rawVerts, int vertexCount,
        int[] rawIndices, int indexCount)
    {
        if (reusableMesh == null)
            reusableMesh = new Mesh();
        else
            reusableMesh.Clear(); // clear previous data before updating

        var meshDataArray = Mesh.AllocateWritableMeshData(1);
        var meshData = meshDataArray[0];

        // Setup vertex buffer layout (positions only)
        var vertexAttributes = new NativeArray<VertexAttributeDescriptor>(1, Allocator.Temp, NativeArrayOptions.UninitializedMemory);
        vertexAttributes[0] = new VertexAttributeDescriptor(
            VertexAttribute.Position,
            VertexAttributeFormat.Float32,
            dimension: 3, stream: 0);
        meshData.SetVertexBufferParams(vertexCount, vertexAttributes);
        vertexAttributes.Dispose();

        // Copy rawVerts (float[]) into mesh vertex buffer (float3) via MemCpy
        var vertexBuffer = meshData.GetVertexData<float3>();
        unsafe
        {
            fixed (float* srcPtr = rawVerts)
            {
                void* dstPtr = NativeArrayUnsafeUtility.GetUnsafeBufferPointerWithoutChecks(vertexBuffer);
                UnsafeUtility.MemCpy(dstPtr, srcPtr, vertexCount * 3L * sizeof(float));
            }
        }

        // Setup index buffer
        meshData.SetIndexBufferParams(indexCount, IndexFormat.UInt32);
        var indexBuffer = meshData.GetIndexData<uint>();

        unsafe
        {
            fixed (int* srcPtr = rawIndices)
            {
                void* dstPtr = NativeArrayUnsafeUtility.GetUnsafeBufferPointerWithoutChecks(indexBuffer);
                UnsafeUtility.MemCpy(dstPtr, srcPtr, indexCount * sizeof(int));
            }
        }

        // Setup submesh descriptor
        meshData.subMeshCount = 1;
        meshData.SetSubMesh(0, new SubMeshDescriptor(0, indexCount, MeshTopology.Triangles));

        // Apply mesh data and dispose temporary data
        Mesh.ApplyAndDisposeWritableMeshData(meshDataArray, reusableMesh);

        // Optional: recalculate normals
        reusableMesh.RecalculateNormals();

        return reusableMesh;
    }


} // 


} // SpicyTech 