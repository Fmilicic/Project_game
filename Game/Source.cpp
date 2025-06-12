#include <SFML/Graphics.hpp>
#include "sceneManager.h"
#include <ctime>
#include <algorithm> // For std::clamp

// Function to handle window resizing while maintaining aspect ratio
void handleResize(sf::RenderWindow& window, sf::View& view) {
    const sf::Vector2f originalSize(800.f, 600.f);
    float aspectRatio = originalSize.x / originalSize.y;

    sf::Vector2u newSize = window.getSize();
    float newAspectRatio = static_cast<float>(newSize.x) / static_cast<float>(newSize.y);

    float viewportWidth = 1.f;
    float viewportHeight = 1.f;
    float viewportX = 0.f;
    float viewportY = 0.f;

    if (newAspectRatio > aspectRatio) { // Window is wider than the aspect ratio (letterbox)
        viewportWidth = aspectRatio / newAspectRatio;
        viewportX = (1.f - viewportWidth) / 2.f;
    }
    else { // Window is taller than the aspect ratio (pillarbox)
        viewportHeight = newAspectRatio / aspectRatio;
        viewportY = (1.f - viewportHeight) / 2.f;
    }

    view.setViewport({ sf::Vector2f{viewportX, viewportY}, sf::Vector2f{viewportWidth, viewportHeight} });
}

int main() {
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u{ 800, 600 }), "RPG Bullet Hell", sf::Style::Default);
    window.setVerticalSyncEnabled(true);

    // Main camera for the game world. Its size is fixed to the logical resolution.
    sf::View gameView(sf::FloatRect(sf::Vector2f{ 0, 0 }, sf::Vector2f{ 800, 600 }));
    // A separate, static view for the HUD, which never moves.
    sf::View hudView(sf::FloatRect(sf::Vector2f{ 0, 0 }, sf::Vector2f{ 800, 600 }));

    handleResize(window, gameView); // Initial setup for the viewport

    SceneManager sceneManager;
    srand(time(nullptr));
    sf::Clock clock;

    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();

            // Handle window resizing to maintain aspect ratio
            if (event->is<sf::Event::Resized>()) {
                handleResize(window, gameView);
            }

            sceneManager.handleEvent(*event);
        }

        float dt = clock.restart().asSeconds();
        sceneManager.update(dt, window, gameView);

        window.clear(sf::Color::Black);

        // SceneManager now handles setting the correct view before drawing
        sceneManager.draw(window, gameView, hudView);

        window.display();
    }
    return 0;
}