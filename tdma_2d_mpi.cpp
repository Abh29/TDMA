// Author: Wes Kendall
// Copyright 2012 www.mpitutorial.com
// This code is provided freely with the tutorials on mpitutorial.com. Feel
// free to modify it for your own use. Any distribution of the code must
// either provide a link to www.mpitutorial.com or keep this header intact.
//
// Program that computes the average of an array of elements in parallel using
// MPI_Scatter and MPI_Gather
//

#include "iostream"
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "iostream"
#include "vector"
#include "cmath"
#include "iomanip"
#include "cstring"

#include "imgui.h"
#include "include/imgui_impl_sdl2.h"
#include "include/imgui_impl_opengl3.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "include/Matrix.h"

// define the initial width and height of the matrix, this can be changed at runtime
#define NX  200
#define NY	100

// define the intervals for x and y
#define LX0 0
#define LXn 1.0f
#define LY0 0
#define LYn 0.5f

// define the border values
#define F(x) (0)
#define LMD(x,y) ( (x >= 0.25f && x <= 0.65f && y >= 0.1f && y <= 0.25f) ? 1e-3 : 1e-4 )
#define X0(y) (600)
#define XN(y) (1200)
#define Y0(x) (600 * (1 + x))
#define YN(x) (600 * (1 + x * x))

// define the initial values for the matrix
#define FT0(x, y) 	(300)

// define the coordinates x and y from the indexes i and j
#define X(i, dx)	data_t(LX0 + i * dx)
#define Y(j, dy)	data_t(LY0 + j * dy)

// define the increment of time delta_t
#define DT 0.01f;


// define the data type for the matrix
using data_t = double;

// define the interval for the s component in HSV colors
const float max = 1000, min = 0;

// using two matrices as buffers
using Mtrix = matrix_t<data_t>;
Mtrix GM2;
int g_world_size;

// helper functions for visualization
double mapValInterval(float iMin, float iMax, float jMin, float jMax, float val) {
	if (iMax <= iMin || jMax <= jMin)
		return 0;
	return (val - iMin) * (jMax - jMin) / (iMax - iMin) + jMin;
}

ImU32 mapValueToColor(float value) {

	float normalizedValue;
	if (max != min)
		normalizedValue = (value - min) / (max - min);
	else
		normalizedValue = 0.0f;

	float r,g,b,h,s,v;

	h = 0.1;
	s = normalizedValue;
	v = 0.5f;

	ImGui::ColorConvertHSVtoRGB(h,s,v, r,g,b);
	return ImGui::ColorConvertFloat4ToU32({r,g,b,1.0f});
}

// print the values to the terminal for debugging
void printMatrix(Mtrix& M) {

	for (size_t i = 0; i < M.N() + 2; ++i) {
		for (size_t j = 0; j < M.M() + 2; ++j) {
			std::cout << std::fixed << std::setw(8) <<  std::setprecision(3) <<  std::setfill('0') << M[i][j] << " ";
		}
		std::cout << std::endl;
	}

	std::cout << "----------------------------------------------------------------------------------" << std::endl;
	std::cout << std::endl;

}

// create a matrix and fill it with initial and border values
void initMatrix(Mtrix& M, size_t Nx, size_t Ny) {

	M.init(Nx, Ny);

	data_t dx = (LXn - LX0) / (data_t)Nx;
	data_t dy = (LYn - LY0) / (data_t)Ny;
	(void) dy; (void) dx;


	// fill in initial values for x = 0 and x = n
	for (size_t i = 0; i < Ny + 2; ++i) {
		M[0][i] = X0(Y(i, dy));
		M[Nx + 1][i] = XN(Y(i, dy));
	}

	// fill in initial values for y = 0 and y = m
	for (uint i = 1; i < M.N() + 1; ++i) {
		M[i][0] = Y0(X(i, dx));
		M[i][Ny + 1] = YN(X(i, dx));
	}

	// fill in matrix with init values
	for (size_t i = 1; i < Nx + 1; ++i) {
		for (size_t j = 1; j < Ny + 1; ++j) {
			M[i][j] = FT0(X(i, dx), Y(j, dy));
		}
	}

	GM2 = M;
}

