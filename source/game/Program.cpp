﻿#include "Program.hpp"

#include "../engine/Sprite.hpp"

#include <iostream>

#define isValidShader(shader) shader.has_value()

#define SPRITE_APPLY_NONE_EFFECTS         0
#define SPRITE_APPLY_HOVER_GOOD_EFFECT    1
#define SPRITE_APPLY_HOVER_BAD_EFFECT     2
#define SPRITE_APPLY_MOTION_BLUR_EFFECT   4

using namespace std;
using namespace glm;
using namespace Engine;
using namespace Engine::GFX;

static constexpr const glm::vec2  CARD_ASSET_RATIO                = {1.f,    1.5f};
static constexpr const glm::vec2  CARD_ASSET_SIZE_NON_NORMALIZED  = {150.0f, 150.0f};
static constexpr const glm::ivec2 CARD_ASSET_SIZE_NORMALIZED      = CARD_ASSET_SIZE_NON_NORMALIZED*CARD_ASSET_RATIO;
static constexpr const float      CARDS_ROW_OTHER_PLAYERS_Y_COORD = 0.02f;

static constexpr const vec3  CARD_INTENCITY_MASK_BAD   = {0.9, 0.8, 0.8};
static constexpr const vec3  CARD_INTENCITY_MASK_GOOD  = {0.8, 0.9, 0.8};

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

#include <iostream>

namespace Game
{
	Engine::Error GameProgram::onUserInitialize()
	{
        auto windowDimensions  = getWindowDimensions();

		// Load game background texture
  		ResourceManager::loadTexture("data/assets/background.jpg", false, "background");
		
        // Load the card reverse side textures.
	    ResourceManager::loadTexture("data/assets/card-back1.png", true, "card-back-blue");
        ResourceManager::loadTexture("data/assets/card-back2.png", true, "card-back-red");
        ResourceManager::loadTexture("data/assets/card-back3.png", true, "card-back-green");
	    ResourceManager::loadTexture("data/assets/card-back4.png", true, "card-back-yellow");	
		
		// Create and set sprite for the background.
		Sprite backgroundSprite;
		backgroundSprite .setSpriteSize({ windowDimensions.x, windowDimensions.y });
		backgroundSprite .bindTexture  ("background");

		m_mainMenuSprites .push_back(backgroundSprite);
        m_gameBoardGeneral.push_back(backgroundSprite);
		
		// Set up game board
		m_gameBoard.generateDeck();
		m_gameBoardPendingUpdate = true;

		// The entry point of the game is main menu.
		m_gameInfo.gameState = GameState::Main_Menu;
		
		// Generate the ares where the player cards are rendering
		calculateRenderAreas();

		return(Error::Ok);
	}

	Error GameProgram::onUserRelease()
	{
		return(Error::Ok);
	}

