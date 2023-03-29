$input a_position, a_normal, a_texcoord0
$output v_normal, v_texcoord0, v_position, v_shadowcoord, v_view

#include "common.sh"

uniform mat4 u_lightMtx;

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));

	vec3 normal = a_normal * 2.0 - 1.0;
	vec3 wnormal = mul(u_model[0], vec4(normal.xyz, 0.0) ).xyz;
	v_normal = normalize(wnormal);
	v_view = mul(mul(u_model[0], u_view), vec4(a_position, 1.0)).xyz;

	v_texcoord0 = a_texcoord0;
	v_position = gl_Position;

	const float shadowMapOffset = 0.001;
	vec3 posOffset = a_position + normal.xyz * shadowMapOffset;
	v_shadowcoord = (mul(mul(u_lightMtx, u_model[0]), vec4(posOffset, 1.0)) + vec4(1.0, 1.0, 0.0, 0.0))
		* vec4(0.5, 0.5, 1.0, 1.0);
	v_shadowcoord.y = 1.0 - v_shadowcoord.y;
}
