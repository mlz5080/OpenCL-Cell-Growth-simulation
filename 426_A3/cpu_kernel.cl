// TODO: Add OpenCL kernel code here.
#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable
#define HEIGHT 769
#define WIDTH 1024

__kernel void update_position(__global char* data, __global char* outData, __global char* surrounded) {
	//get_global_id is the index
	int x = get_global_id(0);
	int y = get_global_id(1);

	if (surrounded[getindex(x, y)]) {
		if (data[getindex(x, y)] % 2) {
			for (int i = x - 1; i < x + 2; i++) {
				for (int j = y - 1; j < y + 2; j++) {
					if (withinbound(x, y)) {
						outData[getindex(x, y)] = 2;
					}
				}
			}
		}
		else {
			outData[getindex(x, y)] = 3;
		}
	}
	else {
		outData[getindex(x, y)] = data[getindex(x, y)];
	}

}