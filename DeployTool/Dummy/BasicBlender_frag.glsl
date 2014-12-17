#version 120

uniform sampler2D ssoaTex;
uniform sampler2D baseTex;

void main()
{
	vec4 ssaoTex	= texture2D( ssoaTex, gl_TexCoord[0].st );
	vec4 baseTex	= texture2D( baseTex, gl_TexCoord[0].st );
	float redVal	= 1.0 - ssaoTex.r;
	
	vec4 resultTex	= vec4( baseTex.r - redVal, baseTex.g - redVal, baseTex.b - redVal, baseTex.a - redVal );
	
	gl_FragColor = resultTex;
}