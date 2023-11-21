#include "button.hpp"
#include "resources.hpp"

Button::Button(const sf::String& str)
:m_text(str, RS::font)
{
}

void Button::setSize(const sf::Vector2f& size)
{
   m_background.setSize(size);
   sf::FloatRect text_bounds = m_text.getGlobalBounds();
   float ypos = m_background.getPosition().y + (size.y - text_bounds.height)/2.f;
   float xpos = m_background.getPosition().x + (size.x - text_bounds.width)/2.f;
   m_text.setPosition({xpos, ypos});
}

void Button::setFillColor(const sf::Color& color)
{
   m_background.setFillColor(color);
}

bool Button::mouseIsOver(const sf::Vector2f& point) const
{
   return m_background.getGlobalBounds().contains(point);
}

void Button::setPosition(const sf::Vector2f& position)
{
   sf::Vector2f d = position - m_background.getPosition();
   m_background.move(d);
   m_text.move(d);
}

void Button::draw(sf::RenderTarget& target, sf::RenderStates) const
{
   target.draw(m_background);
   target.draw(m_text);
}