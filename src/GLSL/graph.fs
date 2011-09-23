varying vec3 tangentR3; // Tangent vector in world space
varying float s_param; // s parameter of texture [-1..1]
varying float tangent_dot_view;
varying vec4 myColor;
uniform bool globalColor;
uniform float animation;
varying float dist;
varying float leng;
uniform float thickness;

/*
 * simple fragment shader that does rendering of tubes with diffuse illumination
 */
void main()
{

    vec3 color;
    if (globalColor)
        color = abs(normalize(myColor.rgb));
    else
        color = abs(normalize(tangentR3));// * (animation / 4.0);


    vec3 view = vec3(0., 0., -1.);
    float view_dot_normal = sqrt(1. - s_param * s_param) + .1;

    gl_FragColor.a = 1.0;
    gl_FragColor.rgb = clamp(view_dot_normal * (color + 0.15 * pow( view_dot_normal, 10.) *
            pow(tangent_dot_view, 10.) * vec3(1., 1., 1.)), 0., 1.); //< set the color of this fragment (i.e. pixel)

    float blobSize = 1./leng;

    float thickness2 = thickness * 2.0;

    int v1 = int(thickness2);

    float ani = ( animation - leng * floor(animation/leng));

    for ( int i = 0 ; i < v1 ; ++i)
    {
        float ani2 =  ani +  ( leng /thickness2  * float(i) );
        ani2 = ( ani2 - leng * floor( ani2 / leng));

        if ( (ani2 / leng) - dist > 0.0 )
        {
            if ( (ani2 / leng - blobSize) < dist )
            gl_FragColor = vec4(1.0);
        }
    }

}

