#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
class Base : public sf::Drawable {
private:
	int width, height;
	float tile_size;
	std::vector<std::vector<int>> map; // replace with something that will store tile occupation. (two maps maybe? one for tiles, one for characters)
	
	//load textures/sprites
	//load audio files

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
public:
	Base(int width, int height, float tile_size);
};