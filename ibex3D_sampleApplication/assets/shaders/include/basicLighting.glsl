float diffuse_lambertian(vec3 normal, vec3 lightPos)
{
	float NdotL = dot(normal, lightPos);
	return max(NdotL, 0.0f);
}