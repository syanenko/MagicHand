// Hand light

uniform sampler2D u_texture;
uniform vec2 u_resolution;
uniform float u_time;
uniform vec2 u_mouse;
uniform vec2 u_hand[22];

float circle(in vec2 _st, in float _radius)
{
  vec2 dist = _st - vec2(1.0);
	return 1. - smoothstep( _radius - ( _radius * 0.01),
                          _radius + ( _radius * 0.01),
                          dot(dist, dist) * 4.0);
}

void main()
{
	vec2 st = gl_FragCoord.xy / u_resolution.xy;

  vec2 handpos = vec2(0.0);
  int active = 0;
  for(int i=0; i<22; i++)
  {
    if((u_hand[i].x <= 0) || (u_hand[i].y <= 0))
      continue;

    handpos += u_hand[i];
    active++; 
  }

  handpos = handpos / active;

  vec4 color;
  if (handpos == vec2(0.0))
    color = 1.0;
  else
    // color = vec4(vec3(distance(st, u_mouse / u_resolution.xy ) * 2.0), 1.0);
    color = vec4(vec3(distance(st, vec2(1.0) - handpos / u_resolution.xy ) * 2.0), 1.0);
    
  
  gl_FragColor = texture2D(u_texture, gl_TexCoord[0].st) - color;
}
