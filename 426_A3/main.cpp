#define __CL_ENABLE_EXCEPTIONS

#include <CL/opencl.h>
#include <windows.h>
#include <thread>
#include <iostream>
#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"
#include <random>
#include <fstream>
#include "cell.h"
#include <array>
#include <math.h>

constexpr auto PI = 3.1415926;;
GLfloat r = 0.0f, g = 0.0f, b = 0.0f;

const int WIDTH = 1024, HEIGHT = 768;
//std::array<std::array<int,HEIGHT>, WIDTH> input;
char input[WIDTH][HEIGHT];
char surrounded[WIDTH][HEIGHT];
char output[WIDTH][HEIGHT];
char direction[WIDTH][HEIGHT];
//std::array<std::array<int, HEIGHT>, WIDTH> output;

cl_context context_nvidia;
cl_context context_intel_gpu;
cl_context context_intel_cpu;

cl_program program_nvidia;
cl_program program_intel_gpu;
cl_program program_intel_cpu;

cl_command_queue nvidia_queue;
cl_command_queue intel_gpu_queue;
cl_command_queue intel_cpu_queue;

cl_kernel kernel_surronded;
cl_kernel kernel_update_position;
cl_kernel kernel_moving;

cl_mem outbuff_nvidia;
cl_mem inbuff_nvidia;

cl_mem outbuff_intel_gpu;
cl_mem surrounded_intel_gpu;
cl_mem inbuff_intel_gpu;

cl_mem outbuff_intel_cpu;
cl_mem direction_intel_cpu;
cl_mem inbuff_intel_cpu;

cl_event nvidia_kernel;
cl_event finish_writing;
static bool withinbound(int x, int y);


/*
* The direction is defined by the angle of med cell - injection point vector.
* There are 8 direction going outwards and depends on the angle we chose the direction.
*/
int setdirection(float injectx, float injecty, float x, float y) {
	std::random_device rd;
	std::uniform_int_distribution<int> dist(1, 4);
	return dist(rd);
}


int getindex(int x, int y) {
	return y * WIDTH + x;
}

void init() {
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(-0.5f, WIDTH - 0.5f, -0.5f, HEIGHT - 0.5f);
}

//initilization of the program
void setup() {
	std::random_device rd;
	std::uniform_int_distribution<int> dist(1, 6);
	int tmp;

	//assign random cell type. 2/5 chance being a cancer cell, 2/5 being healthy and 1/5 with no cell.
	for (int i = 0; i < WIDTH; i++) {
		for (int j = 0; j < HEIGHT; j++) {
			//Initialize each pixel with an arbitry alive/dead value.
			//std::cout << tmp << std::endl

			tmp = dist(rd);
			if (tmp==3 || tmp == 4 || tmp == 5) {
				input[i][j] = 2;
				output[i][j] = 2;
			}
			else {
				input[i][j] = 3;
				output[i][j] = 3;
			}
			direction[i][j] = 0;
		}
	}

}

void changeColor(GLfloat red, GLfloat green, GLfloat blue) {
	r = red;
	g = green;
	b = blue;
}


//Display individual pixels. This function will be called every possible time.
static void display()
{
	glClear(GL_COLOR_BUFFER_BIT);
	GLfloat red, green, blue;
	clWaitForEvents(1, &finish_writing);
	clReleaseMemObject(outbuff_intel_cpu);
	for (int i = 0; i < (WIDTH); i++) {
		for (int j = 0; j < HEIGHT; j++) {
			input[i][j] = output[i][j];
			//Check the updated status of the current cell.
			//If alive, fill in pixel with color. If dead, color value is 0.
			glPointSize(2.0f);
			if(output[i][j]==0)
				glColor3f(0.0f,0.0f,0.0f);
			else if (output[i][j] == 2)
				glColor3f(0.0f, 1.0f, 0.0f);
			else if (output[i][j] == 3)
				glColor3f(1.0f, 0.0f, 0.0f);
			else if (output[i][j] == 4)
				glColor3f(1.0f, 1.0f, 0.0f);
			
			glBegin(GL_POINTS);
			glVertex2i(i, HEIGHT-j-1);
			glEnd();
		}
	}
	glutSwapBuffers();
}