	Error GameProgram::onUserUpdate(GLfloat elapsedTime)
	{
	    auto windowDimensions = getWindowDimensions();
        
		if (m_escapeButtonPressed == true)
			m_showBoardMenuWindow = true;

        // Update mouse cursor position on the current frame.
        glfwGetCursorPos(getWindowPointer(), &m_mousePositionX, &m_mousePositionY);
		
		switch (m_gameInfo.gameState)
		{
			case GameState::Main_Menu:
			{
				for (auto& sprite : m_mainMenuSprites)
					sprite.render(m_SpriteRenderer);

				 renderMainMenuUI(windowDimensions);
			} break;

			case GameState::Game_Board:
			{
				if (!m_gameBoard.isEnded())
				{
					bool waitAnimations = false;

					for (auto& animatedSprite : m_gameBoardCards)
					{
						if (animatedSprite.getIsAnimated())
						{
							waitAnimations = true;
							break;
						}
					}

					if (!waitAnimations)
					{
						if (m_gameBoard.getDeck().size() > 0)
							m_lastCardCopy = m_gameBoard.getDeck().back();

						m_gameBoard.step();

						// Arrange the cards sprites(split the cards array by the different rendering ares by the card owner).
						updateGameBoardCardSprites(windowDimensions);
					}

					// Animate individual sprite that is on gameboard group.
					for (auto& sprite : m_gameBoardGeneral)
					{
						sprite.render(m_SpriteRenderer);
					}

					// Find all sprites that are animating and render each of them
					vector<AnimatedSprite> animatedCards;

					for (ptrdiff_t spriteIndex = 0; spriteIndex < m_gameBoardCards.size(); spriteIndex++)
					{
						if (m_gameBoardCards[spriteIndex].getIsAnimated())
						{
							animatedCards.push_back(m_gameBoardCards[spriteIndex]);
						}
					}

					if (m_gameBoard.getDeck().size() > 0)
					{
						// Render the last card that is in the deck
						AnimatedSprite lastBoardCard;
						lastBoardCard.setSpriteSize(CARD_ASSET_SIZE_NORMALIZED);
						lastBoardCard.bindTexture(m_lastCardCopy.textureHandleMain);
						lastBoardCard.setMoveSpeed({ 1000.0f, 1000.0f});
						lastBoardCard.setRenderFlag(SPRITE_APPLY_NONE_EFFECTS);
						lastBoardCard.setSpritePosition(m_boardPosition);
						lastBoardCard.move(m_boardPosition);

						// The last rendered sprite is now the last deck sprite
						animatedCards.push_back(lastBoardCard);

						if (((int)animatedCards.size() - 2) >= 0)
							iter_swap(animatedCards.rbegin(), animatedCards.rbegin() + 1);
					}

					renderSpriteGroup(m_gameBoardCards);
					renderSpriteGroup(animatedCards);
				}
				else
				{
					m_showScoreBoardMenu = true;

					// Animate individual sprite that is on gameboard group.
					for (auto& sprite : m_gameBoardGeneral)
					{
						sprite.render(m_SpriteRenderer);
					}

					m_gameBoard.calculatePlayerScore();
					renderFinalUI(m_gameBoard.getPlayerScore());
				}
			} break;
		}
		
		return(Error::Ok);
	}

	void GameProgram::renderSpriteGroup(vector<AnimatedSprite>& spriteGroup)
	{
		auto windowDimensions = getWindowDimensions();

		// Try to get the shaders from the global shader/texture storage, if the get request fails
		// exit the program.
		auto shaderWrapperOrError = Engine::ResourceManager::getShader("spriteShader");
		if (shaderWrapperOrError.has_value())
		{
			(*shaderWrapperOrError).setFloat   ("elapsedTime", glfwGetTime() * 6);
			(*shaderWrapperOrError).setVector2f("screenResolution", (ivec2)Engine::Window::instance().getWindowDimensionsKHR());
		}

		const auto spriteGroupSize = m_gameBoardCards.size();
		m_hoveredCardCopy.cardRank = CardRankLast;

		for (auto& sprite : spriteGroup) {
			sprite.animate(m_elapsedTime);

			const bool applyBadEffect  = (sprite.getRenderFlag()  & SPRITE_APPLY_HOVER_BAD_EFFECT)   == SPRITE_APPLY_HOVER_BAD_EFFECT;
			const bool applyGoodEffect = (sprite.getRenderFlag() & SPRITE_APPLY_HOVER_GOOD_EFFECT)  == SPRITE_APPLY_HOVER_GOOD_EFFECT;
			const bool applyBlurEffect = (sprite.getRenderFlag() & SPRITE_APPLY_MOTION_BLUR_EFFECT) == SPRITE_APPLY_MOTION_BLUR_EFFECT;

			const auto   size     = sprite.getSpriteSize();
			const auto   color    = sprite.getSpriteColor();
			const auto   position = sprite.getSpritePosition();
			const auto   rotation = sprite.getSpriteRotation();

			// Apply shadows
			AnimatedSprite shadowSprite;
			shadowSprite.setSpriteSize    (size);
			shadowSprite.setMoveSpeed     (sprite.getMoveSpeed());
			shadowSprite.setSpriteRotation(sprite.getSpriteRotation());
			shadowSprite.bindTexture      (sprite.getBindedTexture());
			shadowSprite.setSpritePosition({ position.x + size.x / 14, position.y + size.y / 14 });

			(*shaderWrapperOrError).setInteger("applyShadowEffect", 1);
			shadowSprite.render(m_SpriteRenderer);
			(*shaderWrapperOrError).setInteger("applyShadowEffect", 0);

			if (applyBlurEffect) // Apply motion blur
			{
				(*shaderWrapperOrError).setInteger("applyGlowingEffect", 0, true);
				const float offsetX = 2.6f;
				const float offsetY = 2.6f;

				sprite.render(m_SpriteRenderer);

				(*shaderWrapperOrError).setInteger("applyMotionEffect", 1, true);
				for (auto x = 0; x < 3; ++x)
				{
					for (auto y = 0; y < 3; ++y)
					{
						AnimatedSprite spriteCopy;
						spriteCopy.setSpriteColor({ color.x, color.y, color.z });
						spriteCopy.setSpriteRotation(rotation);
						spriteCopy.setSpriteSize(size);
						spriteCopy.bindTexture(sprite.getBindedTexture());

						spriteCopy.setSpritePosition({ position.x + offsetX * x, position.y + offsetY * y });
						spriteCopy.render(m_SpriteRenderer);
						spriteCopy.setSpritePosition({ position.x - offsetX * x, position.y - offsetY * y });
						spriteCopy.render(m_SpriteRenderer);
					}
				}
				(*shaderWrapperOrError).setInteger("applyMotionEffect", 0, true);
			}
			else if (applyBadEffect || applyGoodEffect) // Apply glowing effect
			{
				(*shaderWrapperOrError).setVector3f("intencityMask", applyBadEffect ? CARD_INTENCITY_MASK_BAD : CARD_INTENCITY_MASK_GOOD);
				(*shaderWrapperOrError).setInteger("applyGlowingEffect", 1, true);

				sprite.render(m_SpriteRenderer);
			}
			else
			{
				sprite.render(m_SpriteRenderer);
			}

			(*shaderWrapperOrError).setInteger("applyGlowingEffect", 0, true);
			(*shaderWrapperOrError).setInteger("applyBlurEffect", 0, true);
		}

		renderGameBoardUI(windowDimensions);
	}

