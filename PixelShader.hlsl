// sets the color of each pixel on the raster
// positioning on screen (not world) is not needed anymore (done stage befor: rasterizer)
// SV_TARGET  SV = SystemValue: ist vorgegeben

// Output ist float4: rgb + alpha

Texture2D mytexture : register(t0);
SamplerState mysampler : register(s0);

struct VS_Output
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 col : COL;
};

float4 main(VS_Output input) : SV_Target
{
    //return input.col;
    // benutzt den Sampler den ich erstellt habe (CreateAndSetSampler)
    // Dieser erstellte Sampler wurde dort auf GPU kopiert
    // hier verwendet er nun die gesetzten Daten um den Input von der VertexStage zu interpretieren
    // d.h. Texturen auf Polygone (2Dreiecke = 1 Quad) mappen
    return mytexture.Sample(mysampler, input.uv);
}
