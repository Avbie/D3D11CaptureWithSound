/********** Input Struct***********************************/
struct VS_Input
{
    float2 pos : POS; // "POS" ist Semantic/Signatur von IASetInputLayout
    float2 uv : TEX; // "TEX" ist Semantic/Signatur von IASetInputLayout
    float4 col : COL;
};
/********** Output Struct***********************************/
// Output of VertexShader is the Input of PixelShader
struct VS_Output
{
    float4 pos : SV_POSITION; // SV steht für Systemvarible, vorgegeben
    float2 uv : TEXCOORD;
    float4 col : COL;
};
// b = constant buffer
// t = Textur buffer
// s = sampler
// 0 = erste Variable, hier erste Matrix
cbuffer CBuf : register(b0)
{
    matrix transform;
}

//Texture2D mytexture : register(t0);
//SamplerState mysampler : register(s0);
/********** Main-Funktion VertexShader*********************/
VS_Output main(VS_Input input)
{
    VS_Output output; // Output is Input for the Next Stage
    output.pos = mul(float4(input.pos, 0.0f, 1.0f),transform); // input.pos has x,y coord. 
    //output.pos = float4(input.pos, 0.0f, 1.0f);
                                                // 3. 0.0f = z coord, 4. for Transformation
    output.uv = input.uv; // u = horizental, v = vertical(senkrecht)
    output.col = input.col;
    return output; // send it to the PixelShader
} //END-FUNC9