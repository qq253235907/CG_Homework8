#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "shader.h"
#include "camera.h"
#include "Mouse.h"
#include <stack>
#include <vector>
using namespace std;
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouseMove_callback(GLFWwindow* window, double pos_x, double pos_y);
void mouseClick_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void draw(int count, bool helping = false);
// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;
unsigned int VAO, VBO;
#define POINT_MAX 20
bool isRed = false;
float buff_point[6010];
glm::vec3 bgColor = glm::vec3(0.19f, 0.55f, 0.60f);
glm::vec3 pointColor = glm::vec3(0.4f, 0.35f, 0.80f);
glm::vec3 lineColor = glm::vec3(0.7f, 0.6f, 0.2f);


long long int multiple_result[100];
bool overflow = false;
float deltaTime = 0.0f;
float lastFrame = 0.0f;


// Bezier函数
float Bezier(int i, int n, float t) {
	if (n >= POINT_MAX || i >= POINT_MAX || n - i >= POINT_MAX) {
		overflow = true;
		return 0.0f;
	}
	long long int result = multiple_result[n];
	long long int result1 = multiple_result[i];
	long long int result2 = multiple_result[n - i];

	float alpha = result / result1 / result2;
	return (alpha * pow(t, i) * pow(1 - t, n - i));
}
void escapePress(GLFWwindow *window, float& deltaTime);
int buff_set(int count, glm::vec2 cur, glm::vec3 color) {
	buff_point[count * 6] = (cur.x / SCR_WIDTH - 0.5f) * 2.0f;
	buff_point[count * 6 + 1] = -(cur.y / SCR_HEIGHT- 0.5f) * 2.0f;
	buff_point[count * 6 + 2] = 0.0f;
	buff_point[count * 6 + 3] = color.x;
	buff_point[count * 6 + 4] = color.y;
	buff_point[count * 6 + 5] = color.z;
	return count + 1;
}


int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//适应苹果系统
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// 创建一个窗口对象，这个窗口对象存放了所有和窗口相关的数据，而且会被GLFW的其他函数频繁地用到。
	// 此外增加 if (window == NULL) 判断窗口是否创建成功
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Bezier Curve", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSwapInterval(1);
	// tell GLFW to capture our mouse
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouseClick_callback);
	glfwSetCursorPosCallback(window, mouseMove_callback);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	//初始化各种数据
	bool show_window = true;

	//清除屏幕
	glClearColor(bgColor.x, bgColor.y, bgColor.z, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	//创建并绑定ImGui
	const char* glsl_version = "#version 130";
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	//深度测试
	glEnable(GL_DEPTH_TEST);
	//glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//// 加载着色器
	Shader shader("color.vs", "color.frag");


	stack<glm::vec2> press_points;
	vector<glm::vec2> press_points_vec;
	//计算阶乘
	for (int i = 0; i < POINT_MAX; ++i) {
		if (i == 0) multiple_result[i] = 1;
		else if (i == 1) multiple_result[i] = 1;
		else if (i == 2) multiple_result[i] = 2;
		else multiple_result[i] = multiple_result[i - 1] * i;
	}
	
	// 用于辅助线插值
	float helping_t = 0.0f;
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		// per-frame time logic
	   // --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		//创建imgui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Edit", &show_window, ImGuiWindowFlags_MenuBar);
		ImGui::Text("Choose point's color");
		
		ImGui::End();

		// 左键
		if (Mouse::getInstance()->isPress && Mouse::getInstance()->left) {
			if (press_points.size() < POINT_MAX) {
				press_points.push(glm::vec2(Mouse::getInstance()->pos_x, Mouse::getInstance()->pos_y));
				press_points_vec.clear();
				isRed = false;
			}
			else {
				isRed = true;
			}
			Mouse::getInstance()->isPress = false;
		}
		// 右键
		if (Mouse::getInstance()->isPress && !Mouse::getInstance()->left) {
			if (press_points.size() > 0)press_points.pop();
			press_points_vec.clear();
			deltaTime = glfwGetTime();
			isRed = false;
			Mouse::getInstance()->isPress = false;
		}
		
		stack<glm::vec2> tmp = press_points;
		int count = 0;
		bool fill = press_points_vec.empty();
		while (tmp.size() != 0) {
			glm::vec2 point = tmp.top();
			if (fill) press_points_vec.insert(press_points_vec.begin(), point);
			tmp.pop();
			count = buff_set(count, point, pointColor);
		}
		// 画出白点
		draw(count, true);
		// 曲线
		if (press_points.size() > 1) {
			// 曲线
			count = 0;
			// 0.005/ 0.002/ 0.001都可以
			for (float t = 0.0f; t < 1.0f; t += 0.001) {
				glm::vec2 cur = glm::vec2(0.0f, 0.0f);
				for (int i = 0; i < press_points_vec.size(); i++) {
					cur += press_points_vec[i] * Bezier(i, press_points_vec.size() - 1, t);
					if (overflow)
						break;
				}
				if (overflow) 
					break;
				count = buff_set(count, cur, lineColor);
			}
			if (overflow) break;
			draw(count);
			count = 0;
			double currentTime = glfwGetTime();
			if (currentTime - deltaTime > 2.0f) {
				//--------------- 画出辅助线--------------------------------
				vector<glm::vec2> helping_points1 = press_points_vec, helping_points2;
				count = 0;
				helping_t = helping_t > 1.0f ? 0.0f : helping_t + 0.01f;
				while (helping_points1.size() != 0) {
					for (unsigned int i = 0; i < helping_points1.size() - 1; ++i) {
						glm::vec2 cur = (1.0f - helping_t) * helping_points1[i] + helping_t * helping_points1[1 + i];
						count = buff_set(count, cur, lineColor);
						helping_points2.push_back(cur);
					}
					draw(count, true);
					count = 0;
					helping_points1.clear();
					helping_points1 = helping_points2;
					helping_points2.clear();
				}
				glm::vec2 cur = glm::vec2(0.0f, 0.0f);
				for (unsigned int i = 0; i < press_points_vec.size(); i++) {
					cur += press_points_vec[i] * Bezier(i, press_points_vec.size() - 1, helping_t);
				}
				count = buff_set(count, cur, pointColor);
				draw(1);
				count = 0;
			}
		}


		
		// 渲染窗口颜色
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	
		count = 0;
		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
	}


	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}
