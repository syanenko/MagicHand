// Glowing Line
// By: Brandon Fogerty
// bfogerty at gmail dot com
// xdpixel.com

#ifdef GL_ES
precision mediump float;
#endif

uniform float u_time;
uniform vec2 u_resolution;

void main( void )
 {
	vec2 uv = ( gl_FragCoord.xy / u_resolution.xy ) * 2.0 - 1.0;

	vec3 finalColor = vec3 ( 0.1, 0.9, 0.2 );
	
	finalColor *= abs( 10.1 / (sin( uv.x + sin(uv.y+u_time)* 0.10 ) * 40.0) );

	gl_FragColor = vec4( finalColor, 1.0 );
}
