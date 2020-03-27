#version 150
uniform sampler2D heightmap;
uniform sampler2D dispX;
uniform sampler2D dispZ;
uniform sampler2D nX;
uniform sampler2D nZ;
uniform vec3 cameraPos;


uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat4 Model;

in vec3 position;
in vec2 texCoord;

out vec3 normal;
out vec3 viewVector;
out vec3 iPos;

void main()
{
	vec3 pt = position;
	pt.y+= texture2D(heightmap,texCoord).r;
	//pt.x-= texture2D(dispX,texCoord).r;
	//pt.z-= texture2D(dispZ,texCoord).r;
	normal.x = -texture2D(nX,texCoord).r;
	normal.y = 1.0;
	normal.z = -texture2D(nZ,texCoord).r;
	normal = normalize(normal);
	vec3 worldSpacePt = (Model*vec4(pt,1.0)).xyz;
	iPos = worldSpacePt;
	viewVector = normalize(cameraPos - worldSpacePt);
	gl_Position = Projection * ModelView * vec4(pt,1.0);
}
