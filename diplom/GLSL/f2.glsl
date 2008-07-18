uniform int sector;
uniform int cutX, cutY, cutZ;
uniform int dimX, dimY, dimZ;

uniform bool showFS;
uniform bool useTex;

uniform sampler3D tex[10];
uniform bool show[10];
uniform float threshold[10];
uniform int type[10];
uniform int countTextures;

varying vec3 TexCoord;
varying vec3 normal;
varying vec4 vertex;
varying vec4 half;

const vec4 AMBIENT_BLACK = vec4(0.0, 0.0, 0.0, 1.0);
const vec4 DEFAULT_BLACK = vec4(0.0, 0.0, 0.0, 0.0);

vec3 defaultColorMap( float value )
{
    value *= 5.0;
	vec3 color;
	
	if( value < 0.0 )
		color = vec3( 0.0, 0.0, 0.0 );
    else if( value < 1.0 )
		color = vec3( 0.0, value, 1.0 );
	else if( value < 2.0 )
		color = vec3( 0.0, 1.0, 2.0-value );
    else if( value < 3.0 )
		color =  vec3( value-2.0, 1.0, 0.0 );
    else if( value < 4.0 )
		color = vec3( 1.0, 4.0-value, 0.0 );
    else if( value <= 5.0 )
		color = vec3( 1.0, 0.0, value-4.0 );
    else 
		color =  vec3( 1.0, 0.0, 1.0 );
	return color;
}

bool isLightEnabled(in int i)
{
    /* A separate variable is used to get
    // rid of a linker error.*/
    bool enabled = true;
   
    /* If all the colors of the Light are set
    // to BLACK then we know we don't need to bother
    // doing a lighting calculation on it. */
    if ((gl_LightSource[i].ambient  == AMBIENT_BLACK) &&
        (gl_LightSource[i].diffuse  == DEFAULT_BLACK) &&
        (gl_LightSource[i].specular == DEFAULT_BLACK))
        enabled = false;
       
    return(enabled);
}

float calculateAttenuation(in int i, in float dist)
{
    return(1.0 / (gl_LightSource[i].constantAttenuation +
                  gl_LightSource[i].linearAttenuation * dist +
                  gl_LightSource[i].quadraticAttenuation * dist * dist));
}

void directionalLight(in int i, in vec3 normal, in float shininess,
                      inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
	float nDotVP;
	float nDotHV;
	float pf;
	
	vec3 L = normalize (gl_LightSource[i].position - vertex.xyz);
	vec3 H = normalize (L+ half.xyz); 

	nDotVP = max(0.0, dot(normal, normalize(vec3(gl_LightSource[i].position))));
	nDotHV = max(0.0, dot(normal, H));
	
	if (nDotVP == 0.0)
		pf = 0.0;
	else
		pf = pow(nDotHV, gl_FrontMaterial.shininess);
	
	ambient += gl_LightSource[i].ambient;
	diffuse += gl_LightSource[i].diffuse * nDotVP;
	specular += gl_LightSource[i].specular * pf;     
    
}

void pointLight(in int i, in vec3 N, in vec3 V, in float shininess,
                inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
    vec3 D = gl_LightSource[i].position.xyz - V;
    vec3 L = normalize(D);

    float dist = length(D);
    float attenuation = calculateAttenuation(i, dist);

    float nDotL = dot(N,L);

    if (nDotL > 0.0)
    {   
        vec3 E = normalize(-V);
        vec3 R = reflect(-L, N);
       
        float pf = pow(max(dot(R,E), 0.0), shininess);

        diffuse  += gl_LightSource[i].diffuse  * attenuation * nDotL;
        specular += gl_LightSource[i].specular * attenuation * pf;
    }
   
    ambient  += gl_LightSource[i].ambient * attenuation;
}

void spotLight(in int i, in vec3 N, in vec3 V, in float shininess,
               inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
    vec3 D = gl_LightSource[i].position.xyz - V;
    vec3 L = normalize(D);

    float dist = length(D);
    float attenuation = calculateAttenuation(i, dist);

    float nDotL = dot(N,L);

    if (nDotL > 0.0)
    {   
        float spotEffect = dot(normalize(gl_LightSource[i].spotDirection), -L);
       
        if (spotEffect > gl_LightSource[i].spotCosCutoff)
        {
            attenuation *=  pow(spotEffect, gl_LightSource[i].spotExponent);

            vec3 E = normalize(-V);
            vec3 R = reflect(-L, N);
       
            float pf = pow(max(dot(R,E), 0.0), shininess);

            diffuse  += gl_LightSource[i].diffuse  * attenuation * nDotL;
            specular += gl_LightSource[i].specular * attenuation * pf;
        }
    }
   
    ambient  += gl_LightSource[i].ambient * attenuation;
}

