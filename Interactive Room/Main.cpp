#include "glew.h"
#include "glfw3.h"
#include "glm.hpp"

#include "shader.h"
#include "camera.h"
#include "model.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void selectObject(double x, double y);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
int width, height;
const float scaling = 0.02f;

// camera
Camera camera(scaling * glm::vec3(2280, 260, -121.5f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

// view/projection transformations
glm::mat4 projection;
glm::mat4 view;

// timing
float deltaTime = 0.0f;

//pointer to selected object
Model *selected;

//for some reason C++ wants other classes' static datatypes to be declared globally if we're going to use them.
// who the f made that design decision???
vector<Model*> Model::models;

//shader pointers to switch between shaders in functions
Shader* general;
Shader* selection;
//boolean determining whether an object is selected or not
bool isSelected;

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 8);
	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "House", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	if (glewInit() != GLEW_OK)
	{
		cout << "Failed to initialize GLEW" << endl;
		return -1;
	}
	// Define the viewport dimensions
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	//set vertical sync to prevent screen tearing
	glfwSwapInterval(1);

	//shaders
	Shader generalShader("Shaders/general_vert.shader", "Shaders/general_frag.shader");
	general = &generalShader;
	Shader selectionShader("Shaders/selection_vert.shader", "Shaders/selection_frag.shader");
	selection = &selectionShader;
	//Shader skyBoxShader("Shaders/skybox_vertex.shader", "Shaders/skybox_fragment.shader");

	generalShader.use();
	// scaled light positions
	// ----------------------
	glm::vec3 livLamp1 = scaling * glm::vec3(2066.43f, 375.f, -693.06f);
	glm::vec3 livLamp2 = scaling * glm::vec3(2608.79f, 375.f, -692.68f);
	glm::vec3 bedLamp = scaling * glm::vec3(2308.93f, 375.f, -1994.81f);
	glm::vec3 kitchLamp = scaling * glm::vec3(765.54f, 375.f, -670.18f);
	generalShader.setVec3("livLamp1", livLamp1);
	generalShader.setVec3("livLamp2", livLamp2);
	generalShader.setVec3("bedLamp", bedLamp);
	generalShader.setVec3("kitchLamp", kitchLamp);

	// load models
	// -----------

	//GLuint skyboxTexture;
	//glGenTextures(1, &skyboxTexture);

	//skyboxTexture = SOIL_load_OGL_single_cubemap
	//(
	//	"Models/skybox.jpg",
	//	"WNESUD",
	//	SOIL_LOAD_AUTO,
	//	SOIL_CREATE_NEW_ID,
	//	SOIL_FLAG_MIPMAPS
	//);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

	//number of objects to load, hard-coded
	int objNum = 28;
	//bedroom
	Model bed("Models/bed/bed.obj");
	cout << "bed loaded,\t\tposition -> " << bed.displacement().x << " : " << bed.displacement().y << " : " << bed.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model ironman("Models/bed/ironman.obj");
	cout << "ironman loaded,\t\tposition -> " << ironman.displacement().x << " : " << ironman.displacement().y << " : " << ironman.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model wardrobe("Models/bed/wardrobe.obj");
	cout << "wardrobe loaded,\tposition -> " << wardrobe.displacement().x << " : " << wardrobe.displacement().y << " : " << wardrobe.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model nightstand("Models/bed/nightstand.obj");
	cout << "nightstand loaded,\tposition -> " << nightstand.displacement().x << " : " << nightstand.displacement().y << " : " << nightstand.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model phone("Models/bed/phone.obj");
	cout << "phone loaded,\t\tposition -> " << phone.displacement().x << " : " << phone.displacement().y << " : " << phone.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';

	//kitchen
	Model kitchen("Models/kitchen/kitchen.obj");
	cout << "kitchen loaded,\t\tposition -> " << kitchen.displacement().x << " : " << kitchen.displacement().y << " : " << kitchen.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model kitchenTable("Models/kitchen/kitchen table.obj");
	cout << "kitchen table loaded,\tposition -> " << kitchenTable.displacement().x << " : " << kitchenTable.displacement().y << " : " << kitchenTable.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model kitchenChair1("Models/kitchen/chair 1.obj");
	cout << "kitchen chair 1 loaded,\tposition -> " << kitchenChair1.displacement().x << " : " << kitchenChair1.displacement().y << " : " << kitchenChair1.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model kitchenChair2("Models/kitchen/chair 2.obj");
	cout << "kitchen chair 2 loaded,\tposition -> " << kitchenChair2.displacement().x << " : " << kitchenChair2.displacement().y << " : " << kitchenChair2.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model kitchenChair3("Models/kitchen/chair 3.obj");
	cout << "kitchen chair 3 loaded,\tposition -> " << kitchenChair3.displacement().x << " : " << kitchenChair3.displacement().y << " : " << kitchenChair3.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model kitchenChair4("Models/kitchen/chair 4.obj");
	cout << "kitchen chair 4 loaded,\tposition -> " << kitchenChair4.displacement().x << " : " << kitchenChair4.displacement().y << " : " << kitchenChair4.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model kettle("Models/kitchen/kettle.obj");
	cout << "kettle loaded,\t\tposition -> " << kettle.displacement().x << " : " << kettle.displacement().y << " : " << kettle.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model gun("Models/kitchen/gun.obj");
	cout << "gun loaded,\t\tposition -> " << gun.displacement().x << " : " << gun.displacement().y << " : " << gun.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model apples("Models/kitchen/apples.obj");
	cout << "apples loaded,\t\tposition -> " << apples.displacement().x << " : " << apples.displacement().y << " : " << apples.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';

	//living room
	Model tv("Models/living/TV.obj");
	cout << "TV loaded,\t\tposition -> " << tv.displacement().x << " : " << tv.displacement().y << " : " << tv.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model couch("Models/living/couch.obj");
	cout << "couch loaded,\t\tposition -> " << couch.displacement().x << " : " << couch.displacement().y << " : " << couch.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model coffeTable("Models/living/coffee table.obj");
	cout << "coffee table loaded,\tposition -> " << coffeTable.displacement().x << " : " << coffeTable.displacement().y << " : " << coffeTable.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model tablePlant("Models/living/table plant.obj");
	cout << "table plant loaded,\tposition -> " << tablePlant.displacement().x << " : " << tablePlant.displacement().y << " : " << tablePlant.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model tray("Models/living/tray.obj");
	cout << "tray loaded,\t\tposition -> " << tray.displacement().x << " : " << tray.displacement().y << " : " << tray.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model laptop("Models/living/laptop.obj");
	cout << "laptop loaded,\t\tposition -> " << laptop.displacement().x << " : " << laptop.displacement().y << " : " << laptop.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model plant("Models/living/indoor plant.obj");
	cout << "indoor plant loaded,\tposition -> " << plant.displacement().x << " : " << plant.displacement().y << " : " << plant.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model dragon("Models/living/dragon.obj");
	cout << "dragon loaded,\t\tposition -> " << dragon.displacement().x << " : " << dragon.displacement().y << " : " << dragon.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';

	//house
	Model house("Models/house/house.obj");
	cout << "house loaded,\t\tposition -> " << house.displacement().x << " : " << house.displacement().y << " : " << house.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';

	//transparent objects
	Model lamps("Models/house/lamps.obj");
	cout << "lamps loaded,\t\tposition -> " << lamps.displacement().x << " : " << lamps.displacement().y << " : " << lamps.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model blender("Models/kitchen/blender.obj");
	cout << "blender loaded,\t\tposition -> " << blender.displacement().x << " : " << blender.displacement().y << " : " << blender.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model glass1("Models/living/glass 1.obj");
	cout << "glass 1 loaded,\t\tposition -> " << glass1.displacement().x << " : " << glass1.displacement().y << " : " << glass1.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model glass2("Models/living/glass 2.obj");
	cout << "glass 2 loaded,\t\tposition -> " << glass2.displacement().x << " : " << glass2.displacement().y << " : " << glass2.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model windows("Models/house/windows.obj");
	cout << "windows loaded,\t\tposition -> " << windows.displacement().x << " : " << windows.displacement().y << " : " << windows.displacement().z << ".\t\t";
	cout << "Objects left: " << --objNum << endl;
	//Model skyBox("Models/cube.obj");

	//sets the shader that each model is going to use.
	for (int i = 0; i < Model::models.size(); ++i) {
		(*(Model::models[i])).setShader(general);
		(*(Model::models[i])).setCamera(&camera);
	}

	//set clear color
	glClearColor(0, 0, 0, 0);

	//timing
	float lastFrame = 0.0f;
	float currentFrame = 0.0f;

	// game loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);
		glfwPollEvents();

		// update view and projection
		// --------------------------
		view = camera.GetViewMatrix();
		projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);

		// update shaders with view and projection
		// ---------------------------------------
		generalShader.use();
		generalShader.setMat4("projection", projection);
		generalShader.setMat4("view", view);
		if (isSelected) {
			selectionShader.use();
			selectionShader.setMat4("projection", projection);
			selectionShader.setMat4("view", view);
		}

		// render
		// ------
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for (int i = 0; i < Model::models.size(); ++i) {
			(*(Model::models[i])).Draw();
		}

		//skyBoxShader.use();
		//glActiveTexture(GL_TEXTURE1);
		//skyBoxShader.setMat4("view_matrix", view);
		//skyBoxShader.setMat4("projection_matrix", projection);
		//skyBoxShader.setInt("skyboxTexture", skyboxTexture);
		//glDepthMask(GL_FALSE);
		//skyBox.Draw(skyBoxShader);
		//glDepthMask(GL_TRUE);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		// glfw: swap buffers
		// ------------------
		glfwSwapBuffers(window);
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}
//determines whether rotating or shifting
bool rotating = false;
// process all input that needs instant and continuous response
// ------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessMovement(MOVE_FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessMovement(MOVE_BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessMovement(MOVE_LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessMovement(MOVE_RIGHT, deltaTime);
	if (isSelected)
	{
		if (rotating)
		{
			if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
				(*selected).rotate(ROTATE_UP_LEFT);
			if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
				(*selected).rotate(ROTATE_UP_RIGHT);
			if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
				(*selected).rotate(ROTATE_UP);
			if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
				(*selected).rotate(ROTATE_DOWN);
			if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
				(*selected).rotate(ROTATE_LEFT);
			if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
				(*selected).rotate(ROTATE_RIGHT);
		}
		else
		{
			if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
				(*selected).shift(SHIFT_UP);
			if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
				(*selected).shift(SHIFT_DOWN);
			if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
				(*selected).shift(SHIFT_FORWARD);
			if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
				(*selected).shift(SHIFT_BACKWARD);
			if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
				(*selected).shift(SHIFT_LEFT);
			if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
				(*selected).shift(SHIFT_RIGHT);
		}
	}
}
// Process all input that doesn't need continuous response
// -------------------------------------------------------
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE &&action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
		rotating = true;
	if (key == GLFW_KEY_R && action == GLFW_RELEASE)
		rotating = false;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int w, int h)
{
	// make sure the viewport matches the new window dimensions
	width = w;
	height = h;
	glViewport(0, 0, width, height);
}
//determines whether camera should follow mouse movement or not
bool processCam = false;
// Process all mouse input
// -----------------------
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		lastX = xpos;
		lastY = ypos;
		processCam = true;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		processCam = false;
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		selectObject(xpos, ypos);
	}
}

