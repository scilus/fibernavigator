#include lighting.fs

varying vec4 myColor;
varying vec4 VaryingTexCoord0;

uniform bool showFS;
uniform int sector;
uniform float cutX, cutY, cutZ;

uniform int dimX, dimY, dimZ;
uniform float voxX, voxY, voxZ;
uniform float alpha_;

uniform bool useTex;
uniform bool blendTex;
uniform bool isGlyph;

uniform sampler2D cutTex;

uniform bool lightOn;

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

void main()
{

	if (!showFS) cutFrontSector();

	vec4 ambient = vec4(0.0);
	vec4 diffuse = vec4(0.0);
	vec4 specular = vec4(0.0);

	if ( lightOn )
	   calculateLighting(-normal, gl_FrontMaterial.shininess, ambient, diffuse, specular);

	vec4 color = gl_FrontLightModelProduct.sceneColor + (ambient
            * gl_FrontMaterial.ambient) + (diffuse
            * gl_FrontMaterial.diffuse) + (specular
            * gl_FrontMaterial.specular);

	if ( useTex )
	{
	   vec3 v = VaryingTexCoord0.xyz;
		v.x = v.x / (float(dimX) * voxX);
		v.y = v.y / (float(dimY) * voxY);
		v.z = v.z / (float(dimZ) * voxZ);

		if (blendTex)
		{
		     /*if ( type9 > 0 ) lookupTex( color, type9, tex9, 0.0, v, alpha9 );
             if ( type8 > 0 ) lookupTex( color, type8, tex8, 0.0, v, alpha8 );
             if ( type7 > 0 ) lookupTex( color, type7, tex7, 0.0, v, alpha7 );
             if ( type6 > 0 ) lookupTex( color, type6, tex6, 0.0, v, alpha6 );*/
             if ( type5 > 0 ) lookupTex( color, type5, tex5, 0.0, v, alpha5 );
             if ( type4 > 0 ) lookupTex( color, type4, tex4, 0.0, v, alpha4 );
             if ( type3 > 0 ) lookupTex( color, type3, tex3, 0.0, v, alpha3 );
             if ( type2 > 0 ) lookupTex( color, type2, tex2, 0.0, v, alpha2 );
             if ( type1 > 0 ) lookupTex( color, type1, tex1, 0.0, v, alpha1 );
             if ( type0 > 0 ) lookupTex( color, type0, tex0, 0.0, v, alpha0 );
		}
		else
		{
		    /* if ( type9 > 0 ) lookupTex( color, type9, tex9, threshold9, v, alpha9 );
		     if ( type8 > 0 ) lookupTex( color, type8, tex8, threshold8, v, alpha8 );
		     if ( type7 > 0 ) lookupTex( color, type7, tex7, threshold7, v, alpha7 );
		     if ( type6 > 0 ) lookupTex( color, type6, tex6, threshold6, v, alpha6 );*/
		     if ( type5 > 0 ) lookupTex( color, type5, tex5, threshold5, v, alpha5 );
		     if ( type4 > 0 ) lookupTex( color, type4, tex4, threshold4, v, alpha4 );
		     if ( type3 > 0 ) lookupTex( color, type3, tex3, threshold3, v, alpha3 );
		     if ( type2 > 0 ) lookupTex( color, type2, tex2, threshold2, v, alpha2 );
		     if ( type1 > 0 ) lookupTex( color, type1, tex1, threshold1, v, alpha1 );
		     if ( type0 > 0 ) lookupTex( color, type0, tex0, threshold0, v, alpha0 );
		}

		color = color; /*+ (ambient * color / 2.0) + (diffuse * color) + (specular * color / 2.0);*/
	}
	else if (isGlyph && !useTex)
	{
	    colorMap( color.rgb, myColor.r);
	    color = color + (ambient * color / 2.0) + (diffuse * color / 2.0) + (specular * color / 2.0);
	}

	color.a = alpha_;
	color = clamp(color, 0.0, 1.0);

    gl_FragColor = color;
}
