#include lighting.fs

uniform int sector;
uniform float cutX, cutY, cutZ;
uniform int dimX, dimY, dimZ;
uniform float alpha_;
varying vec4 myColor;

uniform bool showFS;
uniform bool useTex;
uniform bool useCMAP;
uniform bool blendTex;
uniform bool cutAtSurface;

uniform sampler3D texes[10];
uniform sampler2D cutTex;
uniform bool show[10];
uniform float threshold[10];
uniform float alpha[10];
uniform int type[10];
uniform int countTextures;
uniform bool useLic;

#include functions.fs

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

void main() {
	if (gl_FrontFacing)
		;//discard;

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

	if ( useTex ) 
	{
		vec3 v = gl_TexCoord[0].xyz;
		v.x = v.x / float(dimX);
		v.y = v.y / float(dimY);
		v.z = v.z / float(dimZ);

		for (int i = 9; i > -1; i--) {
			float threshold_ = 0.0;
			if (blendTex) threshold_ = threshold[i];
			if (show[i]) lookupTexMesh(color, type[i], texes[i], threshold_, v, alpha[i]);
		}

		color = color + (ambient * color / 2.0) + (diffuse * color / 2.0)
				+ (specular * color / 2.0);
	}
	
	if ( useCMAP )
	{
		color.rgb = defaultColorMap( myColor.r );
	} 

	if ( color.rgb == vec3(0.0) ) 
	{
		color = gl_FrontLightModelProduct.sceneColor + (ambient
				* gl_FrontMaterial.ambient) + (diffuse
				* gl_FrontMaterial.diffuse) + (specular
				* gl_FrontMaterial.specular);
	}

	color.a = alpha_;
	color = clamp(color, 0.0, 1.0);

	if (useLic)
			gl_FragColor = (0.7  * gl_Color) + (0.3 * color);
		else
			gl_FragColor = color;
	//gl_FragColor = color;
}
