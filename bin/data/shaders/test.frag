//Fragment Shader

uniform sampler2D tex; //forground image
uniform sampler2D tex1;	 //kinect gray image
uniform sampler2D tex2;	//background image

void main()
{
	vec4 temp = texture2D(tex, gl_TexCoord[0].xy);
	temp.a = texture2D(tex1, gl_TexCoord[1].xy).r; //if kinect image is RGB, if gray… don't know yet.
	vec4 t2 = texture2D(tex2, gl_TexCoord[2].xy);
	gl_FragColor = vec4(temp.a) * temp + t2;
}
