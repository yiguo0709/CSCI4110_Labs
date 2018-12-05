uniform vec3 u_color;//rgb
uniform vec4 u_lightPosDir[8]; //W component is 'boolean
uniform float u_lightIntensity[8];
uniform vec3 u_cameraPos; // Position of the camera in world space
uniform int u_numLights; // How many lights do we have in the scene
uniform int u_isBlinnPhong; // 0 is false
uniform float u_shininess;

uniform sampler2D u_diffuse;

//Moon doesn't have specular or clouds
uniform sampler2D u_specular;
uniform sampler2D u_clouds;

uniform int u_planetSelector; //0 is moon

varying vec3 surfaceNormal;
varying vec3 worldPosition;
varying vec2 textureCoordinates;

void main() {

  vec3 N = normalize(surfaceNormal);
  vec3 V = normalize(u_cameraPos - worldPosition);

  vec4 diffuseCol = texture2D(u_diffuse, textureCoordinates);
  vec4 specularCol = texture2D(u_specular, textureCoordinates);
  vec4 cloudsCol = texture2D(u_clouds, textureCoordinates);

  if (u_planetSelector == 0) //moon
  {
    specularCol = vec4(1.0);
  }
  else if (u_planetSelector == 1) //earth
  {
    //diffuseCol = vec4(1.0) * cloudsCol.r; //additive blending
	diffuseCol.rgb = diffuseCol.rgb * (1.0f - cloudsCol.r) + vec3(1.0f) * cloudsCol.r; // Alpha blending
  }
  else if (u_planetSelector == 2)//sun
  {
    gl_FragColor = diffuseCol;
    return;
  }
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
    vec3 R = reflect(-L, N);

    if(u_isBlinnPhong == 1)
    {
      float specularAngle = max(0.0, dot(N, H));
      specular += pow(specularAngle, specularCol.r * u_shininess)
        * u_lightIntensity[i];
    }
    else //is Phong
    {
      float specularAngle = max(0.0, dot(V, R));
      specular += pow(specularAngle, (specularCol.r * u_shininess) / 4.0) * u_lightIntensity[i];
    }

    diffuse += max(0.0, dot(N, L))* u_lightIntensity[i];

  }

  gl_FragColor = vec4(vec3(diffuseCol.rgb * (ambient + diffuse) + specular), 1.0);
}
