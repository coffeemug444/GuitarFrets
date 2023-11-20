#include <SFML/Graphics.hpp>
#include <cmath>
#include "guitarNeck.hpp"


void pollEvents(sf::RenderWindow& window) {
   sf::Event event;
   while (window.pollEvent(event))
   {
      switch (event.type)
      {
      case sf::Event::Closed:
         window.close();
         break;
      case sf::Event::KeyPressed:
         if (event.key.code == sf::Keyboard::Key::Escape)
            window.close();
         break;
      default:
         break;
      }
   }
}

int main()
{
   sf::RenderWindow window(sf::VideoMode(1600, 300), "SFML works!");

   GuitarNeck guitar_neck{1600.f, 300.f};

   sf::CircleShape indicator;
   indicator.setFillColor(sf::Color::Green);
   indicator.setRadius(10.f);
   indicator.setOrigin({10.f, 10.f});

   indicator.setPosition(guitar_neck.getNotePos(6, 3));

   while (window.isOpen())
   {
      pollEvents(window);

      window.clear();
      window.draw(guitar_neck);
      window.draw(indicator);
      window.display();
   }

   return 0;
}