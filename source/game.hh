#ifndef __GAME_HH__
#define __GAME_HH__

#include "card.hh"
#include "player.hh"

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

namespace Game {

  enum State
  {
    STATE_UNASSIGNED  = 0x0,
    STATE_MAIN_MENU   = 0x1,
    STATE_GAME_START  = 0x4,
  };

  class Board
  {
    public:
      Board(Board&)  = delete;
      Board(Board&&) = delete;
      Board();
      
      void shuffleCards();
      void turn();
      Card getCard(std::size_t cardIndex) const;

      inline std::vector< Player >& getPlayersRef() { return m_Players; }

    private:
      sf::Texture m_CardTextureAtlas;
      std::unique_ptr< Card[] > m_Cards;
      std::unique_ptr< sf::Sprite[] > m_CardSprites;

      std::vector< Player > m_Players;
      int unsigned m_TurnNumber;
  };
}

#endif
