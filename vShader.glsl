#version 430
// 顶点着色器
attribute vec3 position; //坐标
attribute vec3 vertNormal;

struct PLight {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec3 position;
};
struct Material {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};
uniform mat4 mvMat;  //mv矩阵
uniform mat4 pMat;  //视图矩阵
uniform mat4 nMat;
uniform vec4 globalAmbient;
uniform PLight light;
uniform Material material;
uniform float mix_val; //高度(0-1)
uniform float rx;
uniform float ry;
 
out vec3 varyingNormal;
out vec3 varyingLightDir;
out vec3 varyingVertPos;
uniform bool enableLight; //启用光照
uniform int colorMode;   //颜色模式

void main( void )
{
	if(enableLight) {
		varyingVertPos = (mvMat * vec4(position,1.0)).xyz;
		varyingLightDir = light.position - varyingVertPos;
		varyingNormal = (nMat * vec4(vertNormal,1.0)).xyz;
	} else {
		varyingVertPos = vec3(0.0,0.0,0.0);
		varyingLightDir = vec3(0.0,0.0,0.0);
		varyingNormal = vec3(0.0,0.0,0.0);
	}
	
	gl_Position = pMat * mvMat * vec4( position, 1.0 );
}