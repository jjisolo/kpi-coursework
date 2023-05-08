#include "Program.hpp"

#include "../engine/Sprite.hpp"

static constexpr const glm::vec2  CARD_ASSET_RATIO                = {1.f,    1.5f};
static constexpr const glm::vec2  CARD_ASSET_SIZE_NON_NORMALIZED  = {150.0f, 150.0f};
static constexpr const glm::ivec2 CARD_ASSET_SIZE_NORMALIZED      = CARD_ASSET_SIZE_NON_NORMALIZED*CARD_ASSET_RATIO;
static constexpr const float      CARDS_ROW_OTHER_PLAYERS_Y_COORD = 0.02f;

static constexpr auto GAME_DESCRIPTION =
R"(The goal of the game is to score the least number of points. In order to play, you need a 
deck of 36 cards and from 2 to 4 players.The first card dealer in the game is determined by
lot, in  the following games, players deal cards in turn. The deck is carefully shuffled, 
removed and 5 cards are dealt to each player.

The deliverer deals himself 4 cards, and the fifth card is put on the line.The remaining
deck is placed in the center of the table in a closed form.The player to the left of the
deliverer continues the game. He must put a card of the same suit or a card of the same 
value on the fifth card of the deliverer, if there is no such card, then the player takes
one card from the deck and if this card does not fit, then the player skips the turn. Some
cards have their own features in the game, namely: aces, queens, king of spades, nines,
sevens and sixes.
)";

static constexpr auto GAME_SPECIFICATION =
R"(The game is baked using GLFW(platform layer) OpenGL library, GLAD OpenGL functions
loader library. GLM for the mathematical calculations(such as vector manipulations,
matricies calculus etc.), and the SPDLOG for logging into the files, and finally,
immediate-mode UI library Dear ImGui.
)";

static constexpr auto GAME_CREDITS =
R"(This game was create as a subject of my university coursework.

The source code is publicly available under the MIT license, and
hosted on https://github.com/jjisolo/coursework-1.
)";

using namespace std;

namespace Game
{
	Engine::Error GameProgram::onUserInitialize()
	{
        auto windowDimensions  = getWindowDimensions();
		
		// Load game background texture
  		Engine::ResourceManager::loadTexture("data/assets/background.jpg", false, "background");
		
        // Load the card reverse side textures.
	    Engine::ResourceManager::loadTexture("data/assets/card-back1.png", true, "card-back-blue");
        Engine::ResourceManager::loadTexture("data/assets/card-back2.png", true, "card-back-red");
        Engine::ResourceManager::loadTexture("data/assets/card-back3.png", true, "card-back-green");
	    Engine::ResourceManager::loadTexture("data/assets/card-back4.png", true, "card-back-yellow");	
				
		// Set up game board
		m_gameBoard.generateDeck();
		m_gameBoard.shuffleDeck();
	    m_gameBoard.assignCardsToThePlayers();

		// The entry point of the game is main menu.
		m_gameInfo.gameState = GameState::Main_Menu;
		
		return(Engine::Error::Ok);
	}

	Engine::Error GameProgram::onUserRelease()
	{

		return(Engine::Error::Ok);
	}

	Engine::Error GameProgram::onUserUpdate(GLfloat elapsedTime)
	{
	    auto windowDimensions = getWindowDimensions();
		
		// Create sprite for the background.
		Engine::GFX::Sprite backgroundSprite;
		backgroundSprite .setSpriteSize({ windowDimensions.x, windowDimensions.y });
		backgroundSprite .bindTexture  ("background");

		if(m_gameInfo.gameState == GameState::Game_Board)
		{		  
		  m_gameBoardSprites.clear();
		  m_gameBoardSprites.push_back(backgroundSprite);

		  updateGameBoardSprites(windowDimensions);
		 
		  for (auto& sprite : m_gameBoardSprites)
			  sprite.render(m_SpriteRenderer);

          renderGameBoardUI(windowDimensions);
		}

		if (m_gameInfo.gameState == GameState::Main_Menu)
		{
		    m_mainMenuSprites.clear();
			m_mainMenuSprites.push_back(backgroundSprite);

		    for (auto& sprite : m_mainMenuSprites)
			    sprite.render(m_SpriteRenderer);
			
			renderMainMenuUI(windowDimensions);
		}
		
		return(Engine::Error::Ok);
	}

    Engine::Error GameProgram::onMousePress(int button, int action)
    {

	  return(Engine::Error::Ok);
    }

    Engine::Error GameProgram::onMouseMove(double positionX, double positionY)
    {
	  m_cursorPosition = {positionX, positionY};

	  return(Engine::Error::Ok);
    }

