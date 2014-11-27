uniform int useColorMap;

uniform sampler3D tex0;
uniform sampler3D tex1;
uniform sampler3D tex2;
uniform sampler3D tex3;
uniform sampler3D tex4;
uniform sampler3D tex5;
uniform sampler3D tex6;
uniform sampler3D tex7;
uniform sampler3D tex8;
uniform sampler3D tex9;

uniform float threshold0;
uniform float threshold1;
uniform float threshold2;
uniform float threshold3;
uniform float threshold4;
uniform float threshold5;
uniform float threshold6;
uniform float threshold7;
uniform float threshold8;
uniform float threshold9;

uniform float alpha0;
uniform float alpha1;
uniform float alpha2;
uniform float alpha3;
uniform float alpha4;
uniform float alpha5;
uniform float alpha6;
uniform float alpha7;
uniform float alpha8;
uniform float alpha9;

uniform int type0;
uniform int type1;
uniform int type2;
uniform int type3;
uniform int type4;
uniform int type5;
uniform int type6;
uniform int type7;
uniform int type8;
uniform int type9;

vec3 defaultColorMap( in float value )
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

vec3 colorMap1 ( in float value )
{
	float i = floor(6.*value);
	float f = 6.*value - i;
	float q = 1.0-f;

	int iq = int(mod(i, 6.));

	if ((iq == 0) || (iq == 6))
		return vec3(1., f, 0.);
	else if (iq == 1)
		return vec3(q, 1., 0.);
	else if (iq == 2)
		return vec3(0., 1., f);
	else if (iq == 3)
		return vec3(0., q, 1.);
	else if (iq == 4)
		return vec3(f, 0., 1.);
	else // iq == 5
		return vec3(1., 0., q);
}

vec3 colorMap2( in float value )
{
	vec4 color8  = vec4(255./255., 255./255., 204./255., 1.);
	vec4 color7  = vec4(255./255., 237./255., 160./255., 1.);
	vec4 color6  = vec4(254./255., 217./255., 118./255., 1.);
	vec4 color5  = vec4(254./255., 178./255.,  76./255., 1.);
	vec4 color4  = vec4(253./255., 141./255.,  60./255., 1.);
	vec4 color3  = vec4(252./255.,  78./255.,  42./255., 1.);
	vec4 color2  = vec4(227./255.,  26./255.,  28./255., 1.);
	vec4 color1  = vec4(189./255.,   0./255.,  38./255., 1.);
	vec4 color0  = vec4(128./255.,   0./255.,  38./255., 1.);

	float colorValue = value * 8.;
	int sel = int(floor(colorValue));

	if ( sel >= 8 )
		return color0.rgb;
	else if ( sel < 0 )
		return color0.rgb;
	else
	{
		colorValue -= float(sel);

		if (sel < 1)
			return ( color1*colorValue + color0*(1.-colorValue)).rgb;
		else if (sel < 2)
			return ( color2*colorValue + color1*(1.-colorValue)).rgb;
		else if (sel < 3)
			return ( color3*colorValue + color2*(1.-colorValue)).rgb;
		else if (sel < 4)
			return ( color4*colorValue + color3*(1.-colorValue)).rgb;
		else if (sel < 5)
			return ( color5*colorValue + color4*(1.-colorValue)).rgb;
		else if (sel < 6)
			return ( color6*colorValue + color5*(1.-colorValue)).rgb;
		else if (sel < 7)
			return ( color7*colorValue + color6*(1.-colorValue)).rgb;
		else if (sel < 8)
			return ( color8*colorValue + color7*(1.-colorValue)).rgb;
		else return color0.rgb;
	}
}

vec3 colorMap3( in float value )
{
	vec4 color0 = vec4(1., 0., 0., 1.);
	vec4 color1 = vec4(1., 1., 0., 1.);
    return ( color1*value + color0*(1.-value)).rgb;
}

vec3 colorMap4( in float value )
{
	vec4 color0 = vec4(0., 0., 1., 1.);
	vec4 color1 = vec4(0.78, 1., 1., 1.);
    return ( color1*value + color0*(1.-value)).rgb;
}

