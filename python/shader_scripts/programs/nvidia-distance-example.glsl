
v2fConnector distanceVertex(a2vConnector a2v,    uniform float4x4 modelViewProj,    uniform float3 eyeCoord,    uniform float3 lightCoord,    uniform float invBumpDepth)  {
          v2fConnector v2f;      // Project position into screen space      // and pass through texture coordinate    
          v2f.projCoord = mul(modelViewProj, float4 (a2v.objCoord, 1));    
          v2f.texCoord = float3 (a2v.texCoord, 1);      // Transform the eye vector into tangent space.    // Adjust the slope in tangent space based on bump depth      
          float3 eyeVec = eyeCoord - a2v.objCoord;    
          float3 tanEyeVec;    
          tanEyeVec.x = dot(a2v.objTangent, eyeVec);    
          tanEyeVec.y = dot(a2v.objBinormal, eyeVec);    
          tanEyeVec.z = -invBumpDepth * dot(a2v.objNormal, eyeVec);    
          v2f.tanEyeVec = tanEyeVec;      // Transform the light vector into tangent space.    // We will use this later for tangent-space normal mapping      
          float3 lightVec = lightCoord - a2v.objCoord;    
          float3 tanLightVec;    
          tanLightVec.x = dot(a2v.objTangent, lightVec);    
          tanLightVec.y = dot(a2v.objBinormal, lightVec);    
          tanLightVec.z = dot(a2v.objNormal, lightVec);    
          v2f.tanLightVec = tanLightVec;      
          return v2f;    } 

 f2fConnector distanceFragment(v2fConnector v2f,    uniform sampler2D colorTex,    uniform sampler2D normalTex,    uniform sampler3D distanceTex,    uniform float3 normalizationFactor)  {
        f2fConnector f2f;      
        // Normalize the offset vector in texture space.     
         // The normalization factor ensures we are normalized with respect    
         // to a distance which is defined in terms of pixels. 
        float3 offset = normalize(v2f.tanEyeVec);    
        offset *= normalizationFactor;      
        float3 texCoord = v2f.texCoord;      // March a ray      
        for (int i = 0; i < NUM_ITERATIONS; i++) {      
            float distance = f1tex3D(distanceTex, texCoord);      
            texCoord += distance * offset;    
            }      // Compute derivatives of unperturbed texcoords.    // This is because the offset texcoords will have discontinuities      // which lead to incorrect filtering.    
            float2 dx = ddx(v2f.texCoord.xy);    
            float2 dy = ddy(v2f.texCoord.xy);      // Do bump-mapped lighting in tangent space.    // 'normalTex' stores tangent-space normals remapped      // into the range [0, 1].    
            float3 tanNormal = 2 * f3tex2D(normalTex, texCoord.xy, dx, dy) - 1;    
            float3 tanLightVec = normalize(v2f.tanLightVec);    
            float diffuse = dot(tanNormal, tanLightVec);      // Multiply diffuse lighting by texture color      
            f2f.COL.rgb = diffuse * f3tex2D(colorTex, texCoord.xy, dx, dy);    
            f2f.COL.a = 1;      
            return f2f;    } 