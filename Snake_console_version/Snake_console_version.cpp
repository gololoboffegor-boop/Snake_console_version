

#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <functional> 
#include <cmath>      
#include <algorithm>  
#include <random>
#include <ctime>

struct Button {
    sf::RectangleShape rect;
    sf::Text text;
    bool hovered = false;
    bool clicked = false;
    std::string action;

    Button(const sf::Font& font, const std::string& textStr, sf::Vector2f pos, sf::Color bgColor, sf::Color textColor) {
        rect.setSize(sf::Vector2f(200, 60));
        rect.setPosition(pos);
        rect.setFillColor(bgColor);
        rect.setOutlineThickness(2);
        rect.setOutlineColor(sf::Color::Black);

        text.setFont(font);
        text.setString(textStr);
        text.setCharacterSize(24);
        text.setFillColor(textColor);
        sf::FloatRect textBounds = text.getLocalBounds();
        text.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
        text.setPosition(pos.x + rect.getSize().x / 2.0f, pos.y + rect.getSize().y / 2.0f);
    }
};

struct Fruit {
    sf::Vector2f position;
    float lifetime = 10.0f; // Время жизни в секундах
    float maxLifetime = 10.0f; // Максимальное время жизни
    float spoilRate = 1.0f; // Скорость порчи (зависит от сложности)
    sf::CircleShape shape;
    bool isGood; 

    Fruit(sf::Vector2f pos, float life, float spoil = 1.0f) {
        position = pos;
        maxLifetime = life;
        lifetime = life;
        spoilRate = spoil;
        shape.setRadius(15.0f);
        shape.setPosition(pos);
        isGood = true; // ИНИЦИАЛИЗИРОВАТЬ
        updateAppearance();
    }

    void update(float deltaTime) {
        lifetime -= deltaTime * spoilRate;
        if (lifetime < 0) lifetime = 0;

        
        isGood = (lifetime > 0.2f * maxLifetime);

        updateAppearance();
    }

    void updateAppearance() {
        float freshness = lifetime / maxLifetime; // 1.0 = свежий, 0.0 = испортился

        // Меняем цвет в зависимости от свежести
        if (freshness > 0.7f) {
            shape.setFillColor(sf::Color(0, 255, 0, 255)); // Зеленый - свежий
        }
        else if (freshness > 0.4f) {
            shape.setFillColor(sf::Color(255, 255, 0, 200)); // Желтый - начинает портиться
        }
        else if (freshness > 0.2f) {
            shape.setFillColor(sf::Color(255, 165, 0, 150)); // Оранжевый - почти испорчен
        }
        else {
            shape.setFillColor(sf::Color(255, 0, 0, 100)); // Красный - испорчен
        }

        // Меняем размер в зависимости от свежести
        float scale = 0.5f + freshness * 0.5f;
        shape.setScale(scale, scale);
    }

    
    bool isGoodFruit() const {
        return lifetime > 0.2f * maxLifetime;
    }

    
    bool getIsGood() const {
        return isGood;
    }
};

struct Settings {
    std::string playerName = "Player";
    sf::Color playerColor = sf::Color::Blue;
    sf::Color botColor = sf::Color::Yellow;
    int fieldSize = 20;
    std::vector<sf::Keyboard::Key> playerKeys = {
        sf::Keyboard::W, sf::Keyboard::A, sf::Keyboard::S, sf::Keyboard::D
    };
    int difficulty = 1; // 1 - легкая, 2 - средняя, 3 - сложная
};



enum GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER
};

struct GameData {
    // Игрок
    std::string playerName;
    sf::Color playerColor;
    int score;
    int roundWins;

    // Игра
    int currentRound;
    int totalRounds;
    int totalBots;
    float gameTime;
    bool isMultiplayer;

    // Настройки управления
    sf::Keyboard::Key turnLeft;
    sf::Keyboard::Key turnRight;
    sf::Keyboard::Key accelerate;
    sf::Keyboard::Key decelerate;

    // Состояние
    bool isAlive;
    bool gameStarted;

    GameData() {
        playerName = "Player";
        playerColor = sf::Color::Blue;
        score = 0;
        roundWins = 0;
        currentRound = 1;
        totalRounds = 1;
        totalBots = 0;
        gameTime = 0.0f;
        isMultiplayer = false;
        turnLeft = sf::Keyboard::A;
        turnRight = sf::Keyboard::D;
        accelerate = sf::Keyboard::W;
        decelerate = sf::Keyboard::S;
        isAlive = true;
        gameStarted = false;
    }
};

// Класс змейки
class Snake {
public:
    std::vector<sf::Vector2f> body;
    sf::Color color;
    float speed;
    float rotation;
    float rotationSpeed;
    int score;
    std::string name;
    bool isPlayer;
    bool isAlive;

    // Управление
    sf::Keyboard::Key turnLeft;
    sf::Keyboard::Key turnRight;
    sf::Keyboard::Key accelerate;
    sf::Keyboard::Key decelerate;

    Snake(const std::string& n, const sf::Color& c, bool player = true) {
        name = n;
        color = c;
        speed = 150.0f;
        rotation = 0.0f;
        rotationSpeed = 180.0f;
        score = 0;
        isPlayer = player;
        isAlive = true;

        // Начальная позиция
        body.push_back(sf::Vector2f(400, 300));
        for (int i = 1; i < 5; i++) {
            body.push_back(sf::Vector2f(380 - i * 20, 300));
        }

        // Управление по умолчанию
        turnLeft = sf::Keyboard::A;
        turnRight = sf::Keyboard::D;
        accelerate = sf::Keyboard::W;
        decelerate = sf::Keyboard::S;
    }

    void update(float deltaTime) {
        if (!isAlive) return;

        // Обновление позиции
        float radians = rotation * 3.14159f / 180.0f;
        sf::Vector2f velocity(cos(radians) * speed * deltaTime,
            sin(radians) * speed * deltaTime);

        // Двигаем голову
        body[0] += velocity;

        // Двигаем тело
        for (size_t i = 1; i < body.size(); i++) {
            sf::Vector2f dir = body[i - 1] - body[i];
            float dist = sqrt(dir.x * dir.x + dir.y * dir.y);
            if (dist > 20.0f) {
                dir = dir / dist;
                body[i] = body[i - 1] - dir * 20.0f;
            }
        }
    }

    void draw(sf::RenderWindow& window) {
        for (size_t i = 0; i < body.size(); i++) {
            sf::CircleShape segment(15.0f);
            if (i == 0) segment.setRadius(18.0f); // Голова больше
            segment.setFillColor(color);
            segment.setPosition(body[i]);
            if (i == 0) {
                segment.setOrigin(3, 3); // Центрируем голову
            }
            window.draw(segment);
        }
    }

    void grow() {
        if (body.size() > 1) {
            sf::Vector2f last = body.back();
            sf::Vector2f secondLast = body[body.size() - 2];
            sf::Vector2f dir = last - secondLast;
            dir = dir / sqrt(dir.x * dir.x + dir.y * dir.y);
            body.push_back(last + dir * 20.0f);
        }
        else {
            body.push_back(body[0]);
        }
        score += 10;
    }

    sf::Vector2f getHeadPosition() const {
        return body[0];
    }
};




class PauseDialog {
private:
    sf::RenderWindow* window;
    sf::Font* font;
    bool visible;

    struct DialogButton {
        sf::RectangleShape rect;
        sf::Text text;
        std::string action;
    };

    std::vector<DialogButton> buttons;

public:
    PauseDialog(sf::RenderWindow* win, sf::Font* fnt) : window(win), font(fnt), visible(false) {
        createButtons();
    }

    void createButtons() {
        buttons.clear();

        // Кнопка "Resume"
        DialogButton btn1;
        btn1.rect.setSize(sf::Vector2f(200, 50));
        btn1.rect.setPosition(400, 300);
        btn1.rect.setFillColor(sf::Color(50, 150, 50, 220));
        btn1.rect.setOutlineThickness(2);
        btn1.rect.setOutlineColor(sf::Color::White);
        btn1.text.setFont(*font);
        btn1.text.setString("Resume Game");
        btn1.text.setCharacterSize(24);
        btn1.text.setFillColor(sf::Color::White);
        btn1.text.setPosition(450, 325);
        btn1.action = "resume";
        buttons.push_back(btn1);

        // Кнопка "Restart"
        DialogButton btn2;
        btn2.rect.setSize(sf::Vector2f(200, 50));
        btn2.rect.setPosition(400, 370);
        btn2.rect.setFillColor(sf::Color(50, 100, 150, 220));
        btn2.rect.setOutlineThickness(2);
        btn2.rect.setOutlineColor(sf::Color::White);
        btn2.text.setFont(*font);
        btn2.text.setString("Restart Game");
        btn2.text.setCharacterSize(24);
        btn2.text.setFillColor(sf::Color::White);
        btn2.text.setPosition(450, 395);
        btn2.action = "restart";
        buttons.push_back(btn2);

        // Кнопка "Exit to Menu"
        DialogButton btn3;
        btn3.rect.setSize(sf::Vector2f(200, 50));
        btn3.rect.setPosition(400, 440);
        btn3.rect.setFillColor(sf::Color(150, 50, 50, 220));
        btn3.rect.setOutlineThickness(2);
        btn3.rect.setOutlineColor(sf::Color::White);
        btn3.text.setFont(*font);
        btn3.text.setString("Exit to Menu");
        btn3.text.setCharacterSize(24);
        btn3.text.setFillColor(sf::Color::White);
        btn3.text.setPosition(450, 465);
        btn3.action = "exit";
        buttons.push_back(btn3);
    }

