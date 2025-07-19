Shader "Custom/FaceColorByNormal"
{
    Properties {}
    SubShader
    {
        Tags { "RenderType"="Opaque" }
        LOD 100

        Pass
        {
            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag
            #include "UnityCG.cginc"

            struct appdata {
                float4 vertex : POSITION;
                float3 normal : NORMAL;
            };

            struct v2f {
                float4 pos : SV_POSITION;
                float3 normal : TEXCOORD0;
            };

            v2f vert(appdata v) {
                v2f o;
                o.pos = UnityObjectToClipPos(v.vertex);
                o.normal = v.normal;
                return o;
            }

            fixed4 frag(v2f i) : SV_Target {
                float3 n = normalize(i.normal);
                float3 color = abs(n); // Use direction to color: x = red, y = green, z = blue
                return fixed4(color, 1.0);
            }
            ENDCG
        }
    }
}
