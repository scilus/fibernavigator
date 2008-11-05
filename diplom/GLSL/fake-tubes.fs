varying vec3 tangentR3;				// Tangent vector in world space
varying float s_param;				// s parameter of texture [-1..1]
varying float tangent_dot_view;

/*
 * simple fragment shader that does rendering of tubes with diffuse illumination
 */
void main()
{
  //discard;
  vec3 color = abs(normalize(tangentR3));

  vec3 view = vec3(0.,0.,-1.);
  float view_dot_normal = sqrt(1.-s_param*s_param);

  float s = (s_param+1.)*0.5;			  //< normalize in [0..1]
  gl_FragColor.rgb = clamp(view_dot_normal *(color +pow(view_dot_normal,30.)*pow(tangent_dot_view,30.)*vec3(1.,1.,1.)), 0., 1.);         //< set the color of this fragment (i.e. pixel)
  gl_FragColor.a   = 1.;
}