	pair<vec2, vec2> GameProgram::getRenderAreaBasedOnCardOwner(CardOwner cardOwner)
	{
		switch (cardOwner)
		{
		case CARD_OWNER_PLAYER1:
			return make_pair(m_playerRenderAreaStart1, m_playerRenderAreaEnd1);
		case CARD_OWNER_PLAYER2:
			return make_pair(m_playerRenderAreaStart2, m_playerRenderAreaEnd2);
		case CARD_OWNER_PLAYER3:
			return make_pair(m_playerRenderAreaStart3, m_playerRenderAreaEnd3);
		case CARD_OWNER_PLAYER4:
			return make_pair(m_playerRenderAreaStart4, m_playerRenderAreaEnd4);
		};
	}

	void GameProgram::calculateRenderAreas()
	{
		auto  windowDimensions = getWindowDimensions();
		auto& cards            = m_gameBoard.getCards();

		// Player 1
		m_playerRenderAreaStart1.x = windowDimensions.x * 0.30f;
		m_playerRenderAreaStart1.y = windowDimensions.y * 0.76;
		m_playerRenderAreaEnd1.x   = windowDimensions.x * 0.70f;
		m_playerRenderAreaEnd1.y   = m_playerRenderAreaStart1.y;
		
		// Player 2
		m_playerRenderAreaStart2.x = windowDimensions.x * 0.05f;
		m_playerRenderAreaStart2.y = windowDimensions.y * CARDS_ROW_OTHER_PLAYERS_Y_COORD;
		m_playerRenderAreaEnd2.x   = windowDimensions.x * 0.25f;
		m_playerRenderAreaEnd2.y   = m_playerRenderAreaStart2.y;

		// Player 3
		m_playerRenderAreaStart3.x = windowDimensions.x * 0.38f;
		m_playerRenderAreaStart3.y = windowDimensions.y * CARDS_ROW_OTHER_PLAYERS_Y_COORD;
		m_playerRenderAreaEnd3.x   = windowDimensions.x * 0.60f;
		m_playerRenderAreaEnd3.y   = m_playerRenderAreaStart3.y;

		// Player 4
		m_playerRenderAreaStart4.x = windowDimensions.x * 0.75f;
		m_playerRenderAreaStart4.y = windowDimensions.y * CARDS_ROW_OTHER_PLAYERS_Y_COORD;
		m_playerRenderAreaEnd4.x   = windowDimensions.x * 0.95f;
		m_playerRenderAreaEnd4.y   = m_playerRenderAreaStart4.y;

		// Board
		m_boardPosition.x = windowDimensions.x * 0.4f;
		m_boardPosition.y = windowDimensions.y * 0.4f;

		// Deck
		m_deckPosition.x = windowDimensions.x * 0.85f;
		m_deckPosition.y = windowDimensions.y * 0.40f;
	}

