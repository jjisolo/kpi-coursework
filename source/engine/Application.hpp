// This file declares the `Application` class.
// 
// The `Application` class is the core class of the engine API, and the only
// data structure that can be seen and used by the endpoint graphics API user.
#pragma once 

#include "_EngineIncludes.hpp"

#include "Window.hpp"
#include "ResourseManager.hpp"

#include "rendering/SpriteRenderer.hpp"
 
// This namespace holds the data, that should be accessed by the endpoint API user.
namespace One
{
	// This class is the core data structure of the graphics API.
	// 
	// It is responsible actions like: initializing internal API's, such as GLFW, GLAD,
	// GLM, SPDLOG; handle the user keyboard input, perform the main game endless loop.  
	//
	// This is the main base class for the application.
	class Application
	{
	protected:
		// Lifetime object cycle management.
		Application()  {};
		virtual ~Application() {};

	public:
		// As a part of the singleton pattern.
		Application(Application const&)            = delete;
		Application& operator=(Application const&) = delete;

		// This function is the way to realize the Singleton OOP programming pattern,
		// so that this class can only be instantiated only once.
		static Application& instance()
		{
			static Application _instance;
			return(_instance);
		}

		// The formal(by the parameters) callback that is used for the GLFW window callbacks.
		// This function(static formal function, that is calling an internal function) is kinda
		// workaround about the C background of the GLFW library.
	    static void setCursorPosCallback(GLFWwindow* window, double positionX, double positionY)
	    {
		  UnreferencedParameter(window);

		  // Call the internal callback function(that has access to the class members).
		  instance().onMouseMove(positionX, positionY);
	    }

	public:
		// Clear the screen with solid color.
		inline void ClearScreen(GLfloat r, GLfloat g, GLfloat b)
		{
			glClearColor(r, g, b, 1.0f);
			glClear     (GL_COLOR_BUFFER_BIT);
		}

		// Create new frame in the ImGUI library, with the OpenGL and GLFW bindings.
		inline void ImguiCreateNewFrameKHR()
		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
		}

		// Get the GLFW's window pointer
		inline GLFWwindow* getWindowPointer()
		{
			return(Engine::Window::instance().getWindowPointerKHR());
		}

		// Get the logic window size.
		inline glm::ivec2 getWindowDimensions()
		{
			return(Engine::Window::instance().getWindowDimensionsKHR());
		}

		// Here starts the interfaces, that endpoint user must override.
		// It is the bridge between the engine itself and the user.
		//
		// Called once upon application startup. Used to load resources etc.
		virtual Engine::Error onUserInitialize();

		// Called once upon application termination. Used to unload recourses.
		virtual Engine::Error onUserRelease();

		// Called upon every frame, and provides the a time per frame
		// value(typically used for the animations physics etc.).
		virtual Engine::Error onUserUpdate(GLfloat elapsedTime);
	  
	    // Called upon every time whenever the user moves the mouse. 
	    Engine::Error onMouseMove(double positionX, double positionY);
	  
	    // Execute the engine main loop, in which the user code combined with
		// code executing the actual game. 
		Engine::Error execute();

		// Initialize the engine and its subsystems.
		Engine::Error initializeGameEngine(void) noexcept;

		// Safely deallocate memory that was used during the game loop, quit the subsystems.
		Engine::Error destroyGameEngine(void) noexcept;

		// Engage the game main loop, in which happens rendering, updating, input handling.
		Engine::Error updateGameEngine(void) noexcept;

	private:
		// Stylish the Dear ImGUI
		void applyImGuiStyles();

		// Gather the input from the keyboard.
		void processInput(GLFWwindow* windowPointer) noexcept;

	protected:
		GLfloat m_monitorHighDPIScaleFactor;

		bool   m_mouseButtonPressed  = false;
		bool   m_escapeButtonPressed = false;

        double m_mousePositionX;
        double m_mousePositionY;

        double m_elapsedTime;

		std::shared_ptr<Engine::GFX::SpriteRenderer> m_SpriteRenderer;
	};
}
