#version 330 core

in vec4 v2f_positionW; // Pos world space
in vec4 v2f_normalW; // Surf norm world space
in vec2 v2f_texcoord;

uniform vec4 EyePosW;   // Camera position in world space
uniform vec4 LightPosW; // Light position in world space
uniform vec4 LightColor;

uniform vec4 MaterialEmissive;//material property
uniform vec4 MaterialDiffuse;//material property
uniform vec4 MaterialSpecular;//material property
uniform float MaterialShininess;//material property
uniform vec4 Ambient; // Global ambient contribution.
uniform sampler2D diffuseSampler;//fetching texel
layout (location=0) out vec4 out_color;//output

void main()
{
    vec4 Emissive = MaterialEmissive;//computation of emessive

    // diffuse lighting
    vec4 N = normalize( v2f_normalW );
    vec4 L = normalize( LightPosW - v2f_positionW );
    float NdotL = max( dot( N, L ), 0 );//returns the highest of both norm and light
    vec4 Diffuse =  NdotL * LightColor * MaterialDiffuse;
    
    //  specular lighting
    vec4 V = normalize( EyePosW - v2f_positionW );//calculate the view direction vector
    vec4 H = normalize( L + V );
    vec4 R = reflect( -L, N );//the corresponding reflect vector along the normal axis
    float RdotV = max( dot( R, V ), 0 );//model of phong lighting
    float NdotH = max( dot( N, H ), 0 );//model of phong lighting
    vec4 Specular = pow( RdotV, MaterialShininess ) * LightColor * MaterialSpecular;//calculating specular components
    
    out_color = ( Emissive + Ambient + Diffuse + Specular ) * texture( diffuseSampler, v2f_texcoord );//add it to the ambient,diffuse components and multiply the  result
}