	void GameProgram::arrangePlayerSprite(CardOwner cardOwner)
	{
		auto ownerGroup     = searchCard(cardOwner);
		auto ownerGroupSize = ownerGroup.size();

		// Generate the sprite for each card in the card group.
		for (size_t cardIndex = 0; cardIndex < ownerGroupSize; ++cardIndex)
		{
			AnimatedSprite playerCard;
			playerCard.setSpriteSize(CARD_ASSET_SIZE_NORMALIZED);
			playerCard.setMoveSpeed ({ 450.0f, 450.0f });

			// If the card previous position was in the deck
			auto& rewindedCard = m_gameBoard.getCardRef(ownerGroup[cardIndex].cardSuit, ownerGroup[cardIndex].cardRank, true);

			if (rewindedCard.cardOwner == CARD_OWNER_DECK)
			{
				playerCard.setSpritePosition(m_deckPosition);
			}
			else if (rewindedCard.cardOwner == CARD_OWNER_BOARD)
			{
				playerCard.setSpritePosition(m_boardPosition);
			}
			else
			{
				auto renderAreaRewind      = getRenderAreaBasedOnCardOwner(rewindedCard.cardOwner);
				auto renderAreaRewindStart = renderAreaRewind.first;
				auto renderAreaRewindEnd   = renderAreaRewind.second;

				playerCard.setSpritePosition({
					renderAreaRewindStart.x + ((renderAreaRewindEnd.x - renderAreaRewindStart.x) / ownerGroupSize) * cardIndex,
					renderAreaRewindStart.y
				});
			}

			// Set the current position relating on the current card owner
			auto renderArea      = getRenderAreaBasedOnCardOwner(ownerGroup[cardIndex].cardOwner);
			auto renderAreaStart = renderArea.first;
			auto renderAreaEnd   = renderArea.second;

			playerCard.move({
				renderAreaStart.x + ((renderAreaEnd.x - renderAreaStart.x) / ownerGroupSize) * cardIndex,
				renderAreaStart.y
			});
			
			// Hide card faces of the opponents
			if ((cardOwner == CARD_OWNER_PLAYER1 || cardOwner == CARD_OWNER_BOARD) || m_openCardsMode)
				playerCard.bindTexture(ownerGroup[cardIndex].textureHandleMain);
			else
				playerCard.bindTexture(ownerGroup[cardIndex].textureHandleBack);

			if (playerCard.getIsAnimated())
				playerCard.setRenderFlag(playerCard.getRenderFlag() | SPRITE_APPLY_MOTION_BLUR_EFFECT);

			// Main player card controls
			if (cardOwner == CARD_OWNER_PLAYER1)
			{
				// If the current player is deliverer, track its move
				if (m_gameBoard.getDeliverer() == CARD_OWNER_PLAYER1)
				{
					if (playerCard.isHovered({ m_mousePositionX, m_mousePositionY }, { 1.0f, 1.0f }))
					{
						if (m_gameBoard.moveIsValid(ownerGroup[cardIndex]))
						{
							playerCard.setRenderFlag(SPRITE_APPLY_HOVER_GOOD_EFFECT);

							if (m_mouseButtonPressed)
							{
								m_gameBoard.move(ownerGroup[cardIndex]);
								playerCard.move(m_boardPosition);
							}
						}
						else
						{
							playerCard.setRenderFlag(SPRITE_APPLY_HOVER_BAD_EFFECT);
						}
					}
					else
					{
						playerCard.setRenderFlag(SPRITE_APPLY_NONE_EFFECTS);
					}
				}
				else
				{
					playerCard.setRenderFlag(SPRITE_APPLY_NONE_EFFECTS);
				}
			}
			else
			{
				playerCard.setRenderFlag(SPRITE_APPLY_NONE_EFFECTS);
			}

			m_gameBoardCards   .push_back(playerCard);
			m_gameBoardCardsRef.push_back(ownerGroup[cardIndex]);
		}
	}

