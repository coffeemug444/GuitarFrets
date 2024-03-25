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

enum GuessMode
{
   FRET,
   NOTE
};

const float SCREEN_W = 1600.f;
const float SCREEN_H = 600.f;
const float NECK_H = SCREEN_H/3;

bool include_sharps = false;
std::vector<int> included_strings {};
bool menu = true;
GuessMode guess_mode = NOTE;

bool showing_correct = false;

std::deque<std::pair<int,int>> last_few_notes;
void newRandomNote();
void guessed();
std::future<void> guess_thread;

std::random_device rd;
std::mt19937 gen(rd());

GuitarNeck guitar_neck{SCREEN_W, NECK_H};
sf::CircleShape fret_indicator;
XShape open_string_indicator{SCREEN_H/24.f};

sf::CircleShape guess_fret_indicator;
XShape guess_open_string_indicator{SCREEN_H/24.f};

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
   {"Guess\nfret"},
   {"Guess\nnote"}
};

int note = 0;
int fret = 0;
int string = 0;

std::pair<int, int> getRandomStringFretPair()
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
         guessButtons.at(note).setFillColor(sf::Color::Green);
         guess_thread = std::async(guessed);
         return;
      }
   }
}

void pollFrets(const sf::Vector2f& point)
{
   if (guessButtons.back().mouseIsOver(point))
   {
      // back to menu
      menu = true;
      guitar_neck.resetStrings();
      for (Button& button : guessButtons) {
         if (&button == &guessButtons.back()) break;
         button.setFillColor(sf::Color::Blue);
      }
      return;
   }

   sf::FloatRect neck_bounds = guitar_neck.getGlobalBounds();
   
   // check if within the correct height
   if (point.y > (neck_bounds.top + neck_bounds.height)) return;
   if (point.y < neck_bounds.top) return;

   // check if on the nut
   
   float first_fret_w = guitar_neck.getNotePos(1,1).x - neck_bounds.left;
   if (point.x > (neck_bounds.left - first_fret_w*0.2f) &&
       point.x < (neck_bounds.left + first_fret_w*0.2f))
   {
      // clicked nut
      guess_fret_indicator.setFillColor(sf::Color::Transparent);
      guess_open_string_indicator.setPosition(guitar_neck.getNotePos(0, string));
      guess_open_string_indicator.setFillColor(sf::Color::Red);

      if (fret == 12)
      {
         // it's also correct so this is counted
         open_string_indicator.setPosition(guitar_neck.getNotePos(0, string));
         open_string = true;
      }

      guess_thread = std::async(guessed);
      return;
   }

   if (not neck_bounds.contains(point)) return;

   // definitely clicked one of the frets
   guess_open_string_indicator.setFillColor(sf::Color::Transparent);
   guess_fret_indicator.setFillColor(sf::Color::Red);

   for (int i = 1; i <= 12; i++)
   {
      float fret_x = guitar_neck.getFretXPos(i);

      if (point.x > fret_x) continue;
      guess_fret_indicator.setPosition(guitar_neck.getNotePos(i, string));

      if (i == 12 and fret == 0)
      {
         // it's also correct so this is counted
         fret_indicator.setPosition(guitar_neck.getNotePos(12, string));
         open_string = false;
      }
      break;
   }
   guess_thread = std::async(guessed);
}