vec3 colorMap5( in float value )
{
	vec4 color0 = vec4(255./255., 255./255., 217./255., 1.);
	vec4 color1 = vec4(237./255., 248./255., 177./255., 1.);
	vec4 color2 = vec4(199./255., 233./255., 180./255., 1.);
	vec4 color3 = vec4(127./255., 205./255., 187./255., 1.);
	vec4 color4 = vec4( 65./255., 182./255., 196./255., 1.);
	vec4 color5 = vec4( 29./255., 145./255., 192./255., 1.);
	vec4 color6 = vec4( 34./255.,  94./255., 168./255., 1.);
	vec4 color7 = vec4( 37./255.,  52./255., 148./255., 1.);
	vec4 color8 = vec4(  8./255.,  29./255.,  88./255., 1.);

	float colorValue = value * 8.;
	int sel = int(floor(colorValue));

	if ( sel >= 8 )
		return color0.rgb;
	else if ( sel < 0 )
		return color0.rgb;
	else
	{
		colorValue -= float(sel);

		if (sel < 1)
			return ( color1*colorValue + color0*(1.-colorValue)).rgb;
		else if (sel < 2)
			return ( color2*colorValue + color1*(1.-colorValue)).rgb;
		else if (sel < 3)
			return ( color3*colorValue + color2*(1.-colorValue)).rgb;
		else if (sel < 4)
			return ( color4*colorValue + color3*(1.-colorValue)).rgb;
		else if (sel < 5)
			return ( color5*colorValue + color4*(1.-colorValue)).rgb;
		else if (sel < 6)
			return ( color6*colorValue + color5*(1.-colorValue)).rgb;
		else if (sel < 7)
			return ( color7*colorValue + color6*(1.-colorValue)).rgb;
		else if (sel < 8)
			return ( color8*colorValue + color7*(1.-colorValue)).rgb;
		else return color0.rgb;
	}
}

vec3 colorMap6( in float value )
{
	float basecolor = 0.0;
	float frequency = 5.0;
	float sqrt3 = sqrt(3.);
	float onedtwodsqrt3 = 1. / 2. / sqrt3;
	float onepsqrt3 = 1. + sqrt3;
	float onemsqrt3 = 1. - sqrt3;
	float wvalue = sqrt(3./2.) * value * (1.-value); // = omega(value)
	float twotz = 2. * sqrt3 * value; // = 2. * z(value)
	float sinTerm = sin(frequency*value+basecolor);
	float cosTerm = cos(frequency*value+basecolor);
	float wtsinTerm = wvalue * sinTerm;
	float wtcosTerm = wvalue * cosTerm;

	float colorRed   = (onedtwodsqrt3*(onepsqrt3*wtsinTerm + onemsqrt3*wtcosTerm + twotz));
	float colorGreen = (onedtwodsqrt3*(onemsqrt3*wtsinTerm + onepsqrt3*wtcosTerm + twotz));
	float colorBlue  = (onedtwodsqrt3*(-2.*(wtsinTerm + wtcosTerm) + twotz));

	return vec3(colorRed, colorGreen, colorBlue);
}

void colorMap( inout vec3 col, in float value )
{

	if ( useColorMap == 1 )
		col = colorMap1( value );
	else if ( useColorMap == 2 )
		col = colorMap2( value );
	else if ( useColorMap == 3 )
		col = colorMap3( value );
	else if ( useColorMap == 4 )
		col = colorMap4( value );
	else
	    col = defaultColorMap( value );
}

void lookupTex(inout vec4 col, in int type, in sampler3D tex, in float threshold, in vec3 v, in float alpha)
{
	vec3 col1 = vec3(0.0);

	col1 = clamp( texture3D(tex, v).rgb, 0.0, 1.0);

	if ( ( col1.r + col1.g + col1.b ) / 3.0  - threshold <= 0.0) return;

	if ( type == 3 && useColorMap != -1)
	{
		if (threshold < 1.0)
			col1.r = (col1.r - threshold) / (1.0 - threshold);

		colorMap(col1, col1.r);
	}

	col.rgb = mix( col.rgb, col1.rgb, alpha);
}
