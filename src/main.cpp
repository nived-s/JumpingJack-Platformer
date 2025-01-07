#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <iostream>

// Obstacle class
class Obstacle
{
public:
    sf::RectangleShape shape;
    float speed;

    Obstacle(float x, float y, float width, float height, float speed)
        : speed(speed)
    {
        shape.setSize(sf::Vector2f(width, height));
        shape.setFillColor(sf::Color::Red);
        shape.setPosition({ x, y });
    }

    void move()
    {
        shape.move({ speed, 0 });
    }

    bool isOffScreen(float windowWidth) const
    {
        return shape.getPosition().x > windowWidth;
    }
};

int main()
{
    // Create the window
    sf::RenderWindow window(sf::VideoMode({ 960, 540 }), "Jumpy Jack");
    window.setFramerateLimit(60);

    // Loading font
    sf::Font font;
    if (!font.openFromFile("..\\..\\..\\..\\fonts\\framd.ttf"))             // path should be relative to executable
    {
        return -1;
    }

    // Loading bg and ground textures
    sf::Texture backgroundTexture, groundTexture;
    if (!backgroundTexture.loadFromFile("..\\..\\..\\..\\Assets\\Background.png") || !groundTexture.loadFromFile("..\\..\\..\\..\\Assets\\Tiles.png"))
    {
        std::cerr << "Error loading textures!" << std::endl;
        return -1;
    }

    // Background sprites
    sf::Sprite background1(backgroundTexture), background2(backgroundTexture);
    background1.setScale({ 2.0f, 2.0f });
    background2.setScale({ 2.0f, 2.0f });
    background2.setPosition({ background1.getGlobalBounds().size.x, 0 });

    // Ground sprites
    int screenWidth = window.getSize().x;
    int groundHeight = groundTexture.getSize().y;
    groundTexture.setRepeated(true); // Enable texture repetition
    // Create a sprite that spans the screen width
    sf::Sprite ground1(groundTexture, sf::IntRect({0, 0}, { screenWidth, groundHeight}));
    sf::Sprite ground2(groundTexture, sf::IntRect({ 0, 0 }, { screenWidth, groundHeight }));

    // Set positions
    ground1.setPosition({ 0.f, window.getSize().y - 80.f});
    ground2.setPosition({ ground1.getGlobalBounds().size.x, window.getSize().y - 80.f });
    

    // Scoring system
    int score = 0;
    sf::Text scoreText(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::Black);
    scoreText.setPosition({ window.getSize().x - 200.f, 10.f });

    // Score timer 
    float scoreTimer = 0.f;
    const float scoreInterval = 1.f; // increment score every 1 second

    // Ground
    sf::RectangleShape ground(sf::Vector2f(window.getSize().x, 80.f));
    ground.setFillColor(sf::Color(29, 30, 10));
    ground.setPosition({ 0.f, window.getSize().y - ground.getSize().y });

    // Player
    sf::RectangleShape player(sf::Vector2f(50.f, 120.f));
    player.setFillColor(sf::Color(0, 0, 0));
    player.setPosition({ (player.getSize().x) * 3, window.getSize().y - ground.getSize().y - player.getSize().y });

    // Player velocity & gravity
    bool isJumping = false;
    const float gravity = 1.0f;
    const float jumpVelocity = -22.f;
    float playerVelocityY = 0.f;

    // Obstacles
    std::vector<Obstacle> obstacles;
    const float obstacleSpeed = -10.f;
    const float spawnInterval = 0.5f; // spawn every 0.5 seconds
    float spawnTimer = 0.f;


    // Game end Text
    sf::Text endText(font);
    endText.setCharacterSize(48);
    endText.setFillColor(sf::Color::Red);
    endText.setString("Game Ended");
    endText.setPosition({ window.getSize().x / 2.f - endText.getGlobalBounds().size.x / 2.f, window.getSize().y / 2.f - ground.getSize().y - endText.getGlobalBounds().size.y / 2.f });

    // Restart Text 
    sf::Text restartText(font);
    restartText.setCharacterSize(24);
    restartText.setFillColor(sf::Color::Black);
    restartText.setString("Press \"F\" to restart");
    restartText.setPosition({ window.getSize().x / 2.f - restartText.getGlobalBounds().size.x / 2.f, window.getSize().y / 2.f - ground.getSize().y + 80.f });

    // Possible obst. sizes
    std::vector<sf::Vector2f> obstacleSizes = {
        {40.f, 20.f},
        {50.f, 40.f},
        {30.f, 50.f},
        {10.f, 50.f},
        {20.f, 60.f},
        {60.f, 70.f},
        {30.f, 80.f},
    };

    std::srand(std::time(0));

    // Game state
    bool isGameRunning = true;

    // Run the program as long as the window is open
    while (window.isOpen())
    {
        // Check all the window's events that were triggered since the last iteration of the loop
        while (const std::optional event = window.pollEvent())
        {
            // "Close requested" event: we close the window
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }

            // *** PLAYER MOVEMENTS ***
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                // Spacebar: player jump
                if (keyPressed->scancode == sf::Keyboard::Scan::Space && !isJumping)
                {
                    playerVelocityY = jumpVelocity;
                    isJumping = true;
                }

                // F: restart game
                if (keyPressed->scancode == sf::Keyboard::Scan::F && !isGameRunning)
                {
                    // Restarting game processes
                    isGameRunning = true;
                    score = 0;      // reset score
                    player.setPosition({ (player.getSize().x) * 3, window.getSize().y - ground.getSize().y - player.getSize().y });     // update player location to default
                    obstacles.clear();      // reset obstacles list
                }
            }
        }

        // Game is running without collision
        if (isGameRunning)
        {
            // Background movement
            background1.move({ -4.f, 0 });
            background2.move({ -4.f, 0 });

            if (background1.getPosition().x + background1.getGlobalBounds().size.x < 0)
                background1.setPosition({ background2.getPosition().x + background2.getGlobalBounds().size.x, 0 });

            if (background2.getPosition().x + background2.getGlobalBounds().size.x < 0)
                background2.setPosition({ background1.getPosition().x + background1.getGlobalBounds().size.x, 0 });

            // Ground movement
            float groundSpeed = -8.0f;
            ground1.move({ groundSpeed, 0 });
            ground2.move({ groundSpeed, 0 });

            // Check if the sprites move off-screen and reposition them
            if (ground1.getPosition().x + ground1.getGlobalBounds().size.x < 0)
            {
                ground1.setPosition({ground2.getPosition().x + ground2.getGlobalBounds().size.x, ground1.getPosition().y});
            }

            if (ground2.getPosition().x + ground2.getGlobalBounds().size.x < 0)
            {
                ground2.setPosition({ ground1.getPosition().x + ground1.getGlobalBounds().size.x, ground2.getPosition().y });
            }


            // Apply gravity and make player jump
            playerVelocityY += gravity;
            player.move({ 0, playerVelocityY });

            // Check if player landed back on ground
            if (player.getPosition().y >= window.getSize().y - ground.getSize().y - player.getSize().y)
            {
                player.setPosition({ player.getPosition().x, window.getSize().y - ground.getSize().y - player.getSize().y });
                playerVelocityY = 0.f;
                isJumping = false;
            }

            // Spawn obstacles
            spawnTimer += 1.f / 60; // 60 FPS
            if (spawnTimer >= spawnInterval)
            {
                spawnTimer = 0.f;
                // Choose random obstacle shape
                sf::Vector2f randomSize = obstacleSizes[std::rand() % obstacleSizes.size()];
                // spawning location of obstacle
                float obstacleX = window.getSize().x + randomSize.x;
                float minY = 150.f;
                float maxY = window.getSize().y - ground.getSize().y - randomSize.y;
                float obstacleY = minY + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX / (maxY - minY));
                obstacles.emplace_back(obstacleX, obstacleY, randomSize.x, randomSize.y, obstacleSpeed);
            }

            // Move and remove obstacles
            for (auto it = obstacles.begin(); it != obstacles.end(); )
            {
                it->move();
                if (it->isOffScreen(window.getSize().x))
                {
                    it = obstacles.erase(it);
                }
                else
                {
                    ++it;
                }
            }

            // Collision detection using AABB
            for (const auto& obstacle : obstacles)
            {
                if (player.getGlobalBounds().findIntersection(obstacle.shape.getGlobalBounds()))
                {
                    // set flag to stop game state
                    isGameRunning = false;
                }
            }

            // Updating score
            scoreTimer += 1.f / 60;
            if (scoreTimer >= scoreInterval)
            {
                scoreTimer = 0.f; // reset the timer 
                score += 1; // increment the score 
            }
        }

        // Updating score text
        std::stringstream ss;
        ss << "Score: " << score;
        scoreText.setString(ss.str());

        // Clear the window with white color
        window.clear(sf::Color::White);

        // Draw entities to screen
        window.draw(background1);
        window.draw(background2);
        window.draw(ground);
        window.draw(ground1);
        window.draw(ground2);
        window.draw(player);
        window.draw(scoreText);
        for (const auto& obstacle : obstacles)
        {
            window.draw(obstacle.shape);
        }

        // Collision is detected and game needs to stop
        if (!isGameRunning)
        {
            // Display "Game Ended" text 
            window.draw(endText);
            // Display Restart button text
            window.draw(restartText);

        }

        // End the current frame
        window.display();
    }
}