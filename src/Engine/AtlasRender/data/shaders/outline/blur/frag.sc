$input v_texcoord0

#include "common.sh"

#define _KernelSize 8

SAMPLER2D(s_texColor,  0);

void main()
{
	vec2 _MainTex_TexelSize = vec2(1.0f / 1200.0f, 1.0f / 800.0f);
	int samples = 2 * _KernelSize + 1;

	// Vertical box blur.
	half4 sumV = 0;
	for (float y = 0; y < samples; y++)
	{
		float2 offset = float2(0, y - _KernelSize);
		sumV += texture2D(s_texColor, v_texcoord0 + offset * _MainTex_TexelSize.xy);
	}

	vec4 vertical = sumV / samples;
					
	// Horizontal box blur.
	half4 sumH = 0;
	for (float x = 0; x < samples; x++)
	{
		float2 offset = float2(x - _KernelSize, 0);
		sumH += texture2D(s_texColor, v_texcoord0 + offset * _MainTex_TexelSize.xy);
	}

	vec4 horizontal = sumH / samples;

	gl_FragColor = vertical + horizontal;
	if (gl_FragColor.r < 0.8)
	{
		gl_FragColor.a = 0.0;
	}
	else
	{
		gl_FragColor.a = 1.0;
	}
}