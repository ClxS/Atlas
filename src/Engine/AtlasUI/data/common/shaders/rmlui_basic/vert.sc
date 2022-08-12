$input a_position, a_color0, a_texcoord0
$output v_texcoord0, v_color0

uniform mat3 u_transform;

#include "../../common/common.sh"

void main()
{
	vec2 position = mul(u_transform, vec3(a_position, 1.0f)).xy;
	gl_Position = vec4(position, 1.0, 1.0);

	//gl_Position = vec4((a_position + u_transform.xy) / u_frameBufferSize.xy, 1.0, 1.0);
	//gl_Position = mul(u_modelViewProj, position);

	v_color0 = a_color0 / vec4(255, 255, 255, 255);
	v_texcoord0 = a_texcoord0;
}