#version 430
//面着色器
in vec3 varyingNormal;
in vec3 varyingLightDir;
in vec3 varyingVertPos;
out vec4 fragColor;

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
uniform bool enableLight; //启用光照
uniform int colorMode;   //颜色模式

void main( void )
{
	float x = (rx+5.0)/100.0;
	float y = (ry+5.0)/100.0;
	mat4 mm = mat4(x,y,x/y,y/x,x*x,y*y,x-y,y-x,x-0.2,y+0.1,x*x*x,1-x,1-y,1-x*x,1-y*y,y*y/x/x);
	vec4 c;
	if(colorMode == 0) c = vec4((mm * vec4(1-y,x,1-x*x,1.0)).yxz,1.0);
	else if(colorMode == 1) c = vec4(x*(1.2-y)+0.4,x/1.2+y/2.0+0.4,y*(1.0-x)+0.2,1.0);
	
	vec4 mAmb;
	if(y < 0.001) mAmb = vec4(0.0,0.0,0.0,1.0);
	else mAmb = mix(vec4(0.0,0.0,0.0,1.0),c,mix_val);
	
	if(enableLight) {
		vec3 L = normalize(varyingLightDir);
		vec3 N = normalize(varyingNormal);
		vec3 V = normalize(-varyingVertPos);
		
		vec3 R = normalize(reflect(-L,N));
		
		float cosTheta = dot(L,N);
		float cosPhi = dot(V,R);
		
		//float s = (rx+ry+19.2)/192.0;
		//vec4 c = vec4(s*s*s,1-s,s*s,1.0);
		//vec4 c = vec4(x*(1.2-y)+0.4,x/1.2+y/2.0+0.4,y*(1.0-x)+0.2,1.0);
		
		vec3 ambient = ((globalAmbient*mAmb) + (light.ambient*mAmb)).xyz;
		vec3 diffuse = light.diffuse.xyz * material.diffuse.xyz * max(cosTheta,0.0);
		vec3 specular = light.specular.xyz * material.specular.xyz * pow(max(cosPhi,0.0f),material.shininess);
		fragColor = vec4((ambient+diffuse+specular), 1.0);
	} else { //未启用光照
		fragColor = mAmb;
	}
	//varyingColor = globalAmbient;
	//varyingColor = mAmb;
	//varyingColor = vec4(1.0,0.0,0.0,1.0);
}