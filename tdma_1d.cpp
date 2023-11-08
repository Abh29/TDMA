#include "iostream"
#include "vector"
#include "cmath"
#include "imgui.h"
#include "include/imgui_impl_sdl2.h"
#include "include/imgui_impl_opengl3.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>


#define MAXDIFF 0
#define DEBUG	0
#define PLOT	1

/*
 * Thomas Algorithm:
 *
 * d²f/dx² = F(x)
 * yi = alph * yi+1 + beta			and 		Ci * yi = Ai * yi+1 + Bi * yi-1 + Di
 * => d²f/dx² = d/dx (yi+1 / h) - d/dx (yi / h)
 * = 1/h² (yi+1  - 2yi + yi-1) = F(x)
 *
 * => Ai = 1  && 	Bi = 1   &&		Ci = 2		&&		Di = - h² * F(x)
 *		alph(i) = Ai / (Ci - Bi * alph(i - 1))
 *		beta(i) = (Bi + Di) / (Ci - Bi * alph(i - 1))
 *
 *		for i = 0 	B0 = 0 		&&		for i = N	AN = 0
 */

#define L0	0
#define LN	(M_PI/2 +  4 * M_PI)
#define F(x) (std::sin(x))
#define FN	-1

#define YI(x) (-std::sin(x))


void calculate(std::vector<double>& v_yi, int N) {

	double h;

	v_yi.resize(N + 1, 0.0f);

	h = (double)(LN - L0) / N;
	std::vector<double> v_alph(N + 1);
	std::vector<double> v_beta(N + 1);


	v_alph[0] = 0.0f;
	v_beta[0] = - h * h * F(L0);

	// forward substitution
	for (int i = 1; i < N; ++i) {
		v_alph[i] = 1.0f / (2 - v_alph[i - 1]);
		v_beta[i] = (v_beta[i - 1] - h * h * F(L0 + i * h)) / (2 - v_alph[i - 1]);
	}

	v_yi[N - 1] = FN;
	// backward substitution
	for (int i = N - 2; i >= 0; --i) {
		v_yi[i] = v_alph[i] * v_yi[i + 1] + v_beta[i];
	}

#if MAXDIFF
	double maxdif = -1.0;
	for (int i = 0; i < N; ++i) {
		if (std::fabs(v_yi[i] - YI(L0 + i * h)) > maxdif)
			maxdif = std::fabs(v_yi[i] + F(L0 + i * h));
#if DEBUG
		std::cout << "x= " << L0 + i * h << ",  yi = " << v_yi[i] << ",  -sin(x) = " <<
		YI(L0 + i * h) << ",  diff= " << v_yi[i] - YI(L0 + i * h) << std::endl;
#endif
	}
	std::cout << "maxDifference = " << maxdif << std::endl;
#endif
}

struct Funcs
{
	static float Get(void* data, int i) {
		double*	d = (double*) data;
		return d[i];
	}
};

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

void freeImGui(SDL_Window** window, SDL_GLContext* gl_context) {

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(*gl_context);
	SDL_DestroyWindow(*window);
	SDL_Quit();
}

int plot(std::vector<double> v_yi, int* N) {
	(void)v_yi; (void) N;
	SDL_Window* window;
	SDL_GLContext gl_context;
	ImGuiWindowFlags window_flag;
	ImVec4 clear_color;
	static int display_count = *N;
	double h;


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

	while (!done)
	{
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

		// Main body of the Demo window starts here.
		ImGui::Begin("Plotter", NULL, window_flag);
		ImGui::SliderInt("Sample count", &display_count, 10, 1000);

		if (display_count != *N) {
			*N = display_count;
			h = (double)(LN - L0) / *N;
			v_yi.clear();
			calculate(v_yi, *N);
		}

//		float (*func)(void*, int) = Funcs::Sin;
		ImGui::PlotHistogram("", Funcs::Get, v_yi.begin().base(), *N, h, NULL, -1.5f, 1.5f, ImVec2(io.DisplaySize.x, 600));
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
int main(int argc, char** argv) {

	std::vector<double> v_yi;
	int N = 100;

	if (argc > 1) {
		N = std::strtol(argv[1], nullptr, 10);
	}
	N = N <= 0 ? 100 : N;



	calculate(v_yi, N);
#if PLOT
	plot(v_yi, &N);
#endif
}
