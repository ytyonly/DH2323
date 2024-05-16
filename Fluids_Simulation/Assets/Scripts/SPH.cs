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
        public Vector3 Force;
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

    [Header("Sphere Rendering")]
    public Mesh particleMesh;
    public float particleRenderSize = 8f;
    public Material material;

    [Header("Compute")]
    public ComputeShader shader;
    public Sphere[] particles;

    [Header("Fluid Constants")]
    public float boundDamping = -0.3f;
    public float viscosity = 0.003f;
    public float particleMass = 1f;
    public float gasConstant = 2f;
    public float restingDensity = 1f;
    public float timestep = 0.007f;

    private ComputeBuffer _argsBuffer;
    public ComputeBuffer _particlesBuffer;
    private int integrateKernel;
    private int computeForceKernel;
    private int densityPressureKernel;

    private static readonly int SizeProperty = Shader.PropertyToID("_size");
    private static readonly int ParticlesBufferProperty = Shader.PropertyToID("_particlesBuffer");

    private void Awake()
    {
        SpawnParticlesInBox();

        uint[] args =
        {
            particleMesh.GetIndexCount(0),
            (uint)totalSpheres,
            particleMesh.GetIndexStart(0),
            particleMesh.GetBaseVertex(0),
            0
        };
        _argsBuffer = new ComputeBuffer(1, args.Length * sizeof(uint), ComputeBufferType.IndirectArguments);
        _argsBuffer.SetData(args);

        _particlesBuffer = new ComputeBuffer(totalSpheres, 44);
        Debug.Log(particles.Count());

        _particlesBuffer.SetData(particles);

        SetupComputeBuffers();
    }

    // Update is called once per frame
    void Update()
    {
        material.SetFloat(SizeProperty, particleRenderSize);
        material.SetBuffer(ParticlesBufferProperty, _particlesBuffer);
        //Debug.Log(_particlesBuffer);

        if (visibleSpheres)
        {
            Graphics.DrawMeshInstancedIndirect(
                particleMesh,
                0,
                material,
                new Bounds(Vector3.zero, cubeSize),
                _argsBuffer,
                castShadows: UnityEngine.Rendering.ShadowCastingMode.Off
        );
        }

    }

    private void SetupComputeBuffers()
    {
        integrateKernel = shader.FindKernel("Integrate");
        computeForceKernel = shader.FindKernel("ComputeForces");
        densityPressureKernel = shader.FindKernel("ComputeDensityPressure");

        Debug.Log($"Integrate Kernel: {integrateKernel}");
        Debug.Log($"ComputeForces Kernel: {computeForceKernel}");
        Debug.Log($"ComputeDensityPressure Kernel: {densityPressureKernel}");

        shader.SetInt("particleLength", totalSpheres);
        shader.SetFloat("particleMass", particleMass);
        shader.SetFloat("viscosity", viscosity);
        shader.SetFloat("gasConstant", gasConstant);
        shader.SetFloat("restDensity", restingDensity);
        shader.SetFloat("boundDamping", boundDamping);
        shader.SetFloat("pi", Mathf.PI);
        shader.SetVector("cubeSize", cubeSize);

        shader.SetFloat("radius", sphereRadius);
        shader.SetFloat("radius2", sphereRadius * sphereRadius);
        shader.SetFloat("radius3", sphereRadius * sphereRadius * sphereRadius);
        shader.SetFloat("radius4", sphereRadius * sphereRadius * sphereRadius * sphereRadius);
        shader.SetFloat("radius5", sphereRadius * sphereRadius * sphereRadius * sphereRadius * sphereRadius);

        shader.SetBuffer(integrateKernel, "_particles", _particlesBuffer);
        shader.SetBuffer(computeForceKernel, "_particles", _particlesBuffer);
        shader.SetBuffer(densityPressureKernel, "_particles", _particlesBuffer);
    }

    void FixedUpdate()
    {
        shader.SetVector("cubeSize", cubeSize);
        shader.SetFloat("timestep", timestep);

        int threadGroups = Mathf.CeilToInt(totalSpheres / 100.0f);
        shader.Dispatch(densityPressureKernel, threadGroups, 1, 1);
        shader.Dispatch(computeForceKernel, threadGroups, 1, 1);
        shader.Dispatch(integrateKernel, threadGroups, 1, 1);
    }

    private void SpawnParticlesInBox()
    {
        List<Sphere> _particles = new List<Sphere>();

        for (int x = 0; x < sphereAmount.x; x++)
        {
            for (int y = 0; y < sphereAmount.y; y++)
            {
                for (int z = 0; z < sphereAmount.z; z++)
                {

                    Vector3 spawnPosition = spawnPoint + new Vector3(x * sphereRadius * 2, y * sphereRadius * 2, z * sphereRadius * 2);
                    spawnPosition += Random.onUnitSphere * sphereRadius * spawnRandomness;
                    Debug.Log(spawnPosition);

                    Sphere p = new Sphere
                    {
                        position = spawnPosition
                    };

                    _particles.Add(p);

                }
            }
        }
        Debug.Log(_particles.Count);
        particles = _particles.ToArray();
    }

    private void OnDrawGizmos()
    {
        Gizmos.color = Color.black;
        Gizmos.DrawWireCube(Vector3.zero, cubeSize);

        if (!Application.isPlaying)
        {
            Gizmos.color = Color.cyan;
            Gizmos.DrawWireCube(spawnPoint, new Vector3(0.1f, 0.1f, 0.1f));
        }
    }
}
