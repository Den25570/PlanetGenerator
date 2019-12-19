#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D image;
  
uniform bool horizontal;
uniform float weight[5] = float[] (0.227027, 0.1345946, 0.1216216, 0.104054, 0.116216);

void main()
{             
    // получаем размер одного текселя
    vec2 tex_offset = 1.0 / textureSize(image, 0); 
    // вклад текущего фрагмента
    vec3 result = texture(image, TexCoords).rgb * weight[0]; 
    if(horizontal)
    {
        for(int i = 4; i < 20; ++i)
        {
            result += texture(image, TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i % 5];
            result += texture(image, TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i % 5];
        }
    }
    else
    {
        for(int i = 4; i < 20; ++i)
        {
            result += texture(image, TexCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(image, TexCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
    FragColor = vec4(result, 1.0);
}