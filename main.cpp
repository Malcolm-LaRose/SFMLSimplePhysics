// THE PLAN
// 
// Learn stack vs. heap
// std::array -> c style array
// Optimize ball collision by only checking in neighborhood around ball for 'ball2'
// Fix collision skipping --> Multiple 'physics frames' per 'render frame' 
// Dont rerender pixels that didnt change? esp background and border
// Real physics
// ????
// Profit


#include "color.h"

#include <GL/glew.h>

#include <SFML/Graphics.hpp>

#include <array>
#include <random>
#include <cmath>
#include <iostream>
#include <string>

std::mt19937 gen(std::random_device{}());
constexpr double PI = 3.141592653;
const sf::Color fillColor = Color::BLUEPRINT;

int genRandomInt(int min, int max) {
	std::uniform_int_distribution<int> dist(min, max);
	return dist(gen);
}

class Ball {
public:

	Ball() : xPos(0), yPos(0), radius(0) {}

	Ball(int x, int y, int r) : xPos(x), yPos(y), radius(r) {}

	void create(int x, int y, int r) {
		radius = r;
		xPos = x;
		yPos = y;

	}

	const sf::Vector2f& getPosition() {
		return sf::Vector2f(xPos, yPos);
	}

	const bool contains(const int& x, const int& y) { // Checks if a point is within a ball
		const float distX = x - (getPosition().x);
		const float distY = y - (getPosition().y);
		return (distX * distX + distY * distY) <= (radius * radius);
	}



	void setPosition(const float& x, const float& y) {
		xPos = x;
		yPos = y;
	}

	bool isDragging = false;

	float radius;

	float xPos; // Center position
	float yPos; // Center position

	sf::Color color = fillColor;

private:

	//float xVel = 0; // May be unnecessary
	//float yVel = 0;


};

class World {
public:

	World() {

		// Init SFML
		window.create(sf::VideoMode(initScreenWidth, initScreenHeight), "Physics Box", sf::Style::Titlebar | sf::Style::Close);
		window.setKeyRepeatEnabled(false);
		window.setVerticalSyncEnabled(false); // Probably ignored by driver
		// window.setFramerateLimit(targetFPS);


		// Init background
		setupVertexBuffer(borderAndBGRect, xPos, yPos, width, height, Color::EIGENGRAU);
		

		// Init balls
		for (int i = 0; i < numRows; i++) {
			for (int j = 0; j < numCols; j++) {
				Ball ball;
				ball.create(sep * i + rad, sep * j + rad, rad);
				listOfBalls[numRows * i + j] = ball;
			}
		}

		// Init Font
		initFont();

		// Set framecounter position
		frameText.setPosition(borderSize, borderSize);
	}

	void setupVertexBuffer(sf::VertexBuffer& vertexBuffer, const int& xPos, const int& yPos, const int& width, const int& height, const sf::Color& color) {

		sf::Vertex vertices[6];

		// First triangle (top-left, top-right, bottom-right)
		vertices[0].position = sf::Vector2f(xPos, yPos);
		vertices[1].position = sf::Vector2f(xPos + width, yPos);
		vertices[2].position = sf::Vector2f(xPos + width, yPos + height);

		// Second triangle (top-left, bottom-right, bottom-left)
		vertices[3].position = sf::Vector2f(xPos, yPos);
		vertices[4].position = sf::Vector2f(xPos + width, yPos + height);
		vertices[5].position = sf::Vector2f(xPos, yPos + height);

		for (int i = 0; i < 6; i++) {
			vertices[i].color = color;
		}

		// Create the vertex buffer
		vertexBuffer.create(6);
		vertexBuffer.setPrimitiveType(sf::Triangles);
		vertexBuffer.update(vertices);


	}

	void placeNRandomBalls(int num) {
		int i = 0;
		while (i < num) {



			Ball ball;
			ball.create(200 * i + (initScreenWidth / 2), (initScreenHeight / 2), 100);
			listOfBalls[i] = ball;
		}
	}


	void frameCounterDisplay(const float& frameTime, const float& avg) {
		frameText.setString("FrameTime (us): " + std::to_string(frameTime) + "\nAvg. FPS: " + std::to_string(avg));

		window.draw(frameText);
	}

	void mainLoop() {

		while (running) {

			handleEvents();
			updateLogic();
			renderAll();

			frameTime = clock.restart().asMicroseconds();
			totalFrameTime += frameTime;

			frameCounterDisplay(frameTime, frameCount / (totalFrameTime / 1000000));
			frameCount++;
			window.display();

		}
	}



private:

	float frameTime = 0;
	float totalFrameTime = 0;

	sf::Text frameText;

	int frameCount = 0;

	bool running = true;

	static constexpr short borderSize = 4;

	static constexpr int xPos = borderSize;
	static constexpr int yPos = borderSize;

