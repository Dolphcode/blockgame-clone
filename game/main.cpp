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
//

const char* fragShader{
	"#version 460 core\n"
	"in vec2 TexCoord;"
	"uniform sampler2D tex;"
	"out vec4 FragColor;"
	"void main() {"
	"	FragColor = texture(tex, TexCoord);;"
	"}"
};

//


int main() {
	std::cout << "Hello World!\n";

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window{};
	window = glfwCreateWindow(800, 600, "Block Game", NULL, NULL);
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

	glViewport(0, 0, 800, 600);

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
			0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
			0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			-0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
			0.5f, 0.5f, 0.0f, 1.0f, 1.0f
	};

	// Drawing the first square
	unsigned int VAO, VBO;
	glCreateVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0); // Enabling the vertex location component

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1); // Enabling the texture location component

	glEnable(GL_COLOR_BUFFER_BIT); // Don't know if this is necessary, I think this is enabled by default
	//glEnable(GL_DEPTH_BUFFER_BIT);
	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // Clear the screen every frame with this color
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the color and depth buffer before making draw calls

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		glm::mat4 model { glm::mat4(1.0f) };
		model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		glm::mat4 view { glm::mat4(1.0f) };
		view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

		glm::mat4 projection { glm::perspective(45.0f, 800.0f / 600.0f, 0.1f, 100.0f) };

		glm::mat4 transform = projection * view * model;

		glUseProgram(program);
		glUniform1i(glGetUniformLocation(program, "tex"), 0);
		glUniformMatrix4fv(glGetUniformLocation(program, "transform"), 1, GL_FALSE, &transform[0][0]);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	glfwTerminate();
	return 0;
}