	vector<Card> GameProgram::searchCard(CardOwner owner, bool rewind)
	{
		vector<Card> ownerGroup;

		// Iterate through each card and if the card owner matches with the
		// requested owner add it to the result group(that contains only
		// owners that user specified).
		for (auto& card : m_gameBoard.getCards())
			if (card.cardOwner == owner)
				ownerGroup.push_back(card);

		return(move(ownerGroup));
	}


	void GameProgram::arrangeDeckSprites()
	{
		auto deckOwnerGroup     = searchCard(CARD_OWNER_DECK);
		auto deckOwnerGroupSize = deckOwnerGroup.size();

		// The idea is to render the deck like it is not aligned
		// perfectly(not each card stacked on each other). Like	  
		// there are some cards went out of the deck.
		if (deckOwnerGroupSize >= 3)
		{
			AnimatedSprite veryTintedCard;
			veryTintedCard.setSpritePosition(m_deckPosition);
			veryTintedCard.setSpriteSize(CARD_ASSET_SIZE_NORMALIZED);
			veryTintedCard.setSpriteRotation(25.0f);
			veryTintedCard.bindTexture(deckOwnerGroup[2].textureHandleBack);
			veryTintedCard.move(m_deckPosition); // No move, the sprite is static.
			veryTintedCard.setRenderFlag(SPRITE_APPLY_NONE_EFFECTS);

			m_gameBoardCards.push_back(veryTintedCard);
		}

		if (deckOwnerGroupSize >= 2)
		{
			AnimatedSprite slightlyTintedCard;
			slightlyTintedCard.setSpritePosition(m_deckPosition);
			slightlyTintedCard.setSpriteSize(CARD_ASSET_SIZE_NORMALIZED);
			slightlyTintedCard.setSpriteRotation(15.0f);
			slightlyTintedCard.bindTexture(deckOwnerGroup[1].textureHandleBack);
			slightlyTintedCard.move(m_deckPosition); // No move, the sprite is static.			
			slightlyTintedCard.setRenderFlag(SPRITE_APPLY_NONE_EFFECTS);

			m_gameBoardCards.push_back(slightlyTintedCard);
		}

		if (deckOwnerGroupSize >= 1)
		{
			AnimatedSprite regularCard;
			regularCard.setSpritePosition(m_deckPosition);
			regularCard.setSpriteSize(CARD_ASSET_SIZE_NORMALIZED);
			regularCard.setSpriteRotation(3.0f);
			regularCard.bindTexture(deckOwnerGroup[0].textureHandleBack);
			regularCard.move(m_deckPosition); // No move, the sprite is static.			
			regularCard.setRenderFlag(SPRITE_APPLY_NONE_EFFECTS);

			m_gameBoardCards.push_back(regularCard);
		}

	}

