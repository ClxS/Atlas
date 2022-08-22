$input v_normal, v_texcoord0, v_position, v_shadowcoord, v_view

#include "common.sh"

#define SHADOW_MAP_COUNT 1
#define SHADOW_MAP_SLOT_BASE 0
#include "shadow_mapping.h"

uniform vec4 u_lightDirection0;
uniform vec4 u_lightColour0;
uniform vec4 u_ambientColour;

vec2 lit(vec3 _ld, vec3 _n, vec3 _vd, float _exp)
{
	//diff
	float ndotl = dot(_n, _ld);

	//spec
	vec3 r = 2.0*ndotl*_n - _ld; //reflect(_ld, _n);
	float rdotv = dot(r, _vd);
	float spec = step(0.0, ndotl) * pow(max(0.0, rdotv), _exp) * (2.0 + _exp)/8.0;

	return max(vec2(ndotl, spec), 0.0);
}

void main()
{
	vec3 diffuseColor = vec3(0.8, 0.8, 0.8);

	float shadowMapBias = 0.005;
	vec2 texelSize = vec2_splat(1.0/2048.0);
	float visibility = PCF(s_shadowMap, v_shadowcoord, shadowMapBias, texelSize);

	vec3 v  = v_view;
	vec3 vd = -normalize(v);
	vec3 n  = v_normal;
	vec3 ld = normalize(u_lightDirection0 * vec3(-1.0, 1.0, -1.0));
	vec2 lc = lit(ld, n, vd, 1.0);

	vec3 ambient = u_ambientColour * diffuseColor;
	vec3 brdf = (lc.x + lc.y)  * u_lightColour0 * diffuseColor * visibility;
	vec3 final = toGamma(abs(ambient + brdf) );
	gl_FragColor = vec4(abs(ambient + brdf) , 1.0);

	gl_FragColor = vec4(diffuseColor, 1.0f);
}