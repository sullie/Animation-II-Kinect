//Vertex Shader

void main()
{
	gl_FrontColor = gl_Color;
	gl_TexCoord[0] = gl_MultiTexCoord0; //forground image
	gl_TexCoord[1] = gl_MultiTexCoord1; //kinect gray image
	gl_TexCoord[2] = gl_MultiTexCoord2; // background image
	gl_Position = ftransform();
}

/*
void main(void)  
{  
    //this is a default vertex shader all it does is this...  
    gl_Position = ftransform();  
    //.. and passes the multi texture coordinates along to the fragment shader  
    gl_TexCoord[0] = gl_MultiTexCoord0;  
    gl_TexCoord[1] = gl_MultiTexCoord1;  
}  
*/