	static constexpr short targetFPS = 60;
	static constexpr int initScreenWidth = 1920;
	static constexpr int initScreenHeight = 1080;

	static constexpr int width = initScreenWidth - (2 * borderSize);
	static constexpr int height = initScreenHeight - (2 * borderSize);

	static constexpr int numRows = 100;
	static constexpr int numCols = 100;
	static constexpr int rad = 5;
	static constexpr int sep = 10;

	static constexpr int numBalls = numRows * numCols;

	sf::VertexBuffer borderAndBGRect;

	std::array<Ball, numBalls> listOfBalls;

	sf::RenderWindow window;

	// int deltaTime = 0.001; // Time step? save spot for now

	// static constexpr int gravity = 0.1; // Arbitrary for now, probably in units of px/s^2

	sf::Clock clock;

	sf::Font font;

	float dragOffsetX;
	float dragOffsetY;

	void initFont() {
		font.loadFromFile(".\\Montserrat-Regular.ttf");
		frameText.setCharacterSize(34);
		frameText.setFillColor(Color::WHITE);
		frameText.setFont(font);
	}


	void handleEvents() {
		sf::Event event;
		while (window.pollEvent(event)) {
			switch (event.type) {
			case sf::Event::Closed:
				window.close();
				running = false;
				break;
			case sf::Event::MouseButtonPressed:
				if (event.mouseButton.button == sf::Mouse::Left) {
					sf::Vector2i mousePos = sf::Mouse::getPosition(window);
					for (Ball& ball : listOfBalls) {
						if (ball.contains(mousePos.x, mousePos.y)) {
							ball.color = Color::BLUE;
							ball.isDragging = true;
							dragOffsetX = mousePos.x - ball.xPos;
							dragOffsetY = mousePos.y - ball.yPos;
							break;
						}
					}
				}
				break;
			case sf::Event::MouseButtonReleased:
				if (event.mouseButton.button == sf::Mouse::Left) {
					for (Ball& ball : listOfBalls) {
						ball.color = Color::BLUEPRINT;
						ball.isDragging = false;
					}
				}
				break;
			case sf::Event::MouseMoved:
				for (Ball& ball : listOfBalls) {
					dragBall(ball);
				}
				break;

				/*case sf::Event::KeyPressed:
					if (event.key.scancode == sf::Keyboard::Scan::Space) {
						jiggle();
					}
					break;
				*/
			}
		}
	}


	void dragBall(Ball& ball) {
		if (ball.isDragging) {
			const sf::Vector2i& mousePos = sf::Mouse::getPosition(window);

			// Putting boundary checking here caused ball overlapping problems
			ball.setPosition(mousePos.x - dragOffsetX, mousePos.y - dragOffsetY);
		}
	}


	void updateLogic() {

		// gravity();

		collisionChecking();

	}

	void collisionChecking() {
		for (size_t i = 0; i < listOfBalls.size(); ++i) {
			// Putting boundary checking here caused ball overlapping problems
			for (size_t j = i + 1; j < listOfBalls.size(); ++j) {
				checkCollision(listOfBalls[i], listOfBalls[j]); // Should only check balls in a neighborhood around the ball
			}
			checkBoundaryCollision(listOfBalls[i]); // Should only check balls near the border
		}

		//for (Ball& ball : listOfBalls) {
		//	checkCollision(ball);
		//	checkBoundaryCollision(ball); // Still check boundary collisions
		//}

	}

	void checkBoundaryCollision(Ball& ball) {
		// Check for boundary collisions and adjust position
		sf::Vector2f newPos = ball.getPosition();

		// Does this if actually improve things?
		if (newPos.x - ball.radius > borderSize && newPos.x + ball.radius < initScreenWidth - borderSize &&
			newPos.y - ball.radius > borderSize && newPos.y + ball.radius < initScreenHeight - borderSize) {
			return; // Far from all walls, no need to check
		}


		if (newPos.x - ball.radius < xPos) {
			newPos.x = xPos + ball.radius;
		}
		else if (newPos.x + ball.radius > xPos + width) {
			newPos.x = xPos + width - ball.radius;
		}

		if (newPos.y - ball.radius < yPos) {
			newPos.y = yPos + ball.radius;
		}
		else if (newPos.y + ball.radius > yPos + height) {
			newPos.y = yPos + height - ball.radius;
		}

		if (ball.getPosition() != newPos) { // Shouldn't change position if position hasn't changed 
			ball.setPosition(newPos.x, newPos.y);
		}

	}

	bool pointContainsBall(const float& x, const float& y, const float& sDist) {

		bool isBall = false;

		Ball* ball;

		if (ball->contains(x,y)) {
			
		}


	}

