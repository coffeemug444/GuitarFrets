#include "guitarNeck.hpp"
#include <cmath>

GuitarNeck::GuitarNeck(float width, float height)
{
   m_strings.setPrimitiveType(sf::Lines);
   m_frets.setPrimitiveType(sf::Lines);

   m_pad_y = 0.1f * height;
   m_pad_x = 0.02f * width;
   m_neck_length = width - 2*m_pad_x;
   m_neck_height = height - 2*m_pad_y;

   float nut_w = 6.f;

   m_top.setSize({m_neck_length + nut_w, 3.f});
   m_top.move({m_pad_x - nut_w, m_pad_y - 3.f});
   m_top.setFillColor(m_grey);

   m_bottom.setSize({m_neck_length + nut_w, 3.f});
   m_bottom.move({m_pad_x - nut_w, m_pad_y + m_neck_height});
   m_bottom.setFillColor(m_grey);

   m_nut.setSize({nut_w, m_neck_height});
   m_nut.move({m_pad_x - nut_w, m_pad_y});
   m_nut.setFillColor(m_grey);

   float full_neck_length = 2*m_neck_length;

   for (int i = 1; i <= 12; i++)
   {
      float factor = std::pow(2.f, -i/12.f);
      float xpos = full_neck_length * (1 - factor);

      m_frets.append(sf::Vertex{sf::Vector2f{m_pad_x + xpos, m_pad_y},m_grey});
      m_frets.append(sf::Vertex{sf::Vector2f{m_pad_x + xpos, m_pad_y + m_neck_height},m_grey});
   }

   float pad_y_inner = m_neck_height * 0.08f;
   for (int i = 0; i < 6; i++)
   {
      float ypos = m_pad_y + pad_y_inner + i*0.84f*m_neck_height/5.f;
      m_strings.append(sf::Vertex{sf::Vector2f{m_pad_x, ypos},sf::Color::White});
      m_strings.append(sf::Vertex{sf::Vector2f{m_pad_x + m_neck_length, ypos},sf::Color::White});
   }

   prepareDot(m_three);
   prepareDot(m_five);
   prepareDot(m_seven);
   prepareDot(m_nine);
   prepareDot(m_twelve1);
   prepareDot(m_twelve2);

   m_twelve1.move({0,m_neck_height/6.f});
   m_twelve2.move({0,-m_neck_height/6.f});

   m_three.move({fretDistance(3),0});
   m_five.move({fretDistance(5)},0);
   m_seven.move({fretDistance(7)},0);
   m_nine.move({fretDistance(9)},0);
   m_twelve1.move({fretDistance(12), 0});
   m_twelve2.move({fretDistance(12), 0});
}

void GuitarNeck::draw(sf::RenderTarget& target, sf::RenderStates) const
{
   target.draw(m_three);
   target.draw(m_five);
   target.draw(m_seven);
   target.draw(m_nine);
   target.draw(m_twelve1);
   target.draw(m_twelve2);

   target.draw(m_top);
   target.draw(m_bottom);
   target.draw(m_nut);
   
   target.draw(m_strings);
   target.draw(m_frets);
}

void GuitarNeck::prepareDot(sf::CircleShape &dot)
{
   dot.setFillColor(m_grey);
   float rad = m_pad_y / 2.f;
   dot.setRadius(rad);
   dot.move({m_pad_x - rad, m_pad_y + m_neck_height/2 - rad});
}


float GuitarNeck::fretDistance(int fret) const
{
   float factor = (std::pow(2.f, -fret/12.f) + std::pow(2.f, -(fret - 1)/12.f))/2.f;
   return (2 * m_neck_length) * (1 - factor);
}

sf::Vector2f GuitarNeck::getNotePos(int fret, int string) const
{
   float pad_y_inner = m_neck_height * 0.08f;
   float ypos = m_pad_y + pad_y_inner + (string-1) * 0.84f * m_neck_height / 5.f;
   float xpos = m_pad_x;
   if (fret > 0) xpos += fretDistance(fret);
   return sf::Vector2f{xpos, ypos};
}