// Whenever the mouse moves, this callback is called
// -------------------------------------------------
void cursor_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (processCam) {
		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

		lastX = xpos;
		lastY = ypos;

		camera.ProcessMouseMovement(xoffset, yoffset);
	}
}

// whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

// this function selects the object the user clicked on
// ----------------------------------------------------
void selectObject(double x, double y) {
	unsigned int col[4];
	//clear frame buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	(*selection).use();
	(*selection).setMat4("projection", projection);
	(*selection).setMat4("view", view);
	// draw all objects with their id as a parameter for their color
	for (int i = 0; i < Model::models.size(); ++i) {
		(*(Model::models[i])).setShader(selection);
		(*(Model::models[i])).Draw();
	}
	//get the color of the pixel which the user clicked on
	glReadPixels(x, height - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &col);
	//for some reason the function does r*255+4278190080... oh well
	switch (col[0] - 4278190080) {
	//don't select immovable objects, such as windows, lamps, house, and emptyness
	case 0:
	case 6:
	case 23:
	case 24:
	case 28:
		//deselect object
		isSelected = false;
		selected = nullptr;
		break;
	default:
		//select object
		isSelected = true;
		selected = Model::models[col[0] - 4278190080 - 1];
		break;
	}
	//clear frame buffer again
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//set the usual shader for all objects except selected one
	for (int i = 0; i < Model::models.size(); ++i) {
		(*(Model::models[i])).setShader(general);
		if (isSelected)
			(*selected).setShader(selection);
	}
}