//Update function will update each cell's information
void update(int value) {
	
	cl_int err;
	int medcell = 0;
	bool flag = true;
	std::random_device rd;
	std::uniform_int_distribution<int> dist(1, 8);

	//Use nvidia GPU to find out the surrounding information
	inbuff_nvidia = clCreateBuffer(context_nvidia, CL_MEM_READ_WRITE  | CL_MEM_COPY_HOST_PTR, WIDTH * HEIGHT * sizeof(char), &input, &err);
	outbuff_nvidia = clCreateBuffer(context_nvidia, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY , WIDTH * HEIGHT * sizeof(char), NULL, &err);
	
	err = clSetKernelArg(kernel_surronded, 0, sizeof(inbuff_nvidia), &inbuff_nvidia);
	err = clSetKernelArg(kernel_surronded, 1, sizeof(outbuff_nvidia), &outbuff_nvidia);
	size_t global_size[] = { WIDTH,HEIGHT };
	err = clEnqueueNDRangeKernel(nvidia_queue,kernel_surronded, 2,NULL, global_size,NULL,0,NULL,NULL);
	clFinish(nvidia_queue);
	err = clEnqueueReadBuffer(nvidia_queue, outbuff_nvidia, CL_TRUE, 0, sizeof(char) * WIDTH * HEIGHT, &output , 0, NULL, NULL);
	//err = clEnqueueReadBuffer(nvidia_queue, outbuff_nvidia, CL_FALSE, 0, sizeof(char) * WIDTH * HEIGHT, &input, 0, NULL, NULL);
	clFinish(nvidia_queue);
	
	clReleaseMemObject(inbuff_nvidia);
	clReleaseMemObject(outbuff_nvidia);

	/*
	//================================================================
	//Use intel GPU to change the cell type, and update cell positions
	inbuff_intel_gpu = clCreateBuffer(context_intel_gpu, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, WIDTH * HEIGHT * sizeof(char), &input, &err);
	surrounded_intel_gpu = clCreateBuffer(context_intel_gpu, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, WIDTH * HEIGHT * sizeof(char), &surrounded, &err);
	outbuff_intel_gpu = clCreateBuffer(context_intel_gpu, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, WIDTH * HEIGHT * sizeof(char),NULL, &err);


	err = clSetKernelArg(kernel_update_position, 0, sizeof(inbuff_intel_gpu), &inbuff_intel_gpu);
	err = clSetKernelArg(kernel_update_position, 1, sizeof(outbuff_intel_gpu), &outbuff_intel_gpu);
	err = clSetKernelArg(kernel_update_position, 2, sizeof(surrounded_intel_gpu), &surrounded_intel_gpu);

	err = clEnqueueNDRangeKernel(intel_gpu_queue, kernel_update_position, 2, NULL, global_size, NULL, 0, NULL, NULL);
	clFinish(intel_gpu_queue);
	err = clEnqueueReadBuffer(intel_gpu_queue, outbuff_intel_gpu, CL_TRUE, 0, sizeof(char) * WIDTH * HEIGHT, &input, 0, NULL, NULL);
	err = clEnqueueReadBuffer(intel_gpu_queue, outbuff_intel_gpu, CL_TRUE, 0, sizeof(char) * WIDTH * HEIGHT, &output, 0, NULL, NULL);
	clFinish(intel_gpu_queue);

	clReleaseMemObject(inbuff_intel_gpu);
	clReleaseMemObject(surrounded_intel_gpu);
	clReleaseMemObject(outbuff_intel_gpu);
	*/
	

	//================================================================
	//Use intel CPU to move cells

	for (int i = 0; i < WIDTH; i++) {
		for (int j = 0; j < HEIGHT; j++) {
			input[i][j]=output[i][j];
			if (output[i][j] == 4)
				direction[i][j] = dist(rd);
			else
				direction[i][j] = -1;
		}
	}
	
	
	inbuff_intel_cpu = clCreateBuffer(context_intel_cpu, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, WIDTH * HEIGHT * sizeof(char), &input, &err);
	outbuff_intel_cpu = clCreateBuffer(context_intel_cpu, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_COPY_HOST_PTR, WIDTH * HEIGHT * sizeof(char), &output, &err);
	direction_intel_cpu = clCreateBuffer(context_intel_cpu, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, WIDTH * HEIGHT * sizeof(char), &direction, &err);

	err = clSetKernelArg(kernel_moving, 0, sizeof(inbuff_intel_cpu), &inbuff_intel_cpu);
	err = clSetKernelArg(kernel_moving, 1, sizeof(outbuff_intel_cpu), &outbuff_intel_cpu);
	err = clSetKernelArg(kernel_moving, 2, sizeof(direction_intel_cpu), &direction_intel_cpu);

	err = clEnqueueNDRangeKernel(intel_cpu_queue, kernel_moving, 2, NULL, global_size, NULL, 0, NULL, NULL);
	clFinish(intel_cpu_queue);
	//err = clEnqueueReadBuffer(intel_cpu_queue, outbuff_intel_cpu, CL_FALSE, 0, sizeof(char) * WIDTH * HEIGHT, &input, 0, NULL, NULL);
	err = clEnqueueReadBuffer(intel_cpu_queue, outbuff_intel_cpu, CL_FALSE, 0, sizeof(char) * WIDTH * HEIGHT, &output, 0, NULL, &finish_writing);
	//clFinish(intel_gpu_queue);

	clReleaseMemObject(inbuff_intel_cpu);
	clReleaseMemObject(direction_intel_cpu);
	//clReleaseMemObject(outbuff_intel_cpu);
	
	glutPostRedisplay();
	glutTimerFunc(33, update, 0); 
}

