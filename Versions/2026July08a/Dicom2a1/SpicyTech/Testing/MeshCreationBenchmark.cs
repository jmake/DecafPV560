using UnityEngine;
using Unity.Collections;
using Unity.Collections.LowLevel.Unsafe;
using Unity.Mathematics;
using UnityEngine.Rendering;
using System.Diagnostics;

public class MeshCreationBenchmark : MonoBehaviour
{
    public int triangleCount = 10000;

    void Start()
    {
        int vertexCount = triangleCount * 3;
        float[] rawVerts = new float[vertexCount * 3];
        float[] rawUVs = new float[vertexCount]; // one float per vertex
        int[] rawIndices = new int[vertexCount];

        for (int i = 0; i < vertexCount; i++)
        {
            rawVerts[i * 3 + 0] = UnityEngine.Random.value;
            rawVerts[i * 3 + 1] = UnityEngine.Random.value;
            rawVerts[i * 3 + 2] = UnityEngine.Random.value;
            rawUVs[i] = UnityEngine.Random.value;
            rawIndices[i] = i;
        }

        Stopwatch sw = new Stopwatch();

        sw.Start();
        Mesh meshA = CreateWithVector3Array(rawVerts, rawUVs, vertexCount, rawIndices);
        sw.Stop();
        UnityEngine.Debug.Log("Vector3[] method: " + sw.ElapsedMilliseconds + " ms");

        sw.Reset();
        sw.Start();
        Mesh meshB = CreateWithMemCpy(rawVerts, rawUVs, vertexCount, rawIndices);
        sw.Stop();
        UnityEngine.Debug.Log("MemCpy method: " + sw.ElapsedMilliseconds + " ms");
    }

    Mesh CreateWithVector3Array(float[] rawVerts, float[] rawUVs, int vertexCount, int[] indices)
    {
        Vector3[] verts = new Vector3[vertexCount];
        Vector2[] uvs = new Vector2[vertexCount];

        for (int i = 0; i < vertexCount; i++)
        {
            verts[i] = new Vector3(
                rawVerts[i * 3 + 0],
                rawVerts[i * 3 + 1],
                rawVerts[i * 3 + 2]);

            uvs[i] = new Vector2(rawUVs[i], 0); // simulate custom float as uv.x
        }

        Mesh mesh = new Mesh();
        mesh.vertices = verts;
        mesh.uv = uvs;
        mesh.SetIndices(indices, MeshTopology.Triangles, 0);
        return mesh;
    }

    Mesh CreateWithMemCpy(float[] rawVerts, float[] rawUVs, int vertexCount, int[] indices)
    {
        Mesh mesh = new Mesh();
        var meshDataArray = Mesh.AllocateWritableMeshData(1);
        var meshData = meshDataArray[0];

        var attributes = new NativeArray<VertexAttributeDescriptor>(2, Allocator.Temp);
        attributes[0] = new VertexAttributeDescriptor(VertexAttribute.Position, VertexAttributeFormat.Float32, 3);
        attributes[1] = new VertexAttributeDescriptor(VertexAttribute.TexCoord0, VertexAttributeFormat.Float32, 2);
        meshData.SetVertexBufferParams(vertexCount, attributes);
        attributes.Dispose();

        var vBuffer = meshData.GetVertexData<float3>();
        var uvBuffer = meshData.GetVertexData<float2>(1);

        unsafe
        {
            fixed (float* srcVerts = rawVerts)
            {
                void* dst = NativeArrayUnsafeUtility.GetUnsafeBufferPointerWithoutChecks(vBuffer);
                UnsafeUtility.MemCpy(dst, srcVerts, vertexCount * 3L * sizeof(float));
            }

            fixed (float* srcUVs = rawUVs)
            {
                void* dst = NativeArrayUnsafeUtility.GetUnsafeBufferPointerWithoutChecks(uvBuffer);
                for (int i = 0; i < vertexCount; i++)
                    uvBuffer[i] = new float2(srcUVs[i], 0);
            }
        }

        meshData.SetIndexBufferParams(vertexCount, IndexFormat.UInt32);
        var iBuffer = meshData.GetIndexData<uint>();
        unsafe
        {
            fixed (int* src = indices)
            {
                void* dst = NativeArrayUnsafeUtility.GetUnsafeBufferPointerWithoutChecks(iBuffer);
                UnsafeUtility.MemCpy(dst, src, vertexCount * sizeof(int));
            }
        }

        meshData.subMeshCount = 1;
        meshData.SetSubMesh(0, new SubMeshDescriptor(0, vertexCount, MeshTopology.Triangles));
        Mesh.ApplyAndDisposeWritableMeshData(meshDataArray, mesh);
        return mesh;
    }
}