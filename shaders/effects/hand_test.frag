// Glowing Line
// By: Brandon Fogerty
// bfogerty at gmail dot com
// xdpixel.com

#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D u_texture;
uniform vec2      u_resolution;
uniform float     u_time;
uniform vec2      u_mouse_move;
uniform vec4      u_mouse_click;

void main( void )
 {
	vec2 uv = ( gl_FragCoord.xy / u_resolution.xy ) * 2.0 - 1.0;

	vec3 finalColor = vec3 ( 0.1, 0.6, 0.2 );
	
	finalColor *= abs( 10.1 / (sin( uv.x + sin(uv.y+u_time)* 0.10 ) * 40.0) );

	gl_FragColor = vec4( finalColor, 1.0 );
}
