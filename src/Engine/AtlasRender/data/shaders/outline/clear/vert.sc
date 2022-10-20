$input a_position

#include "common.sh"

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position.xyz, 1.0));
}