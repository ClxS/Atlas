$input v_texcoord0

#include "../../common/common.sh"
#include "../../common/colour_utility.h"

SAMPLER2D(s_texColor,  0);

void main()
{
	const float strength = 15.0;
	const float extent = 0.15;

#if BGFX_SHADER_LANGUAGE_HLSL && (BGFX_SHADER_LANGUAGE_HLSL < 400)
	vec2 uv = gl_FragCoord.xy * u_viewTexel.xy + u_viewTexel.xy * vec2_splat(0.5);
#else
	vec2 uv = gl_FragCoord.xy * u_viewTexel.xy;
#endif

	vec4 diffuse = texture2D(
		s_texColor, 
		v_texcoord0);

    uv *=  1.0 - uv.yx;  
    float vig = uv.x * uv.y * strength; 
    vig = pow(vig, extent);
    
    vec3 hsv = rgb2hsv(diffuse.rgb);
    vec3 rgbOut = hsv2rgb(vec3(hsv.x, lerp(vec2(1.0, 0.0), hsv.yz, vig)));

    gl_FragColor = vec4(rgbOut, diffuse.a);
}