#version 330 core
const int MAX_LIGHTS = 8;

struct Light
{
    int type;
    vec3 color;
    float intensity;
    vec3 position;
    vec3 direction;
    float range;
    float innerCos;
    float outerCos;
};

uniform Light lights[MAX_LIGHTS];
uniform int lightCount;
uniform vec3 ambientLight;
uniform vec3 viewPosition;
uniform vec3 materialColor;
uniform float shininess;
uniform bool unlit;
uniform bool useTexture;
uniform sampler2D colorTexture;
in vec3 worldPosition;
in vec3 worldNormal;
in vec2 uv;
out vec4 fragColor;

vec3 SafeNormalize(vec3 value)
{
    return value / max(length(value), 0.00001);
}

void main()
{
    vec4 base = vec4(materialColor, 1.0);
    if (useTexture)
    {
        base *= texture(colorTexture, uv);
    }
    if (unlit)
    {
        fragColor = base;
        return;
    }
    vec3 n = SafeNormalize(worldNormal);
    vec3 viewDirection = SafeNormalize(viewPosition - worldPosition);
    vec3 result = ambientLight * base.rgb;
    for (int i = 0; i < lightCount; ++i)
    {
        vec3 toLight;
        float attenuation = 1.0;
        if (lights[i].type == 0)
        {
            toLight = -SafeNormalize(lights[i].direction);
        }
        else
        {
            vec3 offset = lights[i].position - worldPosition;
            float distance = length(offset);
            toLight = SafeNormalize(offset);
            float falloff = max(1.0 - distance / max(lights[i].range, 0.001), 0.0);
            attenuation = falloff * falloff;
            if (lights[i].type == 2)
            {
                float angleCos = dot(-toLight, SafeNormalize(lights[i].direction));
                attenuation *= smoothstep(lights[i].outerCos, lights[i].innerCos, angleCos);
            }
        }
        float diffuse = max(dot(n, toLight), 0.0);
        float specular = 0.0;
        if (diffuse > 0.0)
        {
            specular = pow(max(dot(viewDirection, reflect(-toLight, n)), 0.0), max(shininess, 1.0));
        }
        result += lights[i].color * lights[i].intensity * attenuation * (base.rgb * diffuse + vec3(specular * 0.25));
    }
    fragColor = vec4(result, base.a);
}
