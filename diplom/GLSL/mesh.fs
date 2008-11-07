#include GLSL/lighting.fs

uniform int sector;
uniform float cutX, cutY, cutZ;
uniform int dimX, dimY, dimZ;
uniform float alpha_;

uniform bool showFS;
uniform bool useTex;
uniform bool blendTex;
uniform bool cutAtSurface;

uniform sampler3D texes[10];
uniform sampler2D cutTex;
uniform bool show[10];
uniform float threshold[10];
uniform float alpha[10];
uniform int type[10];
uniform int countTextures;

varying float flag;

vec3 defaultColorMap(float value) 
{
	value *= 5.0;
	vec3 color;

	if (value < 0.0)
		color = vec3(0.0, 0.0, 0.0);
	else if (value < 1.0)
		color = vec3(0.0, value, 1.0);
	else if (value < 2.0)
		color = vec3(0.0, 1.0, 2.0 - value);
	else if (value < 3.0)
		color = vec3(value - 2.0, 1.0, 0.0);
	else if (value < 4.0)
		color = vec3(1.0, 4.0 - value, 0.0);
	else if (value <= 5.0)
		color = vec3(1.0, 0.0, value - 4.0);
	else
		color = vec3(1.0, 0.0, 1.0);
	return color;
}

void cutFrontSector() {
	if (sector == 1 && vertex.x > cutX && vertex.y > cutY && vertex.z > cutZ)
		discard;
	if (sector == 2 && vertex.x > cutX && vertex.y > cutY && vertex.z < cutZ)
		discard;
	if (sector == 3 && vertex.x > cutX && vertex.y < cutY && vertex.z < cutZ)
		discard;
	if (sector == 4 && vertex.x > cutX && vertex.y < cutY && vertex.z > cutZ)
		discard;
	if (sector == 5 && vertex.x < cutX && vertex.y < cutY && vertex.z > cutZ)
		discard;
	if (sector == 6 && vertex.x < cutX && vertex.y < cutY && vertex.z < cutZ)
		discard;
	if (sector == 7 && vertex.x < cutX && vertex.y > cutY && vertex.z < cutZ)
		discard;
	if (sector == 8 && vertex.x < cutX && vertex.y > cutY && vertex.z > cutZ)
		discard;
}

void cutAtSplineSurface() {
	vec3 u = gl_TexCoord[0].xyz;
	u.y = u.y / float(dimY);
	u.z = u.z / float(dimZ);

	for (int i = 9; i > -1; i--) {
		if (type[i] == 5) {
			if (vertex.x < (texture2D(cutTex, u.yz).r * float(dimX)))
				discard;
		}
	}
}

void lookupTex(inout vec4 color, in int type, in sampler3D tex, in float threshold, in vec3 v, in float alpha)
{
	vec3 col1;
	if (!blendTex)
	threshold = 0.0;
	if (type == 3)
	{
		col1.r = clamp( texture3D(tex, v).r, 0.0, 1.0);

		if (col1.r - threshold> 0.0)
		{
			color.rgb = ((1.0 - alpha) * color.rgb) + (alpha * defaultColorMap( col1.r));
		}
	}
	if (type == 1 || type == 2 || type == 4)
	{
		col1.r = clamp( texture3D(tex, v).r, 0.0, 1.0);
		col1.g = clamp( texture3D(tex, v).g, 0.0, 1.0);
		col1.b = clamp( texture3D(tex, v).b, 0.0, 1.0);

		if ( ((col1.r + col1.g + col1.b) / 3.0 - threshold)> 0.0)
		{
			color.rgb = ((1.0 - alpha) * color.rgb) + (alpha * col1.rgb);
		}
	}
}

void main() {
	if (flag < -0.0)
		discard;

	if (!showFS)
		cutFrontSector();

	if (cutAtSurface)
		cutAtSplineSurface();

	/* Normalize the normal. A varying variable CANNOT
	 // be modified by a fragment shader. So a new variable
	 // needs to be created. */
	vec3 n = normal.xyz;

	vec4 ambient = vec4(0.0);
	vec4 diffuse = vec4(0.0);
	vec4 specular = vec4(0.0);

	/* In this case the built in uniform gl_MaxLights is used
	 // to denote the number of lights. A better option may be passing
	 // in the number of lights as a uniform or replacing the current
	 // value with a smaller value. */
	calculateLighting(gl_MaxLights, -n, vertex.xyz, gl_FrontMaterial.shininess,
			ambient, diffuse, specular);

	vec4 color = vec4(0.0);

	if (useTex) {
		vec3 v = gl_TexCoord[0].xyz;
		v.x = v.x / float(dimX);
		v.y = v.y / float(dimY);
		v.z = v.z / float(dimZ);

		for (int i = 9; i > -1; i--) {
			if (show[i])
				lookupTex(color, type[i], texes[i], threshold[i], v, alpha[i]);
		}

		color = color + (ambient * color / 2.0) + (diffuse * color / 2.0)
				+ (specular * color / 2.0);
	}

	if (color.rgb == vec3(0.0)) {
		color = gl_FrontLightModelProduct.sceneColor + (ambient
				* gl_FrontMaterial.ambient) + (diffuse
				* gl_FrontMaterial.diffuse) + (specular
				* gl_FrontMaterial.specular);
	}

	color.a = alpha_;
	color = clamp(color, 0.0, 1.0);

	gl_FragColor = color;
}
