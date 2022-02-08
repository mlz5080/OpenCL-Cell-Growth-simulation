#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable
#define HEIGHT 768
#define WIDTH 1024


size_t getindex(int x, int y) {
	return y * get_global_size(0) + x;
}


bool withinbound(int i, int j) {
	return !(i < 0 || j < 0 || i >= WIDTH || j >= HEIGHT);
}

__kernel void issurrounded(__global char* data, __global char* outData) {
	//get_global_id is the index
	int x = get_global_id(0);
	int y = get_global_id(1);
	int different_cell = 0;

	//outData[getindex(x, y)] = 1;
	
	for (int i = x - 1; i < x + 2; i++) {
		for (int j = y - 1; j < y + 2; j++) {
			if (withinbound(i, j) && (i != x && j != y)) {
				different_cell += data[getindex(i, j)] % data[getindex(x, y)] % 2;
			}
		}
	}

	if (different_cell >= 6) {
		if (data[getindex(x, y)] == 3) {
			for (int i = x - 1; i < x + 2; i++) {
				for (int j = y - 1; j < y + 2; j++) {
					if (withinbound(i, j)) {
						outData[getindex(i, j)] = 2;
					}
				}
			}
		}
		else {
			outData[getindex(x, y)] = 3;
		}
	}
	else
		outData[getindex(x, y)] = data[getindex(x, y)];
	
}

__kernel void update_position(__global char* data, __global char* outData, __global char* surrounded) {
	//get_global_id is the index
	int x = get_global_id(0);
	int y = get_global_id(1);

	if (surrounded[getindex(x, y)]>=5) {
		if (data[getindex(x, y)] == 3) {
			for (int i = x - 1; i < x + 2; i++) {
				for (int j = y - 1; j < y + 2; j++) {
					if (withinbound(i, j)) {
						outData[getindex(i, j)] = 2;
						surrounded[getindex(i, j)] = -1;
					}
				}
			}
		}
		else {
			outData[getindex(x, y)] = 3;
		}
	}
	else if(surrounded[getindex(x, y)] > -1){
		outData[getindex(x, y)] = data[getindex(x, y)];
	}

}

__kernel void moving(__global char* data, __global char* outData, __global char* direction) {
	//get_global_id is the index
	int x = get_global_id(0);
	int y = get_global_id(1);
	int tmp;
	if (direction[(getindex(x,y))]==1 && withinbound(x -1, y+1)) {
		//swap(i, j, i + 1, j);
		tmp = direction[getindex(x, y)];
		outData[getindex(x - 1, y + 1)] = data[getindex(x, y)];
		outData[getindex(x, y)] = data[getindex(x - 1, y + 1)];
	}
	else if ((direction[(getindex(x, y))] == 2) && withinbound(x, y + 1)) {
		tmp = direction[getindex(x, y)];
		outData[getindex(x, y + 1)] = data[getindex(x, y)];
		outData[getindex(x, y)] = data[getindex(x, y + 1)];
	}
	else if ((direction[(getindex(x, y))] == 3) && withinbound(x+1, y+1)) {
		tmp = direction[getindex(x, y)];
		outData[getindex(x + 1, y + 1)] = data[getindex(x, y)];
		outData[getindex(x, y)] = data[getindex(x + 1, y + 1)];
	}
	else if ((direction[(getindex(x,y))] == 4) && withinbound(x - 1, y)) {
		tmp = direction[getindex(x, y)];
		outData[getindex(x - 1, y)] = data[getindex(x, y)];
		outData[getindex(x, y)] = data[getindex(x - 1, y)];
	}
	else if ((direction[(getindex(x, y))] == 5) && withinbound(x + 1, y)) {
		tmp = direction[getindex(x, y)];
		outData[getindex(x + 1, y)] = data[getindex(x, y)];
		outData[getindex(x, y)] = data[getindex(x + 1, y)];
	}
	else if ((direction[(getindex(x, y))] == 6) && withinbound(x - 1, y-1)) {
		tmp = direction[getindex(x, y)];
		outData[getindex(x - 1, y - 1)] = data[getindex(x, y)];
		outData[getindex(x, y)] = data[getindex(x - 1, y - 1)];
	}
	else if ((direction[(getindex(x, y))] == 7) && withinbound(x, y-1)) {
		tmp = direction[getindex(x, y)];
		outData[getindex(x, y - 1)] = data[getindex(x, y)];
		outData[getindex(x, y)] = data[getindex(x, y - 1)];
	}
	else if ((direction[(getindex(x, y))] == 8) && withinbound(x+1, y-1)) {
		tmp = direction[getindex(x, y)];
		outData[getindex(x + 1, y - 1)] = data[getindex(x, y)];
		outData[getindex(x, y)] = data[getindex(x + 1, y - 1)];
	}
	else {
		outData[getindex(x, y)]= data[getindex(x, y)];
	}

	
	

}