/*******************************************************
* Fixed.frag Fixed Function Equivalent Fragment Shader *
*   Automatically Generated by 3Dlabs GLSL ShaderGen   *
*             http://developer.3dlabs.com              *
*******************************************************/
uniform sampler2D texUnit0;

void main (void) 
{
    vec4 color;

    color = gl_Color;

    color *= texture2D(texUnit0, gl_TexCoord[0].xy);

    gl_FragColor = color;
}
