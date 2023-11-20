#include <SFML/Graphics.hpp>
#include <cmath>
#include <random>
#include <utility>
#include <map>
#include "guitarNeck.hpp"
#include "xShape.hpp"
#include "button.hpp"
#include "resources.hpp"

const float SCREEN_W = 1600.f;
const float SCREEN_H = 400.f;
const float NECK_H = 200.f;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> fretDistr(0, 12);
std::uniform_int_distribution<> stringDistr(1, 6);

GuitarNeck guitar_neck{SCREEN_W, NECK_H};
sf::CircleShape fret_indicator;
XShape open_string_indicator;

bool open_string = false;

std::map<int, int> string_note_offsets {
   {1, 4},  // E -> C + 4  half steps
   {2, 11}, // B -> C + 11 half steps
   {3, 7},  // G -> C + 7  half steps
   {4, 2},  // D -> C + 2  half steps
   {5, 9},  // A -> C + 9  half steps
   {6, 4}   // E -> C + 4  half steps
};

std::vector<Button> buttons {
   {"C"},
   {"C#"},
   {"D"},
   {"D#"},
   {"E"},
   {"F"},
   {"F#"},
   {"G"},
   {"G#"},
   {"A"},
   {"A#"},
   {"B"}
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
      case sf::Event::MouseButtonPressed:
      {
         if (event.mouseButton.button != sf::Mouse::Left) break;
         float x = static_cast<float>(event.mouseButton.x);
         float y = static_cast<float>(event.mouseButton.y);
         sf::Vector2f point {x, y};
         bool clicked = false;
         for (Button& button : buttons) {
            if (button.mouseIsOver(point))
            {
               button.setFillColor(sf::Color::Red);
               clicked = true;
               break;
            }
         }
         if (not clicked) break;
         buttons.at(current_note).setFillColor(sf::Color::Green);
         break;
      }
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

void setupButtons()
{
   float padding = (SCREEN_W - 12 * 100.f) / 13.f;

   for (int i = 0; i < 12; i++)
   {
      buttons.at(i).setSize({100.f,100.f});
      buttons.at(i).setPosition({i*100.f + (i+1)*padding, NECK_H + (SCREEN_H - NECK_H - 100.f) / 2.f});
      buttons.at(i).setFillColor(sf::Color::Blue);
   }
}

int main()
{
   RS::init();

   sf::RenderWindow window(sf::VideoMode(SCREEN_W, SCREEN_H), "SFML works!");

   fret_indicator.setFillColor(sf::Color::Green);
   fret_indicator.setRadius(10.f);
   fret_indicator.setOrigin({10.f, 10.f});

   open_string_indicator.setFillColor(sf::Color::Green);

   setupButtons();

   newRandomNote();

   while (window.isOpen())
   {
      pollEvents(window);

      window.clear();
      window.draw(guitar_neck);
      if (open_string) window.draw(open_string_indicator);
      else window.draw(fret_indicator);
      for (const Button& button : buttons)
         window.draw(button);
      window.display();
   }

   return 0;
}