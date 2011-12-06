//Vertex Shader

void main()
{
	gl_FrontColor = gl_Color;
	gl_TexCoord[0] = gl_MultiTexCoord0; //forground image
	gl_TexCoord[1] = gl_MultiTexCoord1; //kinect gray image
	gl_TexCoord[2] = gl_MultiTexCoord2; // background image
	gl_Position = ftransform();
}

