#include <SFML/Graphics.hpp>
#include "sceneManager.h"
#include <ctime>
#include <algorithm> // For std::clamp

// This function remains unchanged. It correctly handles the game view's aspect ratio.
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

    sf::View gameView(sf::FloatRect(sf::Vector2f{ 0, 0 }, sf::Vector2f{ 800, 600 }));
    sf::View hudView(sf::FloatRect(sf::Vector2f{ 0, 0 }, sf::Vector2f{ 800, 600 }));

    handleResize(window, gameView); // Initial setup for the viewport

    SceneManager sceneManager;
    srand(time(nullptr));
    sf::Clock clock;

    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();

            // --- MODIFIED RESIZE LOGIC ---
            if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                // 1. Update the game view to maintain the aspect ratio (letterboxing)
                handleResize(window, gameView);

                // 2. Update the HUD view to match the new window size exactly
                //    This prevents the HUD from stretching.
                sf::Vector2f newSize(static_cast<float>(resized->size.x), static_cast<float>(resized->size.y));
                hudView.setSize(newSize);
                hudView.setCenter(newSize / 2.f);
            }
            // --- END OF MODIFICATION ---

            sceneManager.handleEvent(*event);
        }

        float dt = clock.restart().asSeconds();
        sceneManager.update(dt, window, gameView);

        window.clear(sf::Color::Black);

        sceneManager.draw(window, gameView, hudView);

        window.display();
    }
    return 0;
}
