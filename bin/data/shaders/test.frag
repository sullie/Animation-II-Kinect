//Fragment Shader

uniform sampler2DRect tex; //forground image
uniform sampler2DRect tex1;	 //kinect gray image
uniform sampler2DRect tex2;	//background image


void main()
{
	vec4 temp = texture2DRect(tex, gl_TexCoord[0].xy);
	temp.a = texture2DRect(tex1, gl_TexCoord[1].xy).r; //if kinect image is RGB, if gray… don't know yet.
	vec4 t2 = texture2DRect(tex2, gl_TexCoord[2].xy);
	gl_FragColor = mix(t2, temp, temp.a);
}

/*
uniform sampler2DRect Tex0, Tex1; //these are our texture names, set in openFrameworks on the shader object in set up  
  
void main (void)  
{  
    //sample both textures  
    vec4 image = texture2DRect(Tex0, gl_TexCoord[0].st);  
    vec4 composite = texture2DRect(Tex1, gl_TexCoord[1].st);  
      
    //use the color from the image, but use the r channel of the mask as the alpha channel of our output  
    gl_FragData[0] = vec4(image.rgb,composite.r);        
}  
*/