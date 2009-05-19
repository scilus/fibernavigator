#include lighting.fs

uniform int sector;
uniform float cutX, cutY, cutZ;
uniform int dimX, dimY, dimZ;
uniform float voxX, voxY, voxZ;
uniform float alpha_;
varying vec4 myColor;

uniform bool showFS;
uniform bool useTex;
uniform bool blendTex;
uniform bool cutAtSurface;
uniform bool useCMAP;

uniform sampler3D texes[10];
uniform sampler2D cutTex;
uniform bool show[10];
uniform float threshold[10];
uniform float alpha[10];
uniform int type[10];
uniform int countTextures;
uniform bool useLic;

#include functions.fs

void cutFrontSector() 
{
    float cx = cutX * voxX;
    float cy = cutY * voxY;
    float cz = cutZ * voxZ;

	if (sector == 1 && vertex.x > cx && vertex.y > cy && vertex.z > cz)
		discard;
	if (sector == 2 && vertex.x > cx && vertex.y > cy && vertex.z < cz)
		discard;
	if (sector == 3 && vertex.x > cx && vertex.y < cy && vertex.z < cz)
		discard;
	if (sector == 4 && vertex.x > cx && vertex.y < cy && vertex.z > cz)
		discard;
	if (sector == 5 && vertex.x < cx && vertex.y < cy && vertex.z > cz)
		discard;
	if (sector == 6 && vertex.x < cx && vertex.y < cy && vertex.z < cz)
		discard;
	if (sector == 7 && vertex.x < cx && vertex.y > cy && vertex.z < cz)
		discard;
	if (sector == 8 && vertex.x < cx && vertex.y > cy && vertex.z > cz)
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
		v.x = v.x / (float(dimX) * voxX);
		v.y = v.y / (float(dimY) * voxY);
		v.z = v.z / (float(dimZ) * voxZ);

		for (int i = 9; i > -1; i--) {
			float threshold_ = 0.0;
			if (blendTex) threshold_ = threshold[i];
			if (show[i]) lookupTexMesh(color, type[i], texes[i], threshold_, v, alpha[i]);
		}
		color = color + (ambient * color / 2.0) + (diffuse * color) + (specular * color / 2.0);	
	}
	
	else if ( useCMAP && !useTex )
	{
		colorMap( color.rgb, myColor.r);
		color = color + (ambient * color / 2.0) + (diffuse * color / 2.0) + (specular * color / 2.0);			
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
    {
        // gl_Color.r = LIC texture gray value
        // gl_Color.g = scalar product of triangle normal and input vector
        // gl_Color.b = FA value
        // gl_Color.a = noise texture gray value
       float fa = clamp( (gl_Color.b - 0.1) * 3., 0., 1. ); 
        
       float licBlend = (1.0 - gl_Color.g) * fa;
       vec4 tempColor = vec4(gl_Color.r, gl_Color.r, gl_Color.r, (1.0 - gl_Color.g) );
       vec4 licColor = clamp( tempColor * 1.8 - vec4(0.4), 0., 1.);
       
       
       float noiseBlend = clamp((gl_Color.g - 0.6),0., 1.) * clamp((fa - 0.2),0., 1.) * 3.;
       vec4 noiseColor = vec4( gl_Color.a );
       
       gl_FragColor = ((noiseColor - vec4(0.5)) * color * noiseBlend) + ((licColor - vec4(0.5)) * color * licBlend) + color;
    }
    else
        gl_FragColor = color;
}
