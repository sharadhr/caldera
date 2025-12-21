RWTexture2D<float4> image : register(u0);

[numthreads(16, 16, 1), shader("compute")]
void computeMain(uint3 dispatch_thread_id: SV_DispatchThreadID, uint3 group_thread_id: SV_GroupThreadID)
{
	var texel_coord = int2(dispatch_thread_id.xy);
    var image_size = int2(0, 0);
    image.GetDimensions(image_size.x, image_size.y);

	if (texel_coord.x < image_size.x && texel_coord.y < image_size.y) {
		var color = float4(0.0, 0.0, 0.0, 1.0);

		if (group_thread_id.x != 0 && group_thread_id.y != 0) {
			color.x = float(texel_coord.x) / float(image_size.x);
			color.y = float(texel_coord.y) / float(image_size.y);
		}

		image[texel_coord] = color;
	}
}