bool withinbound(int x, int y) {
	return (x >= 0 && x < WIDTH) && (y >= 0 && y < HEIGHT);
}

//Injection function simulate a injection which will generate a square of med cells.
void injection(int button, int state, int x, int y)
{
	//std::cout << x << " " << y << std::endl;
	//y = HEIGHT - y;
	std::random_device rd;
	std::uniform_int_distribution<int> dist(35, 40);
	int mul = dist(rd);
	if (button == GLUT_LEFT_BUTTON) {
		
		for (int i = x - (mul); i <= x + (mul); i++) {
			for (int j = y - (mul); j <= y + (mul); j++) {
				//Make sure that position x and y are not out of bounds
				if (withinbound(i, j)) {
					input[i][j]=4;
					output[i][j] = 4;
					//setting direction of the new med cell going outward of initial injection
					if (x != i && y != j){
						direction[i][j] = setdirection(x, y, i, j);
					}
						
				}
			}
		}
		
		glutPostRedisplay();
	}
}

static cl_platform_id platform = NULL;
static cl_device_id device = NULL;
static cl_context context = NULL;
static cl_command_queue queue = NULL;
static cl_kernel kernel = NULL;

std::vector<cl_platform_id> platforms(3);
std::vector<cl_device_id> devices(0);
int find_devices() {
	
	cl_uint num_platforms = 0;
	cl_int err = clGetPlatformIDs(0, NULL, &num_platforms);
	char vendor_name[128] = { 0 };
	platforms.resize(num_platforms);
	err = clGetPlatformIDs(num_platforms, &platforms[0], NULL);
	cl_uint numDevices = 0;
	std::vector<cl_device_id> available_devices(0);
	size_t charsize = 128;
	for (cl_uint ui = 0; ui < num_platforms; ++ui)
	{
		available_devices.clear();
		err = clGetPlatformInfo(platforms[ui],CL_PLATFORM_NAME,128 * sizeof(char),vendor_name,NULL);
		err = clGetDeviceIDs(platforms[ui], CL_DEVICE_TYPE_ALL, 0, NULL, &numDevices);
		std::cout << vendor_name <<" has " << numDevices << " devices." << std::endl;
		available_devices.resize(numDevices);
		err = clGetDeviceIDs(platforms[ui], CL_DEVICE_TYPE_ALL, numDevices, &available_devices[0], &numDevices);

		for (cl_device_id device : available_devices){
			devices.push_back(device);
		}
	}

	
	char extension[1280] = { 0 };
	for (cl_device_id device : devices) {
		err = clGetDeviceInfo(device, CL_DEVICE_NAME, 128 * sizeof(char), vendor_name, NULL);
		std::cout << "device: "<< vendor_name << std::endl;
		//err = clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, 1280 * sizeof(char), extension, NULL);
		//std::cout << "device: " << extension << std::endl;
	}
	
	return 0;
}

