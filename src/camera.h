
#ifndef _CAMERA_H_
#define _CAMERA_H_
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include<glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// ͨ�����п��ܵ�����ƶ����������ͼ�ı仯
enum Direction {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

/*��������������*/
class Camera {
public:

	// ����ֵ
	// �����λ��
	glm::vec3 Position = glm::vec3(4.7f, 2.6f, 11.4f);
	// �����ǰ����
	glm::vec3 Front = glm::vec3(-0.37f, -0.03f, -9.28f);
	// �����������
	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
	// �����������
	glm::vec3 Right;
	// �۲�����
	glm::vec3 WorldUp = glm::vec3(0.0f, 0.0f, 0.0f);

	// ŷ����
	float Yaw = -90.0f;
	float Pitch = 0.0f;

	// �����ж�������λ��
	float lastX;
	float lastY;
	float fov = 45.0f;
	// �Ƿ��һ���ƶ����
	bool firstMouse = true;

	// �����ѡ��
	float MovementSpeed = 3.0f;
	// ������ж�
	float MouseSensitivity = 0.1f;
	// ���ű���
	float Zoom = 60.0f;

	static Camera* getInstance() {
		if (instance == NULL) {
			instance = new Camera();
		}
		return instance;
	}

	/**
	* @brief ͨ����õķ�����ж����Ĵ���
	* @param direction ����ķ���
	* @param deltaTIme �����ʱ��
	*/
	void ProcessKeyboard(Direction direction, float deltaTime)
	{

		// ͨ���ƶ��ٶ��Լ�����ʱ�����ٶ�
		float cameraSpeed = MovementSpeed * deltaTime;
		// �������ַ�����ж�
		if (direction == FORWARD)
			moveForward(Front * cameraSpeed);
		if (direction == BACKWARD)
			moveBack(Front * cameraSpeed);
		if (direction == LEFT)
			moveLeft(glm::normalize(glm::cross(Front, Up)) * cameraSpeed);
		if (direction == RIGHT)
			moveRight(glm::normalize(glm::cross(Front, Up)) * cameraSpeed);
	}

	/**
	* @brief ����ƶ��Ӷ��ƶ���ͼ
	* @param xoffset x����ƫ����
	* @param yoffset y����ƫ����
	*/
	void lookAround(float xoffset, float yoffset)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		if (Pitch > 89.0f)
			Pitch = 89.0f;
		if (Pitch < -89.0f)
			Pitch = -89.0f;

		// Calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
	}

	/**
	* @brief ����������λ��
	* @param x x����
	* @param y y����
	*/
	void setLastXY(float x, float y) {
		lastX = x;
		lastY = y;
	}

private:
	/**
	* @brief ǰ���ƶ�
	* @param distance �ƶ�����
	**/
	void moveForward(glm::vec3 distance) {
		Position += distance;
	}

	/**
	* @brief ǰ���ƶ�
	* @param distance �ƶ�����
	**/
	void moveBack(glm::vec3 distance) {
		Position -= distance;
	}

	/**
	* @brief ǰ���ƶ�
	* @param distance �ƶ�����
	**/
	void moveRight(glm::vec3 distance) {
		Position += distance;
	}

	/**
	* @brief ǰ���ƶ�
	* @param distance �ƶ�����
	**/
	void moveLeft(glm::vec3 distance) {
		Position -= distance;
	}

	static Camera* instance;
};


#endif
