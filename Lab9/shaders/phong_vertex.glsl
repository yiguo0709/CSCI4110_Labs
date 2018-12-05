
uniform mat4 u_MVP;

attribute vec4 position;
attribute vec2 textureCoords;
attribute vec3 normal;

varying vec3 surfaceNormal;
varying vec3 worldPosition;
varying vec2 textureCoordinates;

void main() {
    gl_Position = u_MVP * position;
    worldPosition = gl_Position.xyz;
    surfaceNormal = normal;
    textureCoordinates = textureCoords;
	textureCoordinates.y = 1.0f - textureCoordinates.y;
}
