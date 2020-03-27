#version 150
in vec3 position;
in vec2 texCoord;

out vec2 TexCoordI;

void main()
{
	TexCoordI = texCoord;
	gl_Position = vec4(position,1.0);
}