// calculate the values of a given row in the matrix
void calculateFixRow(Mtrix& M, int row, Mtrix& M2) {

	data_t dt = DT;

	std::vector<data_t> v_alph(M.M() + 2);
	std::vector<data_t> v_beta(M.M() + 2);

	data_t dx = (LXn - LX0) / M.N();
	data_t dy = (LYn - LY0) / M.M();

	auto lpi2 = [&](int j) {return (data_t)(LMD(X(row + 1, dx), Y(j, dy)) + LMD(X(row, dx), Y(j, dy))) / 2;};
	auto lmi2 = [&](int j) {return (data_t)(LMD(X(row - 1, dx), Y(j, dy)) + LMD(X(row, dx), Y(j, dy))) / 2;};
	auto lpj2 = [&](int j) {return (data_t)(LMD(X(row , dx), Y(j + 1, dy)) + LMD(X(row, dx), Y(j, dy))) / 2;};
	auto lmj2 = [&](int j) {return (data_t)(LMD(X(row, dx), Y(j - 1, dy)) + LMD(X(row, dx), Y(j, dy))) / 2;};

	auto Ai =  [&](int j) {return (data_t)(- lmj2(j) / (2 * dy * dy));};
	auto Bi =  [&](int j) {return (data_t)(- lpj2(j) / (2 * dy * dy));};
	auto Ci =  [&](int j) {return (data_t)((1 / dt - Ai(j) - Bi(j)));};
	auto Di =  [&](int j) {
		double d1 = lpi2(j) * (M[row + 1][j] - M[row][j]);
		double d2 = lmi2(j) * (M[row][j] - M[row][j - 1]);
		double d3 = M[row][j] / dt;
		return (data_t)(d3 + (d1 - d2) / (dx * dx)); };


	v_alph[0] = 0.0f;
	v_beta[0] = M[row][0];

	// forward substitution
	for (size_t i = 1; i < M.M() + 2; ++i) {
		v_alph[i] = - Bi(i) / (Ci(i) + Ai(i) * v_alph[i - 1]);
		v_beta[i] = (- Ai(i) * v_beta[i - 1] + Di(i)) / (Ci(i) + Ai(i) * v_alph[i - 1]);
	}

	// backward substitution
	for (size_t i = M.M(); i > 0; --i) {
		M2[row][i] = v_alph[i] * M[row][i + 1] + v_beta[i];
	}
}

// calculate the values of a given column in the matrix
void calculateFixCol(Mtrix& M, int col, Mtrix& M2) {

	data_t dt = DT;

	std::vector<data_t> v_alph(M.N() + 2);
	std::vector<data_t> v_beta(M.N() + 2);

	data_t dx = (LXn - LX0) / M.N();
	data_t dy = (LYn - LY0) / M.M();

	auto lpi2 = [&](int i) {return (data_t)(LMD(X(i + 1, dx), Y(col, dy)) + LMD(X(i, dx), Y(col, dy))) / 2;};
	auto lmi2 = [&](int i) {return (data_t)(LMD(X(i - 1, dx), Y(col, dy)) + LMD(X(i, dx), Y(col, dy))) / 2;};
	auto lpj2 = [&](int i) {return (data_t)(LMD(X(i, dx), Y(col + 1, dy)) + LMD(X(i, dx), Y(col, dy))) / 2;};
	auto lmj2 = [&](int i) {return (data_t)(LMD(X(i, dx), Y(col - 1, dy)) + LMD(X(i, dx), Y(col, dy))) / 2;};

	auto Ai =  [&](int i) {return (data_t)(-lmi2(i) / (2 * dx * dx));};
	auto Bi =  [&](int i) {return (data_t)(-lpi2(i) / (2 * dx * dx));};
	auto Ci =  [&](int i) {return (data_t)((1 / dt) - Ai(i) - Bi(i));};
	auto Di =  [&](int i) {
		double d1 = lpj2(i) * (M[i][col + 1] - M[i][col]);
		double d2 = lmj2(i) * (M[i][col] - M[i][col - 1]);
		double d3 = M[i][col] / dt;
		return (data_t)(d3 + (d1 - d2) / (dy * dy)); };


	v_alph[0] = 0.0f;
	v_beta[0] = M[1][col];

	// forward substitution
	for (size_t i = 1; i < M.N() + 2; ++i) {
		v_alph[i] = -Bi(i) / (Ci(i) + Ai(i) * v_alph[i - 1]);
		v_beta[i] = (Di(i) - Ai(i) * v_beta[i - 1]) / (Ci(i) + Ai(i) * v_alph[i - 1]);
	}

	// backward substitution
	for (size_t i = M.N(); i > 0; --i) {
		M2[i][col] = v_alph[i] * M[i + 1][col] + v_beta[i];
	}
}

// calculate the values of each row then each column
void calculate(Mtrix& M) {

	GM2 = M;

	for (size_t i = 1; i < M.N() + 1; ++i)
		calculateFixRow(M, i, GM2);

	for (size_t j = 1; j < M.M() + 1; ++j)
		calculateFixCol(GM2, j, M);
}

// initialize imgui with SDL
void initImGui(SDL_Window** window, SDL_GLContext* gl_context) {

	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);


	// From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

	// Create window with graphics context
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	*window = SDL_CreateWindow("Plotter SDL2+OpenGL3", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
	*gl_context = SDL_GL_CreateContext(*window);
	SDL_GL_MakeCurrent(*window, gl_context);
	SDL_GL_SetSwapInterval(1); // Enable vsync

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	ImGui::SetNextWindowPos({0, 0});
	ImGui::SetNextWindowSize(io.DisplaySize);


	ImGuiWindowFlags window_flag = 0;
	window_flag |= ImGuiWindowFlags_NoTitleBar;
	window_flag |= ImGuiWindowFlags_NoMove;
	window_flag |= ImGuiWindowFlags_NoResize;
	window_flag |= ImGuiWindowFlags_NoCollapse;
	window_flag |= ImGuiWindowFlags_NoNav;

	(void) window_flag;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(*window, gl_context);
	ImGui_ImplOpenGL3_Init(glsl_version);
}