	void GameProgram::arrangeBoardSprites()
	{
		vector<AnimatedSprite> animatedSprites;

		auto      boardOwnerGroup         = searchCard(CARD_OWNER_BOARD);
		auto      boardOwnerGroupSize     = boardOwnerGroup.size();
		ptrdiff_t cardsRotatedRenderTotal = 0;

		bool somethingIsAnimating = false;

		for (ptrdiff_t boardSpriteIndex = boardOwnerGroupSize; boardSpriteIndex--> 0;)
		{
			// Card previous position in the deck
			auto& rewindedCard = m_gameBoard.getCardRef(boardOwnerGroup[boardSpriteIndex].cardSuit, boardOwnerGroup[boardSpriteIndex].cardRank, true);

			AnimatedSprite boardCard;
			boardCard.setSpriteSize(CARD_ASSET_SIZE_NORMALIZED);
			boardCard.bindTexture(boardOwnerGroup[boardSpriteIndex].textureHandleMain);
			boardCard.setMoveSpeed({ 450.0f, 450.0f });
			boardCard.setRenderFlag(SPRITE_APPLY_NONE_EFFECTS);

			if (rewindedCard.cardOwner == CARD_OWNER_BOARD)
			{
				// Render the slightly rotated cards so it is like
				// that the deck is fullfilling.
				if (cardsRotatedRenderTotal <= 3)
				{
					boardCard.setSpritePosition(m_boardPosition);
					boardCard.setSpriteRotation(3.2f * cardsRotatedRenderTotal);
					++cardsRotatedRenderTotal;
				}
				else
				{
					continue;
				}
				
			}
			else if(rewindedCard.cardOwner != CARD_OWNER_PLAYER1)
			{
				auto renderAreaRewind      = getRenderAreaBasedOnCardOwner(rewindedCard.cardOwner);
				auto renderAreaRewindStart = renderAreaRewind.first;
				auto renderAreaRewindEnd   = renderAreaRewind.second;

				boardCard.setSpritePosition({
					renderAreaRewindStart.x + ((renderAreaRewindEnd.x - renderAreaRewindStart.x) / boardOwnerGroupSize),
					renderAreaRewindStart.y
				});
			}
			else
			{
				continue;
			}

			boardCard.move   (m_boardPosition);
			boardCard.animate(m_elapsedTime);

			if (boardCard.getIsAnimated())
			{			
				animatedSprites.push_back(boardCard);
			}
			else
			{
				m_gameBoardCards.push_back(boardCard);
			}
		}

		for(auto sprite: animatedSprites)
			m_gameBoardCards.push_back(sprite);
	}

	void GameProgram::renderFinalUI(PlayerScore playerScores)
	{
		ImguiCreateNewFrameKHR();
		ImGui::NewFrame();

		ImGuiWindowFlags windowFlags = 0;
		//windowFlags |= ImGuiWindowFlags_NoMove;
		//windowFlags |= ImGuiWindowFlags_NoResize;
		windowFlags |= ImGuiWindowFlags_NoCollapse;

		if (m_showScoreBoardMenu)
		{
			if (!ImGui::Begin("Player Score", NULL, windowFlags))
			{
				ImGui::End();
			}
			else
			{
				auto tableColor       = 3;
				auto tableColorBorder = 11;

				ImGui::Text("So the game ended, and it is time to find out who is the winner...");
				ImGui::Text("\nThe player with the least number of points considered the winner if there more than one player with the same points number ... there's a tie(multiple winners)");

				ImGui::Text("\n\n Player Statics:\n");
				ImGui::BeginTable("Player scores", tableColor, tableColorBorder);
				ImGui::TableSetupColumn("Player");
				ImGui::TableSetupColumn("Score");
				ImGui::TableSetupColumn("Winner?");
				
				ImGui::TableHeadersRow();
				ImGui::TableNextColumn();

				ImGui::Text("Player 1");
				ImGui::TableNextColumn();
				ImGui::Text(to_string(playerScores.Player1).c_str());
				ImGui::TableNextColumn();
				ImGui::Text(playerScores.WinnerIndex == playerScores.Player1 ? "YES" : "NO");
				ImGui::TableNextColumn();

				ImGui::Text("Player 2");
				ImGui::TableNextColumn();
				ImGui::Text(to_string(playerScores.Player2).c_str());
				ImGui::TableNextColumn();
				ImGui::Text(playerScores.WinnerIndex == playerScores.Player2 ? "YES" : "NO");
				ImGui::TableNextColumn();

				ImGui::Text("Player 3");
				ImGui::TableNextColumn();
				ImGui::Text(to_string(playerScores.Player3).c_str());
				ImGui::TableNextColumn();
				ImGui::Text(playerScores.WinnerIndex == playerScores.Player3  ? "YES" : "NO");
				ImGui::TableNextColumn();

				ImGui::Text("Player 4");
				ImGui::TableNextColumn();
				ImGui::Text(to_string(playerScores.Player4).c_str());
				ImGui::TableNextColumn();
				ImGui::Text(playerScores.WinnerIndex == playerScores.Player4 ? "YES" : "NO");
				ImGui::TableNextColumn();
				ImGui::EndTable();
				ImGui::Text("\n\n\t\t\t\tThanks for playing!!");

				ImGui::End();
			}
		}

		ImGui::EndFrame();
		ImGui::Render();
	}

