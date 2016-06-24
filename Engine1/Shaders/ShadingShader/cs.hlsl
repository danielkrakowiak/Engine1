#pragma pack_matrix(column_major) //informs only about the memory layout of input matrices

cbuffer ConstantBuffer : register( b0 )
{
    float3 cameraPos;
    float  pad1;
};

// Input.
Texture2D<float4> g_positionTexture : register( t0 );
Texture2D<float4> g_emissiveTexture : register( t1 );
Texture2D<float4> g_albedoTexture   : register( t2 );
Texture2D<float4> g_normalTexture   : register( t3 ); 

// Output.
RWTexture2D<float4> g_colorTexture : register( u0 );

static const float3 lightPos = float3(0.0f, 5.0f, 0.0f);

float3 getDiffuseColor( float3 albedo, float3 dirToLight, float3 surfaceNormal );
float3 getSpecularColor( float3 albedo, float3 dirToLight, float3 dirToCamera, float3 surfaceNormal );

// SV_GroupID - group id in the whole computation.
// SV_GroupThreadID - thread id within its group.
// SV_DispatchThreadID - thread id in the whole computation.
// SV_GroupIndex - index of the group within the whole computation.
[numthreads(16, 16, 1)]
void main( uint3 groupId : SV_GroupID,
           uint3 groupThreadId : SV_GroupThreadID,
           uint3 dispatchThreadId : SV_DispatchThreadID,
           uint  groupIndex : SV_GroupIndex )
{
    const float3 position = g_positionTexture[ dispatchThreadId.xy ].xyz;
    const float3 emissive = g_emissiveTexture[ dispatchThreadId.xy ].xyz;
    const float3 albedo   = g_albedoTexture[ dispatchThreadId.xy ].xyz;

    const float3 normal = g_normalTexture[ dispatchThreadId.xy ].xyz;
    //normal.z = sqrt( 1.0f - normal.x*normal.x - normal.y*normal.y );

    const float3 dirToLight  = normalize( lightPos - position );
    const float3 dirToCamera = normalize( cameraPos - position );

    const float3 diffuseColor  = getDiffuseColor( albedo, dirToLight, normal );
    const float3 specularColor = getSpecularColor( albedo, dirToLight, dirToCamera, normal );

    g_colorTexture[ dispatchThreadId.xy ] = float4( emissive + diffuseColor + specularColor, 1.0f );
}

float3 getDiffuseColor( float3 albedo, float3 dirToLight, float3 surfaceNormal )
{
    return max( 0.0f, dot( dirToLight, surfaceNormal ) ) * albedo;
}

float3 getSpecularColor( float3 albedo, float3 dirToLight, float3 dirToCamera, float3 surfaceNormal )
{
    return max( 0.0f, dot( normalize( dirToLight + dirToCamera ), surfaceNormal ) ) * albedo;
}
