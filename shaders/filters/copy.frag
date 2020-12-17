// Copy verbatim

uniform sampler2D u_texture;

void main()
{
  gl_FragColor = texture2D(u_texture, gl_TexCoord[0].st);
}
