varying vec3 normal;
varying vec4 vertex;
varying vec3 halfvec;

const vec4 AMBIENT_BLACK = vec4(0.0, 0.0, 0.0, 1.0);
const vec4 DEFAULT_BLACK = vec4(0.0, 0.0, 0.0, 0.0);


void directionalLight( in int i, in vec3 normal, in float shininess,
                      inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
	float nDotVP;
	float nDotHV;
	float pf;

	vec3 L = normalize (gl_LightSource[i].position.xyz - vertex.xyz);
	vec3 H = normalize (L + halfvec.xyz);

	nDotVP = max(0.0, dot(normal, normalize((gl_LightSource[i].position.xyz))));
	nDotHV = max(0.0, dot(normal, H));

	if (nDotVP == 0.0)
		pf = 0.0;
	else
		pf = pow(nDotHV, gl_FrontMaterial.shininess);

	ambient += gl_LightSource[i].ambient;
	diffuse += gl_LightSource[i].diffuse * nDotVP;
	specular += gl_LightSource[i].specular * pf;

}


void calculateLighting(in vec3 N, in float shininess, inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
	directionalLight(0, N, shininess, ambient, diffuse, specular);
}