cl_program CreateProgram(std::string file, const cl_device_id* device, cl_context &mycontext) {
	cl_int err = 0;
	cl_program program = NULL;
	mycontext = clCreateContext(NULL,1, device,NULL,NULL,&err);
	std::ifstream kernel_file(file);
	std::string sourceCode(std::istreambuf_iterator<char>(kernel_file), (std::istreambuf_iterator<char>()));
	const char* source = sourceCode.c_str();
	size_t sourceSize[] = { strlen(source) };
	program = clCreateProgramWithSource(mycontext,1, &source, sourceSize,&err);
	err = clBuildProgram(program, 1, device, "-cl-std=CL1.2", NULL, NULL);
	if (err == CL_BUILD_PROGRAM_FAILURE) {
		// Determine the size of the log
		size_t log_size;
		clGetProgramBuildInfo(program, device[0], CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

		// Allocate memory for the log
		char* log = (char*)malloc(log_size);

		// Get the log
		clGetProgramBuildInfo(program, device[0], CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

		// Print the log
		printf("%s\n", log);
	}
	return program;
}

int main(int argc, char** argv)
{
	find_devices();
	program_nvidia = CreateProgram("kernel.cl", &devices[0],context_nvidia);
	program_intel_gpu = CreateProgram("kernel.cl", &devices[1],context_intel_gpu);
	program_intel_cpu = CreateProgram("kernel.cl", &devices[2], context_intel_cpu);
	cl_int err = 0;

	// Create the memory buffers
	nvidia_queue = clCreateCommandQueueWithProperties(context_nvidia, devices[0],NULL,&err);
	intel_gpu_queue = clCreateCommandQueueWithProperties(context_intel_gpu, devices[1], NULL, &err);
	intel_cpu_queue = clCreateCommandQueueWithProperties(context_intel_cpu, devices[2], NULL, &err);

	
	//outbuff_intel_cpu = clCreateBuffer(context_intel_cpu, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, WIDTH * HEIGHT * sizeof(char), &surrounded, &err);
	// Build the program for the devices
	// Make program from the source code
	kernel_surronded = clCreateKernel(program_nvidia, "issurrounded",&err);
	kernel_update_position = clCreateKernel(program_intel_gpu, "update_position", &err);
	kernel_moving = clCreateKernel(program_intel_cpu, "moving", &err);

	//intel_gpu_queue = cl::CommandQueue(context_intel_gpu, devices[1]);
	//intel_cpu_queue = cl::CommandQueue(context_intel_cpu, devices[2]);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Demo OPEN GL");
	init();
	setup();
	glutDisplayFunc(display);
	//Use glut time function to call update function every 1/30 seconds.
	glutTimerFunc(33, update, 0);
	changeColor(0.0f, 1.0f, 0.0f);
	//mouse click function
	glutMouseFunc(injection);
	glutMainLoop();
	return 0;
}