    void GameProgram::updateGameBoardCardSprites(ivec2& windowDimensions)
    { 
      // Clear the previous sprites.
      m_gameBoardCards.clear();
	 
	  // Adjust the sprite positions for the each player
	  arrangePlayerSprite(CARD_OWNER_PLAYER1);
	  arrangePlayerSprite(CARD_OWNER_PLAYER2);
	  arrangePlayerSprite(CARD_OWNER_PLAYER3);
	  arrangePlayerSprite(CARD_OWNER_PLAYER4);

	  // Adjust the sprite positions for the different game rendering areas
	  arrangeDeckSprites();
	  arrangeBoardSprites();
    }

    void GameProgram::renderPlayerStatUI(CardOwner owner)
    {
      ImGuiWindowFlags windowFlags = 0;
      windowFlags |= ImGuiWindowFlags_NoMove;
      windowFlags |= ImGuiWindowFlags_NoResize;
      windowFlags |= ImGuiWindowFlags_NoCollapse;

      if(!ImGui::Begin("Player Statistics", NULL, windowFlags))
      {
        ImGui::End();
      }
      else
      {
        ImGui::End();
      }
    }

    void GameProgram::renderGameBoardUI(ivec2& windowDimensions)
    {
	  ImguiCreateNewFrameKHR();
	  ImGui::NewFrame(); 
	  
	  ImGuiWindowFlags window_flags = 0;
	  window_flags |= ImGuiWindowFlags_NoResize;
	  window_flags |= ImGuiWindowFlags_NoCollapse;
	  window_flags |= ImGuiWindowFlags_NoMove;

	  if (m_showBoardMenuWindow)
	  {
		  if (!ImGui::Begin("101 Menu", &m_showBoardMenuWindow, window_flags))
		  {
			  ImGui::End();
		  }
		  else
		  {
			  if (ImGui::Button("\t\t\tContinue\t\t\t"))
			  {
				  m_showSettingsWindow = !m_showSettingsWindow;
			  } ImGui::SameLine();

			  
			  if (ImGui::Button("\t\t\tSave\t\t\t"))
			  {
				  
			  } ImGui::SameLine();

			  if (ImGui::Button("\t\t\tSettings\t\t\t"))
			  {
				  m_showSettingsWindow = true;
			  } ImGui::SameLine();

			  if (ImGui::Button("\t\t\tQuit\t\t\t"))
			  {
				  m_showQuitApproveWindow = !m_showQuitApproveWindow;
			  } ImGui::SameLine();

			  ImGui::End();
		  }
	  }

	  renderSettingsUI();
	  renderQuitApproveUI();

	  ImGui::EndFrame();
	  ImGui::Render();
    }

	void GameProgram::renderQuitApproveUI()
	{
		if (m_showQuitApproveWindow)
		{
			auto windowDimensions = getWindowDimensions();

			ImGuiWindowFlags window_flags = 0;
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
	}

	void GameProgram::renderSettingsUI()
	{
		if (m_showSettingsWindow)
		{
			auto windowDimensions = getWindowDimensions();

			ImGuiWindowFlags window_flags = 0;
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
					ImGui::MenuItem("Show Debug Window", NULL, &m_showDebugWindow);
					ImGui::MenuItem("Show Enemy Card Faces", NULL, &m_openCardsMode);

					ImGui::EndMenu();
				}

				ImGui::End();
			}
		}
	}

    void GameProgram::renderMainMenuUI(ivec2& windowDimensions)
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
			
			renderQuitApproveUI();
			renderSettingsUI();
            
			ImGui::EndFrame();
			ImGui::Render();
    }
}
