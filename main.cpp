#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <random>
#include <utility>
#include <map>
#include <deque>
#include <future>
#include <thread>
#include <chrono>
#include <sstream>
#include "guitarNeck.hpp"
#include "xShape.hpp"
#include "button.hpp"
#include "resources.hpp"

using namespace std::chrono_literals;

const float SCREEN_W = 800.f;
const float SCREEN_H = 300.f;
const float NECK_H = SCREEN_H/3;

bool include_sharps = false;
std::vector<int> included_strings {};
bool menu = true;

std::deque<int> last_3_notes;
void newRandomNote();
void guessed();
std::future<void> guess_thread;

std::random_device rd;
std::mt19937 gen(rd());

GuitarNeck guitar_neck{SCREEN_W, NECK_H};
sf::CircleShape fret_indicator;
XShape open_string_indicator{SCREEN_H/24.f};

sf::SoundBuffer sound_buffer;
sf::Sound sound;

bool open_string = false;

std::map<int, int> string_note_offsets {
   {1, 4},  // E -> C + 4  half steps
   {2, 11}, // B -> C + 11 half steps
   {3, 7},  // G -> C + 7  half steps
   {4, 2},  // D -> C + 2  half steps
   {5, 9},  // A -> C + 9  half steps
   {6, 4}   // E -> C + 4  half steps
};

std::vector<Button> guessButtons {
   {"C"},
   {L"C\u266F\nD\u266D"},
   {"D"},
   {L"D\u266F\nE\u266D"},
   {"E"},
   {"F"},
   {L"F\u266F\nG\u266D"},
   {"G"},
   {L"G\u266F\nA\u266D"},
   {"A"},
   {L"A\u266F\nB\u266D"},
   {"B"},
   {"back"}
};

std::vector<Button> menuButtons {
   {"e"},
   {"B"},
   {"G"},
   {"D"},
   {"A"},
   {"E"},
   {"Include\nsharps"},
   {"begin"}
};

int current_note = 0;

std::pair<int, int> getRandomNote()
{
   std::uniform_int_distribution<> stringDistr(0, included_strings.size() - 1);
   int string = included_strings.at(stringDistr(gen));

   if (include_sharps)
   {
      std::uniform_int_distribution<> fretDistr(0,12);
      int fret = fretDistr(gen);
      return {fret, string};
   }

   std::uniform_int_distribution<> fretDistr(0,7);
   std::vector<int> available_frets;
   switch (string)
   {
      // high e
      case 1: available_frets = {0, 1, 3, 5, 7, 8, 10, 12}; break;
      // B
      case 2: available_frets = {0, 1, 3, 5, 6, 8, 10, 12}; break;
      // G
      case 3: available_frets = {0, 2, 4, 5, 7, 9, 10, 12}; break;
      // D
      case 4: available_frets = {0, 2, 3, 5, 7, 9, 10, 12}; break;
      // A
      case 5: available_frets = {0, 2, 3, 5, 7, 8, 10, 12}; break;
      // E
      case 6: available_frets = {0, 1, 3, 5, 7, 8, 10, 12}; break;
   }

   int fret = available_frets.at(fretDistr(gen));
   return {fret, string};
}

void pollGuessButtons(const sf::Vector2f& point)
{
   for (Button& button : guessButtons) {
      if (button.mouseIsOver(point))
      {
         if (&button == &guessButtons.back())
         {
            menu = true;
            return;
         }
         button.setFillColor(sf::Color::Red);
         guessButtons.at(current_note).setFillColor(sf::Color::Green);
         guess_thread = std::async(guessed);
         return;
      }
   }
}

void pollMenuButtons(const sf::Vector2f& point)
{
   for (int i = 1; Button& button : menuButtons) {
      if (button.mouseIsOver(point))
      {
         if (&button == &menuButtons.back())
         {
            if (included_strings.size() == 0) return;
            menu = false;
            newRandomNote();
            return;
         }
         if (&button == &menuButtons.at(menuButtons.size() - 2))
         {
            include_sharps = !include_sharps;
            button.setFillColor(include_sharps ? sf::Color::Blue : sf::Color::Green);
            return;
         }

         auto vec_pos = std::find(included_strings.begin(), included_strings.end(), i);

         if (vec_pos != included_strings.end())
         {
            included_strings.erase(vec_pos);
            button.setFillColor(sf::Color::Green);
            return;
         }

         included_strings.push_back(i);
         button.setFillColor(sf::Color::Blue);
         return;
      }
      i++;
   }
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
         menu ? pollMenuButtons(point) : pollGuessButtons(point);
      }
      default:
         break;
      }
   }
}

void playNote(int note)
{
   std::stringstream pathbuilder;
   pathbuilder << "samples/" << note << ".wav";
   sound_buffer.loadFromFile(pathbuilder.str());
   sound.play();
}

void newRandomNote()
{
   int fret, string;
   // want a note than was not in the last 3
   do
   {
      std::tie(fret, string) = getRandomNote();
      current_note = (string_note_offsets.at(string) + fret) % 12;
   } while (std::find(last_3_notes.begin(), last_3_notes.end(), current_note) != last_3_notes.end());

   last_3_notes.push_front(current_note);
   if (last_3_notes.size() > 3) last_3_notes.pop_back();

   open_string = fret == 0;

   open_string_indicator.setPosition(guitar_neck.getNotePos(fret, string));
   fret_indicator.setPosition(guitar_neck.getNotePos(fret, string));

   playNote((6 - string) * 13 + fret);
}

void guessed()
{
   std::this_thread::sleep_for(1000ms);
   for (Button& button : guessButtons)
   {
      if (&button == &guessButtons.back()) continue;
      button.setFillColor(sf::Color::Blue);
   }
   newRandomNote();
}

void setupButtons()
{
   float button_size = SCREEN_W / 13.f;
   float padding = (SCREEN_W - 12 * button_size) / 13.f;

   guessButtons.back().setSize({button_size,button_size});
   guessButtons.back().setPosition({padding, NECK_H + button_size + 2*padding});
   guessButtons.back().setFillColor(sf::Color::Green);
   for (int i = 0; i < 12; i++)
   {
      guessButtons.at(i).setSize({button_size,button_size});
      guessButtons.at(i).setPosition({i*button_size + (i+1)*padding, NECK_H + padding});
      guessButtons.at(i).setFillColor(sf::Color::Blue);
   }

   for (int i = 0; Button& button : menuButtons)
   {
      button.setSize({button_size,button_size});
      button.setPosition({i*button_size + (i+1)*padding, NECK_H + padding});
      button.setFillColor(sf::Color::Green);
      i++;
   }

}

int main()
{
   RS::init();
   sound.setBuffer(sound_buffer);

   sf::RenderWindow window(sf::VideoMode(SCREEN_W, SCREEN_H), "SFML works!");
   window.setFramerateLimit(60);

   fret_indicator.setFillColor(sf::Color::Green);
   float ind_r = SCREEN_H/60.f;
   fret_indicator.setRadius(ind_r);
   fret_indicator.setOrigin({ind_r, ind_r});

   open_string_indicator.setFillColor(sf::Color::Green);

   setupButtons();

   while (window.isOpen())
   {
      pollEvents(window);

      window.clear();
      if (menu)
      {
         for (const Button& button : menuButtons)
            window.draw(button);
      }
      else
      {
         window.draw(guitar_neck);
         if (open_string) window.draw(open_string_indicator);
         else window.draw(fret_indicator);
         for (const Button& button : guessButtons)
            window.draw(button);
      }
      window.display();
   }

   return 0;
}
