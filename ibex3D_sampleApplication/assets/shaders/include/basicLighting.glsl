float diffuse_lambertian(vec3 normal, vec3 lightDir)
{
	float NdotL = dot(normal, lightDir);
	return max(NdotL, 0.0);
}

float specular_blinnPhong(vec3 normal, vec3 lightDir, vec3 viewDir, float shininess)
{
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float NdotH = dot(normal, halfwayDir);

	return pow(max(NdotH, 0.0), shininess);
}