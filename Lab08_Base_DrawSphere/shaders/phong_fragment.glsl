uniform vec3 u_color;//rgb
uniform vec4 u_lightPosDir[8]; //W component is 'boolean
uniform float u_lightIntensity[8];
uniform vec3 u_cameraPos; // Position of the camera in world space
uniform int u_numLights; // How many lights do we have in the scene
uniform int u_isBlinnPhong; // 0 is false
uniform float u_shininess;

varying vec3 surfaceNormal;
varying vec3 worldPosition;

void main() {

  vec3 N = normalize(surfaceNormal);
  vec3 V = normalize(u_cameraPos - worldPosition);

  //Phong/Blinn-Phong - ASD (Ambient,Specular, diffuse)
  float ambient = 0.2;
  float diffuse = 0.0;
  float specular = 0.0;

  for (int i = 0; i < u_numLights; i++){
    vec3 L = vec3(0.0);

    if (u_lightPosDir[i].w == 1.0) // point light
    {
      //Do the math here
      L = normalize(u_lightPosDir[i].xyz - worldPosition);
    }
      else // directional light
    {
      L = normalize(-u_lightPosDir[i].xyz);
    }

    //Blinn Phong
    vec3 H = normalize(L + V);
    //Phong
    vec3 R = reflect(N, -1.0 * L);

    if(u_isBlinnPhong == 1)
    {
      float specularAngle = max(0.0, dot(V, H));
      specular += pow(specularAngle, u_shininess) * u_lightIntensity[i];
    }
    else //is Phong
    {
      float specularAngle = max(0.0, dot(V, R));
      specular += pow(specularAngle, u_shininess / 4.0) * u_lightIntensity[i];
    }

    diffuse += max(0.0, dot(N, L))* u_lightIntensity[i];

  }

  gl_FragColor = vec4(u_color * (ambient + diffuse + specular), 1.0);
}
