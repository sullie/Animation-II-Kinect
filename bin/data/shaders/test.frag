//Fragment Shader

uniform sampler2DRect tex; //forground image
uniform sampler2DRect tex1;	 //kinect gray image
uniform sampler2DRect tex2;	//background image


void main()
{
<<<<<<< .merge_file_a07624
	
	vec4 temp = texture2DRect(tex, gl_TexCoord[0].xy);
	temp.a = texture2DRect(tex1, gl_TexCoord[1].xy).r;    //if kinect image is RGB, if gray… don't know yet.
	vec4 t2 = texture2DRect(tex2, gl_TexCoord[2].xy);
	//gl_FragColor =vec4(temp.a) *temp +t2;  
=======
	vec4 temp = texture2DRect(tex, gl_TexCoord[0].xy);
	temp.a = texture2DRect(tex1, gl_TexCoord[1].xy).r; //if kinect image is RGB, if gray… don't know yet.
	vec4 t2 = texture2DRect(tex2, gl_TexCoord[2].xy);
>>>>>>> .merge_file_a05584
	gl_FragColor = mix(t2, temp, temp.a);
}

