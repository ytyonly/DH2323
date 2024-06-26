using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class FluidRayMarching : MonoBehaviour
{

    public struct Sphere
    {
        public float pressure;
        public float density;
        public Vector3 force;
        public Vector3 velocity;
        public Vector3 position;
    }

    public ComputeShader raymarching;
    public Camera cam;

    List<ComputeBuffer> buffersToDispose = new List<ComputeBuffer>();

    public SPH sph;

    RenderTexture target;

    [Header("Params")]
    public float viewRadius;
    public float blendStrength;
    public Color waterColor;

    public Color ambientLight;

    public Light lightSource;


    void InitRenderTexture()
    {
        if (target == null || target.width != cam.pixelWidth || target.height != cam.pixelHeight)
        {
            if (target != null)
            {
                target.Release();
            }

            cam.depthTextureMode = DepthTextureMode.Depth;

            target = new RenderTexture(cam.pixelWidth, cam.pixelHeight, 0, RenderTextureFormat.ARGBFloat, RenderTextureReadWrite.Linear);
            target.enableRandomWrite = true;
            target.Create();
        }
    }


    private bool render = false;

    public ComputeBuffer _spheresBuffer;

    private void SpawnSpheres()
    {
        _spheresBuffer = new ComputeBuffer(1, 44);
        _spheresBuffer.SetData(new Sphere[] {
        new Sphere {
        position = new Vector3(0,0,0)
       }});

    }

    // This method prepares the raymarching shader for rendering.
    public void Begin()
    {
        // SpawnSpheres();
        InitRenderTexture();
        //Here different values are set 
        raymarching.SetBuffer(0, "spheres", sph._spheresBuffer);
        raymarching.SetInt("numParticles", sph.spheres.Length);
        raymarching.SetFloat("sphereRadius", viewRadius);
        raymarching.SetFloat("blendStrength", blendStrength);
        raymarching.SetVector("waterColor", waterColor);
        raymarching.SetVector("_AmbientLight", ambientLight);
        raymarching.SetTextureFromGlobal(0, "_DepthTexture", "_CameraDepthTexture");
        render = true;
    }


    // This method is called by Unity to process and render images.
    void OnRenderImage(RenderTexture source, RenderTexture destination)
    {

        // InitRenderTexture();

        //If rendering hasn't start then begin doing it
        if (!render)
        {
            Begin();
        }

        if (render)
        {

            raymarching.SetVector("_Light", lightSource.transform.forward);

            raymarching.SetTexture(0, "Source", source);
            raymarching.SetTexture(0, "Destination", target);
            raymarching.SetVector("_CameraPos", cam.transform.position);
            raymarching.SetMatrix("_CameraToWorld", cam.cameraToWorldMatrix);
            raymarching.SetMatrix("_CameraInverseProjection", cam.projectionMatrix.inverse);

            int threadGroupsX = Mathf.CeilToInt(cam.pixelWidth / 8.0f);
            int threadGroupsY = Mathf.CeilToInt(cam.pixelHeight / 8.0f);
            raymarching.Dispatch(0, threadGroupsX, threadGroupsY, 1);

            Graphics.Blit(target, destination);
        }
    }

}
