//Fragment Shader

uniform sampler2DRect tex; //forground image
uniform sampler2DRect tex1;	 //kinect gray image
uniform sampler2DRect tex2;	//background image


void main()
{
	vec4 temp = texture2D(tex, gl_TexCoord[0].xy);
	temp.a = texture2D(tex1, gl_TexCoord[1].xy).r; //if kinect image is RGB, if gray… don't know yet.
	vec4 t2 = texture2D(tex2, gl_TexCoord[2].xy);
	gl_FragColor = vec4(temp.a) * temp + t2;
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