    void show() { visible = true; }
    void hide() { visible = false; }
    bool isVisible() const { return visible; }

    void handleClick(sf::Vector2f mousePos, std::function<void(std::string)> callback) {
        if (!visible) return;

        for (auto& btn : buttons) {
            if (btn.rect.getGlobalBounds().contains(mousePos)) {
                callback(btn.action);
                break;
            }
        }
    }

    void render() {
        if (!visible) return;

        // Полупрозрачный фон
        sf::RectangleShape overlay;
        overlay.setSize(sf::Vector2f(1000, 900));
        overlay.setFillColor(sf::Color(0, 0, 0, 150));
        window->draw(overlay);

        // Диалоговое окно
        sf::RectangleShape dialog;
        dialog.setSize(sf::Vector2f(400, 300));
        dialog.setPosition(300, 250);
        dialog.setFillColor(sf::Color(30, 30, 50, 230));
        dialog.setOutlineThickness(3);
        dialog.setOutlineColor(sf::Color::White);
        window->draw(dialog);

        // Заголовок
        sf::Text title;
        title.setFont(*font);
        title.setString("GAME PAUSED");
        title.setCharacterSize(36);
        title.setFillColor(sf::Color::Yellow);
        title.setPosition(350, 260);
        window->draw(title);

        // Кнопки
        for (auto& btn : buttons) {
            window->draw(btn.rect);
            window->draw(btn.text);
        }
    }
};

class GameOverDialog {
private:
    sf::RenderWindow* window;
    sf::Font* font;
    bool visible;

    struct DialogButton {
        sf::RectangleShape rect;
        sf::Text text;
        std::string action;
    };

    std::vector<DialogButton> buttons;
    std::string winnerName;
    sf::Color winnerColor;
    int playerScore;
    int roundWins;
    int totalRounds;

public:
    GameOverDialog(sf::RenderWindow* win, sf::Font* fnt) :
        window(win), font(fnt), visible(false), playerScore(0),
        roundWins(0), totalRounds(1) {
        createButtons();
    }

    void createButtons() {
        buttons.clear();

        // Кнопка "Restart"
        DialogButton btn1;
        btn1.rect.setSize(sf::Vector2f(200, 50));
        btn1.rect.setPosition(350, 500);
        btn1.rect.setFillColor(sf::Color(50, 150, 50, 220));
        btn1.rect.setOutlineThickness(2);
        btn1.rect.setOutlineColor(sf::Color::White);
        btn1.text.setFont(*font);
        btn1.text.setString("Restart");
        btn1.text.setCharacterSize(24);
        btn1.text.setFillColor(sf::Color::White);
        btn1.text.setPosition(425, 525);
        btn1.action = "restart";
        buttons.push_back(btn1);

        // Кнопка "Exit"
        DialogButton btn2;
        btn2.rect.setSize(sf::Vector2f(200, 50));
        btn2.rect.setPosition(550, 500);
        btn2.rect.setFillColor(sf::Color(150, 50, 50, 220));
        btn2.rect.setOutlineThickness(2);
        btn2.rect.setOutlineColor(sf::Color::White);
        btn2.text.setFont(*font);
        btn2.text.setString("Exit");
        btn2.text.setCharacterSize(24);
        btn2.text.setFillColor(sf::Color::White);
        btn2.text.setPosition(625, 525);
        btn2.action = "exit";
        buttons.push_back(btn2);
    }

    void show(const std::string& winner, const sf::Color& color,
        int score, int wins, int rounds) {
        winnerName = winner;
        winnerColor = color;
        playerScore = score;
        roundWins = wins;
        totalRounds = rounds;
        visible = true;
    }

    void hide() { visible = false; }
    bool isVisible() const { return visible; }

    void handleClick(sf::Vector2f mousePos, std::function<void(std::string)> callback) {
        if (!visible) return;

        for (auto& btn : buttons) {
            if (btn.rect.getGlobalBounds().contains(mousePos)) {
                callback(btn.action);
                break;
            }
        }
    }

    void render() {
        if (!visible) return;

        // Полупрозрачный фон
        sf::RectangleShape overlay;
        overlay.setSize(sf::Vector2f(1000, 900));
        overlay.setFillColor(sf::Color(0, 0, 0, 180));
        window->draw(overlay);

        // Диалоговое окно
        sf::RectangleShape dialog;
        dialog.setSize(sf::Vector2f(600, 400));
        dialog.setPosition(200, 250);
        dialog.setFillColor(sf::Color(20, 20, 40, 230));
        dialog.setOutlineThickness(3);
        dialog.setOutlineColor(sf::Color::Yellow);
        window->draw(dialog);

        // Заголовок
        sf::Text title;
        title.setFont(*font);
        title.setString("GAME OVER");
        title.setCharacterSize(48);
        title.setFillColor(sf::Color::Red);
        title.setPosition(350, 260);
        window->draw(title);

        // Победитель
        sf::Text winnerText;
        winnerText.setFont(*font);
        winnerText.setString("Winner: " + winnerName);
        winnerText.setCharacterSize(32);
        winnerText.setFillColor(winnerColor);
        winnerText.setPosition(250, 330);
        window->draw(winnerText);

        // Счет
        sf::Text scoreText;
        scoreText.setFont(*font);
        if (totalRounds > 1) {
            scoreText.setString("Game Score: " + std::to_string(roundWins) +
                " wins (Total: " + std::to_string(playerScore) + " points)");
        }
        else {
            scoreText.setString("Score: " + std::to_string(playerScore) + " points");
        }
        scoreText.setCharacterSize(28);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(250, 380);
        window->draw(scoreText);

        // Кнопки
        for (auto& btn : buttons) {
            window->draw(btn.rect);
            window->draw(btn.text);
        }
    }
};


class GameMenu {
private:
    // Игровые объекты
    std::vector<Snake*> snakes;
    std::vector<Fruit*> gameFruits;

    // Игровое состояние
    GameState gameState;
    GameData gameData;

    // UI для игры
    sf::Text scoreText;
    sf::Text roundText;
    sf::Text playerInfoText;
    sf::Text timerText;

    // Диалоги
    PauseDialog* pauseDialog;
    GameOverDialog* gameOverDialog;

    // Таймеры
    sf::Clock gameClock;
    sf::Clock fruitSpawnClock;
    sf::Clock inputClock;

    // Фрукты
    float fruitSpawnInterval;

    

    sf::RenderWindow* window;
    sf::Font font;
    float demoTime = 0.0f; // для демонстрации фруктов
    sf::Color bgColor = sf::Color(30, 30, 50);
    sf::Color titleColor = sf::Color::White;

    // Фоновая текстура и спрайт
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;
    bool backgroundLoaded = false;

    // текучее состояние экрана
    enum class Screen { MAIN, GAME_SETTINGS, PLAYER_SETTINGS, ABOUT, EXIT_CONFIRM, EDIT_KEYS };
    Screen currentScreen = Screen::MAIN;

    // настройки игры
    int rounds = 1;
    int bots = 0;

    // настройки игрока
    Settings settings;

    // кнопки
    std::vector<Button*> difficultyButtons;//выбора сложности
    std::vector<Fruit*> exampleFruits;//визуализация примеров фруктов
    std::vector<Button*> mainButtons;
    std::vector<Button*> gameSettingsButtons;
    std::vector<Button*> playerSettingsButtons;
    std::vector<Button*> aboutButtons;
    std::vector<Button*> exitConfirmButtons;
    std::vector<Button*> editKeysButtons;

    // внутреннее обращение
    bool nameInputActive = false;
    bool keyInputActive = false;
    int currentKeyIndex = 0;
    std::string nameInputText = "";
    std::vector<std::string> keyNames = { "Up", "Left", "Down", "Right" };

