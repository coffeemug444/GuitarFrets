#pragma once
#include <SFML/Graphics.hpp>

class Button : public sf::Drawable
{
public:
   Button(const sf::String& str);
   void setSize(const sf::Vector2f& size);
   void setPosition(const sf::Vector2f& position);
   void setFillColor(const sf::Color& color);
   bool mouseIsOver(const sf::Vector2f& point) const;
   void draw(sf::RenderTarget& target, sf::RenderStates) const override;
private:
   sf::RectangleShape m_background;
   sf::Text m_text;
};