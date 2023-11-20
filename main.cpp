#include <SFML/Graphics.hpp>
#include <cmath>
#include <random>
#include <utility>
#include <map>
#include "guitarNeck.hpp"
#include "xShape.hpp"

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> fretDistr(0, 12);
std::uniform_int_distribution<> stringDistr(1, 6);

GuitarNeck guitar_neck{1600.f, 300.f};
sf::CircleShape fret_indicator;
XShape open_string_indicator;

bool open_string = false;

std::map<int, int> string_note_offsets {
   {1, 4},  // E -> C + 4  half steps
   {2, 9},  // A -> C + 9  half steps
   {3, 2},  // D -> C + 2  half steps
   {4, 7},  // G -> C + 7  half steps
   {5, 11}, // B -> C + 11 half steps
   {6, 4}   // E -> C + 4  half steps
};

int current_note = 0;

std::pair<int, int> getRandomNote()
{
   int fret = fretDistr(gen);
   int string = stringDistr(gen);

   return {fret, string};
}

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

void newRandomNote()
{
   auto [fret, string] = getRandomNote();
   current_note = (string_note_offsets.at(string) + fret) % 12;

   open_string = fret == 0;

   open_string_indicator.setPosition(guitar_neck.getNotePos(fret, string));
   fret_indicator.setPosition(guitar_neck.getNotePos(fret, string));

   // playNote((string - 1) * 13 + fret);
}

int main()
{
   sf::RenderWindow window(sf::VideoMode(1600, 300), "SFML works!");

   fret_indicator.setFillColor(sf::Color::Green);
   fret_indicator.setRadius(10.f);
   fret_indicator.setOrigin({10.f, 10.f});

   open_string_indicator.setFillColor(sf::Color::Green);

   newRandomNote();

   while (window.isOpen())
   {
      pollEvents(window);

      window.clear();
      window.draw(guitar_neck);
      if (open_string) window.draw(open_string_indicator);
      else window.draw(fret_indicator);
      window.display();
   }

   return 0;
}