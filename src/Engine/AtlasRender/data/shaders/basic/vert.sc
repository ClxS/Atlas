$input a_position, a_normal, a_texcoord0, i_data0, i_data1, i_data2, i_data3, i_data4
$output v_normal, v_texcoord0, v_position, v_shadowcoord, v_view

#include "common.sh"

uniform mat4 u_lightMtx;

void main()
{
	mat4 model = mtxFromCols(i_data0, i_data1, i_data2, i_data3);

	vec4 worldPos = mul(model, vec4(a_position, 1.0) );
	gl_Position = mul(u_viewProj, worldPos);
	
	vec3 normal = a_normal * 2.0 - 1.0;
	vec3 wnormal = mul(model, vec4(normal.xyz, 0.0) ).xyz;
	v_normal = normalize(wnormal);
	v_view = mul(mul(model, u_view), vec4(a_position, 1.0)).xyz;

	v_texcoord0 = a_texcoord0;
	v_position = gl_Position;

	const float shadowMapOffset = 0.001;
	vec3 posOffset = a_position + normal.xyz * shadowMapOffset;
	v_shadowcoord = (mul(mul(u_lightMtx, model), vec4(posOffset, 1.0)) + vec4(1.0, 1.0, 0.0, 0.0)) 
		* vec4(0.5, 0.5, 1.0, 1.0);
	v_shadowcoord.y = 1.0 - v_shadowcoord.y;
}