// clean up
void freeImGui(SDL_Window** window, SDL_GLContext* gl_context) {

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(*gl_context);
	SDL_DestroyWindow(*window);
	SDL_Quit();
}

// divide the workload on the nodes of the comm and read results from node 0
void scatter(Mtrix& M) {
	int	begin;
	int cols = NY + 2;
	int rows = NX / (g_world_size - 1) + 2;

	// sending workloads
	begin = 0;
	for (int i = 1; i < g_world_size; ++i) {
		MPI_Send(M[begin], cols * rows, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
		begin += rows - 2;
	}
	MPI_Barrier(MPI_COMM_WORLD);

	// receiving results
	begin = 0;
	for (int i = 1; i < g_world_size; ++i) {
		MPI_Recv(M[begin + 1], (rows - 2) * cols, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		begin += rows - 2;
	}
	MPI_Barrier(MPI_COMM_WORLD);

	// do some preprocessing to the result before rendering with the results
	// ...
}

// receive a workload do calculation and send the results back to node 0
void receive(Mtrix &subM, int world_rank) {
	int cols = NY + 2;
	int rows = NX / (g_world_size - 1) + 2;

	(void) world_rank;
	while (true) {
		// receiving the workload
		MPI_Recv(subM.data(), rows * cols, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Barrier(MPI_COMM_WORLD);

		// do calculations
		calculate(subM);

		// send back the results
		MPI_Send(subM[1], cols * (rows - 2), MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		MPI_Barrier(MPI_COMM_WORLD);
	}
}


// draw the matrix to the window surface
int plot(Mtrix& M, int* Nx, int *Ny) {
	SDL_Window* window;
	SDL_GLContext gl_context;
	ImGuiWindowFlags window_flag;
	ImVec4 clear_color;
	static int ti;
	(void) Nx; (void) Ny;

	float tj = 0.0f;

	// Setup SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
		return -1;

	initImGui(&window, &gl_context);

	window_flag = 0;
	window_flag |= ImGuiWindowFlags_NoTitleBar;
	window_flag |= ImGuiWindowFlags_NoMove;
	window_flag |= ImGuiWindowFlags_NoResize;
	window_flag |= ImGuiWindowFlags_NoCollapse;
	window_flag |= ImGuiWindowFlags_NoNav;


	ImGuiIO& io = ImGui::GetIO();
	clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// Main loop
	bool done = false;

	ti = tj = 0;

	while (!done)
	{
		// poll sdl events
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_QUIT)
				done = true;
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
				done = true;
		}

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(io.DisplaySize);

		// start imgui frame and add some settings to the window
		ImGui::Begin("Plotter", NULL, window_flag);
		ImGui::SliderInt("T", &ti, 0, 0);


		// calculate a new iteration after dt
		tj += DT;
		ti = int(std::floor(tj));

		// divide the workload
		scatter(M);

		// draw the matrix to the surface
		ImDrawList* dl = ImGui::GetWindowDrawList();
		float xi, yi;
		for (size_t i = 0; i < M.N() + 2; ++i) {
			xi = 20.0f + (io.DisplaySize.x - 20) / float(M.N() + 2) * i;
			for (size_t j = 0; j < M.M() + 2; ++j) {
				yi = 90.0f + (io.DisplaySize.y - 90) / float(M.M() + 2) * j;
				dl->AddRectFilled({xi,yi}, {xi + (io.DisplaySize.x - 20) / float(M.N() + 2), yi + (io.DisplaySize.y - 90) / float(M.M() + 2)}, mapValueToColor(M[i][j]));
			}
		}

		// end the recording of the frame
		ImGui::PushItemWidth(-1);
		ImGui::End();

		// Rendering
		ImGui::Render();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(window);
	}

	// Cleanup
	freeImGui(&window, &gl_context);
	return 0;
}


// Main program
int main() {
	// init mpi
	MPI_Init(NULL, NULL);

	// get mpi info
	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &g_world_size);


	// We are assuming at least 2 processes for this task
	if (g_world_size < 2) {
		std::cerr << "World size must be greater than 1" << std::endl;
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	if (world_rank == 0) {
		int Nx = NX, Ny = NY;
		Mtrix M;
		initMatrix(M, Nx, Ny);
		plot(M, &Nx, &Ny);
	} else {
		Mtrix subM;
		subM.init(NX /( g_world_size - 1), NY);
		receive(subM, world_rank);
	}

	MPI_Abort(MPI_COMM_WORLD, 0);
	MPI_Finalize();
}

