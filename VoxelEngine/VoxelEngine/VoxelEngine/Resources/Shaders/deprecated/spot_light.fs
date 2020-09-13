#version 450 core

struct Material
{
  sampler2D diffuse;
  sampler2D specular;
  float     shininess;
}; 

struct Light
{
  vec3 position;    // used for point lights
  vec3 direction;   // used for directional lights
  float cutoff;     // used for spot lights
  float outerCutoff;// used for spot lights
  bool directional;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  float constant;
  float linear;
  float quadratic;
};

out vec4 color;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

in vec3 vNormal;
in vec3 vPos;
in vec2 vTexCoords;

void main()
{
  // ambient
  vec3 ambient = light.ambient * texture(material.diffuse, vTexCoords).rgb;

  // diffuse 
  vec3 norm = normalize(vNormal);
  vec3 lightDir = normalize(light.position - vPos);
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = light.diffuse * diff * texture(material.diffuse, vTexCoords).rgb;  

  // specular
  vec3 viewDir = normalize(viewPos - vPos);
  vec3 reflectDir = reflect(-lightDir, norm);  
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
  vec3 specular = light.specular * spec * texture(material.specular, vTexCoords).rgb;  

  // spotlight (soft edges)
  float theta = dot(lightDir, normalize(-light.direction)); 
  float epsilon = (light.cutoff - light.outerCutoff);
  float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
  diffuse  *= intensity;
  specular *= intensity;

  // attenuation
  float distance    = length(light.position - vPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
  ambient  *= attenuation; 
  diffuse  *= attenuation;
  specular *= attenuation;   
  
  vec3 result = ambient + diffuse + specular;
  color = vec4(result, 1.0);
}