//按Esc退出窗口
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		Camera::getInstance()->ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		Camera::getInstance()->ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		Camera::getInstance()->ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		Camera::getInstance()->ProcessKeyboard(RIGHT, deltaTime);
	
}
//鼠标控制
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (Camera::getInstance()->firstMouse) {
		Camera::getInstance()->lastX = xpos;
		Camera::getInstance()->lastY = ypos;
		Camera::getInstance()->firstMouse = false;
	}
	float xoffset = xpos - Camera::getInstance()->lastX;
	float yoffset = Camera::getInstance()->lastY - ypos;
	Camera::getInstance()->lastX = xpos;
	Camera::getInstance()->lastY = ypos;
	Camera::getInstance()->lookAround(xoffset, yoffset);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	// 缩小
	if (Camera::getInstance()->fov >= 1.0f && Camera::getInstance()->fov <= 45.0f)
		Camera::getInstance()->fov -= yoffset;
	// 避免溢出
	if (Camera::getInstance()->fov <= 1.0f)
		Camera::getInstance()->fov = 1.0f;
	if (Camera::getInstance()->fov >= 45.0f)
		Camera::getInstance()->fov = 45.0f;
}
//当用户改变窗口的大小的时候，视口也应该被调整。我们可以对窗口注册一个回调函数，它会在每次窗口大小被调整的时候被调用。
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}
void mouseMove_callback(GLFWwindow* window, double pos_x, double pos_y) {
	Mouse::getInstance()->pos_x = pos_x;
	Mouse::getInstance()->pos_y = pos_y;
}
void mouseClick_callback(GLFWwindow* window, int button, int action, int mods) {
	Mouse::getInstance()->isPress = action == GLFW_PRESS;
	Mouse::getInstance()->left = (button == GLFW_MOUSE_BUTTON_LEFT);
}
void draw(int count, bool helping) {

	// 顶点数组对象
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// 绑定缓冲
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * count, buff_point, GL_STATIC_DRAW);
	// 点的大小
	glPointSize(6);
	glBindVertexArray(VAO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDrawArrays(GL_POINTS, 0, count);
	if (helping) glDrawArrays(GL_LINE_STRIP, 0, count);
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VAO);
	glDeleteVertexArrays(1, &VBO);
}