using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using System.Linq;

public class SPH : MonoBehaviour
{
    [System.Serializable]
    [StructLayout(LayoutKind.Sequential, Size = 44)]
    public struct Sphere
    {
        public float pressure;
        public float density;
        public Vector3 force;
        public Vector3 velocity;
        public Vector3 position;
    }

    [Header("Spawn Settings")]
    public bool visibleSpheres = true;
    public Vector3Int sphereAmount = new Vector3Int(10, 10, 10);
    private int totalSpheres
    {
        get
        {
            return sphereAmount.x * sphereAmount.y * sphereAmount.z;
        }
    }
    public Vector3 cubeSize = new Vector3(4, 10, 3);
    public Vector3 spawnPoint;
    public float sphereRadius = 0.1f;

    public float spawnRandomness = 0.2f;

    [Header("Sphere Settings")]
    public Mesh sphereMesh;
    public float sphereRenderSize = 8f;
    public Material material;

    [Header("Shader")]
    public ComputeShader shader;
    public Sphere[] spheres;

    [Header("Physics Settings")]
    public float boundDamping = -0.3f;
    public float viscosity = 0.003f;
    public float sphereMass = 1f;
    public float gasConstant = 2f;
    public float restingDensity = 1f;
    public float timestep = 0.007f;

    private ComputeBuffer _argsBuffer;
    public ComputeBuffer _spheresBuffer;
    private int integrateKernel;
    private int computeForceKernel;
    private int densityPressureKernel;

    private static readonly int SizeProperty = Shader.PropertyToID("_size");
    private static readonly int ParticlesBufferProperty = Shader.PropertyToID("_spheresBuffer");

    // This method is called when the script instance is being loaded.
    private void Awake()
    {
        // Call the method to spawn spheres in the scene.
        SpawnSpheres();

        uint[] args = new uint[5];
        args[0] = sphereMesh.GetIndexCount(0);
        args[1] = (uint)totalSpheres;
        args[2] = sphereMesh.GetIndexStart(0);
        args[3] = sphereMesh.GetBaseVertex(0);
        args[4] = 0;

        // Create a compute buffer to hold the draw arguments.
        _argsBuffer = new ComputeBuffer(1, args.Length * sizeof(uint), ComputeBufferType.IndirectArguments);
        _argsBuffer.SetData(args);

        // Create a compute buffer to hold the sphere data.
        _spheresBuffer = new ComputeBuffer(totalSpheres, 44);
        Debug.Log(spheres.Count());

        _spheresBuffer.SetData(spheres);

        SetupComputeBuffers();
    }


    // Update is called once per frame
    void Update()
    {
        material.SetFloat(SizeProperty, sphereRenderSize);
        material.SetBuffer(ParticlesBufferProperty, _spheresBuffer);
        //Debug.Log(_spheresBuffer);

        if (visibleSpheres)
        {
            Graphics.DrawMeshInstancedIndirect(
                sphereMesh,
                0,
                material,
                new Bounds(Vector3.zero, cubeSize),
                _argsBuffer,
                castShadows: UnityEngine.Rendering.ShadowCastingMode.Off
        );
        }

    }

    // This method sets up compute buffers and kernels for the shader.
    private void SetupComputeBuffers()
    {
        integrateKernel = shader.FindKernel("Integrate");
        computeForceKernel = shader.FindKernel("SetForces");
        densityPressureKernel = shader.FindKernel("SetDensityAndPressure");

        Debug.Log($"Integrate Kernel: {integrateKernel}");
        Debug.Log($"SetForces Kernel: {computeForceKernel}");
        Debug.Log($"SetDensityAndPressure Kernel: {densityPressureKernel}");

        shader.SetInt("sphereLength", totalSpheres);
        shader.SetFloat("sphereMass", sphereMass);
        shader.SetFloat("viscosity", viscosity);
        shader.SetFloat("gasConstant", gasConstant);
        shader.SetFloat("restDensity", restingDensity);
        shader.SetFloat("boundDamping", boundDamping);
        shader.SetFloat("pi", Mathf.PI);
        shader.SetVector("cubeSize", cubeSize);
        shader.SetFloat("radius", sphereRadius);
        shader.SetFloat("radiusPower2", sphereRadius * sphereRadius);
        shader.SetFloat("radiusPower3", sphereRadius * sphereRadius * sphereRadius);
        shader.SetFloat("radiusPower4", sphereRadius * sphereRadius * sphereRadius * sphereRadius);
        shader.SetFloat("radiusPower5", sphereRadius * sphereRadius * sphereRadius * sphereRadius * sphereRadius);
        shader.SetBuffer(integrateKernel, "_spheres", _spheresBuffer);
        shader.SetBuffer(computeForceKernel, "_spheres", _spheresBuffer);
        shader.SetBuffer(densityPressureKernel, "_spheres", _spheresBuffer);
    }

    // This method is called every fixed frame-rate frame
    void FixedUpdate()
    {
        shader.SetVector("cubeSize", cubeSize);
        shader.SetFloat("timestep", timestep);

        int threadGroups = Mathf.CeilToInt(totalSpheres / 100.0f);
        Vector3Int dispatchSize = new Vector3Int(threadGroups, 1, 1);
        shader.Dispatch(densityPressureKernel, dispatchSize.x, dispatchSize.y, dispatchSize.z);
        shader.Dispatch(computeForceKernel, dispatchSize.x, dispatchSize.y, dispatchSize.z);
        shader.Dispatch(integrateKernel, dispatchSize.x, dispatchSize.y, dispatchSize.z);
    }


    // This method spawns spheres in a 3D grid pattern with some randomness.
    private void SpawnSpheres()
    {
        // Create a list to temporarily hold the sphere data.  
        List<Sphere> _spheres = new List<Sphere>();

        // Loop over the range of x, y, and z dimensions specified by sphereAmount.
        foreach (int x in Enumerable.Range(0, sphereAmount.x))
        {
            foreach (int y in Enumerable.Range(0, sphereAmount.y))
            {
                foreach (int z in Enumerable.Range(0, sphereAmount.z))
                {
                    Vector3 spawnPosition = spawnPoint + new Vector3(x * sphereRadius * 2, y * sphereRadius * 2, z * sphereRadius * 2);
                    // Add some random displacement to the position for randomness.
                    spawnPosition += Random.onUnitSphere * sphereRadius * spawnRandomness;
                    Debug.Log(spawnPosition);

                    // Create a new sphere with the calculated position.
                    Sphere p = new Sphere
                    {
                        position = spawnPosition
                    };

                    _spheres.Add(p);
                }
            }
        }

        Debug.Log(_spheres.Count);
        spheres = _spheres.ToArray();
    }


    //Draws the bounding box
    private void OnDrawGizmos()
    {
        //Set the color
        Gizmos.color = Color.black;

        // Draw a wireframe cube centered at the origin
        Gizmos.DrawWireCube(Vector3.zero, cubeSize);

        if (!Application.isPlaying)
        {
            Gizmos.color = Color.cyan;
            Gizmos.DrawWireCube(spawnPoint, new Vector3(0.1f, 0.1f, 0.1f));
        }
    }
}
