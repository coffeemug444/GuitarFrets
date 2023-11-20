#pragma once

#include <SFML/Graphics.hpp>


class GuitarNeck : public sf::Drawable
{
public:
   GuitarNeck(float width, float height);
   void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

   sf::Vector2f getNotePos(int fret, int string) const;

private:
   void prepareDot(sf::CircleShape& dot);
   float fretDistance(int fret) const;

   float m_pad_y;
   float m_pad_x;
   float m_neck_length;
   float m_neck_height;

   sf::RectangleShape m_top;
   sf::RectangleShape m_bottom;
   sf::RectangleShape m_nut;
   
   sf::VertexArray m_strings;
   sf::VertexArray m_frets;

   sf::CircleShape m_three;
   sf::CircleShape m_five;
   sf::CircleShape m_seven;
   sf::CircleShape m_nine;
   sf::CircleShape m_twelve1;
   sf::CircleShape m_twelve2;
};