    void GameProgram::updateGameBoardSprites(glm::ivec2& windowDimensions)
    { 
	  vector<Card>& cards = m_gameBoard.getCards();
	  
	  // Create a card group based on the card owner.
	  auto search = [](std::vector<Card>& cards, CardOwner owner) -> std::vector<Card> {
		vector<Card> ownerGroup;
		
		// Iterate through each card and if the card owner mathces with the
		// requested owner add it to the result group(that contains only
		// owners that user specified).
		for(auto& card: cards)
		  if(card.cardOwner == owner)
			ownerGroup.push_back(card);
	    
		return(std::move(ownerGroup));
	  };	 
	  
	  // Create owner groups for each CardOwner
	  vector<Card> ownerHeap  = search(cards, CARD_OWNER_DECK);
	  vector<Card> ownerBoard = search(cards, CARD_OWNER_BOARD);
	  
	  auto renderPlayerCards = [&](glm::vec2 renderAreaStart, glm::vec2 renderAreaEnd, CardOwner cardOwner)
	  {
		auto ownerGroup     = search(cards, cardOwner);
	    auto ownerGroupSize = ownerGroup.size();
		
		// The offset between the cards so they can fit in the render area
		const float cardOffset = (renderAreaEnd.x - renderAreaStart.x) / ownerGroupSize;

		// Generate the sprite for each card in the card group.
	    for(size_t cardIndex=0; cardIndex < ownerGroupSize; ++cardIndex)
		{
		  Engine::GFX::Sprite playerCard;
	      playerCard.setSpritePosition({renderAreaStart.x + cardOffset*cardIndex, renderAreaStart.y});
	      playerCard.setSpriteSize    (CARD_ASSET_SIZE_NORMALIZED);
	      playerCard.bindTexture      (ownerGroup[cardIndex].textureHandleMain);
	 
		  m_gameBoardSprites.push_back(playerCard);		  
		}
	  };

	  glm::vec2 renderAreaStart, renderAreaEnd;

	  // Player 1
	  renderAreaStart.x = windowDimensions.x * 0.30f;
	  renderAreaStart.y = windowDimensions.y * 0.76;
	  renderAreaEnd.x   = windowDimensions.x * 0.70f;
	  renderAreaEnd.y   = renderAreaStart.y;
	  renderPlayerCards(renderAreaStart, renderAreaEnd, CARD_OWNER_PLAYER1);
	  
	  // Player 2
	  renderAreaStart.x = windowDimensions.x * 0.05f;
	  renderAreaStart.y = windowDimensions.y * CARDS_ROW_OTHER_PLAYERS_Y_COORD;
	  renderAreaEnd.x   = windowDimensions.x * 0.25f;
	  renderAreaEnd.y   = renderAreaStart.y;
	  renderPlayerCards(renderAreaStart, renderAreaEnd, CARD_OWNER_PLAYER2);

	  // Player 3
	  renderAreaStart.x = windowDimensions.x * 0.38f;
	  renderAreaStart.y = windowDimensions.y * CARDS_ROW_OTHER_PLAYERS_Y_COORD;
	  renderAreaEnd.x   = windowDimensions.x * 0.60f;
	  renderAreaEnd.y   = renderAreaStart.y;
	  renderPlayerCards(renderAreaStart, renderAreaEnd, CARD_OWNER_PLAYER3);

	  // Player 4
	  renderAreaStart.x = windowDimensions.x * 0.75f;
	  renderAreaStart.y = windowDimensions.y * CARDS_ROW_OTHER_PLAYERS_Y_COORD;
	  renderAreaEnd.x   = windowDimensions.x * 0.95f;
	  renderAreaEnd.y   = renderAreaStart.y;
	  renderPlayerCards(renderAreaStart, renderAreaEnd, CARD_OWNER_PLAYER4);

	  // Render the cards that are in the deck.
	  {
		auto ownerGroup     = search(cards, CARD_OWNER_DECK);\
		auto ownerGroupSize = ownerGroup.size();

		// The position of the deck is in middle right corner of
		// the screen.
		glm::vec2 deckPosition;
		deckPosition.x = windowDimensions.x * 0.85f;
		deckPosition.y = windowDimensions.y * 0.40f;
		
		// The idea is to render the deck like it is not aligned
		// perfectly(not each card stacked on each other). Like	  
		// there are some cards went out of the deck.
		if(ownerGroupSize >= 3)
		{
			Engine::GFX::Sprite veryTintedCard;
			veryTintedCard.setSpritePosition(deckPosition);
			veryTintedCard.setSpriteSize    (CARD_ASSET_SIZE_NORMALIZED);
			veryTintedCard.setSpriteRotation(25.0f);		
			veryTintedCard.bindTexture      (ownerGroup[2].textureHandleBack);
			
			m_gameBoardSprites.push_back(veryTintedCard);		  
		}
		
		if(ownerGroupSize >= 2)
		{
			Engine::GFX::Sprite slightlyTintedCard;
			slightlyTintedCard.setSpritePosition(deckPosition);
			slightlyTintedCard.setSpriteSize    (CARD_ASSET_SIZE_NORMALIZED);
			slightlyTintedCard.setSpriteRotation(15.0f);		
			slightlyTintedCard.bindTexture      (ownerGroup[1].textureHandleBack);		
			
			m_gameBoardSprites.push_back(slightlyTintedCard);	  
		}
		
		if(ownerGroupSize >= 1)
		{
			Engine::GFX::Sprite regularCard;
			regularCard.setSpritePosition(deckPosition);
			regularCard.setSpriteSize    (CARD_ASSET_SIZE_NORMALIZED);
			regularCard.setSpriteRotation(3.0f);		
			regularCard.bindTexture      (ownerGroup[0].textureHandleBack);
			
			m_gameBoardSprites.push_back(regularCard);
		}
	  }
	  	  
	  // Render the cards that are in the heap.

	  // Render the cards that are on the board.
    }