	void checkCollisionCheap(Ball& ball) {
		
		const float& xPos = ball.xPos;
		const float& yPos = ball.yPos;

		const float& searchDistance = 2.1f * ball.radius; // Only check a small region around each ball for another ball

		for (Ball& other : listOfBalls) {
			if (&ball == &other) continue; // Skip self

			float dx = other.xPos - xPos;
			float dy = other.yPos - yPos;
			if (dx * dx + dy * dy <= searchDistance * searchDistance) {
				checkCollision(ball, other);
			}
		}


	
	}


	void checkCollision(Ball& ball1, Ball& ball2) { // More expensive collision checking

		// ONLY LOOK FOR BALLS THAT ARE NEARBY

		// Search around ball1
		// Check distance = 2*ball1.radius



		float dx = ball2.xPos - ball1.xPos;
		float dy = ball2.yPos - ball1.yPos;
		float distanceSq = dx * dx + dy * dy;
		float minDistance = ball1.radius + ball2.radius;

		if (distanceSq < minDistance * minDistance) {
			// std::cout << "Collision detected between balls!\n";
			float distance = std::sqrt(distanceSq);
			// Resolve collision
			float overlap = 0.5f * (minDistance - distance); // Why the div by 2

			// Calculate displacement directions
			const float& displacementX = overlap * (dx / distance);
			const float& displacementY = overlap * (dy / distance);

			// Displace balls
			ball1.setPosition(ball1.xPos - displacementX, ball1.yPos - displacementY);
			ball2.setPosition(ball2.xPos + displacementX, ball2.yPos + displacementY);

			// Check if balls are still overlapping
			dx = ball2.xPos - ball1.xPos;
			dy = ball2.yPos - ball1.yPos;
			distanceSq = dx * dx + dy * dy;
			overlap = 0.25f * (minDistance * minDistance - distanceSq);

			if (distanceSq < 0.25) {
				distance = std::sqrt(distanceSq);
				// If balls are still overlapping, further adjust their positions
				const float& adjustX = (overlap + ball1.radius / 2) * (dx / distance);
				const float& adjustY = (overlap + ball1.radius / 2) * (dy / distance);

				ball1.setPosition(ball1.xPos - adjustX, ball1.yPos - adjustY);
				ball2.setPosition(ball2.xPos + adjustX, ball2.yPos + adjustY);
			}

		}

	}

	void gravity() {
		// Move balls downward
		for (size_t i = 0; i < listOfBalls.size(); ++i) {
			Ball& ball = listOfBalls[i];
			if (!ball.isDragging) {
				ball.setPosition(ball.getPosition().x, ball.getPosition().y + 0.5);
			}
		}
	}

	void jiggle() {
		for (size_t i = 0; i < listOfBalls.size(); ++i) {
			Ball& ball = listOfBalls[i];

			float xOff = (80.0f * genRandomInt(-1, 1));
			float yOff = (80.0f * genRandomInt(-1, 1));

			if (!ball.isDragging) {
				ball.setPosition(ball.getPosition().x + xOff, ball.getPosition().y + yOff);
			}
		}
	}





	void renderWorld() {
		window.draw(borderAndBGRect);
	}

	// Num triangles used to render the circles
	static constexpr int numSegments = 12;

	// Calculate the angle between each segment
	static constexpr float angleStep = 2 * PI / numSegments;


	void renderBalls() {

		// Prepare the vertex array to hold all the triangles
		sf::VertexArray vertices(sf::Triangles, listOfBalls.size() * 3 * numSegments);

		// Populate the vertex array with the vertices of each ball
		for (size_t i = 0; i < listOfBalls.size(); ++i) {
			Ball& ball = listOfBalls[i];

			const sf::Vector2f& position = ball.getPosition();
			const float& radius = ball.radius;

			// Iterate over each segment and create triangles
			for (int j = 0; j < numSegments; ++j) {
				// Calculate the angle of the current segment
				const float& angle1 = j * angleStep;
				const float& angle2 = (j + 1) * angleStep;

				// Calculate the vertices of the triangle
				const sf::Vector2f vertex1(position.x, position.y);
				const sf::Vector2f vertex2(position.x + radius * std::cos(angle1), position.y + radius * std::sin(angle1));
				const sf::Vector2f vertex3(position.x + radius * std::cos(angle2), position.y + radius * std::sin(angle2));
				
				// Set the vertices of the triangle in the vertex array
				vertices[i * numSegments * 3 + j * 3].position = vertex1;
				vertices[i * numSegments * 3 + j * 3].color = ball.color;
				vertices[i * numSegments * 3 + j * 3 + 1].position = vertex2;
				vertices[i * numSegments * 3 + j * 3 + 1].color = ball.color;
				vertices[i * numSegments * 3 + j * 3 + 2].position = vertex3;
				vertices[i * numSegments * 3 + j * 3 + 2].color = ball.color;
			}
		}

		// Draw all the triangles at once
		window.draw(vertices);
	}


	void renderAll() {
		renderWorld();
		renderBalls();
	}

};


int main() {

	World world;

	world.mainLoop();

	return 0;
}
