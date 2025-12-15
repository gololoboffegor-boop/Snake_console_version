

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

        // Радиус должен быть меньше половины размера клетки
        shape.setRadius(10.0f);

        // Центрируем фрукт ВНУТРИ клетки
        shape.setOrigin(10.0f, 10.0f);
        // Смещаем к центру клетки
        shape.setPosition(pos.x + 10.0f, pos.y + 10.0f);

        isGood = true;
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
    // Настройки игрока
    std::string playerName = "Player";
    sf::Color playerColor = sf::Color::Blue;
    sf::Color botColor = sf::Color::Yellow;
    int fieldSize = 20;
    int difficulty = 1; // 1 - легкая, 2 - средняя, 3 - сложная

    // Игровой процесс
    float snakeSpeed = 100.0f;
    float snakeRotationSpeed = 150.0f;
    float accelerationRate = 80.0f;
    float decelerationRate = 80.0f;
    float maxSpeed = 200.0f;
    float minSpeed = 50.0f;
    int initialSnakeLength = 1; // Начинать с одной головы
    float fruitSpawnRate = 3.0f;
    float fruitSpoilRateEasy = 0.5f;
    float fruitSpoilRateMedium = 1.0f;
    float fruitSpoilRateHard = 2.0f;

    // Управление
    std::vector<sf::Keyboard::Key> playerKeys = {
        sf::Keyboard::W,    // Ускорение/Вверх
        sf::Keyboard::A,    // Поворот 
        sf::Keyboard::S,    
        sf::Keyboard::D     // Поворот вправо/Вправо
    };

    // Графика
    bool showGrid = true;
    bool smoothMovement = true;
    int snakeSegmentSize = 10;
    int snakeHeadSize = 12;

    // Звук (можно добавить позже)
    float soundVolume = 100.0f;
    float musicVolume = 50.0f;
    bool soundEnabled = true;

    Settings() = default;
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
    std::vector<sf::Vector2f> body; // Позиции сегментов
    sf::Color color;
    float speed;
    sf::Vector2f direction; // Направление движения (по осям)
    sf::Vector2f nextDirection; // Следующее направление
    int score;
    std::string name;
    bool isPlayer;
    bool isAlive;
    float rotation;
    float moveTimer; // Таймер для движения
    float moveInterval = 0.15f; // Интервал между движениями
    int cellSize; // Размер ячейки
    int snakeLength;
    float moveAccumulator = 0.0f; // Добавьте этот член класса
    float rotationSpeed;
    
    sf::Keyboard::Key turnLeft;
    sf::Keyboard::Key turnRight;
    sf::Keyboard::Key accelerate;
    sf::Keyboard::Key decelerate;
    // Управление
    sf::Keyboard::Key moveUp;
    sf::Keyboard::Key moveDown;
    sf::Keyboard::Key moveLeft;
    sf::Keyboard::Key moveRight;

    Snake(const std::string& n, const sf::Color& c, const Settings& gameSettings, bool player = true)
        : name(n), color(c), speed(gameSettings.snakeSpeed),
        rotation(0.0f), score(0), isPlayer(player), isAlive(true),
        moveTimer(0.0f), cellSize(gameSettings.fieldSize),
        moveInterval(0.15f), snakeLength(gameSettings.initialSnakeLength)
    {
        // Выбираем стартовую клетку
        int startGridX = 25;
        int startGridY = 22;

        // Преобразуем в пиксели (выровнено по сетке)
        float startX = startGridX * cellSize;
        float startY = startGridY * cellSize;

        body.clear();
        body.push_back(sf::Vector2f(startX, startY));

        // Начальные сегменты тоже выровнены
        for (int i = 1; i < gameSettings.initialSnakeLength; i++) {
            body.push_back(sf::Vector2f(startX - i * cellSize, startY));
        }

        // Начальное направление
        direction = sf::Vector2f(1, 0); // Вправо
        nextDirection = direction;

        // Управление
        if (player) {
            moveUp = gameSettings.playerKeys[0];
            moveDown = gameSettings.playerKeys[2];
            moveLeft = gameSettings.playerKeys[1];
            moveRight = gameSettings.playerKeys[3];
        }
        else {
            moveUp = sf::Keyboard::W;
            moveDown = sf::Keyboard::S;
            moveLeft = sf::Keyboard::A;
            moveRight = sf::Keyboard::D;
        }
    }

    bool hasEnteredNewCell() {
        if (body.empty()) return false;

        // Получаем текущую клетку головы
        int currentCellX = static_cast<int>(body[0].x / cellSize);
        int currentCellY = static_cast<int>(body[0].y / cellSize);

        // Получаем последнюю известную клетку
        static int lastCellX = currentCellX;
        static int lastCellY = currentCellY;

        // Проверяем, изменилась ли клетка
        bool enteredNewCell = (currentCellX != lastCellX || currentCellY != lastCellY);

        if (enteredNewCell) {
            lastCellX = currentCellX;
            lastCellY = currentCellY;
        }

        return enteredNewCell;
    }

    void update(float deltaTime) {
        if (!isAlive) return;

        // Накопление времени для дискретного движения
        moveTimer += deltaTime;

        // Двигаемся только когда накопилось достаточно времени
        if (moveTimer >= moveInterval) {
            moveTimer = 0.0f;

            // Обновляем текущее направление
            direction = nextDirection;

            // Сохраняем позиции тела перед движением
            std::vector<sf::Vector2f> oldBody = body;

            // Двигаем голову на одну клетку
            sf::Vector2f newHead = body[0];
            newHead.x += direction.x * cellSize;
            newHead.y += direction.y * cellSize;

            // Устанавливаем новую голову
            body[0] = newHead;

            // Двигаем тело: каждый сегмент занимает позицию предыдущего
            for (size_t i = 1; i < body.size(); i++) {
                body[i] = oldBody[i - 1];
            }

            // Отладочный вывод (можно убрать потом)
            if (isPlayer && body.size() > 3) {
                // Проверяем возможное столкновение с собой
                int headCellX = static_cast<int>(body[0].x / cellSize);
                int headCellY = static_cast<int>(body[0].y / cellSize);

                for (size_t i = 3; i < body.size(); i++) {
                    int segmentCellX = static_cast<int>(body[i].x / cellSize);
                    int segmentCellY = static_cast<int>(body[i].y / cellSize);

                    if (headCellX == segmentCellX && headCellY == segmentCellY) {
                        std::cout << "WARNING: Possible self-collision detected!" << std::endl;
                        std::cout << "Head cell: (" << headCellX << ", " << headCellY << ")" << std::endl;
                        std::cout << "Segment " << i << " cell: (" << segmentCellX << ", " << segmentCellY << ")" << std::endl;
                    }
                }
            }
        }
    }

    bool checkFruitCollision(Snake* snake, Fruit* fruit) {
        // Координаты клетки змеи
        int snakeCellX = static_cast<int>(snake->body[0].x / snake->cellSize);
        int snakeCellY = static_cast<int>(snake->body[0].y / snake->cellSize);

        // Координаты клетки фрукта
        int fruitCellX = static_cast<int>(fruit->position.x / snake->cellSize);
        int fruitCellY = static_cast<int>(fruit->position.y / snake->cellSize);

        // Столкновение, если в одной клетке
        return (snakeCellX == fruitCellX && snakeCellY == fruitCellY);
    }

    void draw(sf::RenderWindow& window) {
        for (size_t i = 0; i < body.size(); i++) {
            // Квадрат размером с клетку
            sf::RectangleShape segment(sf::Vector2f(cellSize, cellSize));

            
            
            float posX = body[i].x + 1; // +1 пиксель от левой границы
            float posY = body[i].y + 1; // +1 пиксель от верхней границы
            segment.setPosition(posX, posY);

            // Цвет
            if (i == 0) {
                // Голова - темнее
                sf::Color headColor = color;
                headColor.r = std::min(color.r + 40, 255);
                headColor.g = std::min(color.g + 40, 255);
                headColor.b = std::min(color.b + 40, 255);
                segment.setFillColor(headColor);
            }
            else {
                segment.setFillColor(color);
            }

            // Контур
            segment.setOutlineThickness(1);
            segment.setOutlineColor(sf::Color::Black);

            window.draw(segment);
        }
    }

    void grow() {
        // Увеличиваем длину
        snakeLength++;

        // Добавляем новый сегмент
        if (!body.empty()) {
            if (body.size() > 1) {
                // Вычисляем направление от предпоследнего к последнему сегменту
                sf::Vector2f tail = body.back();
                sf::Vector2f prev = body[body.size() - 2];
                sf::Vector2f dir = tail - prev;

                // Нормализуем
                float length = sqrt(dir.x * dir.x + dir.y * dir.y);
                if (length > 0) {
                    dir = dir / length;
                }
                else {
                    dir = -direction; // Направление назад
                }

                // Новый сегмент позади хвоста
                sf::Vector2f newSegment = tail + dir * (float)cellSize;
                body.push_back(newSegment);
            }
            else {
                // Если только голова, добавляем позади
                sf::Vector2f newSegment = body[0] - direction * (float)cellSize;
                body.push_back(newSegment);
            }
        }

        score += 10;

        // Можно ускорить змейку при росте
        moveInterval = std::max(moveInterval * 0.98f, 0.05f);
    }

    sf::Vector2f getHeadPosition() const {
        return body[0];
    }

    void setDirection(sf::Vector2f newDir) {
        // direction должен быть членом класса
        if (newDir != -direction) {
            nextDirection = newDir;
        }
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

       
        DialogButton btn1;
        btn1.rect.setSize(sf::Vector2f(200, 50));
        btn1.rect.setPosition(400, 320);  
        btn1.rect.setFillColor(sf::Color(50, 150, 50, 220));
        btn1.rect.setOutlineThickness(2);
        btn1.rect.setOutlineColor(sf::Color::White);
        btn1.text.setFont(*font);
        btn1.text.setString("Продолжить");
        btn1.text.setCharacterSize(24);
        btn1.text.setFillColor(sf::Color::White);
        btn1.text.setPosition(450, 375);  
        btn1.action = "resume";
        buttons.push_back(btn1);

       
        DialogButton btn2;
        btn2.rect.setSize(sf::Vector2f(200, 50));
        btn2.rect.setPosition(400, 390);  
        btn2.rect.setFillColor(sf::Color(50, 100, 150, 220));
        btn2.rect.setOutlineThickness(2);
        btn2.rect.setOutlineColor(sf::Color::White);
        btn2.text.setFont(*font);
        btn2.text.setString("Заново");
        btn2.text.setCharacterSize(24);
        btn2.text.setFillColor(sf::Color::White);
        btn2.text.setPosition(450, 445);  
        btn2.action = "restart";
        buttons.push_back(btn2);

        
        DialogButton btn3;
        btn3.rect.setSize(sf::Vector2f(200, 50));
        btn3.rect.setPosition(400, 460);  
        btn3.rect.setFillColor(sf::Color(150, 50, 50, 220));
        btn3.rect.setOutlineThickness(2);
        btn3.rect.setOutlineColor(sf::Color::White);
        btn3.text.setFont(*font);
        btn3.text.setString("Выйти в меню");
        btn3.text.setCharacterSize(24);
        btn3.text.setFillColor(sf::Color::White);
        btn3.text.setPosition(450, 515);  
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

        
        sf::Text title;
        title.setFont(*font);
        title.setString("ПАУЗА");  
        title.setCharacterSize(42);
        title.setFillColor(sf::Color::Yellow);
        title.setStyle(sf::Text::Bold);

        // ЦЕНТРИРОВАНИЕ текста
        sf::FloatRect titleBounds = title.getLocalBounds();
        title.setOrigin(titleBounds.left + titleBounds.width / 2.0f,
            titleBounds.top + titleBounds.height / 2.0f);
        title.setPosition(500, 280);  

        // Тень текста
        sf::Text titleShadow = title;
        titleShadow.setFillColor(sf::Color::Black);
        titleShadow.setPosition(title.getPosition().x + 2, title.getPosition().y + 2);
        window->draw(titleShadow);

        window->draw(title);

        // Кнопки - также центрируем их
        for (auto& btn : buttons) {
            // Центрируем текст внутри кнопок
            sf::FloatRect textBounds = btn.text.getLocalBounds();
            btn.text.setOrigin(textBounds.left + textBounds.width / 2.0f,
                textBounds.top + textBounds.height / 2.0f);

            // Устанавливаем позицию текста в центр кнопки
            sf::FloatRect rectBounds = btn.rect.getGlobalBounds();
            btn.text.setPosition(rectBounds.left + rectBounds.width / 2.0f,
                rectBounds.top + rectBounds.height / 2.0f);

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
    
    std::string deathReason;
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

        // Кнопка "Exit to Menu"
        DialogButton btn2;
        btn2.rect.setSize(sf::Vector2f(200, 50));
        btn2.rect.setPosition(550, 500);
        btn2.rect.setFillColor(sf::Color(150, 50, 50, 220));
        btn2.rect.setOutlineThickness(2);
        btn2.rect.setOutlineColor(sf::Color::White);
        btn2.text.setFont(*font);
        btn2.text.setString("Exit to Menu");
        btn2.text.setCharacterSize(24);
        btn2.text.setFillColor(sf::Color::White);
        btn2.text.setPosition(625, 525);
        btn2.action = "exit";
        buttons.push_back(btn2);
    }

    void show(const std::string& winner, const sf::Color& color,
        int score, int wins, int rounds, const std::string& reason = "") {
        winnerName = winner;
        winnerColor = color;
        playerScore = score;
        roundWins = wins;
        totalRounds = rounds;
        deathReason = reason;
        visible = true;
    }

    void hide() { visible = false; }
    bool isVisible() const { return visible; }

    void handleClick(sf::Vector2f mousePos, std::function<void(std::string)> callback) {
        if (!visible) return;

        for (auto& btn : buttons) {
            if (btn.rect.getGlobalBounds().contains(mousePos)) {
                callback(btn.action);
                return; 
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

        // Причина поражения
        sf::Text reasonText;
        reasonText.setFont(*font);
        reasonText.setString(deathReason);
        reasonText.setCharacterSize(28);
        reasonText.setFillColor(sf::Color(255, 100, 100));
        reasonText.setPosition(300, 330);
        window->draw(reasonText);

        // Счет
        sf::Text scoreText;
        scoreText.setFont(*font);
        scoreText.setString("Score: " + std::to_string(playerScore) + " points");
        scoreText.setCharacterSize(28);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(300, 380);
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
    bool showTitle = true;  // Новый флаг для отображения заголовка
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
        
        mainButtons.push_back(new Button(font, "Начать", { 400, 250 }, sf::Color(65, 205, 50), sf::Color::White));
        mainButtons.push_back(new Button(font, "Настройки", { 400, 330 }, sf::Color(0, 191, 255), sf::Color::White));
        mainButtons.push_back(new Button(font, "О создателях", { 400, 410 }, sf::Color(147, 112, 219), sf::Color::White));
        mainButtons.push_back(new Button(font, "Выход", { 400, 490 }, sf::Color(178, 34, 34), sf::Color::White));

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

        // Только выбор игры:
        gameSettingsButtons.push_back(new Button(font, "Быстрая игра (1 раунд)", { 400, 250 }, sf::Color(70, 170, 70, 200), sf::Color::White));
        gameSettingsButtons.push_back(new Button(font, "3 раунда", { 400, 330 }, sf::Color(70, 170, 70, 200), sf::Color::White));
        gameSettingsButtons.push_back(new Button(font, "5 раундов", { 400, 410 }, sf::Color(70, 170, 70, 200), sf::Color::White));
        gameSettingsButtons.push_back(new Button(font, "Режим выживания", { 400, 490 }, sf::Color(70, 170, 70, 200), sf::Color::White));

        // Боты:
        gameSettingsButtons.push_back(new Button(font, "0 ботов (тренировка)", { 400, 570 }, sf::Color(100, 150, 100, 200), sf::Color::White));
        gameSettingsButtons.push_back(new Button(font, "1 бот", { 400, 650 }, sf::Color(100, 150, 100, 200), sf::Color::White));
        gameSettingsButtons.push_back(new Button(font, "2 ботам", { 400, 730 }, sf::Color(100, 150, 100, 200), sf::Color::White));

        // Кнопки действий:
        gameSettingsButtons.push_back(new Button(font, "Начать игру", { 400, 810 }, sf::Color(50, 200, 50, 200), sf::Color::White));
        gameSettingsButtons.push_back(new Button(font, "Назад", { 400, 890 }, sf::Color(150, 100, 50, 200), sf::Color::White));

        // Назначьте действия
        gameSettingsButtons[0]->action = "rounds_1";
        gameSettingsButtons[1]->action = "rounds_3";
        gameSettingsButtons[2]->action = "rounds_5";
        gameSettingsButtons[3]->action = "rounds_survival";
        gameSettingsButtons[4]->action = "bots_0";
        gameSettingsButtons[5]->action = "bots_1";
        gameSettingsButtons[6]->action = "bots_2";
        gameSettingsButtons[7]->action = "start_game";
        gameSettingsButtons[8]->action = "back";
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
        playerSettingsButtons.push_back(new Button(font, "Reset to Default", { 100, 650 }, sf::Color(150, 100, 50, 200), sf::Color::White));
        
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
        exitConfirmButtons.push_back(new Button(font, "Да", { 300, 400 }, sf::Color(150, 50, 50, 200), sf::Color::White));
        exitConfirmButtons.push_back(new Button(font, "Нет", { 500, 400 }, sf::Color(50, 150, 50, 200), sf::Color::White));

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
        std::ifstream file("config.ini");
        if (!file.is_open()) {
            std::cout << "Config file not found. Using default settings." << std::endl;
            saveSettings(); // Создаем файл с настройками по умолчанию
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            // Пропускаем комментарии и пустые строки
            if (line.empty() || line[0] == '#') continue;

            size_t delimiter = line.find('=');
            if (delimiter == std::string::npos) continue;

            std::string key = line.substr(0, delimiter);
            std::string value = line.substr(delimiter + 1);

            // Убираем пробелы
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            // Загружаем настройки
            if (key == "playerName") {
                settings.playerName = value;
            }
            else if (key == "playerColor") {
                std::istringstream iss(value);
                int r, g, b;
                if (iss >> r >> g >> b) {
                    settings.playerColor = sf::Color(r, g, b);
                }
            }
            else if (key == "botColor") {
                std::istringstream iss(value);
                int r, g, b;
                if (iss >> r >> g >> b) {
                    settings.botColor = sf::Color(r, g, b);
                }
            }
            else if (key == "fieldSize") {
                settings.fieldSize = std::stoi(value);
            }
            else if (key == "difficulty") {
                settings.difficulty = std::stoi(value);
            }
            else if (key == "snakeSpeed") {
                settings.snakeSpeed = std::stof(value);
            }
            else if (key == "snakeRotationSpeed") {
                settings.snakeRotationSpeed = std::stof(value);
            }
            else if (key == "initialSnakeLength") {
                settings.initialSnakeLength = std::stoi(value);
            }
            else if (key == "fruitSpawnRate") {
                settings.fruitSpawnRate = std::stof(value);
            }
            else if (key == "showGrid") {
                settings.showGrid = (value == "true" || value == "1");
            }
            else if (key == "soundVolume") {
                settings.soundVolume = std::stof(value);
            }
            else if (key == "musicVolume") {
                settings.musicVolume = std::stof(value);
            }
            else if (key == "soundEnabled") {
                settings.soundEnabled = (value == "true" || value == "1");
            }
            // Загрузка клавиш управления
            else if (key == "keyUp" || key == "keyAccelerate") {
                settings.playerKeys[0] = static_cast<sf::Keyboard::Key>(std::stoi(value));
            }
            else if (key == "keyLeft" || key == "keyTurnLeft") {
                settings.playerKeys[1] = static_cast<sf::Keyboard::Key>(std::stoi(value));
            }
            else if (key == "keyDown" || key == "keyDecelerate") {
                settings.playerKeys[2] = static_cast<sf::Keyboard::Key>(std::stoi(value));
            }
            else if (key == "keyRight" || key == "keyTurnRight") {
                settings.playerKeys[3] = static_cast<sf::Keyboard::Key>(std::stoi(value));
            }
        }

        file.close();
        std::cout << "Settings loaded from config.ini" << std::endl;
    }
    void resetToDefaultSettings() {
        std::cout << "Resetting to default settings..." << std::endl;

        // Создаем новый объект Settings (он инициализируется значениями по умолчанию)
        Settings defaultSettings;
        settings = defaultSettings;

        // Сохраняем настройки по умолчанию
        saveSettings();

        // Обновляем UI если нужно
        if (window) {
            // Перезагружаем меню настроек
            initPlayerSettings();
        }
    }
    void saveSettings() {
        std::ofstream file("config.ini");
        if (!file.is_open()) {
            std::cout << "Error: Could not save settings to config.ini" << std::endl;
            return;
        }

        // Заголовок с комментариями
        file << "# Snake Game Configuration File\n";
        file << "# Generated on: " << __DATE__ << " " << __TIME__ << "\n\n";

        // Настройки игрока
        file << "# Player Settings\n";
        file << "playerName=" << settings.playerName << "\n";
        file << "playerColor=" << (int)settings.playerColor.r << " "
            << (int)settings.playerColor.g << " "
            << (int)settings.playerColor.b << "\n";
        file << "botColor=" << (int)settings.botColor.r << " "
            << (int)settings.botColor.g << " "
            << (int)settings.botColor.b << "\n";
        file << "fieldSize=" << settings.fieldSize << "\n";
        file << "difficulty=" << settings.difficulty << "\n\n";

        // Игровой процесс
        file << "# Gameplay Settings\n";
        file << "snakeSpeed=" << settings.snakeSpeed << "\n";
        file << "snakeRotationSpeed=" << settings.snakeRotationSpeed << "\n";
        file << "initialSnakeLength=" << settings.initialSnakeLength << "\n";
        file << "fruitSpawnRate=" << settings.fruitSpawnRate << "\n\n";

        // Управление
        file << "# Control Settings\n";
        file << "keyAccelerate=" << static_cast<int>(settings.playerKeys[0]) << "\n";
        file << "keyTurnLeft=" << static_cast<int>(settings.playerKeys[1]) << "\n";
        file << "keyDecelerate=" << static_cast<int>(settings.playerKeys[2]) << "\n";
        file << "keyTurnRight=" << static_cast<int>(settings.playerKeys[3]) << "\n\n";

        // Графика
        file << "# Graphics Settings\n";
        file << "showGrid=" << (settings.showGrid ? "true" : "false") << "\n\n";

        // Звук
        file << "# Audio Settings\n";
        file << "soundVolume=" << settings.soundVolume << "\n";
        file << "musicVolume=" << settings.musicVolume << "\n";
        file << "soundEnabled=" << (settings.soundEnabled ? "true" : "false") << "\n\n";

        // Комментарий с инструкциями
        file << "# Key codes reference:\n";
        file << "# W=22, A=0, S=18, D=3\n";
        file << "# Up=73, Left=71, Down=74, Right=72\n";
        file << "# Space=57, Enter=58, Escape=36\n";

        file.close();
        std::cout << "Settings saved to config.ini" << std::endl;
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
        std::cout << "=== STARTING GAME ===" << std::endl;
        std::cout << "Player name: " << settings.playerName << std::endl;
        std::cout << "Rounds: " << rounds << ", Bots: " << bots << std::endl;
        std::cout << "Difficulty: " << settings.difficulty << std::endl;
        std::cout << "Speed: " << settings.snakeSpeed << std::endl;
        std::cout << "Initial length: " << settings.initialSnakeLength << std::endl;

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

        // Создание игрока - ТОЛЬКО ГОЛОВА
        
        Snake* player = new Snake(settings.playerName, settings.playerColor, settings, true);
        // Настройка управления
        player->moveUp = settings.playerKeys[0];    // W
        player->moveDown = settings.playerKeys[2];  // S
        player->moveLeft = settings.playerKeys[1];  // A
        player->moveRight = settings.playerKeys[3]; // D

        // Настройка скорости в зависимости от сложности
        switch (settings.difficulty) {
        case 1: player->moveInterval = 0.2f; break;   // Легкая - медленно
        case 2: player->moveInterval = 0.15f; break;  // Средняя
        case 3: player->moveInterval = 0.1f; break;   // Сложная - быстро
        }

        snakes.push_back(player);
        gameData.playerName = settings.playerName;
        gameData.playerColor = settings.playerColor;

        std::cout << "Player created - only head" << std::endl;

        // Создание ботов
        for (int i = 0; i < bots; i++) {
            Snake* bot = new Snake("Bot " + std::to_string(i + 1), settings.botColor, settings, false);
            // Боты тоже начинают с одной головы
            bot->body[0] = sf::Vector2f(200 + i * 150, 200 + i * 100);
            snakes.push_back(bot);
        }

        // Создание начальных фруктов
        for (int i = 0; i < 3; i++) { // Меньше фруктов для начала
            spawnGameFruit();
        }

        std::cout << "Fruits created: " << gameFruits.size() << std::endl;

        gameClock.restart();
        fruitSpawnClock.restart();

        std::cout << "Game started successfully!" << std::endl;

    }

    void spawnGameFruit() {
        if (snakes.empty()) return;

        // Размер клетки берем из змейки
        int cellSize = snakes[0]->cellSize;

        // Рассчитываем доступное пространство для спавна в клетках
        int minGridX = 50 / cellSize + 1;
        int maxGridX = 950 / cellSize - 1;
        int minGridY = 50 / cellSize + 1;
        int maxGridY = 850 / cellSize - 1;

        // Генерируем случайные координаты клетки
        int gridX = minGridX + rand() % (maxGridX - minGridX + 1);
        int gridY = minGridY + rand() % (maxGridY - minGridY + 1);

        // Преобразуем координаты клетки в пиксельные координаты
        // +1 чтобы быть внутри клетки, а не на границе
        float x = gridX * cellSize + 1;
        float y = gridY * cellSize + 1;

        float spoilRate = settings.fruitSpoilRateMedium;

        switch (settings.difficulty) {
        case 1: spoilRate = settings.fruitSpoilRateEasy; break;
        case 2: spoilRate = settings.fruitSpoilRateMedium; break;
        case 3: spoilRate = settings.fruitSpoilRateHard; break;
        }

        Fruit* fruit = new Fruit(sf::Vector2f(x, y), 10.0f, spoilRate);
        gameFruits.push_back(fruit);
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
        if (snakes.empty()) return;

        int cellSize = snakes[0]->cellSize;

        // Проверка столкновения с фруктами
        for (size_t i = 0; i < gameFruits.size(); i++) {
            if (!gameFruits[i]->isGood) continue;

            for (auto snake : snakes) {
                if (!snake->isAlive) continue;

                // Проверяем столкновение головы с фруктом
                sf::Vector2f headPos = snake->body[0];
                sf::Vector2f fruitPos = gameFruits[i]->position;

                // Проверяем расстояние
                float dx = headPos.x - fruitPos.x;
                float dy = headPos.y - fruitPos.y;
                float distance = sqrt(dx * dx + dy * dy);

                // Если голова достаточно близко к фрукту
                if (distance < cellSize / 2) {
                    snake->grow();
                    gameFruits[i]->isGood = false;
                    gameData.score = snake->score;
                    break;
                }
            }
        }

        // Удаление испорченных фруктов
        gameFruits.erase(std::remove_if(gameFruits.begin(), gameFruits.end(),
            [](Fruit* f) { return !f->isGood; }), gameFruits.end());

        // Проверка столкновений со стенами
        for (auto snake : snakes) {
            if (!snake->isAlive) continue;

            sf::Vector2f head = snake->body[0];

            // Проверяем границы
            if (head.x < 50 || head.x > 950 - snake->cellSize ||
                head.y < 50 || head.y > 850 - snake->cellSize) {
                snake->isAlive = false;
                std::cout << snake->name << " died by hitting the wall!" << std::endl;
            }
        }

        // Проверка столкновений с телом (САМОСТОЛКНОВЕНИЕ)
        for (auto snake : snakes) {
            if (!snake->isAlive) continue;

            
            for (size_t i = 3; i < snake->body.size(); i++) {
                // Координаты клеток
                int headCellX = static_cast<int>(snake->body[0].x / cellSize);
                int headCellY = static_cast<int>(snake->body[0].y / cellSize);
                int segmentCellX = static_cast<int>(snake->body[i].x / cellSize);
                int segmentCellY = static_cast<int>(snake->body[i].y / cellSize);

                // Если голова в той же клетке, что и сегмент тела
                if (headCellX == segmentCellX && headCellY == segmentCellY) {
                    snake->isAlive = false;
                    std::cout << snake->name << " died by self-collision!" << std::endl;
                    std::cout << "Head at cell: (" << headCellX << ", " << headCellY << ")" << std::endl;
                    std::cout << "Body segment " << i << " at cell: ("
                        << segmentCellX << ", " << segmentCellY << ")" << std::endl;
                    std::cout << "Snake length: " << snake->body.size() << std::endl;
                    break;
                }
            }
        }

        // Проверка столкновений между змейками (если есть несколько игроков/ботов)
        for (size_t i = 0; i < snakes.size(); i++) {
            if (!snakes[i]->isAlive) continue;

            for (size_t j = 0; j < snakes.size(); j++) {
                if (i == j || !snakes[j]->isAlive) continue;

                // Проверяем столкновение головы змейки i с телом змейки j
                int headCellX_i = static_cast<int>(snakes[i]->body[0].x / cellSize);
                int headCellY_i = static_cast<int>(snakes[i]->body[0].y / cellSize);

                for (size_t k = 0; k < snakes[j]->body.size(); k++) {
                    int segmentCellX = static_cast<int>(snakes[j]->body[k].x / cellSize);
                    int segmentCellY = static_cast<int>(snakes[j]->body[k].y / cellSize);

                    if (headCellX_i == segmentCellX && headCellY_i == segmentCellY) {
                        snakes[i]->isAlive = false;
                        std::cout << snakes[i]->name << " died by colliding with "
                            << snakes[j]->name << "!" << std::endl;
                        break;
                    }
                }

                if (!snakes[i]->isAlive) break;
            }
        }

        
    }

    void checkGameOver() {
        if (gameState != PLAYING) return;

        // Проверяем, жив ли игрок
        bool playerAlive = false;
        Snake* playerSnake = nullptr;

        for (auto snake : snakes) {
            if (snake->isPlayer) {
                playerAlive = snake->isAlive;
                playerSnake = snake;
                break;
            }
        }

        // Если игрок мертв - показываем диалог Game Over
        if (!playerAlive) {
            gameState = GAME_OVER;

            // Определяем причину смерти
            std::string deathReason = "Game Over";
            if (playerSnake) {
                sf::Vector2f head = playerSnake->body[0];

                // Проверка столкновения со стеной
                if (head.x < 50 || head.x > 950 - playerSnake->cellSize ||
                    head.y < 50 || head.y > 850 - playerSnake->cellSize) {
                    deathReason = "You hit the wall!";
                }
                else {
                    deathReason = "You collided with yourself!";
                }
            }

            // Показываем диалог Game Over
            gameOverDialog->show("Game Over", sf::Color::Red,
                gameData.score, gameData.roundWins,
                gameData.totalRounds, deathReason);
            return;
        }
    }

    void endRound(bool playerWon) {
        if (playerWon) {
            gameData.roundWins++;
            std::cout << "Player won round " << gameData.currentRound << std::endl;
        }
        else {
            std::cout << "Player lost round " << gameData.currentRound << std::endl;
        }

        if (gameData.currentRound >= gameData.totalRounds) {
            endGame(playerWon);
        }
        else {
            // Начинаем следующий раунд
            gameData.currentRound++;

            // Пересоздаем змей с новыми позициями
            for (auto snake : snakes) {
                snake->isAlive = true;
                snake->rotation = 0.0f;
                snake->speed = 150.0f; // Сброс скорости

                // Новая случайная позиция
                snake->body.clear();
                float x = 200 + rand() % 600; // Не слишком близко к краям
                float y = 200 + rand() % 500;
                snake->body.push_back(sf::Vector2f(x, y));

                // Тело
                for (int i = 1; i < 5; i++) {
                    snake->body.push_back(sf::Vector2f(x - i * 20, y));
                }
            }

            // Очищаем фрукты
            for (auto fruit : gameFruits) delete fruit;
            gameFruits.clear();

            // Создаем новые фрукты
            for (int i = 0; i < 5; i++) {
                spawnGameFruit();
            }

            // Сброс таймеров
            gameData.gameTime = 0.0f;
            gameClock.restart();
            fruitSpawnClock.restart();

            std::cout << "Starting round " << gameData.currentRound << std::endl;
        }
    }

    void endGame(bool playerWon) {
        gameState = GAME_OVER;

        std::string winner;
        sf::Color winnerColor;
        std::string deathReason = "";

        // Определяем победителя
        if (playerWon) {
            winner = gameData.playerName;
            winnerColor = gameData.playerColor;
            deathReason = "You won!";
        }
        else {
            // Ищем живого бота или определяем причину смерти игрока
            bool foundAliveBot = false;
            for (auto snake : snakes) {
                if (!snake->isPlayer && snake->isAlive) {
                    winner = snake->name;
                    winnerColor = snake->color;
                    deathReason = "Defeated by " + snake->name;
                    foundAliveBot = true;
                    break;
                }
            }

            if (!foundAliveBot) {
                // Определяем причину смерти игрока
                Snake* playerSnake = nullptr;
                for (auto snake : snakes) {
                    if (snake->isPlayer) {
                        playerSnake = snake;
                        break;
                    }
                }

                if (playerSnake) {
                    // Проверяем, умер ли игрок от столкновения со стеной или с собой
                    sf::Vector2f head = playerSnake->body[0];

                    // Проверка столкновения со стеной
                    if (head.x < 50 || head.x > 950 - playerSnake->cellSize ||
                        head.y < 50 || head.y > 850 - playerSnake->cellSize) {
                        deathReason = "You hit the wall!";
                    }
                    // Проверка столкновения с собой
                    else {
                        deathReason = "You collided with yourself!";
                    }
                }

                winner = "Game Over";
                winnerColor = sf::Color::Red;
            }
        }

        // Показываем диалог окончания игры с причиной
        gameOverDialog->show(winner, winnerColor,
            gameData.score, gameData.roundWins,
            gameData.totalRounds, deathReason);
    }

    void handleGameInput() {
        if (gameState != PLAYING) return;

        Snake* playerSnake = nullptr;
        for (auto snake : snakes) {
            if (snake->isPlayer) {
                playerSnake = snake;
                break;
            }
        }

        if (!playerSnake) return;

        // Можно менять направление в любой момент, но без разворота на 180°
        if (sf::Keyboard::isKeyPressed(playerSnake->moveUp) &&
            !(playerSnake->direction.x == 0 && playerSnake->direction.y == 1)) {
            playerSnake->nextDirection = sf::Vector2f(0, -1);
        }
        else if (sf::Keyboard::isKeyPressed(playerSnake->moveDown) &&
            !(playerSnake->direction.x == 0 && playerSnake->direction.y == -1)) {
            playerSnake->nextDirection = sf::Vector2f(0, 1);
        }
        else if (sf::Keyboard::isKeyPressed(playerSnake->moveLeft) &&
            !(playerSnake->direction.x == 1 && playerSnake->direction.y == 0)) {
            playerSnake->nextDirection = sf::Vector2f(-1, 0);
        }
        else if (sf::Keyboard::isKeyPressed(playerSnake->moveRight) &&
            !(playerSnake->direction.x == -1 && playerSnake->direction.y == 0)) {
            playerSnake->nextDirection = sf::Vector2f(1, 0);
        }
    }

    void renderGame() {
        if (gameState == MENU) return;

        // Рисуем фон
        sf::RectangleShape gameBg;
        gameBg.setSize(sf::Vector2f(1000, 900));
        gameBg.setFillColor(sf::Color(20, 40, 20));
        window->draw(gameBg);

        // Рисуем игровое поле с сеткой
        sf::RectangleShape field;
        field.setSize(sf::Vector2f(900, 800));
        field.setPosition(50, 50);
        field.setFillColor(sf::Color(10, 30, 10));
        field.setOutlineThickness(2);
        field.setOutlineColor(sf::Color::White);
        window->draw(field);

        // Рисуем сетку ТОЛЬКО если включено в настройках
        if (settings.showGrid && snakes.size() > 0) {
            int cellSize = snakes[0]->cellSize;

            // Рисуем границы клеток - ТОНКИМИ светло-серыми линиями
            for (int y = 50; y <= 850; y += cellSize) {
                sf::RectangleShape hLine(sf::Vector2f(900, 1));
                hLine.setPosition(50, y);
                hLine.setFillColor(sf::Color(150, 150, 150, 80)); // Светло-серый, полупрозрачный
                window->draw(hLine);
            }

            for (int x = 50; x <= 950; x += cellSize) {
                sf::RectangleShape vLine(sf::Vector2f(1, 800));
                vLine.setPosition(x, 50);
                vLine.setFillColor(sf::Color(150, 150, 150, 80)); // Светло-серый, полупрозрачный
                window->draw(vLine);
            }
        }

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

        if (!font.loadFromFile("Ubuntu-Regular.ttf")) {
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
                    // Обработка паузы по ESC
                    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                        gameState = PAUSED;
                        pauseDialog->show();
                    }
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
                                // Начинаем игру заново с теми же параметрами
                                startGame(gameData.totalRounds, gameData.totalBots);
                                gameState = PLAYING;
                                gameOverDialog->hide();
                            }
                            else if (action == "exit") {
                                // Возвращаемся в главное меню
                                gameState = MENU;
                                gameOverDialog->hide();

                                // Очистка игровых объектов
                                for (auto snake : snakes) delete snake;
                                for (auto fruit : gameFruits) delete fruit;
                                snakes.clear();
                                gameFruits.clear();
                                showTitle = true;
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
                handleGameInput();
                updateGame(deltaTime);
                checkCollisions();
                checkGameOver(); // Проверяем, не закончилась ли игра
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
            else if (gameState == PLAYING || gameState == PAUSED || gameState == GAME_OVER) {
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
            showTitle = true;
        }
    }


    void handleClick(sf::Vector2f mousePos) {
        bool clicked = false;

        switch (currentScreen) {
        case Screen::MAIN:
            clicked = handleMouseClick(mainButtons, mousePos);
            if (clicked) {
                std::string action = "";
                for (auto& btn : mainButtons) {
                    if (btn->clicked) {
                        action = btn->action;
                        break;
                    }
                }

                if (action == "Начать") {
                    currentScreen = Screen::GAME_SETTINGS;
                    initGameSettings();
                    showTitle = false;  // Скрываем заголовок
                }
                else if (action == "Настройки") {
                    currentScreen = Screen::PLAYER_SETTINGS;
                    initPlayerSettings();
                    showTitle = false;  // Скрываем заголовок
                }
                else if (action == "О создателях") {
                    currentScreen = Screen::ABOUT;
                    initAbout();
                    showTitle = false;  // Скрываем заголовок
                }
                else if (action == "Выход") {
                    currentScreen = Screen::EXIT_CONFIRM;
                    showTitle = false;  // Скрываем заголовок
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
                if (action == "back") {
                    currentScreen = Screen::MAIN;
                    initMainMenu();
                    showTitle = true;  
                }
                else if (action == "reset_defaults") {
                    resetToDefaultSettings();
                    // Обновить отображаемые настройки
                    playerSettingsButtons[8]->text.setString("Field size: " + std::to_string(settings.fieldSize));
                }
                break;
             }
            
            else {
                clicked = handleMouseClick(difficultyButtons, mousePos);
                if (!clicked) {
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
                if (action == "back") {
                    currentScreen = Screen::MAIN;
                    initMainMenu();
                    showTitle = true;  
                }
            }
            break;

        case Screen::ABOUT:
            clicked = handleMouseClick(aboutButtons, mousePos);
            if (clicked && aboutButtons[0]->clicked) {
                currentScreen = Screen::MAIN;
                initMainMenu();
                showTitle = true;  
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
                if (action == "exit_no") {
                    currentScreen = Screen::MAIN;
                    initMainMenu();
                    showTitle = true;  
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
        window->clear(bgColor);

        if (backgroundLoaded) {
            window->draw(backgroundSprite);
            sf::RectangleShape overlay;
            overlay.setSize(sf::Vector2f(1000, 900));
            overlay.setFillColor(sf::Color(0, 0, 0, 100));
            window->draw(overlay);
        }

        if (gameState == MENU) {
            
            if (showTitle && currentScreen == Screen::MAIN) {
                sf::Text title;
                title.setFont(font);
                title.setCharacterSize(50);
                title.setFillColor(sf::Color::White);
                title.setStyle(sf::Text::Bold);
                title.setString("Snake Game");

                // Центрирование по горизонтали
                sf::FloatRect titleBounds = title.getLocalBounds();
                title.setOrigin(titleBounds.left + titleBounds.width / 2.0f,
                    titleBounds.top + titleBounds.height / 2.0f);
                title.setPosition(500, 100);  // Центр экрана

                // Тень
                sf::Text titleShadow = title;
                titleShadow.setFillColor(sf::Color::Black);
                titleShadow.setPosition(title.getPosition().x + 2, title.getPosition().y + 2);
                window->draw(titleShadow);
                window->draw(title);
            }
            

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
                // Информация о настройках
                info.setString("Rounds: " + std::to_string(rounds) + " | Bots: " + std::to_string(bots));
                sf::FloatRect infoBounds = info.getLocalBounds();
                info.setOrigin(infoBounds.left + infoBounds.width / 2.0f,
                    infoBounds.top + infoBounds.height / 2.0f);
                info.setPosition(500, 150);  // Сдвигаем выше, так как нет заголовка

                sf::Text infoShadow = info;
                infoShadow.setFillColor(sf::Color::Black);
                infoShadow.setPosition(info.getPosition().x + 2, info.getPosition().y + 2);
                window->draw(infoShadow);
                window->draw(info);

                // Отображаем выбранную сложность
                sf::Text diffInfo;
                diffInfo.setFont(font);
                diffInfo.setCharacterSize(20);
                diffInfo.setFillColor(sf::Color::White);
                std::string diffText;
                if (settings.difficulty == 1) diffText = "Easy (slow spoil)";
                else if (settings.difficulty == 2) diffText = "Medium";
                else diffText = "Hard (fast spoil)";
                diffInfo.setString("Difficulty: " + diffText);

                // Центрируем текст сложности
                sf::FloatRect diffBounds = diffInfo.getLocalBounds();
                diffInfo.setOrigin(diffBounds.left + diffBounds.width / 2.0f,
                    diffBounds.top + diffBounds.height / 2.0f);
                diffInfo.setPosition(500, 480);

                sf::Text diffInfoShadow = diffInfo;
                diffInfoShadow.setFillColor(sf::Color::Black);
                diffInfoShadow.setPosition(diffInfo.getPosition().x + 2, diffInfo.getPosition().y + 2);
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

                // Центрируем заголовок демонстрации фруктов
                sf::FloatRect fruitDemoBounds = fruitDemoText.getLocalBounds();
                fruitDemoText.setOrigin(fruitDemoBounds.left + fruitDemoBounds.width / 2.0f,
                    fruitDemoBounds.top + fruitDemoBounds.height / 2.0f);
                fruitDemoText.setPosition(500, 230);
                window->draw(fruitDemoText);

                // Создаем примеры фруктов для демонстрации
                std::vector<Fruit> demoFruits;
                // Располагаем фрукты тоже по центру
                demoFruits.push_back(Fruit(sf::Vector2f(450, 270), 10.0f, 0.5f)); // Легкий
                demoFruits.push_back(Fruit(sf::Vector2f(500, 270), 10.0f, 1.0f)); // Средний  
                demoFruits.push_back(Fruit(sf::Vector2f(550, 270), 10.0f, 2.0f)); // Сложный

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
                    fruitLabel.setPosition(450 + i * 100, 300);
                    window->draw(fruitLabel);
                }

                // Сброс демо-таймера
                if (demoTime > 20.0f) demoTime = 0;
                break;
            }

            case Screen::PLAYER_SETTINGS: {
                // Заголовок по центру
                sf::Text title;
                title.setFont(font);
                title.setString("НАСТРОЙКИ");
                title.setCharacterSize(42);
                title.setFillColor(sf::Color::Yellow);
                title.setStyle(sf::Text::Bold);

                sf::FloatRect titleBounds = title.getLocalBounds();
                title.setOrigin(titleBounds.left + titleBounds.width / 2.0f,
                    titleBounds.top + titleBounds.height / 2.0f);
                title.setPosition(500, 100);
                window->draw(title);

                
                sf::Text colorTitle;
                colorTitle.setFont(font);
                colorTitle.setString("Цвет игрока:");
                colorTitle.setCharacterSize(24);
                colorTitle.setFillColor(sf::Color(200, 200, 255));
                colorTitle.setPosition(100, 160);
                window->draw(colorTitle);

                // Отображаем текущий выбранный цвет рядом с заголовком
                sf::Text currentColorText;
                currentColorText.setFont(font);

                // Определяем название цвета в зависимости от выбранного
                std::string colorName = "Неизвестно";

                // Сравниваем с предопределенными цветами SFML
                if (settings.playerColor == sf::Color::Green) colorName = "Зеленый";
                else if (settings.playerColor == sf::Color::Blue) colorName = "Синий";
                else if (settings.playerColor == sf::Color::Red) colorName = "Красный";
                else if (settings.playerColor == sf::Color::Magenta) colorName = "Фиолетовый";
                else if (settings.playerColor == sf::Color::Yellow) colorName = "Желтый";
                else if (settings.playerColor == sf::Color::Cyan) colorName = "Голубой";
                else if (settings.playerColor == sf::Color::White) colorName = "Белый";
                else if (settings.playerColor == sf::Color::Black) colorName = "Черный";

                currentColorText.setString("(" + colorName + ")");
                currentColorText.setCharacterSize(20);
                currentColorText.setFillColor(sf::Color(255, 255, 150));
                currentColorText.setPosition(260, 160);
                window->draw(currentColorText);

                
                float colorStartX = 100;
                float colorY = 200;
                float colorSpacing = 200;

                for (int i = 0; i < 4 && i < playerSettingsButtons.size(); i++) {
                    playerSettingsButtons[i]->rect.setPosition(colorStartX + i * colorSpacing, colorY);
                    playerSettingsButtons[i]->rect.setSize(sf::Vector2f(180, 50));

                    // Центрируем текст в кнопке
                    sf::FloatRect textBounds = playerSettingsButtons[i]->text.getLocalBounds();
                    playerSettingsButtons[i]->text.setOrigin(
                        textBounds.left + textBounds.width / 2.0f,
                        textBounds.top + textBounds.height / 2.0f
                    );
                    playerSettingsButtons[i]->text.setPosition(
                        colorStartX + i * colorSpacing + 90, 
                        colorY + 25 
                    );

                    window->draw(playerSettingsButtons[i]->rect);
                    window->draw(playerSettingsButtons[i]->text);
                }

                
                sf::Text controlTitle;
                controlTitle.setFont(font);
                controlTitle.setString("Управление:");
                controlTitle.setCharacterSize(24);
                controlTitle.setFillColor(sf::Color(200, 200, 255));
                controlTitle.setPosition(100, 280);
                window->draw(controlTitle);

                // Кнопки управления в ряд (3 штуки)
                float controlStartX = 100;
                float controlY = 320;

                for (int i = 4; i < 7 && i < playerSettingsButtons.size(); i++) {
                    playerSettingsButtons[i]->rect.setPosition(controlStartX + (i - 4) * 250, controlY);
                    playerSettingsButtons[i]->rect.setSize(sf::Vector2f(230, 50));

                    // Центрируем текст
                    sf::FloatRect textBounds = playerSettingsButtons[i]->text.getLocalBounds();
                    playerSettingsButtons[i]->text.setOrigin(
                        textBounds.left + textBounds.width / 2.0f,
                        textBounds.top + textBounds.height / 2.0f
                    );
                    playerSettingsButtons[i]->text.setPosition(
                        controlStartX + (i - 4) * 250 + 115,
                        controlY + 25
                    );

                    window->draw(playerSettingsButtons[i]->rect);
                    window->draw(playerSettingsButtons[i]->text);
                }

                // Отображение текущих клавиш
                sf::Text currentKeysText;
                currentKeysText.setFont(font);
                currentKeysText.setString("Текущие: " +
                    getKeyName(settings.playerKeys[0]) + " " +
                    getKeyName(settings.playerKeys[1]) + " " +
                    getKeyName(settings.playerKeys[2]) + " " +
                    getKeyName(settings.playerKeys[3]));
                currentKeysText.setCharacterSize(20);
                currentKeysText.setFillColor(sf::Color(255, 255, 200));
                currentKeysText.setPosition(100, 380);
                window->draw(currentKeysText);

                
                sf::Text nameTitle;
                nameTitle.setFont(font);
                nameTitle.setString("Имя игрока:");
                nameTitle.setCharacterSize(24);
                nameTitle.setFillColor(sf::Color(200, 200, 255));
                nameTitle.setPosition(100, 420);
                window->draw(nameTitle);

                // Отображаем текущее имя рядом с заголовком
                sf::Text currentNameLabel;
                currentNameLabel.setFont(font);
                currentNameLabel.setString("Текущее: " + settings.playerName);
                currentNameLabel.setCharacterSize(20);
                currentNameLabel.setFillColor(sf::Color(150, 255, 150));
                currentNameLabel.setPosition(260, 420);
                window->draw(currentNameLabel);

                // Кнопка имени (индекс 7)
                if (7 < playerSettingsButtons.size()) {
                    playerSettingsButtons[7]->rect.setPosition(100, 460);
                    playerSettingsButtons[7]->rect.setSize(sf::Vector2f(400, 50));

                    // Центрируем текст в кнопке
                    sf::FloatRect nameBounds = playerSettingsButtons[7]->text.getLocalBounds();
                    playerSettingsButtons[7]->text.setOrigin(
                        nameBounds.left + nameBounds.width / 2.0f,
                        nameBounds.top + nameBounds.height / 2.0f
                    );
                    playerSettingsButtons[7]->text.setPosition(300, 485);

                    window->draw(playerSettingsButtons[7]->rect);
                    window->draw(playerSettingsButtons[7]->text);
                }

                
                sf::Text gameTitle;
                gameTitle.setFont(font);
                gameTitle.setString("Параметры игры:");
                gameTitle.setCharacterSize(24);
                gameTitle.setFillColor(sf::Color(200, 200, 255));
                gameTitle.setPosition(100, 530);
                window->draw(gameTitle);

                // Размер поля (индекс 8) - теперь с нормальными размерами
                if (8 < playerSettingsButtons.size()) {
                    playerSettingsButtons[8]->rect.setPosition(100, 570);
                    playerSettingsButtons[8]->rect.setSize(sf::Vector2f(300, 50));

                    // Обновляем текст кнопки в зависимости от текущего размера поля
                    std::string fieldSizeText;
                    switch (settings.fieldSize) {
                    case 10: fieldSizeText = "Размер поля: 10x10"; break;
                    case 15: fieldSizeText = "Размер поля: 15x15"; break;
                    case 20: fieldSizeText = "Размер поля: 20x20"; break;
                    case 25: fieldSizeText = "Размер поля: 25x25"; break;
                    default: fieldSizeText = "Размер поля: " + std::to_string(settings.fieldSize) + "x" + std::to_string(settings.fieldSize); break;
                    }
                    playerSettingsButtons[8]->text.setString(fieldSizeText);

                    // Центрируем текст
                    sf::FloatRect fieldBounds = playerSettingsButtons[8]->text.getLocalBounds();
                    playerSettingsButtons[8]->text.setOrigin(
                        fieldBounds.left + fieldBounds.width / 2.0f,
                        fieldBounds.top + fieldBounds.height / 2.0f
                    );
                    playerSettingsButtons[8]->text.setPosition(250, 595);

                    window->draw(playerSettingsButtons[8]->rect);
                    window->draw(playerSettingsButtons[8]->text);
                }

                // Сложность (индекс 9)
                if (9 < playerSettingsButtons.size()) {
                    playerSettingsButtons[9]->rect.setPosition(450, 570);
                    playerSettingsButtons[9]->rect.setSize(sf::Vector2f(300, 50));

                    // Центрируем текст
                    sf::FloatRect diffBounds = playerSettingsButtons[9]->text.getLocalBounds();
                    playerSettingsButtons[9]->text.setOrigin(
                        diffBounds.left + diffBounds.width / 2.0f,
                        diffBounds.top + diffBounds.height / 2.0f
                    );
                    playerSettingsButtons[9]->text.setPosition(600, 595);

                    window->draw(playerSettingsButtons[9]->rect);
                    window->draw(playerSettingsButtons[9]->text);
                }

                
                float actionStartX = 100;    // Левая граница
                float actionStartY = 750;    // В самый низ экрана
                float actionSpacing = 220;   // Расстояние между кнопками

                // Установки по умолчанию (индекс 10)
                if (10 < playerSettingsButtons.size()) {
                    playerSettingsButtons[10]->rect.setPosition(actionStartX, actionStartY);
                    playerSettingsButtons[10]->rect.setSize(sf::Vector2f(250, 60));

                    playerSettingsButtons[10]->text.setString("Установки по умолчанию");

                    // Центрируем текст
                    sf::FloatRect defaultBounds = playerSettingsButtons[10]->text.getLocalBounds();
                    playerSettingsButtons[10]->text.setOrigin(
                        defaultBounds.left + defaultBounds.width / 2.0f,
                        defaultBounds.top + defaultBounds.height / 2.0f
                    );
                    playerSettingsButtons[10]->text.setPosition(actionStartX + 125, actionStartY + 30);

                    window->draw(playerSettingsButtons[10]->rect);
                    window->draw(playerSettingsButtons[10]->text);
                }

                // Сохранить (индекс 11)
                if (11 < playerSettingsButtons.size()) {
                    playerSettingsButtons[11]->rect.setPosition(actionStartX + actionSpacing, actionStartY);
                    playerSettingsButtons[11]->rect.setSize(sf::Vector2f(200, 60));

                    // Центрируем текст
                    sf::FloatRect saveBounds = playerSettingsButtons[11]->text.getLocalBounds();
                    playerSettingsButtons[11]->text.setOrigin(
                        saveBounds.left + saveBounds.width / 2.0f,
                        saveBounds.top + saveBounds.height / 2.0f
                    );
                    playerSettingsButtons[11]->text.setPosition(actionStartX + actionSpacing + 100, actionStartY + 30);

                    window->draw(playerSettingsButtons[11]->rect);
                    window->draw(playerSettingsButtons[11]->text);
                }

                // Сбросить (индекс 12)
                if (12 < playerSettingsButtons.size()) {
                    playerSettingsButtons[12]->rect.setPosition(actionStartX + 2 * actionSpacing, actionStartY);
                    playerSettingsButtons[12]->rect.setSize(sf::Vector2f(200, 60));

                    // Центрируем текст
                    sf::FloatRect resetBounds = playerSettingsButtons[12]->text.getLocalBounds();
                    playerSettingsButtons[12]->text.setOrigin(
                        resetBounds.left + resetBounds.width / 2.0f,
                        resetBounds.top + resetBounds.height / 2.0f
                    );
                    playerSettingsButtons[12]->text.setPosition(actionStartX + 2 * actionSpacing + 100, actionStartY + 30);

                    window->draw(playerSettingsButtons[12]->rect);
                    window->draw(playerSettingsButtons[12]->text);
                }

                // Назад (индекс 13) - добавляем новую кнопку или переименовываем существующую
                if (13 < playerSettingsButtons.size()) {
                    playerSettingsButtons[13]->rect.setPosition(actionStartX + 3 * actionSpacing, actionStartY);
                    playerSettingsButtons[13]->rect.setSize(sf::Vector2f(200, 60));

                    // Центрируем текст
                    sf::FloatRect backBounds = playerSettingsButtons[13]->text.getLocalBounds();
                    playerSettingsButtons[13]->text.setOrigin(
                        backBounds.left + backBounds.width / 2.0f,
                        backBounds.top + backBounds.height / 2.0f
                    );
                    playerSettingsButtons[13]->text.setPosition(actionStartX + 3 * actionSpacing + 100, actionStartY + 30);

                    window->draw(playerSettingsButtons[13]->rect);
                    window->draw(playerSettingsButtons[13]->text);
                }

                // Диалог ввода имени (если активен)
                if (nameInputActive) {
                    // Простой диалог поверх всего
                    sf::RectangleShape overlay(sf::Vector2f(1000, 900));
                    overlay.setFillColor(sf::Color(0, 0, 0, 200));
                    window->draw(overlay);

                    sf::RectangleShape dialog(sf::Vector2f(400, 200));
                    dialog.setPosition(300, 350);
                    dialog.setFillColor(sf::Color(50, 50, 70));
                    dialog.setOutlineThickness(2);
                    dialog.setOutlineColor(sf::Color::White);
                    window->draw(dialog);

                    sf::Text prompt;
                    prompt.setFont(font);
                    prompt.setString("Введите имя:");
                    prompt.setCharacterSize(24);
                    prompt.setFillColor(sf::Color::White);
                    prompt.setPosition(320, 370);
                    window->draw(prompt);

                    sf::Text inputText;
                    inputText.setFont(font);
                    inputText.setString(nameInputText + "_");
                    inputText.setCharacterSize(24);
                    inputText.setFillColor(sf::Color::Yellow);
                    inputText.setPosition(320, 420);
                    window->draw(inputText);

                    // Показываем текущее имя под полем ввода
                    sf::Text currentInfo;
                    currentInfo.setFont(font);
                    currentInfo.setString("Текущее: " + settings.playerName);
                    currentInfo.setCharacterSize(18);
                    currentInfo.setFillColor(sf::Color(200, 200, 255));
                    currentInfo.setPosition(320, 470);
                    window->draw(currentInfo);
                }

                break;
            }

            case Screen::ABOUT: {
                // Полупрозрачный фон
                sf::RectangleShape overlay;
                overlay.setSize(sf::Vector2f(400, 320));
                overlay.setPosition(300, 160);
                overlay.setFillColor(sf::Color(30, 30, 50, 180));
                overlay.setOutlineThickness(2);
                overlay.setOutlineColor(sf::Color::White);
                window->draw(overlay);

                // Заголовок
                sf::Text title;
                title.setFont(font);
                title.setString("О создателях");
                title.setCharacterSize(42);
                title.setFillColor(sf::Color::Yellow);
                title.setStyle(sf::Text::Bold);

                // Центрируем заголовок
                sf::FloatRect titleRect = title.getLocalBounds();
                title.setOrigin(titleRect.left + titleRect.width / 2.0f,
                    titleRect.top + titleRect.height / 2.0f);
                title.setPosition(500, 200);

                // Тень заголовка
                sf::Text titleShadow = title;
                titleShadow.setFillColor(sf::Color::Black);
                titleShadow.setPosition(503, 203);
                window->draw(titleShadow);
                window->draw(title);

                // Список создателей
                sf::Text creators;
                creators.setFont(font);
                creators.setString("Баринов Вадим\nГололобов Егор\nЕгорчик");
                creators.setCharacterSize(36);
                creators.setFillColor(sf::Color::White);
                creators.setLineSpacing(1.5f);

                // Центрируем список
                sf::FloatRect creatorsRect = creators.getLocalBounds();
                creators.setOrigin(creatorsRect.left + creatorsRect.width / 2.0f,
                    creatorsRect.top + creatorsRect.height / 2.0f);
                creators.setPosition(500, 350);

                // Тень списка
                sf::Text creatorsShadow = creators;
                creatorsShadow.setFillColor(sf::Color::Black);
                creatorsShadow.setPosition(502, 352);
                window->draw(creatorsShadow);
                window->draw(creators);

                // Кнопка "Назад"
                for (auto btn : aboutButtons) {
                    window->draw(btn->rect);
                    window->draw(btn->text);
                }
                break;
            }

            case Screen::EXIT_CONFIRM: {
                // Полупрозрачный темный фон
                sf::RectangleShape overlay;
                overlay.setSize(sf::Vector2f(1000, 900));
                overlay.setFillColor(sf::Color(0, 0, 0, 180));
                window->draw(overlay);

                // Диалоговое окно
                sf::RectangleShape dialog;
                dialog.setSize(sf::Vector2f(500, 200));
                dialog.setPosition(250, 300);
                dialog.setFillColor(sf::Color(40, 40, 60, 230));
                dialog.setOutlineColor(sf::Color(150, 150, 200));
                dialog.setOutlineThickness(3);
                window->draw(dialog);

                // Вопрос по центру
                sf::Text question;
                question.setFont(font);
                question.setString("Хотите выйти из игры?");
                question.setCharacterSize(34);
                question.setFillColor(sf::Color::White);
                question.setStyle(sf::Text::Bold);

                // Центрируем текст по горизонтали
                sf::FloatRect textBounds = question.getLocalBounds();
                question.setOrigin(textBounds.left + textBounds.width / 2.0f,
                    textBounds.top + textBounds.height / 2.0f);
                question.setPosition(500, 340);

                // Тень текста для лучшей читаемости
                sf::Text questionShadow = question;
                questionShadow.setFillColor(sf::Color(0, 0, 0, 180));
                questionShadow.setPosition(502, 342);
                window->draw(questionShadow);
                window->draw(question);

                // Кнопки
                float buttonSpacing = 30.0f;

                // Кнопка "Да" - красная
                exitConfirmButtons[0]->rect.setSize(sf::Vector2f(150, 50));
                exitConfirmButtons[0]->rect.setPosition(300, 400);
                exitConfirmButtons[0]->rect.setFillColor(sf::Color(180, 60, 60, 220));
                exitConfirmButtons[0]->rect.setOutlineThickness(2);
                exitConfirmButtons[0]->rect.setOutlineColor(sf::Color(220, 100, 100));

                exitConfirmButtons[0]->text.setString("Да");
                exitConfirmButtons[0]->text.setCharacterSize(26);
                exitConfirmButtons[0]->text.setFillColor(sf::Color::White);

                // Центрируем текст в кнопке
                sf::FloatRect yesBounds = exitConfirmButtons[0]->text.getLocalBounds();
                exitConfirmButtons[0]->text.setOrigin(
                    yesBounds.left + yesBounds.width / 2.0f,
                    yesBounds.top + yesBounds.height / 2.0f
                );
                exitConfirmButtons[0]->text.setPosition(
                    300 + 150 / 2.0f,
                    400 + 50 / 2.0f
                );

                // Кнопка "Нет" - зеленая
                exitConfirmButtons[1]->rect.setSize(sf::Vector2f(150, 50));
                exitConfirmButtons[1]->rect.setPosition(550, 400);
                exitConfirmButtons[1]->rect.setFillColor(sf::Color(60, 180, 60, 220));
                exitConfirmButtons[1]->rect.setOutlineThickness(2);
                exitConfirmButtons[1]->rect.setOutlineColor(sf::Color(100, 220, 100));

                exitConfirmButtons[1]->text.setString("Нет");
                exitConfirmButtons[1]->text.setCharacterSize(26);
                exitConfirmButtons[1]->text.setFillColor(sf::Color::White);

                // Центрируем текст в кнопке
                sf::FloatRect noBounds = exitConfirmButtons[1]->text.getLocalBounds();
                exitConfirmButtons[1]->text.setOrigin(
                    noBounds.left + noBounds.width / 2.0f,
                    noBounds.top + noBounds.height / 2.0f
                );
                exitConfirmButtons[1]->text.setPosition(
                    550 + 150 / 2.0f,
                    400 + 50 / 2.0f
                );

                // Рисуем кнопки
                for (auto btn : exitConfirmButtons) {
                    window->draw(btn->rect);
                    window->draw(btn->text);
                }
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
    }
};
int main() {
    setlocale(LC_ALL, "");
    std::locale::global(std::locale(""));
    srand(time(0)); 

    GameMenu menu;
    if (menu.init()) {
        menu.run();
    }
    return 0;
}