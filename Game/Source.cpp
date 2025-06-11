/* S1: take stock: in order to run a game, what does it need? The requirements are as follows:

Let’s develop console-based game involving a player
character that moves on the map, collects food and fights
enemies.

For the optional project task, you need to implement a 2D
topdown game (in C++) in a team

Players must control a character that can enter combat
with enemies

You define the win and loose condition

Write a simple GDD describing their game mechanics and lore
of the game

You can choose to implement the game using console based
rendering (ASCII render :)) or SFML

You can use Windows or Linux

 Design better (more interesting) fighting mechanics
 Maybe even simple turn-based system
 Design way to manage health and optionally, shield

assumption 1: we want to do this in SFML (Shit Fuck My Life)
assumption 2: our better fighting mechanics are simple skills for offense, bullet-hell for defense, unless we want to go the e33 route
assumption 3: health and shield work simple: 1 health taken per hit incurred, shield function either same as health OR it entirely prevents damage for one turn

assumption O: implement all the others' shit as well to blow them out of the water. Liam's doing it. Why shouldn't we?

we need:
assumption 4: at least one window for map management. filled with all the lovely things a map needs
assumption 5: for bullet hell/combat - one more window needed, unless we want to use the same one we use on the map, which seems needlessly complex (though we'll have to bring the fight window to the fore)
assumption 6: we need at least 2 entity types: player and enemy. Further types (tiles, items, weapons etc.) can be added after
assumption 7: player and enemy share map presence, health and atk stats at least. Possibly grouped under characters class
assumption 8: health, shields and possibly atk need to be displayed somewhere on screen. Enemy health needs to be displayed, at least when we are fighting them
assumption 8: skill management can either be done as separate list player instances have access to, or some sort of (dictionary maybe?) that is linked to player instance itself
assumption 9: bullet hell (BH, henceforth) instance will likely need to be a separate class entirely. let's say we:
	assumption 9.1: wrap the game class as wrapper containg game, player, enemy classes in itself, with lists for enemies that specify their coordinates
	assumption 9.2: create BH instance when entering combat. BH instance NEEDS to have acces at least to player health and shield, and assuming we want to have player turn taken in it as well (which seems altogether a more sensible solution) enemy hp as well 
assumption 10: for win condition, let's start of with: collect key (either as map pickup, or on stronger enemy) and reach stairs

*/

#include <SFML/Graphics.hpp>
#include "sceneManager.h"
#include "Entity.h"
int main() {
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u{ 800, 600 }), "RPG Bullet Hell", sf::Style::Default);
    sf::View view(sf::FloatRect(sf::Vector2f{ 0, 0 }, sf::Vector2f{ 800, 600 })); // Logical game area

    SceneManager sceneManager;

    sf::Clock clock;
    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();

            // Handle window resizing
            if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                // Make the view cover the entire new window, in the same logical coords
                sf::FloatRect visibleArea(
                    { 0.f, 0.f },
                    { static_cast<float>(resized->size.x),
                      static_cast<float>(resized->size.y) }
                );
                window.setView(sf::View(visibleArea));
            }

            sceneManager.handleEvent(*event);
        }

        float dt = clock.restart().asSeconds();
        sceneManager.update(dt, window);

        window.setView(view);
        window.clear();
        window.draw(sceneManager);
        window.display();
    }
    return 0;
}