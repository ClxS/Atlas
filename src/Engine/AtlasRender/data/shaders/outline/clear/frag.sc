$input v_texcoord0

#include "common.sh"

#define _KernelSize 16

SAMPLER2D(s_texColor,  0);

void main()
{
	gl_FragColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
}