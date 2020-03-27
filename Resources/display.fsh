#version 150
uniform sampler2D inTexture;
in vec2 TexCoordI;

out vec4 fragColor0;

void main()
{
   float c = texture2D(inTexture, TexCoordI).r;
   fragColor0 = vec4(c,c,c,1.0);
}