    void GameProgram::renderGameBoardUI(glm::ivec2& windowDimensions)
    {
	  ImguiCreateNewFrameKHR();
	  ImGui::NewFrame(); 
	  
	  ImGui::EndFrame();
	  ImGui::Render();
    }

    void GameProgram::renderMainMenuUI(glm::ivec2& windowDimensions)
    {
			ImguiCreateNewFrameKHR();
			ImGui::NewFrame(); 

			ImGuiWindowFlags window_flags = 0;
			window_flags |= ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoResize;
			window_flags |= ImGuiWindowFlags_NoCollapse;

			ImGui::SetNextWindowPos(
				ImVec2(windowDimensions.x/3-100, windowDimensions.y/2-100),
				ImGuiCond_FirstUseEver);
			ImGui::Begin("Main Menu", NULL, window_flags);
			
			if (ImGui::Button("\t\t\tNew Game\t\t\t"))
			  m_gameInfo.gameState = GameState::Game_Board; 
			
			ImGui::SameLine();

			if (ImGui::Button("\t\t\tContinue\t\t\t"))
				(void)0;

			ImGui::SameLine();

			if (ImGui::Button("\t\t\tSettings\t\t\t"))
				m_showSettingsWindow = !m_showSettingsWindow;

			ImGui::SameLine();

			if (ImGui::Button("\t\t\tQuit\t\t\t"))
				m_showQuitApproveWindow = !m_showQuitApproveWindow;
			
			ImGui::Separator();
			ImGui::Text(GAME_DESCRIPTION);
			ImGui::End();
			
			if (m_showQuitApproveWindow)
			{
				window_flags  = 0;
				window_flags |= ImGuiWindowFlags_NoResize;
				window_flags |= ImGuiWindowFlags_NoCollapse;

				ImGui::SetNextWindowPos(ImVec2(windowDimensions.x / 2.5, windowDimensions.y / 3.5), ImGuiCond_FirstUseEver);

				if (!ImGui::Begin("Do you really want to quit?", &m_showQuitApproveWindow, window_flags))
				{			
					ImGui::End();
				}
				else
				{
					if (ImGui::Button("\t\t\tYes\t\t\t"))
					{
						glfwSetWindowShouldClose(getWindowPointer(), true);
					}

					ImGui::SameLine();

					if (ImGui::Button("\t\tNo\t\t\t"))
					{
						m_showQuitApproveWindow = false;
					}

					ImGui::End();
				}
			}

			if (m_showSettingsWindow)
			{
				window_flags = 0;
				window_flags |= ImGuiWindowFlags_NoCollapse;

				ImGui::SetNextWindowPos(ImVec2(windowDimensions.x / 4, windowDimensions.y / 4), ImGuiCond_FirstUseEver);

				if (!ImGui::Begin("Settings", &m_showSettingsWindow, window_flags))
				{
					ImGui::End();
				}
				else
				{
					if (ImGui::CollapsingHeader("Game Rules"))
					{
						ImGui::Text(GAME_DESCRIPTION);
					}

					if (ImGui::CollapsingHeader("Game specification"))
					{
						ImGui::Text(GAME_SPECIFICATION);
					}

					if (ImGui::CollapsingHeader("ImGui User Manual"))
					{
						ImGui::ShowUserGuide();
					}

					if (ImGui::CollapsingHeader("Author Credits"))
					{
						ImGui::Text(GAME_CREDITS);
					}

					ImGui::Separator();

					if (ImGui::BeginMenu("Developer Tools"))
					{
						ImGui::MenuItem("Show Debug Window",     NULL, &m_showDebugWindow);
						ImGui::MenuItem("Show Enemy Card Faces", NULL, &m_openCardsMode);

						ImGui::EndMenu();
					}

					ImGui::End();
				}
			}

			ImGui::EndFrame();
			ImGui::Render();
    }
}
