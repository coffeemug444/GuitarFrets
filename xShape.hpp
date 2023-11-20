#pragma once
#include <SFML/Graphics.hpp>


class XShape : public sf::Drawable
{
public:
   XShape() {
      m_decl.setSize({25.f, 5.f});
      m_decl.setOrigin({12.5f, 2.5f});

      m_incl.setSize({25.f, 5.f});
      m_incl.setOrigin({12.5f, 2.5f});

      m_decl.rotate(45.f);
      m_incl.rotate(-45.f);
   }

   void setPosition(const sf::Vector2f& pos)
   {
      m_incl.setPosition(pos);
      m_decl.setPosition(pos);
   }

   void setFillColor(const sf::Color& color)
   {
      m_decl.setFillColor(color);
      m_incl.setFillColor(color);
   }

   void draw(sf::RenderTarget& target, sf::RenderStates) const override
   {
      target.draw(m_decl);
      target.draw(m_incl);
   }

private:
   sf::Vector2f m_position;
   sf::RectangleShape m_decl;
   sf::RectangleShape m_incl;
};