void pollMenuButtons(const sf::Vector2f& point)
{
   for (int i = 1; Button& button : menuButtons) {
      if (button.mouseIsOver(point))
      {
         if (&button == &menuButtons.at(menuButtons.size() - 1))
         {
            // guessing note
            if (included_strings.size() == 0) return;
            guess_mode = NOTE;
            menu = false;
            newRandomNote();
            return;
         }
         if (&button == &menuButtons.at(menuButtons.size() - 2))
         {
            // guessing fret
            if (included_strings.size() == 0) return;
            guess_mode = FRET;
            menu = false;
            newRandomNote();
            return;
         }
         if (&button == &menuButtons.at(menuButtons.size() - 3))
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
         if (menu)
         {
            pollMenuButtons(point);
         }
         else
         {
            if (guess_mode == NOTE)
               pollGuessButtons(point);
            else
               pollFrets(point);
         }
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

void newRandomNoteFretMode()
{
   guitar_neck.resetStrings();
   showing_correct = false;

   int new_fret, new_string;
   // want a new note that was not shown recently, and not immediately next to the last one
   do std::tie(new_fret, new_string) = getRandomStringFretPair();
   while (std::find(last_few_notes.begin(), 
                    last_few_notes.end(), 
                    std::make_pair(new_fret % 12, new_string)) != last_few_notes.end() ||
          last_few_notes.size() > 0 && new_string == string && std::abs(new_fret - fret) % 12 <= 1);

   last_few_notes.push_front(std::make_pair(new_fret % 12, new_string));
   if (last_few_notes.size() > 3*included_strings.size()) last_few_notes.pop_back();

   std::tie(fret, string) = {new_fret, new_string};
   note = (string_note_offsets.at(string) + fret) % 12;

   guitar_neck.highlightString(string);

   open_string_indicator.setPosition(guitar_neck.getNotePos(fret, string));
   fret_indicator.setPosition(guitar_neck.getNotePos(fret, string));
   open_string = fret == 0;

   for (int i = 0; i < guessButtons.size() - 1; i++)
   {
      guessButtons.at(i).setFillColor(i == note ? sf::Color::Green : sf::Color::Blue);
   }
}

void newRandomNoteNoteMode()
{
   int new_fret, new_string;
   // want a new note that was not shown recently, and not immediately next to the last one
   do std::tie(new_fret, new_string) = getRandomStringFretPair();
   while (std::find(last_few_notes.begin(), 
                    last_few_notes.end(), 
                    std::make_pair(new_fret % 12, new_string)) != last_few_notes.end() ||
          last_few_notes.size() > 0 && new_string == string && std::abs(new_fret - fret) % 12 <= 1);

   last_few_notes.push_front(std::make_pair(new_fret % 12, new_string));
   if (last_few_notes.size() > 3*included_strings.size()) last_few_notes.pop_back();

   std::tie(fret, string) = {new_fret, new_string};
   note = (string_note_offsets.at(string) + fret) % 12;

   open_string = fret == 0;

   open_string_indicator.setPosition(guitar_neck.getNotePos(fret, string));
   fret_indicator.setPosition(guitar_neck.getNotePos(fret, string));

   playNote((6 - string) * 13 + fret);
}

void newRandomNote()
{
   if (guess_mode == NOTE)
   {
      newRandomNoteNoteMode();
   }
   else
   {
      newRandomNoteFretMode();
   }
}

void guessed()
{
   showing_correct = true;
   std::this_thread::sleep_for(1000ms);
   showing_correct = false;
   guess_open_string_indicator.setFillColor(sf::Color::Transparent);
   guess_fret_indicator.setFillColor(sf::Color::Transparent);
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

   sf::RenderWindow window(sf::VideoMode(SCREEN_W, SCREEN_H), "Guitar Frets", sf::Style::Titlebar | sf::Style::Close);
   window.setFramerateLimit(60);

   fret_indicator.setFillColor(sf::Color::Green);
   float ind_r = SCREEN_H/60.f;
   fret_indicator.setRadius(ind_r);
   fret_indicator.setOrigin({ind_r, ind_r});
   guess_fret_indicator.setRadius(ind_r);
   guess_fret_indicator.setOrigin({ind_r, ind_r});

   open_string_indicator.setFillColor(sf::Color::Green);

   setupButtons();

   guess_fret_indicator.getGlobalBounds();

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
         if (guess_mode == NOTE)
         {
            if (open_string) window.draw(open_string_indicator);
            else window.draw(fret_indicator);
         }
         else
         {
            window.draw(guess_open_string_indicator);
            window.draw(guess_fret_indicator);

            if (showing_correct)
            {
               if (open_string) window.draw(open_string_indicator);
               else window.draw(fret_indicator);
            }
         }
         for (const Button& button : guessButtons)
            window.draw(button);
      }
      window.display();
   }

   return 0;
}