    // Map для преобразования кодов клавиш в названия
    std::map<sf::Keyboard::Key, std::string> keyToString = {
        // Буквы
        {sf::Keyboard::A, "A"}, {sf::Keyboard::B, "B"}, {sf::Keyboard::C, "C"}, {sf::Keyboard::D, "D"},
        {sf::Keyboard::E, "E"}, {sf::Keyboard::F, "F"}, {sf::Keyboard::G, "G"}, {sf::Keyboard::H, "H"},
        {sf::Keyboard::I, "I"}, {sf::Keyboard::J, "J"}, {sf::Keyboard::K, "K"}, {sf::Keyboard::L, "L"},
        {sf::Keyboard::M, "M"}, {sf::Keyboard::N, "N"}, {sf::Keyboard::O, "O"}, {sf::Keyboard::P, "P"},
        {sf::Keyboard::Q, "Q"}, {sf::Keyboard::R, "R"}, {sf::Keyboard::S, "S"}, {sf::Keyboard::T, "T"},
        {sf::Keyboard::U, "U"}, {sf::Keyboard::V, "V"}, {sf::Keyboard::W, "W"}, {sf::Keyboard::X, "X"},
        {sf::Keyboard::Y, "Y"}, {sf::Keyboard::Z, "Z"},

        // Цифры
        {sf::Keyboard::Num0, "0"}, {sf::Keyboard::Num1, "1"}, {sf::Keyboard::Num2, "2"},
        {sf::Keyboard::Num3, "3"}, {sf::Keyboard::Num4, "4"}, {sf::Keyboard::Num5, "5"},
        {sf::Keyboard::Num6, "6"}, {sf::Keyboard::Num7, "7"}, {sf::Keyboard::Num8, "8"},
        {sf::Keyboard::Num9, "9"},

        // Стрелки
        {sf::Keyboard::Up, "Up"}, {sf::Keyboard::Down, "Down"}, {sf::Keyboard::Left, "Left"}, {sf::Keyboard::Right, "Right"},

        // Функциональные клавиши
        {sf::Keyboard::F1, "F1"}, {sf::Keyboard::F2, "F2"}, {sf::Keyboard::F3, "F3"}, {sf::Keyboard::F4, "F4"},
        {sf::Keyboard::F5, "F5"}, {sf::Keyboard::F6, "F6"}, {sf::Keyboard::F7, "F7"}, {sf::Keyboard::F8, "F8"},
        {sf::Keyboard::F9, "F9"}, {sf::Keyboard::F10, "F10"}, {sf::Keyboard::F11, "F11"}, {sf::Keyboard::F12, "F12"},

        // Другие клавиши
        {sf::Keyboard::Space, "Space"}, {sf::Keyboard::Enter, "Enter"}, {sf::Keyboard::Escape, "Escape"},
        {sf::Keyboard::LControl, "LCtrl"}, {sf::Keyboard::RControl, "RCtrl"}, {sf::Keyboard::LShift, "LShift"},
        {sf::Keyboard::RShift, "RShift"}, {sf::Keyboard::LAlt, "LAlt"}, {sf::Keyboard::RAlt, "RAlt"},
        {sf::Keyboard::Tab, "Tab"}, {sf::Keyboard::BackSpace, "Backspace"}, {sf::Keyboard::Insert, "Insert"},
        {sf::Keyboard::Delete, "Delete"}, {sf::Keyboard::Home, "Home"}, {sf::Keyboard::End, "End"},
        {sf::Keyboard::PageUp, "PageUp"}, {sf::Keyboard::PageDown, "PageDown"},

        // Специальные клавиши
        {sf::Keyboard::Comma, ","}, {sf::Keyboard::Period, "."}, {sf::Keyboard::SemiColon, ";"},
        {sf::Keyboard::Quote, "'"}, {sf::Keyboard::Slash, "/"}, {sf::Keyboard::BackSlash, "\\"},
        {sf::Keyboard::Tilde, "~"}, {sf::Keyboard::Equal, "="}, {sf::Keyboard::Dash, "-"},
        {sf::Keyboard::LBracket, "["}, {sf::Keyboard::RBracket, "]"}
    };

    std::string getKeyName(sf::Keyboard::Key key) {
        auto it = keyToString.find(key);
        if (it != keyToString.end()) {
            return it->second;
        }
        return "Unknown";
    }

    // Функция для загрузки фонового изображения
    bool loadBackground(const std::string& filename) {
        if (backgroundTexture.loadFromFile(filename)) {
            backgroundSprite.setTexture(backgroundTexture);

            // Масштабируем изображение под размер окна
            sf::Vector2u windowSize = window->getSize();
            sf::Vector2u textureSize = backgroundTexture.getSize();

            float scaleX = static_cast<float>(windowSize.x) / textureSize.x;
            float scaleY = static_cast<float>(windowSize.y) / textureSize.y;

            // Выбираем больший масштаб, чтобы заполнить весь экран
            float scale = std::max(scaleX, scaleY);
            backgroundSprite.setScale(scale, scale);

            // Центрируем изображение
            backgroundSprite.setPosition(
                (windowSize.x - textureSize.x * scale) / 2,
                (windowSize.y - textureSize.y * scale) / 2
            );

            // Добавляем полупрозрачный черный слой для улучшения читаемости текста
            bgColor = sf::Color(0, 0, 0, 150); // Полупрозрачный черный
            backgroundLoaded = true;
            return true;
        }
        return false;
    }

    void initMainMenu() {
        mainButtons.clear();
        mainButtons.push_back(new Button(font, "Play", { 400, 250 }, sf::Color(50, 150, 50, 200), sf::Color::White));
        mainButtons.push_back(new Button(font, "Settings", { 400, 330 }, sf::Color(50, 100, 150, 200), sf::Color::White));
        mainButtons.push_back(new Button(font, "Developers", { 400, 410 }, sf::Color(100, 100, 150, 200), sf::Color::White));
        mainButtons.push_back(new Button(font, "Exit", { 400, 490 }, sf::Color(150, 50, 50, 200), sf::Color::White));

        for (auto& btn : mainButtons) btn->action = btn->text.getString();
    }

    void initDifficultySettings() {
        difficultyButtons.clear();
        difficultyButtons.push_back(new Button(font, "Easy (slow spoil)", { 100, 250 }, sf::Color(50, 200, 50, 200), sf::Color::White));
        difficultyButtons.push_back(new Button(font, "Medium", { 100, 330 }, sf::Color(200, 200, 50, 200), sf::Color::White));
        difficultyButtons.push_back(new Button(font, "Hard (fast spoil)", { 100, 410 }, sf::Color(200, 50, 50, 200), sf::Color::White));

        difficultyButtons[0]->action = "diff_easy";
        difficultyButtons[1]->action = "diff_medium";
        difficultyButtons[2]->action = "diff_hard";

        // Создаем примеры фруктов для визуализации
        exampleFruits.clear();
        exampleFruits.push_back(new Fruit(sf::Vector2f(400, 250), 10.0f, 0.5f)); // Легкий
        exampleFruits.push_back(new Fruit(sf::Vector2f(400, 330), 10.0f, 1.0f)); // Средний
        exampleFruits.push_back(new Fruit(sf::Vector2f(400, 410), 10.0f, 2.0f)); // Сложный
    }

    void initGameSettings() {
        gameSettingsButtons.clear();
        gameSettingsButtons.push_back(new Button(font, "Quickie (1 round)", { 400, 250 }, sf::Color(70, 170, 70, 200), sf::Color::White));
        gameSettingsButtons.push_back(new Button(font, "3 rounds", { 400, 330 }, sf::Color(70, 170, 70, 200), sf::Color::White));
        gameSettingsButtons.push_back(new Button(font, "5 rounds", { 400, 410 }, sf::Color(70, 170, 70, 200), sf::Color::White));
        gameSettingsButtons.push_back(new Button(font, "0 bots (training)", { 400, 490 }, sf::Color(100, 150, 100, 200), sf::Color::White));
        gameSettingsButtons.push_back(new Button(font, "1 bot", { 400, 570 }, sf::Color(100, 150, 100, 200), sf::Color::White));
        gameSettingsButtons.push_back(new Button(font, "2 bots", { 400, 650 }, sf::Color(100, 150, 100, 200), sf::Color::White));
        gameSettingsButtons.push_back(new Button(font, "Play Game", { 400, 730 }, sf::Color(50, 200, 50, 200), sf::Color::White));
        gameSettingsButtons.push_back(new Button(font, "Back", { 400, 810 }, sf::Color(150, 100, 50, 200), sf::Color::White));

        gameSettingsButtons[0]->action = "rounds_1";
        gameSettingsButtons[1]->action = "rounds_3";
        gameSettingsButtons[2]->action = "rounds_5";
        gameSettingsButtons[3]->action = "bots_0";
        gameSettingsButtons[4]->action = "bots_1";
        gameSettingsButtons[5]->action = "bots_2";
        gameSettingsButtons[6]->action = "start_game";
        gameSettingsButtons[7]->action = "back";
    }

    void initPlayerSettings() {
        playerSettingsButtons.clear();
        playerSettingsButtons.push_back(new Button(font, "Blue", { 150, 250 }, sf::Color::Blue, sf::Color::White));
        playerSettingsButtons.push_back(new Button(font, "Green", { 350, 250 }, sf::Color::Green, sf::Color::White));
        playerSettingsButtons.push_back(new Button(font, "Red", { 550, 250 }, sf::Color::Red, sf::Color::White));
        playerSettingsButtons.push_back(new Button(font, "Purple", { 750, 250 }, sf::Color(128, 0, 128), sf::Color::White));
        playerSettingsButtons.push_back(new Button(font, "WASD", { 100, 350 }, sf::Color(70, 100, 150, 200), sf::Color::White));
        playerSettingsButtons.push_back(new Button(font, "Arrows", { 300, 350 }, sf::Color(70, 100, 150, 200), sf::Color::White));
        playerSettingsButtons.push_back(new Button(font, "Edit Keys", { 500, 350 }, sf::Color(70, 150, 100, 200), sf::Color::White));
        playerSettingsButtons.push_back(new Button(font, "Enter Nickname", { 700, 350 }, sf::Color(70, 150, 100, 200), sf::Color::White));
        playerSettingsButtons.push_back(new Button(font, "Field size: " + std::to_string(settings.fieldSize), { 100, 500 }, sf::Color(80, 120, 180, 200), sf::Color::White));
        playerSettingsButtons.push_back(new Button(font, "Save", { 350, 550 }, sf::Color(50, 150, 100, 200), sf::Color::White));
        playerSettingsButtons.push_back(new Button(font, "Back", { 550, 550 }, sf::Color(150, 100, 50, 200), sf::Color::White));

        playerSettingsButtons[0]->action = "color_blue";
        playerSettingsButtons[1]->action = "color_green";
        playerSettingsButtons[2]->action = "color_red";
        playerSettingsButtons[3]->action = "color_purple";
        playerSettingsButtons[4]->action = "keys_wasd";
        playerSettingsButtons[5]->action = "keys_arrows";
        playerSettingsButtons[6]->action = "edit_keys";
        playerSettingsButtons[7]->action = "edit_name";
        playerSettingsButtons[8]->action = "field_size";
        playerSettingsButtons[9]->action = "save_settings";
        playerSettingsButtons[10]->action = "back";
    }

