#version 330 core	     // Minimal GL version support expected from the GPU

//Camera
uniform vec3 camPos;
in vec3 fPosition;
in vec3 fNormal;
in vec2 texCoord;
out vec4 color;

uniform mat4 viewMat, modelMat;

uniform sampler2D texture;

void main() {
	
	//Calculate shade 

	vec3 position_cameraD = (viewMat * modelMat * vec4(fPosition,1)).xyz;
    vec3 direction = vec3(0,0,0) - position_cameraD;

    vec3 light = vec3(0.0f, 0.0f, 0.0f);
    vec3 light_cameraD = (viewMat * vec4(light,1)).xyz;
    vec3 light_direction = light_cameraD + direction;

    //Calculate the light
    vec3 n = normalize(vec3(transpose(inverse(modelMat)) * vec4(fNormal, 1.0f)));
	vec3 l = normalize(light_direction); // light direction vector (not hard-coded anymore)
	vec3 v = normalize(camPos - fPosition);
	vec3 r = (2* dot(n,l) * n) - l;

	// ambiente
    vec3 ambient = vec3(1.0f, 1.0f, 1.0f);

    //diffuse
	float difaux = max(dot(n, l), 0.0);
	vec3 diffuse = ambient*difaux;

	//specular
	float speaux = pow(max(dot(v, r), 0),8);
	vec3 specular = 3*speaux*ambient;

	vec3 lightPho = ambient + diffuse + specular; // build an RGBA from an RGB;

	//texture
	vec3 fragment = texture(texture, texCoord).rgb;
	
	color = vec4(lightPho * fragment, 1.0);
}
