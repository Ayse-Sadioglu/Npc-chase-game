#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>

// vec2: simple integer 2d coordinate for grid cells
struct Vec2 { int x=0, y=0; };

static int sign(int v){ return (v>0)-(v<0); }

// simple game states
enum class GameState { Playing, GameOver };

int main() {
    // grid and tile sizes
    const int W = 20, H = 20;
    const int TILE = 32;

    // hud height at the top 
    const int HUD_H = 40;

    // window size = grid area + hud
    const int WIN_W = W * TILE;
    const int WIN_H = H * TILE + HUD_H;

    // open window
    sf::RenderWindow window(sf::VideoMode(WIN_W, WIN_H), "NPC Chase");
    window.setFramerateLimit(60);

    // game data
    Vec2 player{2,2};
    Vec2 enemy{15,15};
    int ticks = 0;
    GameState state = GameState::Playing;

    // shapes for grid and entities
    sf::RectangleShape tile(sf::Vector2f((float)TILE-1, (float)TILE-1));
    sf::RectangleShape rectP(sf::Vector2f((float)TILE-4, (float)TILE-4));
    sf::RectangleShape rectE(sf::Vector2f((float)TILE-4, (float)TILE-4));
    rectP.setFillColor(sf::Color(80,200,120));   // player color
    rectE.setFillColor(sf::Color(220,80,80));    // enemy color

    // overlay box shown on game over 
    sf::RectangleShape overlay(sf::Vector2f((float)W*TILE, 120.f));
    overlay.setFillColor(sf::Color(0,0,0,160));
    overlay.setOutlineColor(sf::Color(200,200,220));
    overlay.setOutlineThickness(2.f);

    // font + texts
    sf::Font font;
    bool hasFont =
        font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") ||
        font.loadFromFile("DejaVuSans.ttf") ||
        font.loadFromFile("arial.ttf"); // last resort if user provides it

    sf::Text hud, overTitle, overHint;
    if (hasFont) {
        hud.setFont(font);
        hud.setCharacterSize(18);
        hud.setFillColor(sf::Color::White);

        overTitle.setFont(font);
        overTitle.setCharacterSize(28);
        overTitle.setFillColor(sf::Color(255,200,200));

        overHint.setFont(font);
        overHint.setCharacterSize(18);
        overHint.setFillColor(sf::Color(230,230,230));
    }

    // helper to reset game
    auto resetGame = [&](){
        player = {2,2};
        enemy  = {15,15};
        ticks = 0;
        state = GameState::Playing;
    };

    // main loop
    while (window.isOpen()) {
        // ---- events ----
        sf::Event e;
        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed) window.close();

            // key presses that should happen once per press
            if (e.type == sf::Event::KeyPressed) {
                if (e.key.code == sf::Keyboard::Q) {
                    // q quits from anywhere
                    window.close();
                }
                // retry on 'r' or 'enter' when game over
                if (state == GameState::GameOver &&
                    (e.key.code == sf::Keyboard::R || e.key.code == sf::Keyboard::Enter)) {
                    resetGame();
                }
            }

            // mouse click on overlay also retries (only in game over)
            if (state == GameState::GameOver && e.type == sf::Event::MouseButtonPressed) {
                resetGame();
            }
        }

        // ---- update (only while playing) ----
        if (state == GameState::Playing) {//if state is playing
            // continuous movement with wasd (1 cell per check, bounded to grid)
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) player.y = std::max(0, player.y-1);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) player.y = std::min(H-1, player.y+1);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) player.x = std::max(0, player.x-1);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) player.x = std::min(W-1, player.x+1);

            // enemy ai: simple approach behavior towards the player
            Vec2 diff{ player.x - enemy.x, player.y - enemy.y };
            if (std::abs(diff.x) > std::abs(diff.y)) {
                enemy.x += sign(diff.x);
            } else if (std::abs(diff.y) > 0) {
                enemy.y += sign(diff.y);
            }

            // collision switch to game over
            if (player.x == enemy.x && player.y == enemy.y) {
                state = GameState::GameOver;
            }

            ++ticks; // count frames only while playing
        }

        // ---- render ----
        window.clear(sf::Color(20,20,25));

        // draw checkerboard grid
        for (int y=0; y<H; ++y) {
            for (int x=0; x<W; ++x) {
                tile.setPosition((float)x*TILE, (float)y*TILE + (float)HUD_H);
                bool alt = ((x + y) % 2 == 0);
                tile.setFillColor( alt ? sf::Color(35,35,45) : sf::Color(45,45,60) );
                window.draw(tile);
            }
        }

        // draw player and enemy (small offset so they look centered)
        rectP.setPosition(player.x*TILE + 2.f, player.y*TILE + HUD_H + 2.f);
        rectE.setPosition(enemy.x*TILE  + 2.f, enemy.y*TILE  + HUD_H + 2.f);
        window.draw(rectP);
        window.draw(rectE);

        // top hud text
        if (hasFont) {
            hud.setString("MOVE: WASD --- GOOD LUCK  |   ticks: " + std::to_string(ticks) +
                          (state==GameState::GameOver ? "   |   game over" : ""));
            hud.setPosition(8.f, 8.f);
            window.draw(hud);
        }

        // game over overlay with centered texts
        if (state == GameState::GameOver) {
            // center overlay vertically over the grid area
            overlay.setPosition(0.f, (float)HUD_H + (H*TILE - 120.f)*0.5f);
            window.draw(overlay);

            if (hasFont) {
                // center title text
                overTitle.setString("GAME OVER!!!");
                auto r1 = overTitle.getLocalBounds();
                overTitle.setOrigin(r1.left + r1.width/2.f, r1.top + r1.height/2.f);
                overTitle.setPosition((W*TILE)/2.f, overlay.getPosition().y + 38.f);
                window.draw(overTitle);

                // center hint text
                overHint.setString("press 'r' to retry, 'q' to quit");
                auto r2 = overHint.getLocalBounds();
                overHint.setOrigin(r2.left + r2.width/2.f, r2.top + r2.height/2.f);
                overHint.setPosition((W*TILE)/2.f, overlay.getPosition().y + 82.f);
                window.draw(overHint);
            }
        }

        window.display();

        // small delay so movement is not too fast when holding a key
        sf::sleep(sf::milliseconds(90));
    }

    return 0;
}
