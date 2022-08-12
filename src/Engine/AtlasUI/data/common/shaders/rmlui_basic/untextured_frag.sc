$input v_texcoord0, v_color0

#include "../../common/common.sh"

SAMPLER2D(s_texColor,  0);

void main()
{
	gl_FragColor = v_color0;
}