// Cube shader in Slang
// This shader renders a colored cube with support for wireframe mode

struct PerFrameData
{
    float4x4 mvp;
};

[[vk::push_constant]]
ConstantBuffer<PerFrameData> pushConstants;

// Specialization constant for wireframe mode
[[vk::constant_id(0)]]
const bool isWireframe = false;

// Vertex shader stage input/output
struct VertexStageOutput
{
    float3 color : COLOR;
};

// Cube vertex positions
static const float3 pos[8] = {
    float3(-1.0, -1.0,  1.0), float3( 1.0, -1.0,  1.0), 
    float3( 1.0,  1.0,  1.0), float3(-1.0,  1.0,  1.0),
    float3(-1.0, -1.0, -1.0), float3( 1.0, -1.0, -1.0), 
    float3( 1.0,  1.0, -1.0), float3(-1.0,  1.0, -1.0)
};

// Cube vertex colors
static const float3 col[8] = {
    float3(1.0, 0.0, 0.0), float3(0.0, 1.0, 0.0), 
    float3(0.0, 0.0, 1.0), float3(1.0, 1.0, 0.0),
    float3(1.0, 1.0, 0.0), float3(0.0, 0.0, 1.0), 
    float3(0.0, 1.0, 0.0), float3(1.0, 0.0, 0.0)
};

// Cube triangle indices
static const uint indices[36] = {
    0, 1, 2, 2, 3, 0, // front
    1, 5, 6, 6, 2, 1, // right
    7, 6, 5, 5, 4, 7, // back
    4, 0, 3, 3, 7, 4, // left
    4, 5, 1, 1, 0, 4, // bottom
    3, 2, 6, 6, 7, 3  // top
};

// Vertex Shader
[shader("vertex")]
VertexStageOutput cubeVertex(
    uint vertexID : SV_VertexID,
    out float4 position : SV_Position)
{
    VertexStageOutput output;

    uint idx = indices[vertexID];
    position = mul(float4(pos[idx], 1.0), pushConstants.mvp);
    output.color = isWireframe ? float3(0.0, 0.0, 0.0) : col[idx];

    return output;
}

// Fragment Shader
[shader("fragment")]
float4 cubeFragment(VertexStageOutput input) : SV_Target
{
    return float4(input.color, 1.0);
}
