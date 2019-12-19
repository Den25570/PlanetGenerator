#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

vec4 processBloomColor(vec3 color) {
	vec3 ResultColor = color;
	float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
	if(brightness > 0.6f)
	{
     	ResultColor = color;
		ResultColor = vec3(1.0) - exp(-ResultColor * 1.0);
	    ResultColor = pow(ResultColor, vec3(1.0 / 2.2));
	}
	return vec4(ResultColor,1.0f);
}

void main()
{    
    vec3 Color = texture(skybox, TexCoords).rgb;
	gl_FragColor = vec4(Color,1.0);//processBloomColor(Color);
}