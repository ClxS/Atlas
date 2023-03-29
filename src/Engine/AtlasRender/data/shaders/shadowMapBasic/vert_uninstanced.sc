$input a_position, a_normal, a_texcoord0

#include "common.sh"

void main()
{
	vec4 worldPos = mul(u_model[0], vec4(a_position, 1.0) );
	gl_Position = mul(u_viewProj, worldPos);
}
