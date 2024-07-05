#include <glad/glad.h> // NOTE: Include GLAD first
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>;
#include <glm/gtc/matrix_transform.hpp>;
#include <glm/gtc/type_ptr.hpp>;
#include <stb_image.h>

// Temporarily
const char* vertShader{
	"#version 460 core\n"
	"layout (location = 0) in vec3 pos;"
	"layout (location = 1) in vec2 texCoord;"
	"out vec2 TexCoord;"
	"uniform mat4 transform;"
	"void main() {"
	"	gl_Position = transform * vec4(pos.xyz, 1.0);"
	"	TexCoord = texCoord;"
	"}"
};

const char* fragShader{
	"#version 460 core\n"
	"in vec2 TexCoord;"
	"uniform sampler2D tex;"
	"out vec4 FragColor;"
	"void main() {"
	"	FragColor = texture(tex, TexCoord);;"
	"}"
};

const glm::vec3 blocks[]{
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(1.0f, 0.0f, 0.0f),
	glm::vec3(2.0f, 0.0f, 0.0f),
	glm::vec3(-1.0f, 0.0f, 0.0f),
	glm::vec3(-2.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 0.0f, 1.0f),
	glm::vec3(1.0f, 0.0f, 1.0f),
	glm::vec3(2.0f, 0.0f, 1.0f),
	glm::vec3(-1.0f, 0.0f, 1.0f),
	glm::vec3(-2.0f, 0.0f, 1.0f),
	glm::vec3(0.0f, 0.0f, 2.0f),
	glm::vec3(1.0f, 0.0f, 2.0f),
	glm::vec3(2.0f, 0.0f, 2.0f),
	glm::vec3(-1.0f, 0.0f, 2.0f),
	glm::vec3(-2.0f, 0.0f, 2.0f),
	glm::vec3(0.0f, 0.0f, -1.0f),
	glm::vec3(1.0f, 0.0f, -1.0f),
	glm::vec3(2.0f, 0.0f, -1.0f),
	glm::vec3(-1.0f, 0.0f, -1.0f),
	glm::vec3(-2.0f, 0.0f, -1.0f),
	glm::vec3(0.0f, 0.0f, -2.0f),
	glm::vec3(1.0f, 0.0f, -2.0f),
	glm::vec3(2.0f, 0.0f, -2.0f),
	glm::vec3(-1.0f, 0.0f, -2.0f),
	glm::vec3(-2.0f, 0.0f, -2.0f)
};

struct Player {
	glm::vec3 position; // Position is a point at the bottom of the entity AABB and in the center
	glm::vec3 eyeOffset;
	glm::vec3 forwardDir;
	glm::vec3 upDir;
	glm::vec3 velocity;
	float width; // AABB x direction
	float depth; // AABB z direction
	float height; // AABB y direction
	float pitch;
	float yaw;
	float zoom;
};

int winWidth = 800;
int winHeight = 600;
const float moveSpeed{ 2.0f };
const float cameraSensitivity{ 0.1f };
int firstMouse = 1;
float lastX{};
float lastY{};

float deltaTime{};
float frameTime{};

Player player{};

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	winWidth = width;
	winHeight = height;
}

void processCameraInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE))
		glfwSetWindowShouldClose(window, 1); // On the next iteration of the render loop the "Window should close" condition will fail and the window will close

	float actualMoveSpeed = moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
		actualMoveSpeed *= 2.0f;
	}

	glm::vec3 direction{glm::vec3(0.0f, 0.0f, 0.0f)};
	bool moving = false;
	if (glfwGetKey(window, GLFW_KEY_W)) {
		direction += player.forwardDir;
		moving = true;
	}
	else if (glfwGetKey(window, GLFW_KEY_S)) {
		direction -= player.forwardDir;
		moving = true;
	}

	if (glfwGetKey(window, GLFW_KEY_A)) {
		direction += glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), player.forwardDir));
		moving = true;
	}
	else if (glfwGetKey(window, GLFW_KEY_D)) {
		direction -= glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), player.forwardDir));
		moving = true;
	}
	direction.y = 0;
	if (moving) direction = glm::normalize(direction);
	direction = direction * actualMoveSpeed * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_SPACE)) {
		direction.y = 0.01f;
	}
	else {
		direction.y = player.velocity.y;
	}

	player.velocity = direction;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = 0;
	}

	float offsetX = (float)xpos - lastX;
	float offsetY = lastY - (float)ypos;
	lastX = (float)xpos;
	lastY = (float)ypos;

	player.yaw += offsetX * cameraSensitivity;
	player.pitch += offsetY * cameraSensitivity;

	if (player.pitch > 89.0f) {
		player.pitch = 89.0f;
	}
	else if (player.pitch < -89.0f) {
		player.pitch = -89.0f;
	}

	float x = glm::cos(glm::radians(player.yaw)) * glm::cos(glm::radians(player.pitch));
	float y = glm::sin(glm::radians(player.pitch));
	float z = glm::sin(glm::radians(player.yaw)) * glm::cos(glm::radians(player.pitch));
	player.forwardDir = glm::normalize(glm::vec3(x, y, z));
	glm::vec3 cameraRight = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), player.forwardDir));
	player.upDir = glm::normalize(glm::cross(player.forwardDir, cameraRight));
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	player.zoom -= (float)yoffset;
	if (player.zoom < 1.0f)
		player.zoom = 1.0f;
	if (player.zoom > 90.0f)
		player.zoom = 90.0f;
}

