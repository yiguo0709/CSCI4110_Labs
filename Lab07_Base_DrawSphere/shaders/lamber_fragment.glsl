#version 330

uniform vec3 u_color;//rgb
uniform vec4 u_lightPosDir; //W component is 'boolean
'
in vec3 surfaceNormal;
in vec3 worldPosition;

void main() {

  vec3 normal = normalize(surfaceNormal);
  vec3 lightDirection = vec3(0.0f);

  if (u_lightPosDir.w == 1.0f) // point light
  {
    //Do the math here
    lightDirection = normalize(worldPosition - u_lightPosDir.xyz);
  }
  else // directional light
  {
    lightDirection = normalize(u_lightPosDir.xyz);
  }

  //lambert
  float ambient = 0.2f;
  float diffuse = max(0.0f, dot(normal, lightDirection));

  gl_FragColor = vec4(u_color * (ambient + diffuse), 1.0);
}
