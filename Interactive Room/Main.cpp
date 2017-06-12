#include "glew.h"
#include "glfw3.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

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

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
int width, height;
const float scale = 0.02f;

// camera
Camera camera(glm::vec3(45.6f, 5.2f, -2.43f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

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
	glEnable(GL_CULL_FACE); //enable culling
	glCullFace(GL_BACK); //cull the back faces (front, back, or both) glFrontFace(GL_CW); //specifies what to consider the “front” (CW/CCW)
	glFrontFace(GL_CCW); //specifies what to consider the “front” (CW/CCW)

	// build and compile shaders
	// -------------------------
	Shader generalShader("Shaders/general_vert.shader", "Shaders/general_frag.shader");
	//Shader skyBoxShader("Shaders/skybox_vertex.shader", "Shaders/skybox_fragment.shader");

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
	int objNum = 28;
	//bedroom
	Model bed("Models/bed/bed.obj");
	cout << "bed loaded,\t\tposition -> " << bed.displacement(scale).x << " : " << bed.displacement(scale).y << " : " << bed.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model ironman("Models/bed/ironman.obj");
	cout << "ironman loaded,\t\tposition -> " << ironman.displacement(scale).x << " : " << ironman.displacement(scale).y << " : " << ironman.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model wardrobe("Models/bed/wardrobe.obj");
	cout << "wardrobe loaded,\tposition -> " << wardrobe.displacement(scale).x << " : " << wardrobe.displacement(scale).y << " : " << wardrobe.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model nightstand("Models/bed/nightstand.obj");
	cout << "nightstand loaded,\tposition -> " << nightstand.displacement(scale).x << " : " << nightstand.displacement(scale).y << " : " << nightstand.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model phone("Models/bed/phone.obj");
	cout << "phone loaded,\t\tposition -> " << phone.displacement(scale).x << " : " << phone.displacement(scale).y << " : " << phone.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';

	//kitchen
	Model kitchen("Models/kitchen/kitchen.obj");
	cout << "kitchen loaded,\t\tposition -> " << kitchen.displacement(scale).x << " : " << kitchen.displacement(scale).y << " : " << kitchen.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model kitchenTable("Models/kitchen/kitchen table.obj");
	cout << "kitchen table loaded,\tposition -> " << kitchenTable.displacement(scale).x << " : " << kitchenTable.displacement(scale).y << " : " << kitchenTable.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model kitchenChair1("Models/kitchen/chair 1.obj");
	cout << "kitchen chair 1 loaded,\tposition -> " << kitchenChair1.displacement(scale).x << " : " << kitchenChair1.displacement(scale).y << " : " << kitchenChair1.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model kitchenChair2("Models/kitchen/chair 2.obj");
	cout << "kitchen chair 2 loaded,\tposition -> " << kitchenChair2.displacement(scale).x << " : " << kitchenChair2.displacement(scale).y << " : " << kitchenChair2.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model kitchenChair3("Models/kitchen/chair 3.obj");
	cout << "kitchen chair 3 loaded,\tposition -> " << kitchenChair3.displacement(scale).x << " : " << kitchenChair3.displacement(scale).y << " : " << kitchenChair3.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model kitchenChair4("Models/kitchen/chair 4.obj");
	cout << "kitchen chair 4 loaded,\tposition -> " << kitchenChair4.displacement(scale).x << " : " << kitchenChair4.displacement(scale).y << " : " << kitchenChair4.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model kettle("Models/kitchen/kettle.obj");
	cout << "kettle loaded,\t\tposition -> " << kettle.displacement(scale).x << " : " << kettle.displacement(scale).y << " : " << kettle.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model blender("Models/kitchen/blender.obj");
	cout << "blender loaded,\t\tposition -> " << blender.displacement(scale).x << " : " << blender.displacement(scale).y << " : " << blender.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model gun("Models/kitchen/gun.obj");
	cout << "gun loaded,\t\tposition -> " << gun.displacement(scale).x << " : " << gun.displacement(scale).y << " : " << gun.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model apples("Models/kitchen/apples.obj");
	cout << "apples loaded,\t\tposition -> " << apples.displacement(scale).x << " : " << apples.displacement(scale).y << " : " << apples.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';

	//living room
	Model tv("Models/living/TV.obj");
	cout << "TV loaded,\t\tposition -> " << tv.displacement(scale).x << " : " << tv.displacement(scale).y << " : " << tv.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model couch("Models/living/couch.obj");
	cout << "couch loaded,\t\tposition -> " << couch.displacement(scale).x << " : " << couch.displacement(scale).y << " : " << couch.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model coffeTable("Models/living/coffee table.obj");
	cout << "coffee table loaded,\tposition -> " << coffeTable.displacement(scale).x << " : " << coffeTable.displacement(scale).y << " : " << coffeTable.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model tablePlant("Models/living/table plant.obj");
	cout << "table plant loaded,\tposition -> " << tablePlant.displacement(scale).x << " : " << tablePlant.displacement(scale).y << " : " << tablePlant.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model tray("Models/living/tray.obj");
	cout << "tray loaded,\t\tposition -> " << tray.displacement(scale).x << " : " << tray.displacement(scale).y << " : " << tray.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model glass1("Models/living/glass 1.obj");
	cout << "glass 1 loaded,\t\tposition -> " << glass1.displacement(scale).x << " : " << glass1.displacement(scale).y << " : " << glass1.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model glass2("Models/living/glass 2.obj");
	cout << "glass 2 loaded,\t\tposition -> " << glass2.displacement(scale).x << " : " << glass2.displacement(scale).y << " : " << glass2.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model laptop("Models/living/laptop.obj");
	cout << "laptop loaded,\t\tposition -> " << laptop.displacement(scale).x << " : " << laptop.displacement(scale).y << " : " << laptop.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model plant("Models/living/indoor plant.obj");
	cout << "indoor plant loaded,\tposition -> " << plant.displacement(scale).x << " : " << plant.displacement(scale).y << " : " << plant.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model dragon("Models/living/dragon.obj");
	cout << "dragon loaded,\t\tposition -> " << dragon.displacement(scale).x << " : " << dragon.displacement(scale).y << " : " << dragon.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';

	//house
	Model house("Models/house/house.obj");
	cout << "house loaded,\t\tposition -> " << house.displacement(scale).x << " : " << house.displacement(scale).y << " : " << house.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model lamps("Models/house/lamps.obj");
	cout << "lamps loaded,\t\tposition -> " << lamps.displacement(scale).x << " : " << lamps.displacement(scale).y << " : " << lamps.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << '\n';
	Model windows("Models/house/windows.obj");
	cout << "windows loaded,\t\tposition -> " << windows.displacement(scale).x << " : " << windows.displacement(scale).y << " : " << windows.displacement(scale).z << ".\t\t";
	cout << "Objects left: " << --objNum << endl;
	//Model skyBox("Models/cube.obj");

	glm::mat4 model;
	glm::mat4 shadeMod;
	glClearColor(0, 0, 0, 1);

	// game loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);
		glfwPollEvents();

		// render
		// ------
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		//skyBoxShader.use();
		//glActiveTexture(GL_TEXTURE1);
		//skyBoxShader.setMat4("view_matrix", view);
		//skyBoxShader.setMat4("projection_matrix", projection);
		//skyBoxShader.setInt("skyboxTexture", skyboxTexture);
		//glDepthMask(GL_FALSE);
		//skyBox.Draw(skyBoxShader);
		//glDepthMask(GL_TRUE);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		generalShader.use();

		generalShader.setMat4("projection", projection);
		generalShader.setMat4("view", view);
		// render the loaded model
		//scale down the models
		shadeMod = glm::scale(model, glm::vec3(scale));
		generalShader.setMat4("model", shadeMod);
		//lamps first because of a bug
		lamps.Draw(generalShader);
		//kitchen
		kitchen.Draw(generalShader);
		kitchenTable.Draw(generalShader);
		kitchenChair1.Draw(generalShader);
		kitchenChair2.Draw(generalShader);
		kitchenChair3.Draw(generalShader);
		kitchenChair4.Draw(generalShader);
		kettle.Draw(generalShader);
		gun.Draw(generalShader);
		apples.Draw(generalShader);
		//bedroom
		bed.Draw(generalShader);
		ironman.Draw(generalShader);
		wardrobe.Draw(generalShader);
		nightstand.Draw(generalShader);
		phone.Draw(generalShader);
		//living room
		tv.Draw(generalShader);
		couch.Draw(generalShader);
		coffeTable.Draw(generalShader);
		tablePlant.Draw(generalShader);
		tray.Draw(generalShader);
		laptop.Draw(generalShader);
		plant.Draw(generalShader);
		dragon.Draw(generalShader);
		//house
		house.Draw(generalShader);
		//transparent objects
		blender.Draw(generalShader);
		glass1.Draw(generalShader);
		glass2.Draw(generalShader);
		windows.Draw(generalShader);
		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all camera input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessMovement(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessMovement(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessMovement(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessMovement(RIGHT, deltaTime);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE &&action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (key == GLFW_KEY_L &&action == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	if (key == GLFW_KEY_T && action == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int w, int h)
{
	// make sure the viewport matches the new window dimensions; note that width and
	// height will be significantly larger than specified on retina displays.
	width = w;
	height = h;
	glViewport(0, 0, width, height);
}

bool processCam = false;
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
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
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

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}