    void initAbout() {
        aboutButtons.clear();
        aboutButtons.push_back(new Button(font, "Back", { 400, 500 }, sf::Color(150, 100, 50, 200), sf::Color::White));
        aboutButtons[0]->action = "back";
    }

    void initExitConfirm() {
        exitConfirmButtons.clear();
        exitConfirmButtons.push_back(new Button(font, "Yes", { 300, 400 }, sf::Color(150, 50, 50, 200), sf::Color::White));
        exitConfirmButtons.push_back(new Button(font, "No", { 500, 400 }, sf::Color(50, 150, 50, 200), sf::Color::White));

        exitConfirmButtons[0]->action = "exit_yes";
        exitConfirmButtons[1]->action = "exit_no";
    }

    void initEditKeys() {
        editKeysButtons.clear();
        editKeysButtons.push_back(new Button(font, "Back", { 400, 600 }, sf::Color(150, 100, 50, 200), sf::Color::White));
        editKeysButtons.push_back(new Button(font, "Reset to WASD", { 600, 600 }, sf::Color(70, 100, 150, 200), sf::Color::White));
        editKeysButtons.push_back(new Button(font, "Reset to Arrows", { 200, 600 }, sf::Color(70, 100, 150, 200), sf::Color::White));

        editKeysButtons[0]->action = "back";
        editKeysButtons[1]->action = "reset_wasd";
        editKeysButtons[2]->action = "reset_arrows";
    }

    void handleMouseHover(std::vector<Button*>& buttons, sf::Vector2f mousePos) {
        for (auto& btn : buttons) {
            if (btn->rect.getGlobalBounds().contains(mousePos)) {
                if (!btn->hovered) {
                    // Сохраняем оригинальный цвет без альфа-канала
                    sf::Color originalColor = btn->rect.getFillColor();
                    btn->rect.setFillColor(sf::Color(
                        std::min(originalColor.r + 30, 255),
                        std::min(originalColor.g + 30, 255),
                        std::min(originalColor.b + 30, 255),
                        originalColor.a
                    ));
                    btn->hovered = true;
                }
            }
            else {
                if (btn->hovered) {
                    sf::Color originalColor = btn->rect.getFillColor();
                    btn->rect.setFillColor(sf::Color(
                        std::max(originalColor.r - 30, 0),
                        std::max(originalColor.g - 30, 0),
                        std::max(originalColor.b - 30, 0),
                        originalColor.a
                    ));
                    btn->hovered = false;
                }
            }
        }
    }

    bool handleMouseClick(std::vector<Button*>& buttons, sf::Vector2f mousePos) {
        for (auto& btn : buttons) {
            if (btn->rect.getGlobalBounds().contains(mousePos)) {
                btn->clicked = true;
                return true;
            }
        }
        return false;
    }

    void loadSettings() {
        std::ifstream file("settings.txt");
        if (file.is_open()) {
            std::string line;
            std::getline(file, line);
            settings.playerName = line;
            std::getline(file, line);
            std::istringstream iss(line);
            int r, g, b;
            iss >> r >> g >> b;
            settings.playerColor = sf::Color(r, g, b);
            std::getline(file, line);
            settings.fieldSize = std::stoi(line);

            // Загрузка клавиш
            for (int i = 0; i < 4; i++) {
                if (std::getline(file, line)) {
                    int keyCode = std::stoi(line);
                    settings.playerKeys[i] = static_cast<sf::Keyboard::Key>(keyCode);
                }
            }

            // НОВОЕ: Загрузка сложности
            if (std::getline(file, line)) {
                settings.difficulty = std::stoi(line);
            }

            file.close();
        }
    }

    void saveSettings() {
        std::ofstream file("settings.txt");
        if (file.is_open()) {
            file << settings.playerName << std::endl;
            file << (int)settings.playerColor.r << " " << (int)settings.playerColor.g << " "
                << (int)settings.playerColor.b << std::endl;
            file << settings.fieldSize << std::endl;

            // Сохранение клавиш
            for (auto key : settings.playerKeys) {
                file << static_cast<int>(key) << std::endl;
            }

            // НОВОЕ: Сохранение сложности
            file << settings.difficulty << std::endl;

            file.close();
        }
    }

    void handleNameInput(sf::Event& event) {
        if (event.type == sf::Event::TextEntered) {
            // Обрабатываем только печатные символы
            if (event.text.unicode >= 32 && event.text.unicode <= 126) {
                // Добавляем символ к имени
                nameInputText += static_cast<char>(event.text.unicode);
            }
        }

        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Enter) {
                if (!nameInputText.empty()) {
                    settings.playerName = nameInputText;
                }
                nameInputActive = false;
                nameInputText = "";
            }

            // Удаление последнего символа по Backspace
            if (event.key.code == sf::Keyboard::BackSpace) {
                if (!nameInputText.empty()) {
                    nameInputText.pop_back();
                }
            }

            // Отмена ввода по Escape
            if (event.key.code == sf::Keyboard::Escape) {
                nameInputActive = false;
                nameInputText = "";
            }
        }
    }

    void handleKeyInput(sf::Event& event) {
        if (event.type == sf::Event::KeyPressed) {
            // Если нажата клавиша Escape - отмена
            if (event.key.code == sf::Keyboard::Escape) {
                keyInputActive = false;
                return;
            }

            // Проверяем, что клавиша не дублируется
            bool keyAlreadyUsed = false;
            for (int i = 0; i < 4; i++) {
                if (i != currentKeyIndex && settings.playerKeys[i] == event.key.code) {
                    keyAlreadyUsed = true;
                    break;
                }
            }

            if (keyAlreadyUsed) {
                // Можно показать сообщение об ошибке
                return;
            }

            // Сохраняем клавишу
            settings.playerKeys[currentKeyIndex] = event.key.code;

            // Переходим к следующей клавише
            currentKeyIndex++;

            // Если все клавиши назначены, завершаем ввод
            if (currentKeyIndex >= 4) {
                keyInputActive = false;
                currentKeyIndex = 0;
            }
        }
    }