void calculateLighting(in int numLights, in vec3 N, in vec3 V, in float shininess,
                       inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
    /* Just loop through each light, and if its enabled add
    // its contributions to the color of the pixel. */
    for (int i = 0; i < numLights; i++)
    {
        if (isLightEnabled(i))
        {
            if (gl_LightSource[i].position.w == 0.0)
                directionalLight(i, N, shininess, ambient, diffuse, specular);
            else if (gl_LightSource[i].spotCutoff == 180.0)
                pointLight(i, N, V, shininess, ambient, diffuse, specular);
            else
                 spotLight(i, N, V, shininess, ambient, diffuse, specular);
        }
    }
}

void cutFrontSector()
{
	if (sector == 1 &&
		vertex.x > cutX &&
		vertex.y > cutY &&
		vertex.z > cutZ) discard; 
	if (sector == 2 &&
		vertex.x > cutX &&
		vertex.y > cutY &&
		vertex.z < cutZ) discard;
	if (sector == 3 &&
		vertex.x > cutX &&
		vertex.y < cutY &&
		vertex.z < cutZ) discard;
	if (sector == 4 &&
		vertex.x > cutX &&
		vertex.y < cutY &&
		vertex.z > cutZ) discard;
	if (sector == 5 &&
		vertex.x < cutX &&
		vertex.y < cutY &&
		vertex.z > cutZ) discard;
	if (sector == 6 &&
		vertex.x < cutX &&
		vertex.y < cutY &&
		vertex.z < cutZ) discard;
	if (sector == 7 &&
		vertex.x < cutX &&
		vertex.y > cutY &&
		vertex.z < cutZ) discard;
	if (sector == 8 &&
		vertex.x < cutX &&
		vertex.y > cutY &&
		vertex.z > cutZ) discard;
}

void lookupTex(inout vec4 color, in int type, in sampler3D tex, in float threshold, in vec3 v)
{
	vec3 col1;
	if (type == 3)
	{
		col1.r = clamp( texture3D(tex, v).r, 0.0, 1.0);
		
		if (col1.r - threshold > 0.0)
		{
			color.rgb = defaultColorMap( col1.r);
		}
	}	
	if (type == 1 || type == 2 || type == 4)
	{
		col1.r = clamp( texture3D(tex, v).r, 0.0, 1.0);
		col1.g = clamp( texture3D(tex, v).g, 0.0, 1.0);
		col1.b = clamp( texture3D(tex, v).b, 0.0, 1.0);
		
		if ( ((col1.r + col1.g + col1.b) / 3.0 - threshold) > 0.0)
		{
			color.rgb = col1.rgb;
		}
	}
}

 
void main()
{
	vec4 color = vec4(0.0);
	
	if (!showFS)
    	cutFrontSector();
	
	/* Normalize the normal. A varying variable CANNOT
    // be modified by a fragment shader. So a new variable
    // needs to be created. */
    vec3 n = normal;
   
    vec4 ambient  = vec4(0.0);
    vec4 diffuse  = vec4(0.0);
    vec4 specular = vec4(0.0);

    /* In this case the built in uniform gl_MaxLights is used
    // to denote the number of lights. A better option may be passing
    // in the number of lights as a uniform or replacing the current
    // value with a smaller value. */
    calculateLighting(gl_MaxLights, -n, vertex, gl_FrontMaterial.shininess,
                      ambient, diffuse, specular);
   
   vec4 tmpColor = vec4(0.0); 
 
    if (useTex)
    {
	    vec3 v = TexCoord;
	    v.x = (v.x + dimX/2) / (float)dimX;
	    v.y = (v.y + dimY/2) / (float)dimY;
	    v.z = (v.z + dimZ/2) / (float)dimZ;
	    
	    for (int i = 9 ; i > -1 ; i--)
		{
			if (show[i]) lookupTex(tmpColor, type[i], tex[i], threshold[i], v);
		}
		
		
		tmpColor.a = 1.0;	
    }
    else
    {
   		tmpColor = gl_FrontLightModelProduct.sceneColor;
   	} 
  
   color =   tmpColor +
              (ambient  * gl_FrontMaterial.ambient) +
              (diffuse  * gl_FrontMaterial.diffuse) +
              (specular * gl_FrontMaterial.specular);
  
    color = clamp(color, 0.0, 1.0);
   
    gl_FragColor = color;
}
