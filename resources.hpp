#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>

class Resources
{
public:
   static inline sf::Font font;

   static void init()
   {
      font = sf::Font{};
      if (not font.loadFromFile("./font.ttf"))
      {
         throw -1;
      }
   }
};

using RS = Resources;