public:
    GameMenu() : window(nullptr) {
        
        gameState = MENU;
        pauseDialog = nullptr;
        gameOverDialog = nullptr;
        fruitSpawnInterval = 3.0f;
    }

    ~GameMenu() {
        for (auto btn : mainButtons) delete btn;
        for (auto btn : gameSettingsButtons) delete btn;
        for (auto btn : playerSettingsButtons) delete btn;
        for (auto btn : aboutButtons) delete btn;
        for (auto btn : exitConfirmButtons) delete btn;
        for (auto btn : editKeysButtons) delete btn;

        for (auto btn : difficultyButtons) delete btn;
        for (auto fruit : exampleFruits) delete fruit;

        
        delete pauseDialog;
        delete gameOverDialog;
        for (auto snake : snakes) delete snake;
        for (auto fruit : gameFruits) delete fruit;
    }
    

    void startGame(int rounds, int bots) {
        gameState = PLAYING;
        gameData.totalRounds = rounds;
        gameData.totalBots = bots;
        gameData.currentRound = 1;
        gameData.score = 0;
        gameData.roundWins = 0;
        gameData.gameTime = 0.0f;
        gameData.isAlive = true;

        // Очистка старых объектов
        for (auto snake : snakes) delete snake;
        for (auto fruit : gameFruits) delete fruit;
        snakes.clear();
        gameFruits.clear();

        // Создание игрока
        Snake* player = new Snake(settings.playerName, settings.playerColor);
        player->turnLeft = settings.playerKeys[1];  // A
        player->turnRight = settings.playerKeys[3]; // D
        player->accelerate = settings.playerKeys[0]; // W
        player->decelerate = settings.playerKeys[2]; // S

        // Настройка скорости в зависимости от сложности
        switch (settings.difficulty) {
        case 1: player->speed = 120.0f; break;
        case 2: player->speed = 180.0f; break;
        case 3: player->speed = 240.0f; break;
        }

        snakes.push_back(player);
        gameData.playerName = settings.playerName;
        gameData.playerColor = settings.playerColor;

        // Создание ботов
        for (int i = 0; i < bots; i++) {
            Snake* bot = new Snake("Bot " + std::to_string(i + 1), settings.botColor, false);
            bot->body[0] = sf::Vector2f(200 + i * 150, 200 + i * 100);
            for (size_t j = 1; j < bot->body.size(); j++) {
                bot->body[j] = sf::Vector2f(180 + i * 150 - (j - 1) * 20, 200 + i * 100);
            }
            snakes.push_back(bot);
        }

        // Создание начальных фруктов
        for (int i = 0; i < 5; i++) {
            spawnGameFruit();
        }

        gameClock.restart();
        fruitSpawnClock.restart();
    }

    void spawnGameFruit() {
        float x = 100 + rand() % 800;
        float y = 100 + rand() % 600;

        float spoilRate = 1.0f;
        switch (settings.difficulty) {
        case 1: spoilRate = 0.5f; break;
        case 2: spoilRate = 1.0f; break;
        case 3: spoilRate = 2.0f; break;
        }

        
        gameFruits.push_back(new Fruit(sf::Vector2f(x, y), 10.0f, spoilRate));
    }

    void updateGame(float deltaTime) {
        if (gameState != PLAYING) return;

        gameData.gameTime += deltaTime;

        // Обновление змей
        for (auto snake : snakes) {
            if (snake->isAlive) {
                snake->update(deltaTime);
            }
        }

        // Обновление фруктов
        for (auto fruit : gameFruits) {
            fruit->update(deltaTime);
        }

        // Спавн новых фруктов
        if (fruitSpawnClock.getElapsedTime().asSeconds() > fruitSpawnInterval) {
            spawnGameFruit();
            fruitSpawnClock.restart();
        }

        // Проверка столкновений
        checkCollisions();
        checkGameOver();
    }

    void checkCollisions() {
        // Проверка столкновения с фруктами
        for (size_t i = 0; i < gameFruits.size(); i++) {
            if (!gameFruits[i]->isGood) continue; // Использовать поле

            for (auto snake : snakes) {
                if (!snake->isAlive) continue;

                float dx = snake->getHeadPosition().x - gameFruits[i]->position.x;
                float dy = snake->getHeadPosition().y - gameFruits[i]->position.y;
                float distance = sqrt(dx * dx + dy * dy);

                if (distance < 30.0f) {
                    snake->grow();
                    gameFruits[i]->isGood = false; // Использовать поле
                    gameData.score = snake->score;

                    // Увеличение скорости при съедении фрукта
                    snake->speed = std::min(snake->speed + 10.0f, 300.0f);
                    break;
                }
            }
        }

        // Удаление испорченных фруктов
        gameFruits.erase(std::remove_if(gameFruits.begin(), gameFruits.end(),
            [](Fruit* f) { return !f->isGood; }), gameFruits.end()); // Использовать поле

        // Проверка столкновений со стенами
        for (auto snake : snakes) {
            if (!snake->isAlive) continue;

            sf::Vector2f head = snake->getHeadPosition();
            if (head.x < 50 || head.x > 950 || head.y < 50 || head.y > 850) {
                snake->isAlive = false;
            }
        }

        // Проверка столкновений с телами
        for (auto snake : snakes) {
            if (!snake->isAlive) continue;

            for (size_t i = 1; i < snake->body.size(); i++) {
                float dx = snake->getHeadPosition().x - snake->body[i].x;
                float dy = snake->getHeadPosition().y - snake->body[i].y;
                float distance = sqrt(dx * dx + dy * dy);

                if (distance < 25.0f) {
                    snake->isAlive = false;
                    break;
                }
            }
        }

        // Проверка столкновений между змеями
        for (size_t i = 0; i < snakes.size(); i++) {
            if (!snakes[i]->isAlive) continue;

            for (size_t j = 0; j < snakes.size(); j++) {
                if (i == j || !snakes[j]->isAlive) continue;

                // Проверка лобового столкновения
                float dx = snakes[i]->getHeadPosition().x - snakes[j]->getHeadPosition().x;
                float dy = snakes[i]->getHeadPosition().y - snakes[j]->getHeadPosition().y;
                float distance = sqrt(dx * dx + dy * dy);

                if (distance < 30.0f) {
                    // Если игрок столкнулся с ботом "лоб в лоб" - проигрыш игрока
                    if (snakes[i]->isPlayer && !snakes[j]->isPlayer) {
                        snakes[i]->isAlive = false;
                    }
                    else if (!snakes[i]->isPlayer && snakes[j]->isPlayer) {
                        snakes[j]->isAlive = false;
                    }
                    else {
                        // Если два бота столкнулись
                        snakes[i]->isAlive = false;
                        snakes[j]->isAlive = false;
                    }
                }

                // Проверка столкновения с телом другой змеи
                for (size_t k = 1; k < snakes[j]->body.size(); k++) {
                    float dx2 = snakes[i]->getHeadPosition().x - snakes[j]->body[k].x;
                    float dy2 = snakes[i]->getHeadPosition().y - snakes[j]->body[k].y;
                    float distance2 = sqrt(dx2 * dx2 + dy2 * dy2);

                    if (distance2 < 25.0f) {
                        snakes[i]->isAlive = false;
                        break;
                    }
                }
            }
        }
    }

    void checkGameOver() {
        if (gameState != PLAYING) return;

        // Проверка жив ли игрок
        bool playerAlive = false;
        Snake* playerSnake = nullptr;

        for (auto snake : snakes) {
            if (snake->isPlayer && snake->isAlive) {
                playerAlive = true;
                playerSnake = snake;
                break;
            }
        }

        // Если игрок умер
        if (!playerAlive) {
            endRound(false);
            return;
        }

        // Проверка жив ли хоть один бот
        bool botsAlive = false;
        for (auto snake : snakes) {
            if (!snake->isPlayer && snake->isAlive) {
                botsAlive = true;
                break;
            }
        }

        // Если нет ботов и это последний раунд - конец игры
        if (!botsAlive) {
            if (gameData.currentRound >= gameData.totalRounds) {
                endGame(true);
            }
            else {
                endRound(true);
            }
        }
    }

    void endRound(bool playerWon) {
        if (playerWon) {
            gameData.roundWins++;
        }

        if (gameData.currentRound >= gameData.totalRounds) {
            endGame(playerWon);
        }
        else {
            // Начинаем следующий раунд
            gameData.currentRound++;

            // Пересоздаем змей с новыми позициями
            for (auto snake : snakes) {
                // Случайная позиция
                snake->body.clear();
                float x = 100 + rand() % 800;
                float y = 100 + rand() % 600;
                snake->body.push_back(sf::Vector2f(x, y));

                for (int i = 1; i < 5; i++) {
                    snake->body.push_back(sf::Vector2f(x - i * 20, y));
                }

                snake->isAlive = true;
                snake->rotation = 0.0f;
            }

            // Очищаем фрукты
            for (auto fruit : gameFruits) delete fruit;
            gameFruits.clear();

            // Создаем новые фрукты
            for (int i = 0; i < 5; i++) {
                spawnGameFruit();
            }
        }
    }

    void endGame(bool playerWon) {
        gameState = GAME_OVER;

        std::string winner;
        sf::Color winnerColor;

        if (playerWon) {
            winner = gameData.playerName;
            winnerColor = gameData.playerColor;
        }
        else {
            // Ищем живого бота
            for (auto snake : snakes) {
                if (!snake->isPlayer && snake->isAlive) {
                    winner = snake->name;
                    winnerColor = snake->color;
                    break;
                }
            }
            if (winner.empty()) winner = "No one";
        }

        gameOverDialog->show(winner, winnerColor,
            gameData.score, gameData.roundWins,
            gameData.totalRounds);
    }

    void handleGameInput(sf::Event& event) {
        if (gameState != PLAYING) return;

        Snake* playerSnake = nullptr;
        for (auto snake : snakes) {
            if (snake->isPlayer) {
                playerSnake = snake;
                break;
            }
        }

        if (!playerSnake) return;

        if (event.type == sf::Event::KeyPressed) {
            // Пауза по ESC
            if (event.key.code == sf::Keyboard::Escape) {
                gameState = PAUSED;
                pauseDialog->show();
                return;
            }

            // Управление змейкой
            if (event.key.code == playerSnake->turnLeft) {
                playerSnake->rotation -= playerSnake->rotationSpeed * 0.016f;
            }
            else if (event.key.code == playerSnake->turnRight) {
                playerSnake->rotation += playerSnake->rotationSpeed * 0.016f;
            }
            else if (event.key.code == playerSnake->accelerate) {
                playerSnake->speed = std::min(playerSnake->speed + 50.0f, 300.0f);
            }
            else if (event.key.code == playerSnake->decelerate) {
                playerSnake->speed = std::max(playerSnake->speed - 50.0f, 50.0f);
            }
        }

        // Непрерывное управление при удерживании клавиш
        if (sf::Keyboard::isKeyPressed(playerSnake->turnLeft)) {
            playerSnake->rotation -= playerSnake->rotationSpeed * 0.016f;
        }
        if (sf::Keyboard::isKeyPressed(playerSnake->turnRight)) {
            playerSnake->rotation += playerSnake->rotationSpeed * 0.016f;
        }
        if (sf::Keyboard::isKeyPressed(playerSnake->accelerate)) {
            playerSnake->speed = std::min(playerSnake->speed + 30.0f * 0.016f, 300.0f);
        }
        if (sf::Keyboard::isKeyPressed(playerSnake->decelerate)) {
            playerSnake->speed = std::max(playerSnake->speed - 30.0f * 0.016f, 50.0f);
        }
    }

    void renderGame() {
        if (gameState == MENU) return;

        // Рисуем фон
        sf::RectangleShape gameBg;
        gameBg.setSize(sf::Vector2f(1000, 900));
        gameBg.setFillColor(sf::Color(20, 40, 20));
        window->draw(gameBg);

        // Рисуем игровое поле
        sf::RectangleShape field;
        field.setSize(sf::Vector2f(900, 800));
        field.setPosition(50, 50);
        field.setFillColor(sf::Color(10, 30, 10));
        field.setOutlineThickness(2);
        field.setOutlineColor(sf::Color::White);
        window->draw(field);

        // Рисуем фрукты
        for (auto fruit : gameFruits) {
            window->draw(fruit->shape);
        }

        // Рисуем змей
        for (auto snake : snakes) {
            snake->draw(*window);
        }

        // Рисуем UI
        renderGameUI();
    }

    void renderGameUI() {
        // Текст счета
        scoreText.setFont(font);
        scoreText.setString("Score: " + std::to_string(gameData.score));
        scoreText.setCharacterSize(24);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(20, 20);
        window->draw(scoreText);

        // Текст раунда (если многораундовая игра)
        if (gameData.totalRounds > 1) {
            roundText.setFont(font);
            roundText.setString("Round: " + std::to_string(gameData.currentRound) +
                "/" + std::to_string(gameData.totalRounds));
            roundText.setCharacterSize(24);
            roundText.setFillColor(sf::Color::White);
            roundText.setPosition(20, 60);
            window->draw(roundText);

            // Счет игры
            sf::Text gameScoreText;
            gameScoreText.setFont(font);
            gameScoreText.setString("Wins: " + std::to_string(gameData.roundWins));
            gameScoreText.setCharacterSize(24);
            gameScoreText.setFillColor(sf::Color::Yellow);
            gameScoreText.setPosition(20, 100);
            window->draw(gameScoreText);
        }

        // Имя игрока с цветом
        playerInfoText.setFont(font);
        playerInfoText.setString(gameData.playerName);
        playerInfoText.setCharacterSize(28);
        playerInfoText.setFillColor(gameData.playerColor);
        playerInfoText.setPosition(800, 20);
        window->draw(playerInfoText);

        // Таймер
        timerText.setFont(font);
        timerText.setString("Time: " + std::to_string((int)gameData.gameTime) + "s");
        timerText.setCharacterSize(24);
        timerText.setFillColor(sf::Color::White);
        timerText.setPosition(800, 60);
        window->draw(timerText);

        // Инструкция
        if (gameState == PLAYING) {
            sf::Text instruction;
            instruction.setFont(font);
            instruction.setString("ESC - Pause");
            instruction.setCharacterSize(18);
            instruction.setFillColor(sf::Color(200, 200, 200));
            instruction.setPosition(20, 850);
            window->draw(instruction);
        }
    }

    

    

    bool init() {
        window = new sf::RenderWindow(sf::VideoMode(1000, 900), "Snake Game Menu");
        window->setFramerateLimit(60);

        if (!font.loadFromFile("arial.ttf")) {
            return false;
        }

        // Пытаемся загрузить фоновое изображение

        if (!loadBackground("background.jpg")) {
            if (!loadBackground("background.png")) {
                if (!loadBackground("bg.jpg")) {
                    if (!loadBackground("bg.png")) {
                        std::cout << "Could not load background image. Using solid color background.\n";
                        backgroundLoaded = false;
                    }
                }
            }
        }

        loadSettings();
        initMainMenu();
        initGameSettings();
        initPlayerSettings();
        initAbout();
        initExitConfirm();
        initEditKeys();
        initDifficultySettings();

        pauseDialog = new PauseDialog(window, &font);
        gameOverDialog = new GameOverDialog(window, &font);

        return true;
    }

    void run() {
        sf::Clock clock;
        while (window->isOpen()) {
            float deltaTime = clock.restart().asSeconds();
            sf::Event event;
            sf::Vector2f mousePos = window->mapPixelToCoords(sf::Mouse::getPosition(*window));

            while (window->pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window->close();
                    return;
                }

                // Обработка в зависимости от состояния игры
                switch (gameState) {
                case MENU:
                    // Существующая обработка меню
                    if (!nameInputActive && !keyInputActive) {
                        if (event.type == sf::Event::MouseButtonPressed &&
                            event.mouseButton.button == sf::Mouse::Left) {
                            handleClick(mousePos);
                        }
                    }
                    break;

                case PLAYING:
                    handleGameInput(event);
                    break;

                case PAUSED:
                    if (event.type == sf::Event::MouseButtonPressed &&
                        event.mouseButton.button == sf::Mouse::Left) {
                        pauseDialog->handleClick(mousePos, std::bind(&GameMenu::handlePauseDialog, this, std::placeholders::_1));
                    }
                    break;

                case GAME_OVER:
                    if (event.type == sf::Event::MouseButtonPressed &&
                        event.mouseButton.button == sf::Mouse::Left) {
                        gameOverDialog->handleClick(mousePos, [this](std::string action) {
                            if (action == "restart") {
                                startGame(gameData.totalRounds, gameData.totalBots);
                            }
                            else if (action == "exit") {
                                gameState = MENU;
                                gameOverDialog->hide();
                                // Очистка игровых объектов
                                for (auto snake : snakes) delete snake;
                                for (auto fruit : gameFruits) delete fruit;
                                snakes.clear();
                                gameFruits.clear();
                            }
                            });
                    }
                    break;
                }

                // Обработка ввода имени/клавиш (как было)
                if (nameInputActive) {
                    handleNameInput(event);
                }
                if (keyInputActive) {
                    handleKeyInput(event);
                }
            }

            // Обновление игры
            if (gameState == PLAYING) {
                updateGame(deltaTime);
            }

            // Отрисовка
            window->clear(bgColor);

            if (backgroundLoaded) {
                window->draw(backgroundSprite);
                sf::RectangleShape overlay;
                overlay.setSize(sf::Vector2f(1000, 900));
                overlay.setFillColor(sf::Color(0, 0, 0, 100));
                window->draw(overlay);
            }

            if (gameState == MENU) {
                render();
            }
            else {
                renderGame();

                if (gameState == PAUSED) {
                    pauseDialog->render();
                }
                else if (gameState == GAME_OVER) {
                    gameOverDialog->render();
                }
            }

            window->display();
        }
    }
    // И добавить новый метод:
    void handlePauseDialog(std::string action) {
        if (action == "resume") {
            gameState = PLAYING;
            pauseDialog->hide();
        }
        else if (action == "restart") {
            startGame(gameData.totalRounds, gameData.totalBots);
        }
        else if (action == "exit") {
            gameState = MENU;
            pauseDialog->hide();
            // Очистка игровых объектов
            for (auto snake : snakes) delete snake;
            for (auto fruit : gameFruits) delete fruit;
            snakes.clear();
            gameFruits.clear();
        }
    }


    void handleClick(sf::Vector2f mousePos) {
        bool clicked = false;

        switch (currentScreen) {
        case Screen::MAIN:
            clicked = handleMouseClick(mainButtons, mousePos);
            if (clicked) {
                std::string action = mainButtons[0]->clicked ? mainButtons[0]->action :
                    mainButtons[1]->clicked ? mainButtons[1]->action :
                    mainButtons[2]->clicked ? mainButtons[2]->action :
                    mainButtons[3]->clicked ? mainButtons[3]->action : "";

                if (action == "Play") {
                    currentScreen = Screen::GAME_SETTINGS;
                    initGameSettings();
                }
                else if (action == "Settings") {
                    currentScreen = Screen::PLAYER_SETTINGS;
                    initPlayerSettings();
                }
                else if (action == "Developers") {
                    currentScreen = Screen::ABOUT;
                    initAbout();
                }
                else if (action == "Exit") {
                    currentScreen = Screen::EXIT_CONFIRM;
                }
                
            }
            break;

        case Screen::GAME_SETTINGS:
            clicked = handleMouseClick(gameSettingsButtons, mousePos);
            if (clicked) {
                std::string action = "";
                for (auto& btn : gameSettingsButtons) {
                    if (btn->clicked) {
                        action = btn->action;
                        break;
                    }
                }

                if (action.find("rounds_") == 0) {
                    rounds = std::stoi(action.substr(6));
                }
                else if (action.find("bots_") == 0) {
                    bots = std::stoi(action.substr(5));
                }
                else if (action == "start_game") {
                    std::cout << "Game Starting: " << rounds << " rounds, " << bots << " bots, Difficulty: " << settings.difficulty << std::endl;
                    // ЗАМЕНИТЬ window->close() на:
                    startGame(rounds, bots);
                }
                else if (action == "back") {
                    currentScreen = Screen::MAIN;
                    initMainMenu();
                }
            }
            
            else {
                clicked = handleMouseClick(difficultyButtons, mousePos);
                if (clicked) {
                    std::string action = "";
                    for (auto& btn : difficultyButtons) {
                        if (btn->clicked) {
                            action = btn->action;
                            break;
                        }
                    }

                    if (action == "diff_easy") {
                        settings.difficulty = 1;
                    }
                    else if (action == "diff_medium") {
                        settings.difficulty = 2;
                    }
                    else if (action == "diff_hard") {
                        settings.difficulty = 3;
                    }

                    // Подсветка выбранной кнопки
                    for (auto& btn : difficultyButtons) {
                        btn->rect.setOutlineColor(sf::Color::Black);
                        btn->rect.setOutlineThickness(2);
                    }
                }
            }
            break;

        case Screen::PLAYER_SETTINGS:
            clicked = handleMouseClick(playerSettingsButtons, mousePos);
            if (clicked) {
                std::string action = "";
                for (auto& btn : playerSettingsButtons) {
                    if (btn->clicked) {
                        action = btn->action;
                        break;
                    }
                }

                if (action.find("color_") == 0) {
                    if (action == "color_blue") settings.playerColor = sf::Color::Blue;
                    else if (action == "color_green") settings.playerColor = sf::Color::Green;
                    else if (action == "color_red") settings.playerColor = sf::Color::Red;
                    else if (action == "color_purple") settings.playerColor = sf::Color(128, 0, 128);
                }
                else if (action == "keys_wasd") {
                    settings.playerKeys = { sf::Keyboard::W, sf::Keyboard::A, sf::Keyboard::S, sf::Keyboard::D };
                }
                else if (action == "keys_arrows") {
                    settings.playerKeys = { sf::Keyboard::Up, sf::Keyboard::Left, sf::Keyboard::Down, sf::Keyboard::Right };
                }
                else if (action == "edit_keys") {
                    currentScreen = Screen::EDIT_KEYS;
                }
                else if (action == "edit_name") {
                    nameInputActive = true;
                    nameInputText = settings.playerName; // Начинаем с текущего имени
                }
                else if (action == "field_size") {
                    settings.fieldSize = (settings.fieldSize % 3 + 1) * 10; // 10, 20, 30
                    // Обновляем текст кнопки
                    playerSettingsButtons[8]->text.setString("Field size: " + std::to_string(settings.fieldSize));
                    sf::FloatRect textBounds = playerSettingsButtons[8]->text.getLocalBounds();
                    playerSettingsButtons[8]->text.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
                }
                else if (action == "save_settings") {
                    saveSettings();
                    currentScreen = Screen::MAIN;
                    initMainMenu();
                }
                else if (action == "back") {
                    currentScreen = Screen::MAIN;
                    initMainMenu();
                }
            }
            break;

        case Screen::ABOUT:
            clicked = handleMouseClick(aboutButtons, mousePos);
            if (clicked && aboutButtons[0]->clicked) {
                currentScreen = Screen::MAIN;
                initMainMenu();
            }
            break;

        case Screen::EXIT_CONFIRM:
            clicked = handleMouseClick(exitConfirmButtons, mousePos);
            if (clicked) {
                std::string action = "";
                for (auto& btn : exitConfirmButtons) {
                    if (btn->clicked) {
                        action = btn->action;
                        break;
                    }
                }

                if (action == "exit_yes") {
                    window->close();
                }
                else if (action == "exit_no") {
                    currentScreen = Screen::MAIN;
                    initMainMenu();
                }
            }
            break;

        case Screen::EDIT_KEYS:
            clicked = handleMouseClick(editKeysButtons, mousePos);
            if (clicked) {
                std::string action = "";
                for (auto& btn : editKeysButtons) {
                    if (btn->clicked) {
                        action = btn->action;
                        break;
                    }
                }

                if (action == "back") {
                    currentScreen = Screen::PLAYER_SETTINGS;
                    initPlayerSettings();
                }
                else if (action == "reset_wasd") {
                    settings.playerKeys = { sf::Keyboard::W, sf::Keyboard::A, sf::Keyboard::S, sf::Keyboard::D };
                }
                else if (action == "reset_arrows") {
                    settings.playerKeys = { sf::Keyboard::Up, sf::Keyboard::Left, sf::Keyboard::Down, sf::Keyboard::Right };
                }
            }
            else {
                // Проверка кликов по областям с клавишами
                for (int i = 0; i < 4; i++) {
                    sf::FloatRect keyRect(200, 300 + i * 70, 600, 60);
                    if (keyRect.contains(mousePos)) {
                        keyInputActive = true;
                        currentKeyIndex = i;
                        break;
                    }
                }
            }
            break;
        }

        //
        if (clicked) {
            switch (currentScreen) {
            case Screen::MAIN:
                for (auto btn : mainButtons) btn->clicked = false;
                break;
            case Screen::GAME_SETTINGS:
                for (auto btn : gameSettingsButtons) btn->clicked = false;
                break;
            case Screen::PLAYER_SETTINGS:
                for (auto btn : playerSettingsButtons) btn->clicked = false;
                break;
            case Screen::ABOUT:
                for (auto btn : aboutButtons) btn->clicked = false;
                break;
            case Screen::EXIT_CONFIRM:
                for (auto btn : exitConfirmButtons) btn->clicked = false;
                break;
            case Screen::EDIT_KEYS:
                for (auto btn : editKeysButtons) btn->clicked = false;
                break;
            }
        }
    }

    void render() {
        sf::Text title;
        title.setFont(font);
        title.setCharacterSize(48);
        title.setFillColor(sf::Color::White);
        title.setStyle(sf::Text::Bold);
        title.setString("Snake Game");

        // Добавляем тень к заголовку для лучшей читаемости на фоне
        sf::Text titleShadow = title;
        titleShadow.setFillColor(sf::Color::Black);
        titleShadow.setPosition(352, 102);
        window->draw(titleShadow);

        title.setPosition(350, 100);
        window->draw(title);

        sf::Text info;
        info.setFont(font);
        info.setCharacterSize(20);
        info.setFillColor(sf::Color::White);

        switch (currentScreen) {
        case Screen::MAIN:
            for (auto btn : mainButtons) {
                window->draw(btn->rect);
                window->draw(btn->text);
            }
            break;

        case Screen::GAME_SETTINGS: {
            info.setString("Rounds: " + std::to_string(rounds) + " | Bots: " + std::to_string(bots));
            info.setPosition(350, 200);

            // Добавляем тень к тексту
            sf::Text infoShadow = info;
            infoShadow.setFillColor(sf::Color::Black);
            infoShadow.setPosition(352, 202);
            window->draw(infoShadow);
            window->draw(info);

            //  Отображаем выбранную сложность
            sf::Text diffInfo;
            diffInfo.setFont(font);
            diffInfo.setCharacterSize(20);
            diffInfo.setFillColor(sf::Color::White);
            std::string diffText;
            if (settings.difficulty == 1) diffText = "Easy (slow spoil)";
            else if (settings.difficulty == 2) diffText = "Medium";
            else diffText = "Hard (fast spoil)";
            diffInfo.setString("Difficulty: " + diffText);
            
            diffInfo.setPosition(100, 480);
            sf::Text diffInfoShadow = diffInfo;
            diffInfoShadow.setFillColor(sf::Color::Black);
            diffInfoShadow.setPosition(102, 482);
            window->draw(diffInfoShadow);
            window->draw(diffInfo);

            // Отображаем кнопки настроек игры
            for (auto btn : gameSettingsButtons) {
                window->draw(btn->rect);
                window->draw(btn->text);
            }

            
            for (auto btn : difficultyButtons) {
                // Подсветка выбранной сложности
                if ((settings.difficulty == 1 && btn->action == "diff_easy") ||
                    (settings.difficulty == 2 && btn->action == "diff_medium") ||
                    (settings.difficulty == 3 && btn->action == "diff_hard")) {
                    btn->rect.setOutlineColor(sf::Color::Yellow);
                    btn->rect.setOutlineThickness(3);
                }
                else {
                    btn->rect.setOutlineColor(sf::Color::Black);
                    btn->rect.setOutlineThickness(2);
                }

                window->draw(btn->rect);
                window->draw(btn->text);
            }

            
            sf::Text fruitDemoText;
            fruitDemoText.setFont(font);
            fruitDemoText.setCharacterSize(18);
            fruitDemoText.setFillColor(sf::Color::White);
            fruitDemoText.setString("Fruit spoilage demo:");
            fruitDemoText.setPosition(600, 230);
            window->draw(fruitDemoText);

            // Создаем примеры фруктов для демонстрации
            std::vector<Fruit> demoFruits;
            demoFruits.push_back(Fruit(sf::Vector2f(600, 270), 10.0f, 0.5f)); // Легкий
            demoFruits.push_back(Fruit(sf::Vector2f(600, 350), 10.0f, 1.0f)); // Средний  
            demoFruits.push_back(Fruit(sf::Vector2f(600, 430), 10.0f, 2.0f)); // Сложный

            // Обновляем и отрисовываем фрукты
            static float demoTime = 0;
            demoTime += 0.016f;

            for (size_t i = 0; i < demoFruits.size(); i++) {
                // Устанавливаем разную степень свежести для демонстрации
                demoFruits[i].lifetime = 10.0f - (demoTime * (i + 1) * 0.5f);
                if (demoFruits[i].lifetime < 0) demoFruits[i].lifetime = 0;
                demoFruits[i].updateAppearance();
                window->draw(demoFruits[i].shape);

                // Подписи под фруктами
                sf::Text fruitLabel;
                fruitLabel.setFont(font);
                fruitLabel.setCharacterSize(16);
                fruitLabel.setFillColor(sf::Color::White);
                std::string freshness = std::to_string(static_cast<int>((demoFruits[i].lifetime / 10.0f) * 100)) + "%";
                fruitLabel.setString("Fresh: " + freshness);
                fruitLabel.setPosition(600, 300 + i * 80);
                window->draw(fruitLabel);
            }

            // Сброс демо-таймера
            if (demoTime > 20.0f) demoTime = 0;
            break;
        }

        case Screen::PLAYER_SETTINGS: {
            info.setString("Nickname: " + settings.playerName);
            info.setPosition(100, 200);
            sf::Text infoShadow = info;
            infoShadow.setFillColor(sf::Color::Black);
            infoShadow.setPosition(102, 202);
            window->draw(infoShadow);
            window->draw(info);

            info.setString("Player Color");
            info.setPosition(100, 220);
            infoShadow = info;
            infoShadow.setPosition(100, 220);
            window->draw(infoShadow);
            window->draw(info);

            // Отображение текущих клавиш управления
            info.setString("Current Controls: " +
                getKeyName(settings.playerKeys[0]) + " " +
                getKeyName(settings.playerKeys[1]) + " " +
                getKeyName(settings.playerKeys[2]) + " " +
                getKeyName(settings.playerKeys[3]));
            info.setPosition(100, 430);
            infoShadow = info;
            infoShadow.setPosition(100, 430);
            window->draw(infoShadow);
            window->draw(info);

            info.setString("Field size: " + std::to_string(settings.fieldSize));
            info.setPosition(100, 470);
            infoShadow = info;
            infoShadow.setPosition(100, 470);
            window->draw(infoShadow);
            window->draw(info);

            sf::RectangleShape colorRect;
            colorRect.setSize(sf::Vector2f(50, 30));
            colorRect.setFillColor(settings.playerColor);
            colorRect.setPosition(100, 280);
            colorRect.setOutlineColor(sf::Color::Black);
            colorRect.setOutlineThickness(2);
            window->draw(colorRect);

            for (auto btn : playerSettingsButtons) {
                window->draw(btn->rect);
                window->draw(btn->text);
            }

            if (nameInputActive) {
                // Рисуем полупрозрачный фон
                sf::RectangleShape overlay;
                overlay.setSize(sf::Vector2f(1000, 900));
                overlay.setFillColor(sf::Color(0, 0, 0, 200));
                window->draw(overlay);

                // Рисуем диалоговое окно
                sf::RectangleShape dialog;
                dialog.setSize(sf::Vector2f(600, 200));
                dialog.setPosition(200, 300);
                dialog.setFillColor(sf::Color(50, 50, 80, 230));
                dialog.setOutlineColor(sf::Color::White);
                dialog.setOutlineThickness(2);
                window->draw(dialog);

                // Текст инструкции
                sf::Text prompt;
                prompt.setFont(font);
                prompt.setString("Enter player name:");
                prompt.setCharacterSize(28);
                prompt.setFillColor(sf::Color::White);
                prompt.setPosition(250, 320);
                window->draw(prompt);

                // Поле ввода
                sf::RectangleShape inputBox;
                inputBox.setSize(sf::Vector2f(500, 40));
                inputBox.setPosition(250, 380);
                inputBox.setFillColor(sf::Color(30, 30, 30, 230));
                inputBox.setOutlineColor(sf::Color::White);
                inputBox.setOutlineThickness(1);
                window->draw(inputBox);

                // Введенный текст
                sf::Text inputTextDisplay;
                inputTextDisplay.setFont(font);
                inputTextDisplay.setString(nameInputText + "_"); // Курсор в виде подчеркивания
                inputTextDisplay.setCharacterSize(24);
                inputTextDisplay.setFillColor(sf::Color::White);
                inputTextDisplay.setPosition(260, 385);
                window->draw(inputTextDisplay);

                // Инструкция
                sf::Text instruction;
                instruction.setFont(font);
                instruction.setString("Press Enter to confirm, Escape to cancel");
                instruction.setCharacterSize(18);
                instruction.setFillColor(sf::Color(200, 200, 200));
                instruction.setPosition(250, 440);
                window->draw(instruction);
            }
            break;
        }

        case Screen::ABOUT: {
            sf::Text creators;
            creators.setFont(font);
            creators.setString("Creators:\nIvan Ivanov\nBalbes\nShmakov");
            creators.setCharacterSize(32);
            creators.setPosition(300, 200);
            creators.setFillColor(sf::Color::White);

            // Тень для текста
            sf::Text creatorsShadow = creators;
            creatorsShadow.setFillColor(sf::Color::Black);
            creatorsShadow.setPosition(302, 202);
            window->draw(creatorsShadow);
            window->draw(creators);

            for (auto btn : aboutButtons) {
                window->draw(btn->rect);
                window->draw(btn->text);
            }
            break;
        }

        case Screen::EXIT_CONFIRM: {
            // Рисуем полупрозрачный фон
            sf::RectangleShape overlay;
            overlay.setSize(sf::Vector2f(1000, 900));
            overlay.setFillColor(sf::Color(0, 0, 0, 200));
            window->draw(overlay);

            // Рисуем диалоговое окно
            sf::RectangleShape dialog;
            dialog.setSize(sf::Vector2f(500, 250));
            dialog.setPosition(250, 300);
            dialog.setFillColor(sf::Color(50, 50, 80, 230));
            dialog.setOutlineColor(sf::Color::White);
            dialog.setOutlineThickness(2);
            window->draw(dialog);

            // Текст вопроса
            sf::Text question;
            question.setFont(font);
            question.setString("Are you sure you want to exit?");
            question.setCharacterSize(32);
            question.setFillColor(sf::Color::White);
            question.setPosition(270, 320);
            window->draw(question);

            // Кнопки Yes/No
            for (auto btn : exitConfirmButtons) {
                window->draw(btn->rect);
                window->draw(btn->text);
            }

            // Дополнительная инструкция
            sf::Text instruction;
            instruction.setFont(font);
            instruction.setString("Click Yes to exit, No to return to main menu");
            instruction.setCharacterSize(18);
            instruction.setFillColor(sf::Color(200, 200, 200));
            instruction.setPosition(280, 480);
            window->draw(instruction);
            break;
        }

        case Screen::EDIT_KEYS: {
            // Заголовок
            sf::Text editTitle;
            editTitle.setFont(font);
            editTitle.setString("Edit Controls");
            editTitle.setCharacterSize(36);
            editTitle.setFillColor(sf::Color::White);
            editTitle.setPosition(400, 150);

            // Тень для заголовка
            sf::Text editTitleShadow = editTitle;
            editTitleShadow.setFillColor(sf::Color::Black);
            editTitleShadow.setPosition(402, 152);
            window->draw(editTitleShadow);
            window->draw(editTitle);

            // Инструкция
            sf::Text instruction;
            instruction.setFont(font);
            instruction.setCharacterSize(24);
            instruction.setFillColor(sf::Color::White);

            if (keyInputActive) {
                instruction.setString("Press key for: " + keyNames[currentKeyIndex] +
                    "\n(Press Escape to cancel)");
                instruction.setPosition(300, 250);

                // Подсветка текущей клавиши
                sf::RectangleShape highlight;
                highlight.setSize(sf::Vector2f(600, 60));
                highlight.setPosition(200, 300 + currentKeyIndex * 70);
                highlight.setFillColor(sf::Color(100, 100, 150, 150));
                highlight.setOutlineColor(sf::Color::Yellow);
                highlight.setOutlineThickness(2);
                window->draw(highlight);
            }
            else {
                instruction.setString("Click on a direction to change its key\nor press any key to start editing");
                instruction.setPosition(300, 220);
            }

            // Тень для инструкции
            sf::Text instructionShadow = instruction;
            instructionShadow.setFillColor(sf::Color::Black);
            instructionShadow.setPosition(302, 222);
            window->draw(instructionShadow);
            window->draw(instruction);

            // Отображение текущих клавиш
            for (int i = 0; i < 4; i++) {
                sf::RectangleShape keyBox;
                keyBox.setSize(sf::Vector2f(600, 60));
                keyBox.setPosition(200, 300 + i * 70);

                if (i == currentKeyIndex && keyInputActive) {
                    keyBox.setFillColor(sf::Color(100, 150, 100, 180));
                }
                else {
                    keyBox.setFillColor(sf::Color(70, 70, 100, 180));
                }

                keyBox.setOutlineColor(sf::Color::White);
                keyBox.setOutlineThickness(1);
                window->draw(keyBox);

                // Название направления
                sf::Text dirText;
                dirText.setFont(font);
                dirText.setString(keyNames[i] + ":");
                dirText.setCharacterSize(24);
                dirText.setFillColor(sf::Color::White);
                dirText.setPosition(220, 315 + i * 70);
                window->draw(dirText);

                // Текущая клавиша
                sf::Text keyText;
                keyText.setFont(font);
                keyText.setString(getKeyName(settings.playerKeys[i]));
                keyText.setCharacterSize(24);
                keyText.setFillColor(sf::Color::Yellow);
                keyText.setPosition(350, 315 + i * 70);
                window->draw(keyText);

                // Кнопка для изменения этой клавиши
                sf::Text changeText;
                changeText.setFont(font);
                changeText.setString("[Click to change]");
                changeText.setCharacterSize(20);
                changeText.setFillColor(sf::Color(150, 200, 255));
                changeText.setPosition(600, 315 + i * 70);
                window->draw(changeText);
            }

            // Кнопки внизу
            for (auto btn : editKeysButtons) {
                window->draw(btn->rect);
                window->draw(btn->text);
            }

            // Обработка кликов по областям клавиш (вне кнопок)
            if (!keyInputActive) {
                sf::Vector2f mousePos = window->mapPixelToCoords(sf::Mouse::getPosition(*window));
                for (int i = 0; i < 4; i++) {
                    sf::FloatRect keyRect(200, 300 + i * 70, 600, 60);
                    if (keyRect.contains(mousePos)) {
                        sf::RectangleShape hover;
                        hover.setSize(sf::Vector2f(600, 60));
                        hover.setPosition(200, 300 + i * 70);
                        hover.setFillColor(sf::Color(255, 255, 255, 50));
                        window->draw(hover);
                    }
                }
            }

            break;
        }
        }
    }
};

int main() {
    setlocale(LC_ALL, "Russian");
    srand(time(0)); 

    GameMenu menu;
    if (menu.init()) {
        menu.run();
    }
    return 0;
}