int main() {
	// Initialize the window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window{};
	window = glfwCreateWindow(winWidth, winHeight, "Block Game", NULL, NULL);
	if (!window) {
		std::cout << "Failed to create window\n";
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to load GLAD";
		glfwTerminate();
		return -1;
	}

	glViewport(0, 0, winWidth, winHeight);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	
	// Initialize the player
	player.position = glm::vec3(0.0f, 3.0f, 0.0f);
	player.eyeOffset = glm::vec3(0.0f, 1.7f, 0.0f);
	player.width = 0.8f;
	player.depth = 0.8f;
	player.height = 2.0f;
	player.forwardDir = glm::vec3(0.0f, 0.0f, 1.0f);
	player.upDir = glm::vec3(0.0f, 1.0f, 0.0f);
	player.zoom = 45.0f;


	// Loading the texture
	unsigned int texture;
	int width, height, nrChannels;
	unsigned char* data = stbi_load("resources/textures/stone.png", &width, &height, &nrChannels, 0);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else {
		std::cout << "Failed to load texture data";
	}

	// Creating the shader
	unsigned int fShader, vShader, program;
	int success{};
	char log[512];

	vShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vShader, 1, &vertShader, NULL);
	glCompileShader(vShader);
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vShader, sizeof(log), NULL, log);
		std::cout << "ERROR COMPILING SHADER: " << log << std::endl;
	}

	fShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fShader, 1, &fragShader, NULL);
	glCompileShader(fShader);
	glGetShaderiv(fShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fShader, sizeof(log), NULL, log);
		std::cout << "ERROR COMPILING SHADER: " << log << std::endl;
	}

	program = glCreateProgram();
	glAttachShader(program, vShader);
	glAttachShader(program, fShader);
	glLinkProgram(program);

	glDeleteShader(vShader);
	glDeleteShader(fShader);

	float vertices[] = {
			0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
			0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
			-0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
			-0.5f, 0.5f, 0.5f, 0.0f, 1.0f,

			0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
			0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
			-0.5f, 0.5f, -0.5f, 0.0f, 1.0f,

			0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
			0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
			0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
			0.5f, 0.5f, -0.5f, 0.0f, 1.0f,

			-0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
			-0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
			-0.5f, 0.5f, -0.5f, 0.0f, 1.0f,

			0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
			-0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
			-0.5f, 0.5f, -0.5f, 0.0f, 0.0f,
			0.5f, 0.5f, -0.5f, 1.0f, 0.0f,

			0.5f, -0.5f, 0.5f, 1.0f, 1.0f,
			-0.5f, -0.5f, 0.5f, 0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
			0.5f, -0.5f, -0.5f, 1.0f, 0.0f
	};

	unsigned int triangles[] = {
			0, 1, 3, // Front
			1, 2, 3,
			4, 5, 7, // Back
			5, 6, 7,
			8, 9, 11,
			9, 10, 11,
			12, 13, 15,
			13, 14, 15,
			16, 17, 19,
			17, 18, 19,
			20, 21, 23,
			21, 22, 23
	};

	// Drawing the first square
	unsigned int VAO, VBO, EBO;
	glCreateVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangles), triangles, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0); // Enabling the vertex location component

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1); // Enabling the texture location component

	glEnable(GL_COLOR_BUFFER_BIT); // Don't know if this is necessary, I think this is enabled by default
	glEnable(GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window)) {
		processCameraInput(window);

		float currentTime{ (float)glfwGetTime() };
		deltaTime = currentTime - frameTime;
		frameTime = currentTime;

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // Clear the screen every frame with this color
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the color and depth buffer before making draw calls

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUseProgram(program);

		player.velocity.y -= 0.03 * deltaTime;

		glm::vec3 predictedPos {player.position + player.velocity};
		bool grounded = false;
		for (int i = 0; i < sizeof(blocks) / sizeof(glm::vec3); i++) { // Collision detection
			int isOnBlock = (predictedPos.x - player.width > blocks[i].x && predictedPos.z - player.depth > blocks[i].z) ||
				(predictedPos.x + player.width < blocks[i].x && predictedPos.z - player.depth > blocks[i].z) ||
				(predictedPos.x + player.width < blocks[i].x && predictedPos.z + player.depth < blocks[i].z) ||
				(predictedPos.x - player.width > blocks[i].x && predictedPos.z + player.depth < blocks[i].z);
			if (predictedPos.y < blocks[i].y + 0.5f && isOnBlock) {
				predictedPos.y = blocks[i].y + 0.5f;
				player.velocity.y = 0;
				grounded = true;
			}
		}
		player.position = predictedPos;

		for (int i = 0; i < sizeof(blocks) / sizeof(glm::vec3); i++) {
			glm::mat4 model { glm::mat4(1.0f) };
			model = glm::translate(model, blocks[i]);

			glm::vec3 eyePos {player.position + player.eyeOffset};
			glm::mat4 view { glm::lookAt(eyePos, player.forwardDir + eyePos, player.upDir) };
			//view = glm::translate(view, blocks[i]);

			glm::mat4 projection { glm::perspective(45.0f, 800.0f / 600.0f, 0.1f, 100.0f) };

			glm::mat4 transform = projection * view * model;


			glUniform1i(glGetUniformLocation(program, "tex"), 0);
			glUniformMatrix4fv(glGetUniformLocation(program, "transform"), 1, GL_FALSE, &transform[0][0]);

			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	glfwTerminate();
	return 0;
}