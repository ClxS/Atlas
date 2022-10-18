#include "common.sh"
uniform vec4 u_entityId;

void main()
{
	gl_FragColor.xyz = u_entityId.xyz;
	gl_FragColor.w = 1.0;
}