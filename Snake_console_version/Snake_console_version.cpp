

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
        // Смещаем к центру клетки (оригинальная формула)
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
    // Новые поля для ускорения/замедления
    float currentSpeed;
    float baseSpeed;
    float boostMultiplier = 1.5f;    // Множитель ускорения (Shift)
    float slowMultiplier = 0.5f;     // Множитель замедления (Ctrl)
    bool isBoosting = false;
    bool isSlowing = false;

    // Добавляем управление
    sf::Keyboard::Key boostKey = sf::Keyboard::LShift;    // Shift для ускорения
    sf::Keyboard::Key slowKey = sf::Keyboard::LControl;   // Ctrl для замедления

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
        : name(n), color(c), speed(gameSettings.snakeSpeed),  // speed уже существует
        baseSpeed(gameSettings.snakeSpeed),                  // инициализация baseSpeed
        currentSpeed(gameSettings.snakeSpeed),               // инициализация currentSpeed
        rotation(0.0f), score(0), isPlayer(player), isAlive(true),
        moveTimer(0.0f), cellSize(gameSettings.fieldSize),
        moveInterval(0.15f), snakeLength(gameSettings.initialSnakeLength)
    {
        // Выбираем стартовую клетку
        int startGridX = 25;
        int startGridY = 22;
        // Инициализация скоростей
        baseSpeed = gameSettings.snakeSpeed;
        currentSpeed = baseSpeed;

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
    // В структуре Snake добавьте метод для AI бота
    void updateBotAI(const std::vector<Snake*>& allSnakes, const std::vector<Fruit*>& fruits, int fieldWidth, int fieldHeight) {
        if (!isPlayer && isAlive) {
            // Простая логика поиска ближайшего фрукта
            if (!fruits.empty()) {
                // Находим ближайший хороший фрукт
                Fruit* nearestFruit = nullptr;
                float minDistance = std::numeric_limits<float>::max();

                for (auto fruit : fruits) {
                    if (fruit->isGoodFruit()) {
                        float dx = body[0].x - fruit->position.x;
                        float dy = body[0].y - fruit->position.y;
                        float distance = dx * dx + dy * dy;

                        if (distance < minDistance) {
                            minDistance = distance;
                            nearestFruit = fruit;
                        }
                    }
                }

                // Если нашли фрукт, движемся к нему
                if (nearestFruit) {
                    // Вычисляем разницу в координатах
                    float dx = nearestFruit->position.x - body[0].x;
                    float dy = nearestFruit->position.y - body[0].y;

                    // Выбираем направление, избегая столкновений
                    chooseSafeDirection(dx, dy, allSnakes, fieldWidth, fieldHeight);
                }
                else {
                    // Если нет фруктов, движемся случайно, избегая стен
                    wanderRandomly(allSnakes, fieldWidth, fieldHeight);
                }
            }
            else {
                // Если нет фруктов, движемся случайно
                wanderRandomly(allSnakes, fieldWidth, fieldHeight);
            }
        }
    }

    // Метод для выбора безопасного направления
    void chooseSafeDirection(float dx, float dy, const std::vector<Snake*>& allSnakes, int fieldWidth, int fieldHeight) {
        // Предпочитаем двигаться по оси с большей разницей
        if (std::abs(dx) > std::abs(dy)) {
            // Двигаемся по X
            if (dx > 0 && isDirectionSafe(sf::Vector2f(1, 0), allSnakes, fieldWidth, fieldHeight)) {
                nextDirection = sf::Vector2f(1, 0);
            }
            else if (dx < 0 && isDirectionSafe(sf::Vector2f(-1, 0), allSnakes, fieldWidth, fieldHeight)) {
                nextDirection = sf::Vector2f(-1, 0);
            }
            else {
                // Если предпочтительное направление небезопасно, пробуем другое
                tryAlternativeDirections(allSnakes, fieldWidth, fieldHeight);
            }
        }
        else {
            // Двигаемся по Y
            if (dy > 0 && isDirectionSafe(sf::Vector2f(0, 1), allSnakes, fieldWidth, fieldHeight)) {
                nextDirection = sf::Vector2f(0, 1);
            }
            else if (dy < 0 && isDirectionSafe(sf::Vector2f(0, -1), allSnakes, fieldWidth, fieldHeight)) {
                nextDirection = sf::Vector2f(0, -1);
            }
            else {
                tryAlternativeDirections(allSnakes, fieldWidth, fieldHeight);
            }
        }
    }

    // Метод для случайного блуждания
    void wanderRandomly(const std::vector<Snake*>& allSnakes, int fieldWidth, int fieldHeight) {
        // Случайно меняем направление с некоторой вероятностью
        if (rand() % 100 < 20) { // 20% шанс сменить направление
            // Пробуем все возможные направления, начиная со случайного
            std::vector<sf::Vector2f> directions = {
                sf::Vector2f(1, 0),   // вправо
                sf::Vector2f(-1, 0),  // влево
                sf::Vector2f(0, 1),   // вниз
                sf::Vector2f(0, -1)   // вверх
            };

            // Используем std::shuffle вместо std::random_shuffle
            std::shuffle(directions.begin(), directions.end(),
                std::default_random_engine(std::random_device()()));

            for (const auto& dir : directions) {
                if (isDirectionSafe(dir, allSnakes, fieldWidth, fieldHeight)) {
                    nextDirection = dir;
                    return;
                }
            }
        }
    }

    // Проверка безопасности направления
    bool isDirectionSafe(const sf::Vector2f& dir, const std::vector<Snake*>& allSnakes, int fieldWidth, int fieldHeight) {
        // Предсказываем следующую позицию головы
        sf::Vector2f nextHead = body[0] + dir * (float)cellSize;

        // Проверяем столкновение со стенами
        if (nextHead.x < 50 || nextHead.x >= fieldWidth - cellSize ||
            nextHead.y < 50 || nextHead.y >= fieldHeight - cellSize) {
            return false;
        }

        // Проверяем столкновение с собой
        for (size_t i = 0; i < body.size(); i++) {
            // Пропускаем голову и, возможно, первые сегменты для плавности
            if (i > 0) {
                if (std::abs(nextHead.x - body[i].x) < cellSize - 2 &&
                    std::abs(nextHead.y - body[i].y) < cellSize - 2) {
                    return false;
                }
            }
        }

        // Проверяем столкновение с другими змейками
        int nextCellX = static_cast<int>(nextHead.x / cellSize);
        int nextCellY = static_cast<int>(nextHead.y / cellSize);

        for (auto otherSnake : allSnakes) {
            if (otherSnake != this && otherSnake->isAlive) {
                for (size_t i = 0; i < otherSnake->body.size(); i++) {
                    int segmentCellX = static_cast<int>(otherSnake->body[i].x / cellSize);
                    int segmentCellY = static_cast<int>(otherSnake->body[i].y / cellSize);

                    if (nextCellX == segmentCellX && nextCellY == segmentCellY) {
                        return false;
                    }
                }
            }
        }

        return true;
    }

    // Пробуем альтернативные направления
    void tryAlternativeDirections(const std::vector<Snake*>& allSnakes, int fieldWidth, int fieldHeight) {
        std::vector<sf::Vector2f> directions = {
            direction,                          // текущее направление
            sf::Vector2f(-direction.y, direction.x),   // поворот налево
            sf::Vector2f(direction.y, -direction.x),   // поворот направо
            sf::Vector2f(-direction.x, -direction.y)   // разворот (последний вариант)
        };

        for (const auto& dir : directions) {
            if (isDirectionSafe(dir, allSnakes, fieldWidth, fieldHeight)) {
                nextDirection = dir;
                return;
            }
        }

        // Если все направления опасны, остаемся на месте
        nextDirection = direction;
    }

    void updateSpeed() {
        if (isBoosting) {
            currentSpeed = baseSpeed * boostMultiplier;
            moveInterval = 0.15f / boostMultiplier; // Быстрее движение
        }
        else if (isSlowing) {
            currentSpeed = baseSpeed * slowMultiplier;
            moveInterval = 0.15f / slowMultiplier; // Медленнее движение
        }
        else {
            currentSpeed = baseSpeed;
            moveInterval = 0.15f;
        }
    }

    void setBoost(bool boost) {
        isBoosting = boost;
        updateSpeed();
    }

    void setSlow(bool slow) {
        isSlowing = slow;
        updateSpeed();
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

        // Обновляем скорость перед движением
        updateSpeed();

        // Накопление времени для дискретного движения
        // Используем currentSpeed вместо фиксированного значения
        moveTimer += deltaTime * (currentSpeed / 100.0f);

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
            sf::RectangleShape segment(sf::Vector2f(cellSize, cellSize));

            // Используем оригинальные позиции (уже корректные)
            float posX = body[i].x + 1;
            float posY = body[i].y + 1;
            segment.setPosition(posX, posY);

            // Определяем цвет в зависимости от состояния
            sf::Color segmentColor = color;

            if (isBoosting) {
                // При ускорении - более яркий цвет
                segmentColor.r = std::min(color.r + 80, 255);
                segmentColor.g = std::min(color.g + 80, 255);
            }
            else if (isSlowing) {
                // При замедлении - более темный цвет
                segmentColor.r = std::max(color.r - 80, 0);
                segmentColor.g = std::max(color.g - 80, 0);
            }

            if (i == 0) {
                // Голова - темнее
                segmentColor.r = std::min(segmentColor.r + 40, 255);
                segmentColor.g = std::min(segmentColor.g + 40, 255);
                segmentColor.b = std::min(segmentColor.b + 40, 255);
            }

            segment.setFillColor(segmentColor);
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

        // Кнопка "Заново"
        DialogButton btn1;
        btn1.rect.setSize(sf::Vector2f(200, 60));
        btn1.rect.setFillColor(sf::Color(50, 150, 50, 220));
        btn1.rect.setOutlineThickness(2);
        btn1.rect.setOutlineColor(sf::Color::White);
        btn1.text.setFont(*font);
        btn1.text.setString("Заново");
        btn1.text.setCharacterSize(24);
        btn1.text.setFillColor(sf::Color::White);
        btn1.action = "restart";
        buttons.push_back(btn1);

        // Кнопка "Выйти в меню"
        DialogButton btn2;
        btn2.rect.setSize(sf::Vector2f(250, 60));
        btn2.rect.setFillColor(sf::Color(150, 50, 50, 220));
        btn2.rect.setOutlineThickness(2);
        btn2.rect.setOutlineColor(sf::Color::White);
        btn2.text.setFont(*font);
        btn2.text.setString("Выйти в меню");
        btn2.text.setCharacterSize(24);
        btn2.text.setFillColor(sf::Color::White);
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
        dialog.setSize(sf::Vector2f(700, 500));
        dialog.setPosition(150, 200);
        dialog.setFillColor(sf::Color(20, 20, 40, 230));
        dialog.setOutlineThickness(3);
        dialog.setOutlineColor(sf::Color::Yellow);
        window->draw(dialog);

        // Заголовок "ИГРА ОКОНЧЕНА" по центру - СПУСКАЕМ НИЖЕ
        sf::Text title;
        title.setFont(*font);
        title.setString("ИГРА ОКОНЧЕНА");
        title.setCharacterSize(48);
        title.setFillColor(sf::Color::Red);
        title.setStyle(sf::Text::Bold);

        // Центрируем заголовок
        sf::FloatRect titleBounds = title.getLocalBounds();
        title.setOrigin(titleBounds.left + titleBounds.width / 2.0f,
            titleBounds.top + titleBounds.height / 2.0f);
        title.setPosition(500, 280); // Было 250, спускаем на 30 пикселей ниже

        // Тень заголовка
        sf::Text titleShadow = title;
        titleShadow.setFillColor(sf::Color::Black);
        titleShadow.setPosition(502, 282);
        window->draw(titleShadow);
        window->draw(title);

        // Причина поражения (центрированная) - СПУСКАЕМ НИЖЕ
        sf::Text reasonText;
        reasonText.setFont(*font);
        reasonText.setString(deathReason);
        reasonText.setCharacterSize(32);
        reasonText.setFillColor(sf::Color(255, 100, 100));

        // Центрируем причину
        sf::FloatRect reasonBounds = reasonText.getLocalBounds();
        reasonText.setOrigin(reasonBounds.left + reasonBounds.width / 2.0f,
            reasonBounds.top + reasonBounds.height / 2.0f);
        reasonText.setPosition(500, 600); // Было 320, спускаем на 30 пикселей ниже 350
        window->draw(reasonText);





        // Если была многораундовая игра, показываем статистику - СПУСКАЕМ НИЖЕ
        if (totalRounds > 1) {
            sf::Text roundsText;
            roundsText.setFont(*font);
            roundsText.setString("Выиграно раундов: " + std::to_string(roundWins) +
                " из " + std::to_string(totalRounds));
            roundsText.setCharacterSize(24);
            roundsText.setFillColor(sf::Color(200, 200, 255));

            // Центрируем статистику раундов
            sf::FloatRect roundsBounds = roundsText.getLocalBounds();
            roundsText.setOrigin(roundsBounds.left + roundsBounds.width / 2.0f,
                roundsBounds.top + roundsBounds.height / 2.0f);
            roundsText.setPosition(500, 400); // Было 430, спускаем на 30 пикселей ниже
            window->draw(roundsText);
        }

        // Располагаем кнопки по центру - ПОДНИМАЕМ ВЫШЕ
        float totalButtonsWidth = 0;
        for (auto& btn : buttons) {
            totalButtonsWidth += btn.rect.getSize().x + 50; // +50 для отступов
        }

        float startX = 500 - totalButtonsWidth / 2 + 25; // Центрируем группу кнопок
        float buttonY = 520; // Было 520, оставляем на месте или можно поднять выше если нужно

        // Если нужно поднять кнопки выше (например, на 480), раскомментируйте:
        // float buttonY = 480; // Поднимаем кнопки выше

        for (auto& btn : buttons) {
            // Устанавливаем позицию кнопки
            btn.rect.setPosition(startX, buttonY);

            // Центрируем текст в кнопке
            sf::FloatRect textBounds = btn.text.getLocalBounds();
            btn.text.setOrigin(textBounds.left + textBounds.width / 2.0f,
                textBounds.top + textBounds.height / 2.0f);
            btn.text.setPosition(startX + btn.rect.getSize().x / 2.0f,
                buttonY + btn.rect.getSize().y / 2.0f);

            window->draw(btn.rect);
            window->draw(btn.text);

            // Смещаем для следующей кнопки
            startX += btn.rect.getSize().x + 50;
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
    std::vector<std::string> keyNames = { "Вверх", "Влево", "Вниз", "Вправо" };

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

    void startNextRoundSparring(bool player1WonPreviousRound, const std::string& previousRoundResult) {
        // Обновляем статистику для игрока 1
        if (player1WonPreviousRound) {
            gameData.roundWins++;
            std::cout << "Player 1 won round " << gameData.currentRound << std::endl;
        }
        else {
            std::cout << "Player 2 won round " << gameData.currentRound << std::endl;
        }

        // Увеличиваем номер раунда
        gameData.currentRound++;

        // Показываем сообщение о результате раунда
        showRoundResult(previousRoundResult, player1WonPreviousRound, gameData.currentRound - 1);

        // Небольшая пауза перед следующим раундом
        sf::Clock delayClock;
        while (delayClock.getElapsedTime().asSeconds() < 2.0f) {
            // Обрабатываем события, чтобы окно не зависало
            sf::Event event;
            while (window->pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window->close();
                    return;
                }
            }

            // Рисуем экран с результатом раунда
            renderRoundResult(previousRoundResult, player1WonPreviousRound,
                gameData.currentRound - 1, gameData.currentRound);
            window->display();
        }

        // Очистка старых объектов
        for (auto snake : snakes) delete snake;
        for (auto fruit : gameFruits) delete fruit;
        snakes.clear();
        gameFruits.clear();

        // Создание ПЕРВОГО игрока для нового раунда
        Snake* player1 = new Snake("Player 1", sf::Color::Blue, settings, true);
        // Настройка управления для Player 1 - СТРЕЛКИ
        player1->moveUp = sf::Keyboard::Up;
        player1->moveDown = sf::Keyboard::Down;
        player1->moveLeft = sf::Keyboard::Left;
        player1->moveRight = sf::Keyboard::Right;

        // Позиция Player 1 (левая часть поля)
        int startGridX1 = 10;
        int startGridY1 = 22;
        float startX1 = startGridX1 * player1->cellSize;
        float startY1 = startGridY1 * player1->cellSize;
        player1->body.clear();
        player1->body.push_back(sf::Vector2f(startX1, startY1));

        // Начальное направление - вправо
        player1->direction = sf::Vector2f(1, 0);
        player1->nextDirection = player1->direction;

        // Сброс состояния ускорения
        player1->setBoost(false);
        player1->setSlow(false);
        player1->currentSpeed = player1->baseSpeed;

        snakes.push_back(player1);

        // Создание ВТОРОГО игрока для нового раунда
        Snake* player2 = new Snake("Player 2", sf::Color::Red, settings, true);
        // Настройка управления для Player 2 - WASD
        player2->moveUp = sf::Keyboard::W;
        player2->moveDown = sf::Keyboard::S;
        player2->moveLeft = sf::Keyboard::A;
        player2->moveRight = sf::Keyboard::D;

        // Позиция Player 2 (правая часть поля)
        int startGridX2 = 40;
        int startGridY2 = 22;
        float startX2 = startGridX2 * player2->cellSize;
        float startY2 = startGridY2 * player2->cellSize;
        player2->body.clear();
        player2->body.push_back(sf::Vector2f(startX2, startY2));

        // Направление Player 2 - влево (к центру)
        player2->direction = sf::Vector2f(-1, 0);
        player2->nextDirection = player2->direction;

        // Сброс состояния ускорения
        player2->setBoost(false);
        player2->setSlow(false);
        player2->currentSpeed = player2->baseSpeed;

        snakes.push_back(player2);

        // Создание начальных фруктов
        for (int i = 0; i < 5; i++) {
            spawnGameFruit();
        }

        // Сброс таймеров
        gameData.gameTime = 0.0f;
        gameClock.restart();
        fruitSpawnClock.restart();

        std::cout << "Starting sparring round " << gameData.currentRound << " of "
            << gameData.totalRounds << std::endl;
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

    void updateButtonHover(sf::Vector2f mousePos) {
        std::vector<Button*>* currentButtons = nullptr;

        // Определяем, какие кнопки сейчас активны
        switch (currentScreen) {
        case Screen::MAIN:
            currentButtons = &mainButtons;
            break;
        case Screen::GAME_SETTINGS:
            currentButtons = &gameSettingsButtons;
            break;
        case Screen::PLAYER_SETTINGS:
            currentButtons = &playerSettingsButtons;
            break;
        case Screen::ABOUT:
            currentButtons = &aboutButtons;
            break;
        case Screen::EXIT_CONFIRM:
            currentButtons = &exitConfirmButtons;
            break;
        case Screen::EDIT_KEYS:
            currentButtons = &editKeysButtons;
            // Обработка ховера для областей клавиш в режиме EDIT_KEYS
            if (!keyInputActive) {
                for (int i = 0; i < 4; i++) {
                    sf::FloatRect keyRect(175, 240 + i * 90, 650, 60);
                    if (keyRect.contains(mousePos)) {
                        // Можно добавить визуальный эффект для областей клавиш
                    }
                }
            }
            break;
        }

        if (!currentButtons) return;

        // Обрабатываем подсветку для всех кнопок
        for (auto& btn : *currentButtons) {
            if (btn->rect.getGlobalBounds().contains(mousePos)) {
                if (!btn->hovered) {
                    // Сохраняем оригинальный цвет
                    sf::Color originalColor = btn->rect.getFillColor();
                    // Увеличиваем яркость
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
                    // Возвращаем оригинальный цвет
                    sf::Color currentColor = btn->rect.getFillColor();
                    btn->rect.setFillColor(sf::Color(
                        std::max(currentColor.r - 30, 0),
                        std::max(currentColor.g - 30, 0),
                        std::max(currentColor.b - 30, 0),
                        currentColor.a
                    ));
                    btn->hovered = false;
                }
            }
        }
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

        // 1. Кнопка "Начать игру" - САМАЯ ПЕРВАЯ
        gameSettingsButtons.push_back(new Button(font, "Начать игру", { 400, 200 }, sf::Color(50, 200, 50, 200), sf::Color::White));

        // 2. КНОПКА СЛОЖНОСТИ (НОВАЯ)
        std::string diffText;
        switch (settings.difficulty) {
        case 1: diffText = "Сложность:Easy"; break;
        case 2: diffText = "Сложность:Mid"; break;
        case 3: diffText = "Сложность:Hard"; break;
        default: diffText = "Сложность:Easy"; break;
        }
        gameSettingsButtons.push_back(new Button(font, diffText, { 400, 280 }, sf::Color(100, 100, 200, 200), sf::Color::White));

        // 3. Выбор раундов или быстрая игра
        gameSettingsButtons.push_back(new Button(font, "Быстрая игра", { 400, 360 }, sf::Color(70, 170, 70, 200), sf::Color::White));

        // 4. Новый режим "Спаринг" (два игрока)
        gameSettingsButtons.push_back(new Button(font, "Спаринг 2", { 400, 440 }, sf::Color(100, 100, 200, 200), sf::Color::White));

        // 5. Поле для ввода количества раундов
        gameSettingsButtons.push_back(new Button(font, "Раунды: " + std::to_string(rounds), { 400, 520 }, sf::Color(80, 120, 180, 200), sf::Color::White));

        // 6. Выбор количества ботов (только 0 и 1)
        gameSettingsButtons.push_back(new Button(font, "Боты: 0", { 400, 600 }, sf::Color(100, 150, 100, 200), sf::Color::White));
        gameSettingsButtons.push_back(new Button(font, "Боты: 1", { 400, 680 }, sf::Color(100, 150, 100, 200), sf::Color::White));

        // 7. Кнопка "Назад"
        gameSettingsButtons.push_back(new Button(font, "Назад", { 400, 760 }, sf::Color(150, 100, 50, 200), sf::Color::White));

        // Назначаем действия
        gameSettingsButtons[0]->action = "start_game";
        gameSettingsButtons[1]->action = "change_difficulty"; // НОВОЕ ДЕЙСТВИЕ
        gameSettingsButtons[2]->action = "quick_game";
        gameSettingsButtons[3]->action = "sparring_mode";
        gameSettingsButtons[4]->action = "set_rounds";
        gameSettingsButtons[5]->action = "bots_0";
        gameSettingsButtons[6]->action = "bots_1";
        gameSettingsButtons[7]->action = "back";
    }

    void initPlayerSettings() {
        playerSettingsButtons.clear();

        // Кнопки выбора цвета (первые 4)
        playerSettingsButtons.push_back(new Button(font, "Blue", { 150, 250 }, sf::Color::Blue, sf::Color::White));
        playerSettingsButtons.push_back(new Button(font, "Green", { 350, 250 }, sf::Color::Green, sf::Color::White));
        playerSettingsButtons.push_back(new Button(font, "Red", { 550, 250 }, sf::Color::Red, sf::Color::White));
        playerSettingsButtons.push_back(new Button(font, "Purple", { 750, 250 }, sf::Color(128, 0, 128), sf::Color::White));

        // Кнопки управления (3 штуки)
        playerSettingsButtons.push_back(new Button(font, "WASD", { 100, 350 }, sf::Color(70, 100, 150, 200), sf::Color::White));
        playerSettingsButtons.push_back(new Button(font, "Arrows", { 300, 350 }, sf::Color(70, 100, 150, 200), sf::Color::White));
        playerSettingsButtons.push_back(new Button(font, "Edit Keys", { 500, 350 }, sf::Color(70, 150, 100, 200), sf::Color::White));


        playerSettingsButtons.push_back(new Button(font, "Enter Nickname", { 700, 350 }, sf::Color(70, 150, 100, 200), sf::Color::White));

        playerSettingsButtons.push_back(new Button(font, "Field size: " + std::to_string(settings.fieldSize), { 100, 500 }, sf::Color(80, 120, 180, 200), sf::Color::White));


        playerSettingsButtons.push_back(new Button(font, "Save", { 350, 550 }, sf::Color(50, 150, 100, 200), sf::Color::White));


        playerSettingsButtons.push_back(new Button(font, "Назад", { 100, 750 }, sf::Color(150, 100, 50, 200), sf::Color::White));

        playerSettingsButtons.push_back(new Button(font, "Reset to Default", { 350, 750 }, sf::Color(150, 100, 50, 200), sf::Color::White));

        // Назначаем действия для кнопок
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
        playerSettingsButtons[10]->action = "back";  // Назад
        playerSettingsButtons[11]->action = "reset_defaults";  // Сброс к настройкам по умолчанию
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

        // Кнопки с правильными расстояниями и русскими названиями
        editKeysButtons.push_back(new Button(font, "WASD", { 150, 650 }, sf::Color(70, 100, 150, 200), sf::Color::White));
        editKeysButtons.push_back(new Button(font, "Назад", { 400, 650 }, sf::Color(150, 100, 50, 200), sf::Color::White));
        editKeysButtons.push_back(new Button(font, "Стрелки", { 650, 650 }, sf::Color(70, 100, 150, 200), sf::Color::White));

        editKeysButtons[0]->action = "reset_wasd";
        editKeysButtons[1]->action = "back";
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
            else if (key == "fruitSpoilRateEasy") {
                settings.fruitSpoilRateEasy = std::stof(value);
            }
            else if (key == "fruitSpoilRateMedium") {
                settings.fruitSpoilRateMedium = std::stof(value);
            }
            else if (key == "fruitSpoilRateHard") {
                settings.fruitSpoilRateHard = std::stof(value);
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

        // Устанавливаем значения скорости порчи по умолчанию в зависимости от сложности
        switch (settings.difficulty) {
        case 1:
            settings.fruitSpoilRateEasy = 0.5f;
            settings.fruitSpoilRateMedium = 1.0f;
            settings.fruitSpoilRateHard = 1.5f;
            break;
        case 2:
            settings.fruitSpoilRateEasy = 1.0f;
            settings.fruitSpoilRateMedium = 1.5f;
            settings.fruitSpoilRateHard = 2.0f;
            break;
        case 3:
            settings.fruitSpoilRateEasy = 1.5f;
            settings.fruitSpoilRateMedium = 2.0f;
            settings.fruitSpoilRateHard = 2.5f;
            break;
        }

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
        file << "fruitSpawnRate=" << settings.fruitSpawnRate << "\n";
        file << "fruitSpoilRateEasy=" << settings.fruitSpoilRateEasy << "\n";
        file << "fruitSpoilRateMedium=" << settings.fruitSpoilRateMedium << "\n";
        file << "fruitSpoilRateHard=" << settings.fruitSpoilRateHard << "\n\n";

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
    void showRoundResult(const std::string& result, bool playerWon, int roundNumber) {
        // Просто логируем, реальное отображение будет в renderRoundResult
        std::cout << "Round " << roundNumber << " result: " << result << std::endl;
    }

    void renderRoundResult(const std::string& result, bool playerWon,
        int completedRound, int nextRound) {
        // Очищаем экран
        window->clear(bgColor);

        if (backgroundLoaded) {
            window->draw(backgroundSprite);
            sf::RectangleShape overlay;
            overlay.setSize(sf::Vector2f(1000, 900));
            overlay.setFillColor(sf::Color(0, 0, 0, 150));
            window->draw(overlay);
        }

        // Фон для диалога
        sf::RectangleShape dialog;
        dialog.setSize(sf::Vector2f(700, 400));
        dialog.setPosition(150, 250);
        dialog.setFillColor(sf::Color(20, 20, 40, 230));
        dialog.setOutlineThickness(3);
        dialog.setOutlineColor(playerWon ? sf::Color::Green : sf::Color::Red);
        window->draw(dialog);

        // Заголовок
        sf::Text title;
        title.setFont(font);
        title.setString("РАУНД " + std::to_string(completedRound) + " ЗАВЕРШЕН");
        title.setCharacterSize(36);
        title.setFillColor(playerWon ? sf::Color::Green : sf::Color::Red);
        title.setStyle(sf::Text::Bold);

        // Центрируем заголовок
        sf::FloatRect titleBounds = title.getLocalBounds();
        title.setOrigin(titleBounds.left + titleBounds.width / 2.0f,
            titleBounds.top + titleBounds.height / 2.0f);
        title.setPosition(500, 300);

        // Тень заголовка
        sf::Text titleShadow = title;
        titleShadow.setFillColor(sf::Color::Black);
        titleShadow.setPosition(502, 302);
        window->draw(titleShadow);
        window->draw(title);

        // Результат раунда
        sf::Text resultText;
        resultText.setFont(font);
        resultText.setString(result);
        resultText.setCharacterSize(28);
        resultText.setFillColor(playerWon ? sf::Color(150, 255, 150) : sf::Color(255, 150, 150));

        // Центрируем результат
        sf::FloatRect resultBounds = resultText.getLocalBounds();
        resultText.setOrigin(resultBounds.left + resultBounds.width / 2.0f,
            resultBounds.top + resultBounds.height / 2.0f);
        resultText.setPosition(500, 370);
        window->draw(resultText);

        // Статистика
        sf::Text statsText;
        statsText.setFont(font);
        statsText.setString("Выиграно раундов: " + std::to_string(gameData.roundWins) +
            "\nСледующий раунд: " + std::to_string(nextRound) +
            " из " + std::to_string(gameData.totalRounds) +
            "\nПриготовьтесь...");
        statsText.setCharacterSize(24);
        statsText.setFillColor(sf::Color::White);
        statsText.setLineSpacing(1.5f);

        // Центрируем статистику
        sf::FloatRect statsBounds = statsText.getLocalBounds();
        statsText.setOrigin(statsBounds.left + statsBounds.width / 2.0f,
            statsBounds.top + statsBounds.height / 2.0f);
        statsText.setPosition(500, 500);
        window->draw(statsText);
    }


    void startGame(int rounds, int bots) {
        // Проверяем, не режим ли это спаринга
        if (gameData.isMultiplayer) {
            startSparringGame(rounds);
            return;
        }

        std::cout << "=== STARTING GAME ===" << std::endl;
        std::cout << "Player name: " << settings.playerName << std::endl;
        std::cout << "Rounds: " << rounds << ", Bots: " << bots << std::endl;

        gameState = PLAYING;
        gameData.totalRounds = rounds;
        gameData.totalBots = bots;
        gameData.currentRound = 1;  // Начинаем с первого раунда
        gameData.score = 0;
        gameData.roundWins = 0;
        gameData.gameTime = 0.0f;
        gameData.isAlive = true;

        // Очистка старых объектов
        for (auto snake : snakes) delete snake;
        for (auto fruit : gameFruits) delete fruit;
        snakes.clear();
        gameFruits.clear();

        // Создание игрока для первого раунда
        Snake* player = new Snake(settings.playerName, settings.playerColor, settings, true);
        // Настройка управления
        player->moveUp = settings.playerKeys[0];
        player->moveDown = settings.playerKeys[2];
        player->moveLeft = settings.playerKeys[1];
        player->moveRight = settings.playerKeys[3];

        // Сброс состояния ускорения
        player->setBoost(false);
        player->setSlow(false);
        player->currentSpeed = player->baseSpeed;

        // Стартовая позиция для первого раунда
        int startGridX = 25;
        int startGridY = 22; // Оригинальная позиция
        float startX = startGridX * player->cellSize;
        float startY = startGridY * player->cellSize;

        player->body.clear();
        player->body.push_back(sf::Vector2f(startX, startY));

        snakes.push_back(player);
        gameData.playerName = settings.playerName;
        gameData.playerColor = settings.playerColor;

        // Создание ботов для первого раунда
        for (int i = 0; i < bots; i++) {
            Snake* bot = new Snake("Bot " + std::to_string(i + 1), settings.botColor, settings, false);

            // Размещаем бота в случайном месте
            int botGridX, botGridY;
            bool validPosition = false;
            int attempts = 0;

            while (!validPosition && attempts < 100) {
                botGridX = 5 + rand() % 35;
                botGridY = 5 + rand() % 30;

                int distance = std::abs(botGridX - startGridX) + std::abs(botGridY - startGridY);
                if (distance > 10) {
                    validPosition = true;
                }
                attempts++;
            }

            float botX = botGridX * bot->cellSize;
            float botY = botGridY * bot->cellSize;

            bot->body.clear();
            bot->body.push_back(sf::Vector2f(botX, botY));

            // Случайное начальное направление
            int dirIndex = rand() % 4;
            switch (dirIndex) {
            case 0: bot->direction = sf::Vector2f(1, 0); break;
            case 1: bot->direction = sf::Vector2f(-1, 0); break;
            case 2: bot->direction = sf::Vector2f(0, 1); break;
            case 3: bot->direction = sf::Vector2f(0, -1); break;
            }
            bot->nextDirection = bot->direction;

            // Настройка сложности бота
            switch (settings.difficulty) {
            case 1:
                bot->boostMultiplier = 1.2f;
                bot->slowMultiplier = 0.7f;
                break;
            case 2:
                bot->boostMultiplier = 1.5f;
                bot->slowMultiplier = 0.5f;
                break;
            case 3:
                bot->boostMultiplier = 1.8f;
                bot->slowMultiplier = 0.3f;
                if (rand() % 100 < 30) {
                    bot->setBoost(true);
                }
                break;
            }

            bot->baseSpeed = settings.snakeSpeed * 0.8f;
            bot->currentSpeed = bot->baseSpeed;

            snakes.push_back(bot);
        }

        // Создание начальных фруктов
        for (int i = 0; i < 3; i++) {
            spawnGameFruit();
        }

        gameClock.restart();
        fruitSpawnClock.restart();

        std::cout << "Starting round 1 of " << rounds << std::endl;
    }

    void spawnGameFruit() {
        if (snakes.empty()) return;

        // Размер клетки берем из змейки
        int cellSize = snakes[0]->cellSize;

        // Рассчитываем доступное пространство для спавна в клетках
        // Теперь поле начинается с Y=60
        int minGridX = 50 / cellSize + 1;
        int maxGridX = 950 / cellSize - 1;
        int minGridY = 60 / cellSize + 1; // Было 50, стало 60
        int maxGridY = 860 / cellSize - 1; // Было 850, стало 860

        // Генерируем случайные координаты клетки
        int gridX = minGridX + rand() % (maxGridX - minGridX + 1);
        int gridY = minGridY + rand() % (maxGridY - minGridY + 1);

        // Преобразуем координаты клетки в пиксельные координаты
        float x = gridX * cellSize + 1;
        float y = gridY * cellSize + 1;

        // Берем скорость порчи в зависимости от сложности
        float spoilRate = settings.fruitSpoilRateMedium;

        switch (settings.difficulty) {
        case 1:
            spoilRate = settings.fruitSpoilRateEasy;
            break;
        case 2:
            spoilRate = settings.fruitSpoilRateMedium;
            break;
        case 3:
            spoilRate = settings.fruitSpoilRateHard;
            break;
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

                // Обновление AI для ботов
                if (!snake->isPlayer && gameData.totalBots > 0) {
                    snake->updateBotAI(snakes, gameFruits, 950, 850);
                }
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

        // Проверка столкновений со стенами (теперь поле с Y=60..860)
        for (auto snake : snakes) {
            if (!snake->isAlive) continue;

            sf::Vector2f head = snake->body[0];

            // Проверяем границы (поле теперь с X=50..950, Y=60..860)
            if (head.x < 50 || head.x > 950 - snake->cellSize ||
                head.y < 60 || head.y > 860 - snake->cellSize) { // Изменено с 50..850 на 60..860
                snake->isAlive = false;
                std::cout << snake->name << " died by hitting the wall!" << std::endl;
                std::cout << "Head position: (" << head.x << ", " << head.y << ")" << std::endl;
                std::cout << "Wall boundaries: X(" << 50 << " to " << 950 - snake->cellSize
                    << "), Y(" << 60 << " to " << 860 - snake->cellSize << ")" << std::endl;
            }
        }

        // Проверка столкновений с телом (САМОСТОЛКНОВЕНИЕ)
        for (auto snake : snakes) {
            if (!snake->isAlive) continue;

            // Начинаем проверку с 3-го сегмента для реалистичности
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
                    // В режиме спаринга игроки не должны умирать от столкновения с головами друг друга
                    // Проверяем только тело (k > 0) или если это не голова другого игрока
                    if (gameData.isMultiplayer && k == 0) continue;

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

        // Проверка лобового столкновения игрока с ботом
        for (auto playerSnake : snakes) {
            if (!playerSnake->isPlayer || !playerSnake->isAlive) continue;

            for (auto botSnake : snakes) {
                if (botSnake->isPlayer || !botSnake->isAlive) continue;

                // Координаты клеток голов
                int playerHeadX = static_cast<int>(playerSnake->body[0].x / cellSize);
                int playerHeadY = static_cast<int>(playerSnake->body[0].y / cellSize);
                int botHeadX = static_cast<int>(botSnake->body[0].x / cellSize);
                int botHeadY = static_cast<int>(botSnake->body[0].y / cellSize);

                // Лобовое столкновение
                if (playerHeadX == botHeadX && playerHeadY == botHeadY) {
                    // В лобовом столкновении умирают оба
                    playerSnake->isAlive = false;
                    botSnake->isAlive = false;
                    std::cout << "Head-to-head collision! Both "
                        << playerSnake->name << " and "
                        << botSnake->name << " died!" << std::endl;

                    // Можно дать очки обоим
                    playerSnake->score += 25;
                    botSnake->score += 25;
                    gameData.score = playerSnake->score;
                    break;
                }
            }
        }

        // Проверка лобового столкновения в режиме спаринга
        if (gameData.isMultiplayer && snakes.size() >= 2) {
            Snake* player1 = snakes[0];
            Snake* player2 = snakes[1];

            if (player1->isAlive && player2->isAlive) {
                int p1HeadX = static_cast<int>(player1->body[0].x / cellSize);
                int p1HeadY = static_cast<int>(player1->body[0].y / cellSize);
                int p2HeadX = static_cast<int>(player2->body[0].x / cellSize);
                int p2HeadY = static_cast<int>(player2->body[0].y / cellSize);

                // Лобовое столкновение в спарринге
                if (p1HeadX == p2HeadX && p1HeadY == p2HeadY) {
                    // В спарринге при лобовом столкновении умирают оба
                    player1->isAlive = false;
                    player2->isAlive = false;
                    std::cout << "Sparring head-to-head collision! Both players died!" << std::endl;

                    // Даем очки обоим
                    player1->score += 50;
                    player2->score += 50;
                    gameData.score = std::max(player1->score, player2->score);
                }
            }
        }
    }


    void startNextRound(bool playerWonPreviousRound, const std::string& previousRoundResult) {
        // Обновляем статистику
        if (playerWonPreviousRound) {
            gameData.roundWins++;
            std::cout << "Player won round " << gameData.currentRound << std::endl;
        }
        else {
            std::cout << "Player lost round " << gameData.currentRound << std::endl;
        }

        // Увеличиваем номер раунда
        gameData.currentRound++;

        // Показываем сообщение о результате раунда
        showRoundResult(previousRoundResult, playerWonPreviousRound, gameData.currentRound - 1);

        // Небольшая пауза перед следующим раундом
        sf::Clock delayClock;
        while (delayClock.getElapsedTime().asSeconds() < 2.0f) {
            // Обрабатываем события, чтобы окно не зависало
            sf::Event event;
            while (window->pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window->close();
                    return;
                }
            }

            // Рисуем экран с результатом раунда
            renderRoundResult(previousRoundResult, playerWonPreviousRound,
                gameData.currentRound - 1, gameData.currentRound);
            window->display();
        }

        // Очистка старых объектов
        for (auto snake : snakes) delete snake;
        for (auto fruit : gameFruits) delete fruit;
        snakes.clear();
        gameFruits.clear();

        // Создание игрока для нового раунда
        Snake* player = new Snake(settings.playerName, settings.playerColor, settings, true);
        // Настройка управления
        player->moveUp = settings.playerKeys[0];
        player->moveDown = settings.playerKeys[2];
        player->moveLeft = settings.playerKeys[1];
        player->moveRight = settings.playerKeys[3];

        // Сброс состояния ускорения
        player->setBoost(false);
        player->setSlow(false);
        player->currentSpeed = player->baseSpeed;

        // Новая стартовая позиция для разнообразия
        int startGridX = 10 + rand() % 40;  // 10-50
        int startGridY = 10 + rand() % 35;  // 10-45
        float startX = startGridX * player->cellSize;
        float startY = startGridY * player->cellSize;

        player->body.clear();
        player->body.push_back(sf::Vector2f(startX, startY));

        // Начальное направление - случайное
        int dirIndex = rand() % 4;
        switch (dirIndex) {
        case 0: player->direction = sf::Vector2f(1, 0); break;  // вправо
        case 1: player->direction = sf::Vector2f(-1, 0); break; // влево
        case 2: player->direction = sf::Vector2f(0, 1); break;  // вниз
        case 3: player->direction = sf::Vector2f(0, -1); break; // вверх
        }
        player->nextDirection = player->direction;

        snakes.push_back(player);

        // Создание ботов для нового раунда
        for (int i = 0; i < gameData.totalBots; i++) {
            Snake* bot = new Snake("Bot " + std::to_string(i + 1), settings.botColor, settings, false);

            // Размещаем бота в случайном месте, но не слишком близко к игроку
            bool validPosition = false;
            int attempts = 0;

            while (!validPosition && attempts < 100) {
                int botGridX = 10 + rand() % 40;
                int botGridY = 10 + rand() % 35;

                // Проверяем расстояние до игрока
                int distance = std::abs(botGridX - startGridX) + std::abs(botGridY - startGridY);
                if (distance > 15) {  // Минимум 15 клеток расстояния
                    validPosition = true;
                    float botX = botGridX * bot->cellSize;
                    float botY = botGridY * bot->cellSize;

                    bot->body.clear();
                    bot->body.push_back(sf::Vector2f(botX, botY));

                    // Случайное начальное направление
                    int botDirIndex = rand() % 4;
                    switch (botDirIndex) {
                    case 0: bot->direction = sf::Vector2f(1, 0); break;
                    case 1: bot->direction = sf::Vector2f(-1, 0); break;
                    case 2: bot->direction = sf::Vector2f(0, 1); break;
                    case 3: bot->direction = sf::Vector2f(0, -1); break;
                    }
                    bot->nextDirection = bot->direction;
                }
                attempts++;
            }

            // Настройка сложности бота в зависимости от настроек игры
            switch (settings.difficulty) {
            case 1: // Легкая
                bot->boostMultiplier = 1.2f;
                bot->slowMultiplier = 0.7f;
                break;
            case 2: // Средняя
                bot->boostMultiplier = 1.5f;
                bot->slowMultiplier = 0.5f;
                break;
            case 3: // Сложная
                bot->boostMultiplier = 1.8f;
                bot->slowMultiplier = 0.3f;
                // Бот может иногда использовать ускорение
                if (rand() % 100 < 30) {
                    bot->setBoost(true);
                }
                break;
            }

            bot->baseSpeed = settings.snakeSpeed * 0.8f;
            bot->currentSpeed = bot->baseSpeed;

            snakes.push_back(bot);
        }

        // Создание начальных фруктов
        for (int i = 0; i < 3; i++) {
            spawnGameFruit();
        }

        // Сброс таймеров
        gameData.gameTime = 0.0f;
        gameClock.restart();
        fruitSpawnClock.restart();

        std::cout << "Starting round " << gameData.currentRound << " of "
            << gameData.totalRounds << std::endl;
    }

    void checkGameOver() {
        if (gameState != PLAYING) return;

        // Если режим спаринга
        if (gameData.isMultiplayer) {
            // Проверяем, сколько игроков живо
            int alivePlayers = 0;
            Snake* lastAlivePlayer = nullptr;

            for (auto snake : snakes) {
                if (snake->isAlive) {
                    alivePlayers++;
                    lastAlivePlayer = snake;
                }
            }

            // Если остался только один игрок - раунд окончен
            if (alivePlayers <= 1) {
                bool player1Won = false;
                std::string deathReason = "";

                if (lastAlivePlayer) {
                    deathReason = lastAlivePlayer->name + " выиграл раунд!";
                    // Даем очки победителю
                    lastAlivePlayer->score += 50;
                    gameData.score = lastAlivePlayer->score;
                }

                // Определяем, выиграл ли первый игрок
                if (lastAlivePlayer && lastAlivePlayer->name == "Player 1") {
                    player1Won = true;
                }

                // Если это последний раунд - показываем финальный результат
                if (gameData.currentRound >= gameData.totalRounds) {
                    endGame(player1Won, deathReason);
                }
                else {
                    // Начинаем следующий раунд
                    startNextRoundSparring(player1Won, deathReason);
                }
            }
            return;
        }

        // Обычный режим (не спарринг)
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

        // Проверяем, все ли боты мертвы
        bool allBotsDead = true;
        int aliveBots = 0;

        for (auto snake : snakes) {
            if (!snake->isPlayer && snake->isAlive) {
                allBotsDead = false;
                aliveBots++;
            }
        }

        // Условия окончания раунда:
        // 1. Игрок умер - проигрыш раунда
        // 2. Все боты умерли (и боты вообще есть) - победа в раунде
        if (!playerAlive || (gameData.totalBots > 0 && allBotsDead)) {
            bool playerWon = false;
            std::string deathReason = "";

            if (!playerAlive) {
                deathReason = "Вы проиграли раунд!";
                playerWon = false;

                // Определяем причину смерти
                if (playerSnake) {
                    sf::Vector2f head = playerSnake->body[0];
                    if (head.x < 50 || head.x > 950 - playerSnake->cellSize ||
                        head.y < 50 || head.y > 850 - playerSnake->cellSize) {
                        deathReason = "Вы врезались в стену!";
                    }
                    else {
                        deathReason = "Столкновение!";
                    }
                }
            }
            else if (allBotsDead) {
                deathReason = "Вы победили всех ботов в этом раунде!";
                playerWon = true;

                // Даем бонусные очки за победу
                if (playerSnake) {
                    playerSnake->score += 100 * aliveBots;
                    gameData.score = playerSnake->score;
                }
            }

            // Если это последний раунд - показываем финальный результат
            if (gameData.currentRound >= gameData.totalRounds) {
                endGame(playerWon, deathReason);
            }
            else {
                // Начинаем следующий раунд
                startNextRound(playerWon, deathReason);
            }
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
            endGame(playerWon, "");
        }
        else {
            // Начинаем следующий раунд
            gameData.currentRound++;

            // Пересоздаем змей с новыми позициями
            for (auto snake : snakes) {
                snake->isAlive = true;
                snake->rotation = 0.0f;
                snake->speed = 150.0f; // Сброс скорости

                // Сброс состояния ускорения
                snake->setBoost(false);
                snake->setSlow(false);
                snake->currentSpeed = snake->baseSpeed;

                // Новая случайная позиция
                snake->body.clear();
                float x = 200 + rand() % 600;
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

    void endGame(bool playerWonFinalRound, const std::string& finalRoundReason) {
        gameState = GAME_OVER;

        std::string winner;
        sf::Color winnerColor;
        std::string deathReason = "";

        // Определяем общий результат игры
        bool playerWonGame = (gameData.roundWins > (gameData.totalRounds / 2));

        if (playerWonGame) {
            winner = gameData.playerName;
            winnerColor = gameData.playerColor;

        }
        else {
            winner = "Game Over";
            winnerColor = sf::Color::Red;

        }

        // Добавляем статистику по раундам


        deathReason += "\n\nОбщий счет: " + std::to_string(gameData.score);
        deathReason += "\nПоследний раунд: " + finalRoundReason;

        // Показываем диалог окончания игры с полной статистикой
        gameOverDialog->show(winner, winnerColor,
            gameData.score, gameData.roundWins,
            gameData.totalRounds, deathReason);
    }

    void handleGameInput() {
        if (gameState != PLAYING) return;

        // Если режим спаринга, обрабатываем ввод для двух игроков
        if (gameData.isMultiplayer && snakes.size() >= 2) {
            // Player 1 (стрелки)
            Snake* player1 = snakes[0];
            Snake* player2 = snakes[1];

            if (player1->isAlive) {
                // Управление ускорением/замедлением для Player 1
                bool shiftPressed1 = sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
                bool ctrlPressed1 = sf::Keyboard::isKeyPressed(sf::Keyboard::RControl);

                if (shiftPressed1 && !ctrlPressed1) {
                    player1->setBoost(true);
                    player1->setSlow(false);
                }
                else if (ctrlPressed1 && !shiftPressed1) {
                    player1->setBoost(false);
                    player1->setSlow(true);
                }
                else {
                    player1->setBoost(false);
                    player1->setSlow(false);
                }

                // Управление направлением Player 1 (стрелки)
                if (sf::Keyboard::isKeyPressed(player1->moveUp) &&
                    !(player1->direction.x == 0 && player1->direction.y == 1)) {
                    player1->nextDirection = sf::Vector2f(0, -1);
                }
                else if (sf::Keyboard::isKeyPressed(player1->moveDown) &&
                    !(player1->direction.x == 0 && player1->direction.y == -1)) {
                    player1->nextDirection = sf::Vector2f(0, 1);
                }
                else if (sf::Keyboard::isKeyPressed(player1->moveLeft) &&
                    !(player1->direction.x == 1 && player1->direction.y == 0)) {
                    player1->nextDirection = sf::Vector2f(-1, 0);
                }
                else if (sf::Keyboard::isKeyPressed(player1->moveRight) &&
                    !(player1->direction.x == -1 && player1->direction.y == 0)) {
                    player1->nextDirection = sf::Vector2f(1, 0);
                }
            }

            if (player2->isAlive) {
                // Управление ускорением/замедлением для Player 2
                bool shiftPressed2 = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);
                bool ctrlPressed2 = sf::Keyboard::isKeyPressed(sf::Keyboard::LControl);

                if (shiftPressed2 && !ctrlPressed2) {
                    player2->setBoost(true);
                    player2->setSlow(false);
                }
                else if (ctrlPressed2 && !shiftPressed2) {
                    player2->setBoost(false);
                    player2->setSlow(true);
                }
                else {
                    player2->setBoost(false);
                    player2->setSlow(false);
                }

                // Управление направлением Player 2 (WASD)
                if (sf::Keyboard::isKeyPressed(player2->moveUp) &&
                    !(player2->direction.x == 0 && player2->direction.y == 1)) {
                    player2->nextDirection = sf::Vector2f(0, -1);
                }
                else if (sf::Keyboard::isKeyPressed(player2->moveDown) &&
                    !(player2->direction.x == 0 && player2->direction.y == -1)) {
                    player2->nextDirection = sf::Vector2f(0, 1);
                }
                else if (sf::Keyboard::isKeyPressed(player2->moveLeft) &&
                    !(player2->direction.x == 1 && player2->direction.y == 0)) {
                    player2->nextDirection = sf::Vector2f(-1, 0);
                }
                else if (sf::Keyboard::isKeyPressed(player2->moveRight) &&
                    !(player2->direction.x == -1 && player2->direction.y == 0)) {
                    player2->nextDirection = sf::Vector2f(1, 0);
                }
            }
        }
        else {
            // Одиночная игра (существующий код)
            Snake* playerSnake = nullptr;

            // НАЙТИ игрока в списке змей
            for (auto snake : snakes) {
                if (snake->isPlayer) {
                    playerSnake = snake;
                    break;
                }
            }

            if (!playerSnake) return;

            // Управление ускорением/замедлением
            bool shiftPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
            bool ctrlPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::RControl);

            // Проверяем, можно ли ускоряться/замедляться одновременно
            if (shiftPressed && !ctrlPressed) {
                playerSnake->setBoost(true);
                playerSnake->setSlow(false);
            }
            else if (ctrlPressed && !shiftPressed) {
                playerSnake->setBoost(false);
                playerSnake->setSlow(true);
            }
            else {
                playerSnake->setBoost(false);
                playerSnake->setSlow(false);
            }

            // Управление направлением (существующий код)
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
        }  // Закрывающая скобка для else, без точки с запятой
    }  // Закрывающая скобка для метода

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
        field.setPosition(50, 60); // ПОДНИМАЕМ НА 10px (было 50, стало 60)
        field.setFillColor(sf::Color(10, 30, 10));
        field.setOutlineThickness(2);
        field.setOutlineColor(sf::Color::White);
        window->draw(field);

        // Рисуем сетку ТОЛЬКО если включено в настройках
        if (settings.showGrid && snakes.size() > 0) {
            int cellSize = snakes[0]->cellSize;

            // Рисуем границы клеток - ТОНКИМИ светло-серыми линиями
            for (int y = 60; y <= 860; y += cellSize) { // Начинаем с 60 вместо 50
                sf::RectangleShape hLine(sf::Vector2f(900, 1));
                hLine.setPosition(50, y);
                hLine.setFillColor(sf::Color(150, 150, 150, 80));
                window->draw(hLine);
            }

            for (int x = 50; x <= 950; x += cellSize) {
                sf::RectangleShape vLine(sf::Vector2f(1, 800));
                vLine.setPosition(x, 60); // Начинаем с 60 вместо 50
                vLine.setFillColor(sf::Color(150, 150, 150, 80));
                window->draw(vLine);
            }
        }

        // Рисуем фрукты (корректируем позицию)
        for (auto fruit : gameFruits) {
            // Корректируем позицию фрукта для нового расположения поля
            sf::CircleShape fruitShape = fruit->shape;
            fruitShape.setPosition(fruitShape.getPosition().x, fruitShape.getPosition().y + 10);
            window->draw(fruitShape);
        }

        // Рисуем змей (корректируем позицию)
        for (auto snake : snakes) {
            // Временно корректируем позиции для отрисовки
            std::vector<sf::Vector2f> originalPositions = snake->body;

            // Сдвигаем все позиции змеи вниз на 10px
            for (auto& pos : snake->body) {
                pos.y += 10;
            }

            snake->draw(*window);

            // Восстанавливаем оригинальные позиции
            snake->body = originalPositions;
        }

        // Рисуем UI
        renderGameUI();
    }

    void startSparringGame(int rounds) {
        std::cout << "=== STARTING SPARRING GAME ===" << std::endl;
        std::cout << "Sparring mode: 2 players, " << rounds << " rounds" << std::endl;

        gameState = PLAYING;
        gameData.totalRounds = rounds;
        gameData.totalBots = 0;
        gameData.currentRound = 1;  // Начинаем с первого раунда
        gameData.score = 0;
        gameData.roundWins = 0;
        gameData.gameTime = 0.0f;
        gameData.isAlive = true;
        gameData.isMultiplayer = true;

        // Очистка старых объектов
        for (auto snake : snakes) delete snake;
        for (auto fruit : gameFruits) delete fruit;
        snakes.clear();
        gameFruits.clear();

        // Создание ПЕРВОГО игрока (Управление стрелками)
        Snake* player1 = new Snake("Player 1", sf::Color::Blue, settings, true);
        // Настройка управления для Player 1 - СТРЕЛКИ
        player1->moveUp = sf::Keyboard::Up;
        player1->moveDown = sf::Keyboard::Down;
        player1->moveLeft = sf::Keyboard::Left;
        player1->moveRight = sf::Keyboard::Right;

        // Позиция Player 1 (левая часть поля)
        int startGridX1 = 10;
        int startGridY1 = 22;
        float startX1 = startGridX1 * player1->cellSize;
        float startY1 = startGridY1 * player1->cellSize;
        player1->body.clear();
        player1->body.push_back(sf::Vector2f(startX1, startY1));

        // Сброс состояния ускорения
        player1->setBoost(false);
        player1->setSlow(false);
        player1->currentSpeed = player1->baseSpeed;

        snakes.push_back(player1);

        // Создание ВТОРОГО игрока (Управление WASD)
        Snake* player2 = new Snake("Player 2", sf::Color::Red, settings, true);
        // Настройка управления для Player 2 - WASD
        player2->moveUp = sf::Keyboard::W;
        player2->moveDown = sf::Keyboard::S;
        player2->moveLeft = sf::Keyboard::A;
        player2->moveRight = sf::Keyboard::D;

        // Позиция Player 2 (правая часть поля)
        int startGridX2 = 40;
        int startGridY2 = 22;
        float startX2 = startGridX2 * player2->cellSize;
        float startY2 = startGridY2 * player2->cellSize;
        player2->body.clear();
        player2->body.push_back(sf::Vector2f(startX2, startY2));

        // Направление Player 2 - влево (к центру)
        player2->direction = sf::Vector2f(-1, 0);
        player2->nextDirection = player2->direction;

        // Сброс состояния ускорения
        player2->setBoost(false);
        player2->setSlow(false);
        player2->currentSpeed = player2->baseSpeed;

        snakes.push_back(player2);

        // Устанавливаем данные для первого игрока как основного
        gameData.playerName = "Player 1 & Player 2";
        gameData.playerColor = sf::Color::White; // Нейтральный цвет

        // Создание начальных фруктов
        for (int i = 0; i < 5; i++) { // Больше фруктов для двух игроков
            spawnGameFruit();
        }

        gameClock.restart();
        fruitSpawnClock.restart();

        std::cout << "Sparring game started successfully!" << std::endl;
        std::cout << "Player 1: Arrow keys, Player 2: WASD" << std::endl;
    }

    void renderGameUI() {
        // Фон для UI (полупрозрачная панель сверху) - УВЕЛИЧИМ ВЫСОТУ
        sf::RectangleShape uiPanel(sf::Vector2f(1000, 60)); // Уменьшили с 80 до 60
        uiPanel.setFillColor(sf::Color(0, 0, 0, 180)); // Более темный
        uiPanel.setPosition(0, 0);
        window->draw(uiPanel);

        // Декоративная линия под панелью - ПОДНИМАЕМ ВЫШЕ
        sf::RectangleShape line(sf::Vector2f(1000, 2));
        line.setFillColor(sf::Color::Green);
        line.setPosition(0, 60); // Было 80
        window->draw(line);

        // === ЛЕВАЯ ЧАСТЬ ===

        // УБИРАЕМ "SNAKE GAME" - оставляем только счет

        // Счет игрока - УМЕНЬШАЕМ РАЗМЕР
        sf::Text scoreDisplay;
        scoreDisplay.setFont(font);
        scoreDisplay.setString("SCORE: " + std::to_string(gameData.score));
        scoreDisplay.setCharacterSize(24); // Уменьшили с 28
        scoreDisplay.setFillColor(sf::Color::Yellow);
        scoreDisplay.setStyle(sf::Text::Bold);
        scoreDisplay.setPosition(20, 15); // Подняли выше

        window->draw(scoreDisplay);

        // Если боты есть, показываем счет ботов - УМЕНЬШАЕМ
        if (gameData.totalBots > 0) {
            int totalBotScore = 0;

            for (auto snake : snakes) {
                if (!snake->isPlayer) {
                    totalBotScore += snake->score;
                }
            }

            sf::Text botScoreDisplay;
            botScoreDisplay.setFont(font);
            botScoreDisplay.setString("BOTS: " + std::to_string(totalBotScore));
            botScoreDisplay.setCharacterSize(18); // Уменьшили
            botScoreDisplay.setFillColor(sf::Color::Cyan);
            botScoreDisplay.setPosition(20, 40); // Подняли и сдвинули левее
            window->draw(botScoreDisplay);
        }

        // === ЦЕНТРАЛЬНАЯ ЧАСТЬ ===

        // Отображение текущей сложности - ДОБАВЛЯЕМ СЛЕВА
        sf::Text difficultyText;
        difficultyText.setFont(font);
        std::string diffName;
        switch (settings.difficulty) {
        case 1: diffName = "ЛЕГКО"; break;
        case 2: diffName = "СРЕДНЕ"; break;
        case 3: diffName = "СЛОЖНО"; break;
        default: diffName = "ЛЕГКО"; break;
        }
        difficultyText.setString("УРОВЕНЬ: " + diffName);
        difficultyText.setCharacterSize(18); // Уменьшили
        difficultyText.setFillColor(sf::Color::White);

        // Позиционируем левее центра
        difficultyText.setPosition(200, 20); // Слева от центра
        window->draw(difficultyText);

        // Информация о раунде (если больше 1 раунда) - УМЕНЬШАЕМ
        if (gameData.totalRounds > 1) {
            // Номер раунда
            sf::Text roundNumber;
            roundNumber.setFont(font);
            roundNumber.setString("РАУНД " + std::to_string(gameData.currentRound));
            roundNumber.setCharacterSize(20); // Уменьшили
            roundNumber.setFillColor(sf::Color::Yellow);
            roundNumber.setStyle(sf::Text::Bold);

            // Центрирование номера раунда
            sf::FloatRect roundBounds = roundNumber.getLocalBounds();
            roundNumber.setOrigin(roundBounds.left + roundBounds.width / 2.0f,
                roundBounds.top + roundBounds.height / 2.0f);
            roundNumber.setPosition(500, 20); // Подняли выше
            window->draw(roundNumber);

            // Всего раундов
            sf::Text totalRounds;
            totalRounds.setFont(font);
            totalRounds.setString("из " + std::to_string(gameData.totalRounds));
            totalRounds.setCharacterSize(16); // Уменьшили
            totalRounds.setFillColor(sf::Color::White);

            // Центрирование
            sf::FloatRect totalBounds = totalRounds.getLocalBounds();
            totalRounds.setOrigin(totalBounds.left + totalBounds.width / 2.0f,
                totalBounds.top + totalBounds.height / 2.0f);
            totalRounds.setPosition(500, 40); // Подняли выше
            window->draw(totalRounds);
        }
        else {
            // Если один раунд, показываем просто "РАУНД 1"
            sf::Text roundText;
            roundText.setFont(font);
            roundText.setString("РАУНД 1");
            roundText.setCharacterSize(20);
            roundText.setFillColor(sf::Color::White);

            sf::FloatRect roundBounds = roundText.getLocalBounds();
            roundText.setOrigin(roundBounds.left + roundBounds.width / 2.0f,
                roundBounds.top + roundBounds.height / 2.0f);
            roundText.setPosition(500, 30);
            window->draw(roundText);
        }

        // === ПРАВАЯ ЧАСТЬ ===

        // Имя игрока/игроков - УМЕНЬШАЕМ
        if (gameData.isMultiplayer && snakes.size() >= 2) {
            // Режим спаринга - показываем обоих игроков компактно

            // Player 1
            Snake* player1 = snakes[0];
            sf::Text p1Info;
            p1Info.setFont(font);
            p1Info.setString("P1: " + std::to_string(player1->score));
            p1Info.setCharacterSize(20); // Уменьшили
            p1Info.setFillColor(player1->isAlive ? player1->color : sf::Color(150, 150, 150));
            p1Info.setPosition(700, 15);
            window->draw(p1Info);

            // Player 2
            Snake* player2 = snakes[1];
            sf::Text p2Info;
            p2Info.setFont(font);
            p2Info.setString("P2: " + std::to_string(player2->score));
            p2Info.setCharacterSize(20); // Уменьшили
            p2Info.setFillColor(player2->isAlive ? player2->color : sf::Color(150, 150, 150));
            p2Info.setPosition(700, 40);
            window->draw(p2Info);
        }
        else {
            // Одиночная игра - показываем компактно

            // Имя игрока и счет в одну строку
            sf::Text playerInfo;
            playerInfo.setFont(font);

            Snake* playerSnake = nullptr;
            for (auto snake : snakes) {
                if (snake->isPlayer) {
                    playerSnake = snake;
                    break;
                }
            }

            if (playerSnake) {
                std::string status = playerSnake->isAlive ? "" : " (DEAD)";
                playerInfo.setString(gameData.playerName + ": " + std::to_string(gameData.score) + status);
                playerInfo.setCharacterSize(20); // Уменьшили
                playerInfo.setFillColor(playerSnake->isAlive ? gameData.playerColor : sf::Color(150, 150, 150));
                playerInfo.setPosition(700, 20);
                window->draw(playerInfo);
            }
        }

        // === НИЖНЯЯ ЧАСТЬ ===

        // Статус ускорения/замедления - УМЕНЬШАЕМ
        Snake* playerSnake = nullptr;
        for (auto snake : snakes) {
            if (snake->isPlayer) {
                playerSnake = snake;
                break;
            }
        }

        if (playerSnake) {
            if (playerSnake->isBoosting) {
                sf::Text boostText;
                boostText.setFont(font);
                boostText.setString("BOOST");
                boostText.setCharacterSize(16); // Уменьшили
                boostText.setFillColor(sf::Color::Green);
                boostText.setPosition(20, 850);
                window->draw(boostText);
            }
            else if (playerSnake->isSlowing) {
                sf::Text slowText;
                slowText.setFont(font);
                slowText.setString("SLOW");
                slowText.setCharacterSize(16); // Уменьшили
                slowText.setFillColor(sf::Color::Red);
                slowText.setPosition(20, 850);
                window->draw(slowText);
            }
        }

        // Таймер в левом нижнем углу - УМЕНЬШАЕМ
        sf::Text timerDisplay;
        timerDisplay.setFont(font);
        timerDisplay.setString("ВРЕМЯ: " + std::to_string((int)gameData.gameTime) + "с");
        timerDisplay.setCharacterSize(16); // Уменьшили
        timerDisplay.setFillColor(sf::Color(200, 200, 200));
        timerDisplay.setPosition(20, 870);
        window->draw(timerDisplay);

        // Информация о ботах в правом нижнем углу (если есть) - УМЕНЬШАЕМ
        if (gameData.totalBots > 0) {
            int aliveBots = 0;
            for (auto snake : snakes) {
                if (!snake->isPlayer && snake->isAlive) {
                    aliveBots++;
                }
            }

            sf::Text botCounter;
            botCounter.setFont(font);
            botCounter.setString("БОТЫ: " + std::to_string(aliveBots) + "/" + std::to_string(gameData.totalBots));
            botCounter.setCharacterSize(16); // Уменьшили
            botCounter.setFillColor(aliveBots > 0 ? sf::Color::Yellow : sf::Color::Green);
            botCounter.setPosition(850, 850);
            window->draw(botCounter);
        }

        // Инструкция для паузы в правом нижнем углу - УМЕНЬШАЕМ
        if (gameState == PLAYING) {
            sf::Text instruction;
            instruction.setFont(font);
            instruction.setString("ESC - ПАУЗА");
            instruction.setCharacterSize(16); // Уменьшили
            instruction.setFillColor(sf::Color(180, 180, 180));
            instruction.setPosition(850, 870);
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
            // ОБНОВЛЕНИЕ ПОДСВЕТКИ КНОПОК
            updateButtonHover(mousePos);
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
            gameState = PLAYING;
            pauseDialog->hide();
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
            // Сброс hover состояния перед обработкой клика
            for (auto btn : mainButtons) {
                if (btn->hovered) {
                    // Возвращаем оригинальный цвет
                    sf::Color currentColor = btn->rect.getFillColor();
                    btn->rect.setFillColor(sf::Color(
                        std::max(currentColor.r - 30, 0),
                        std::max(currentColor.g - 30, 0),
                        std::max(currentColor.b - 30, 0),
                        currentColor.a
                    ));
                    btn->hovered = false;
                }
            }
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
                    showTitle = false;
                    // Сброс hover для кнопок на новом экране
                    for (auto btn : gameSettingsButtons) {
                        btn->hovered = false;
                    }
                }
                else if (action == "Настройки") {
                    currentScreen = Screen::PLAYER_SETTINGS;
                    initPlayerSettings();
                    showTitle = false;
                    // Сброс hover для кнопок на новом экране
                    for (auto btn : playerSettingsButtons) {
                        btn->hovered = false;
                    }
                }
                else if (action == "О создателях") {
                    currentScreen = Screen::ABOUT;
                    initAbout();
                    showTitle = false;
                    // Сброс hover для кнопок на новом экране
                    for (auto btn : aboutButtons) {
                        btn->hovered = false;
                    }
                }
                else if (action == "Выход") {
                    currentScreen = Screen::EXIT_CONFIRM;
                    showTitle = false;
                    // Сброс hover для кнопок на новом экране
                    for (auto btn : exitConfirmButtons) {
                        btn->hovered = false;
                    }
                }
            }
            break;

        case Screen::GAME_SETTINGS:
            // Сброс hover состояния перед обработкой клика
            for (auto btn : gameSettingsButtons) {
                if (btn->hovered) {
                    sf::Color currentColor = btn->rect.getFillColor();
                    btn->rect.setFillColor(sf::Color(
                        std::max(currentColor.r - 30, 0),
                        std::max(currentColor.g - 30, 0),
                        std::max(currentColor.b - 30, 0),
                        currentColor.a
                    ));
                    btn->hovered = false;
                }
            }
            clicked = handleMouseClick(gameSettingsButtons, mousePos);
            if (clicked) {
                std::string action = "";
                for (auto& btn : gameSettingsButtons) {
                    if (btn->clicked) {
                        action = btn->action;
                        break;
                    }
                }

                if (action == "start_game") {
                    std::cout << "Game Starting: " << rounds << " rounds, " << bots << " bots" << std::endl;
                    // СБРОС ФЛАГА МУЛЬТИПЛЕЕРА ПЕРЕД СТАРТОМ ИГРЫ
                    gameData.isMultiplayer = false;
                    startGame(rounds, bots);
                }
                else if (action == "change_difficulty") {
                    // Циклическое изменение сложности
                    settings.difficulty++;
                    if (settings.difficulty > 3) {
                        settings.difficulty = 1;
                    }

                    // Обновляем текст кнопки
                    std::string diffText;
                    switch (settings.difficulty) {
                    case 1:
                        diffText = "Сложность: Легко";
                        settings.fruitSpoilRateEasy = 0.5f;
                        settings.fruitSpoilRateMedium = 1.0f;
                        settings.fruitSpoilRateHard = 1.5f;
                        break;
                    case 2:
                        diffText = "Сложность: Средне";
                        settings.fruitSpoilRateEasy = 1.0f;
                        settings.fruitSpoilRateMedium = 1.5f;
                        settings.fruitSpoilRateHard = 2.0f;
                        break;
                    case 3:
                        diffText = "Сложность: Сложно";
                        settings.fruitSpoilRateEasy = 1.5f;
                        settings.fruitSpoilRateMedium = 2.0f;
                        settings.fruitSpoilRateHard = 2.5f;
                        break;
                    }

                    gameSettingsButtons[1]->text.setString(diffText);

                    // Центрируем текст
                    sf::FloatRect textBounds = gameSettingsButtons[1]->text.getLocalBounds();
                    gameSettingsButtons[1]->text.setOrigin(textBounds.left + textBounds.width / 2.0f,
                        textBounds.top + textBounds.height / 2.0f);

                    // Сохраняем настройки
                    saveSettings();

                    std::cout << "Difficulty changed to: " << settings.difficulty
                        << " (spoil rates: Easy=" << settings.fruitSpoilRateEasy
                        << ", Medium=" << settings.fruitSpoilRateMedium
                        << ", Hard=" << settings.fruitSpoilRateHard << ")" << std::endl;
                }
                else if (action == "quick_game") {
                    rounds = 1; // Быстрая игра = 1 раунд
                    // ОБЯЗАТЕЛЬНО СБРАСЫВАЕМ ФЛАГ МУЛЬТИПЛЕЕРА
                    gameData.isMultiplayer = false;
                    // Обновляем текст кнопки раундов (индекс 4 теперь)
                    gameSettingsButtons[4]->text.setString("Раунды: 1");
                    sf::FloatRect textBounds = gameSettingsButtons[4]->text.getLocalBounds();
                    gameSettingsButtons[4]->text.setOrigin(textBounds.left + textBounds.width / 2.0f,
                        textBounds.top + textBounds.height / 2.0f);

                    // Сразу запускаем быструю игру: 1 раунд, 0 ботов
                    std::cout << "Starting Quick Game: 1 round, 0 bots, Difficulty: " << settings.difficulty << std::endl;
                    startGame(1, 0);
                }
                else if (action == "sparring_mode") {
                    bots = 0;
                    gameData.isMultiplayer = true; // Включаем режим мультиплеера

                    // Обновляем текст кнопки ботов (индексы 5 и 6 теперь)
                    gameSettingsButtons[5]->text.setString("Боты: 0");
                    gameSettingsButtons[6]->text.setString("Боты: 1");

                    // Обновляем текст для обеих кнопок ботов
                    for (int i = 5; i <= 6; i++) {
                        sf::FloatRect textBounds = gameSettingsButtons[i]->text.getLocalBounds();
                        gameSettingsButtons[i]->text.setOrigin(textBounds.left + textBounds.width / 2.0f,
                            textBounds.top + textBounds.height / 2.0f);

                        // Подсветка выбранного количества ботов
                        if (i == 5) { // Кнопка "Боты: 0"
                            gameSettingsButtons[i]->rect.setOutlineColor(sf::Color::Yellow);
                            gameSettingsButtons[i]->rect.setOutlineThickness(3);
                        }
                        else {
                            gameSettingsButtons[i]->rect.setOutlineColor(sf::Color::Black);
                            gameSettingsButtons[i]->rect.setOutlineThickness(2);
                        }
                    }

                    // Сразу запускаем спарринг с выбранным количеством раундов
                    std::cout << "Starting Sparring Mode: " << rounds << " rounds, 2 players, Difficulty: " << settings.difficulty << std::endl;
                    startGame(rounds, 0);
                }
                else if (action == "set_rounds") {
                    // Здесь меняем количество раундов
                    if (rounds == 1) rounds = 3;
                    else if (rounds == 3) rounds = 5;
                    else if (rounds == 5) rounds = 1;

                    // Обновляем текст кнопки раундов (индекс 4 теперь)
                    gameSettingsButtons[4]->text.setString("Раунды: " + std::to_string(rounds));
                    sf::FloatRect textBounds = gameSettingsButtons[4]->text.getLocalBounds();
                    gameSettingsButtons[4]->text.setOrigin(textBounds.left + textBounds.width / 2.0f,
                        textBounds.top + textBounds.height / 2.0f);
                }
                else if (action.find("bots_") == 0) {
                    bots = std::stoi(action.substr(5));
                    for (int i = 5; i <= 6; i++) {
                        int botCount = i - 5;
                        std::string text = "Боты: " + std::to_string(botCount);
                        if (botCount == 0) text += "";
                        gameSettingsButtons[i]->text.setString(text);

                        sf::FloatRect textBounds = gameSettingsButtons[i]->text.getLocalBounds();
                        gameSettingsButtons[i]->text.setOrigin(textBounds.left + textBounds.width / 2.0f,
                            textBounds.top + textBounds.height / 2.0f);

                        if (botCount == bots) {
                            gameSettingsButtons[i]->rect.setOutlineColor(sf::Color::Yellow);
                            gameSettingsButtons[i]->rect.setOutlineThickness(3);
                        }
                        else {
                            gameSettingsButtons[i]->rect.setOutlineColor(sf::Color::Black);
                            gameSettingsButtons[i]->rect.setOutlineThickness(2);
                        }
                    }
                    gameData.isMultiplayer = false;
                }
                else if (action == "back") {
                    currentScreen = Screen::MAIN;
                    initMainMenu();
                    showTitle = true;
                    // Сброс hover для кнопок главного меню
                    for (auto btn : mainButtons) {
                        btn->hovered = false;
                    }
                }
            }
            break;

        case Screen::PLAYER_SETTINGS:
            // Сброс hover состояния перед обработкой клика
            for (auto btn : playerSettingsButtons) {
                if (btn->hovered) {
                    sf::Color currentColor = btn->rect.getFillColor();
                    btn->rect.setFillColor(sf::Color(
                        std::max(currentColor.r - 30, 0),
                        std::max(currentColor.g - 30, 0),
                        std::max(currentColor.b - 30, 0),
                        currentColor.a
                    ));
                    btn->hovered = false;
                }
            }
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
                    // Сброс hover для кнопок редактирования клавиш
                    for (auto btn : editKeysButtons) {
                        btn->hovered = false;
                    }
                }
                else if (action == "edit_name") {
                    nameInputActive = true;
                    nameInputText = settings.playerName;
                }
                else if (action == "field_size") {
                    settings.fieldSize = (settings.fieldSize % 3 + 1) * 10;
                    playerSettingsButtons[8]->text.setString("Field size: " + std::to_string(settings.fieldSize));
                    sf::FloatRect textBounds = playerSettingsButtons[8]->text.getLocalBounds();
                    playerSettingsButtons[8]->text.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
                }
                else if (action == "save_settings") {
                    saveSettings();
                    currentScreen = Screen::MAIN;
                    initMainMenu();
                    // Сброс hover для кнопок главного меню
                    for (auto btn : mainButtons) {
                        btn->hovered = false;
                    }
                }
                else if (action == "back") {
                    currentScreen = Screen::MAIN;
                    initMainMenu();
                    showTitle = true;
                    // Сброс hover для кнопок главного меню
                    for (auto btn : mainButtons) {
                        btn->hovered = false;
                    }
                }
                else if (action == "reset_defaults") {
                    resetToDefaultSettings();
                    playerSettingsButtons[8]->text.setString("Field size: " + std::to_string(settings.fieldSize));
                }
            }
            break;

        case Screen::ABOUT:
            // Сброс hover состояния перед обработкой клика
            for (auto btn : aboutButtons) {
                if (btn->hovered) {
                    sf::Color currentColor = btn->rect.getFillColor();
                    btn->rect.setFillColor(sf::Color(
                        std::max(currentColor.r - 30, 0),
                        std::max(currentColor.g - 30, 0),
                        std::max(currentColor.b - 30, 0),
                        currentColor.a
                    ));
                    btn->hovered = false;
                }
            }
            clicked = handleMouseClick(aboutButtons, mousePos);
            if (clicked && aboutButtons[0]->clicked) {
                currentScreen = Screen::MAIN;
                initMainMenu();
                showTitle = true;
                // Сброс hover для кнопок главного меню
                for (auto btn : mainButtons) {
                    btn->hovered = false;
                }
            }
            break;

        case Screen::EXIT_CONFIRM:
            // Сброс hover состояния перед обработкой клика
            for (auto btn : exitConfirmButtons) {
                if (btn->hovered) {
                    sf::Color currentColor = btn->rect.getFillColor();
                    btn->rect.setFillColor(sf::Color(
                        std::max(currentColor.r - 30, 0),
                        std::max(currentColor.g - 30, 0),
                        std::max(currentColor.b - 30, 0),
                        currentColor.a
                    ));
                    btn->hovered = false;
                }
            }
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
                    // Сброс hover для кнопок главного меню
                    for (auto btn : mainButtons) {
                        btn->hovered = false;
                    }
                }
            }
            break;

        case Screen::EDIT_KEYS:
            // Сброс hover состояния перед обработкой клика
            for (auto btn : editKeysButtons) {
                if (btn->hovered) {
                    sf::Color currentColor = btn->rect.getFillColor();
                    btn->rect.setFillColor(sf::Color(
                        std::max(currentColor.r - 30, 0),
                        std::max(currentColor.g - 30, 0),
                        std::max(currentColor.b - 30, 0),
                        currentColor.a
                    ));
                    btn->hovered = false;
                }
            }
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
                    // Сброс hover для кнопок настроек игрока
                    for (auto btn : playerSettingsButtons) {
                        btn->hovered = false;
                    }
                }
                else if (action == "reset_wasd") {
                    settings.playerKeys = { sf::Keyboard::W, sf::Keyboard::A, sf::Keyboard::S, sf::Keyboard::D };
                }
                else if (action == "reset_arrows") {
                    settings.playerKeys = { sf::Keyboard::Up, sf::Keyboard::Left, sf::Keyboard::Down, sf::Keyboard::Right };
                }
            }
            else {
                // Проверка кликов по областям с клавишами (только если не активно редактирование)
                if (!keyInputActive) {
                    for (int i = 0; i < 4; i++) {
                        sf::FloatRect keyRect(200, 300 + i * 70, 600, 60);
                        if (keyRect.contains(mousePos)) {
                            keyInputActive = true;
                            currentKeyIndex = i;
                            break;
                        }
                    }
                }
            }
            break;
        }

        // Сброс состояния clicked для всех кнопок на текущем экране
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
                // Заголовок
                sf::Text title;
                title.setFont(font);
                title.setString("НАСТРОЙКИ ИГРЫ");
                title.setCharacterSize(36);
                title.setFillColor(sf::Color::Yellow);
                title.setStyle(sf::Text::Bold);

                // Центрируем заголовок
                sf::FloatRect titleBounds = title.getLocalBounds();
                title.setOrigin(titleBounds.left + titleBounds.width / 2.0f,
                    titleBounds.top + titleBounds.height / 2.0f);
                title.setPosition(500, 100);

                // Тень заголовка
                sf::Text titleShadow = title;
                titleShadow.setFillColor(sf::Color::Black);
                titleShadow.setPosition(502, 102);
                window->draw(titleShadow);
                window->draw(title);

                // Информация о текущих настройках
                sf::Text info;
                info.setFont(font);
                std::string roundsText = (rounds == 999) ? "? (выживание)" : std::to_string(rounds);
                info.setString("Текущие настройки: " + roundsText + " раундов, " + std::to_string(bots) + " ботов");
                info.setCharacterSize(20);
                info.setFillColor(sf::Color::White);

                sf::FloatRect infoBounds = info.getLocalBounds();
                info.setOrigin(infoBounds.left + infoBounds.width / 2.0f,
                    infoBounds.top + infoBounds.height / 2.0f);
                info.setPosition(500, 150);

                sf::Text infoShadow = info;
                infoShadow.setFillColor(sf::Color::Black);
                infoShadow.setPosition(502, 152);
                window->draw(infoShadow);
                window->draw(info);

                // Отображаем кнопки
                for (auto btn : gameSettingsButtons) {
                    window->draw(btn->rect);
                    window->draw(btn->text);
                }



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
                std::string colorName = "Фиолетовый";

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
                currentColorText.setFillColor(sf::Color(144, 238, 144));
                currentColorText.setPosition(260, 160);
                window->draw(currentColorText);

                // ОТРИСОВКА КНОПОК ЦВЕТОВ - ЭТОТ БЛОК ДОЛЖЕН БЫТЬ!
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

                // УПРАВЛЕНИЕ - на одной строке
                sf::Text controlLabel;
                controlLabel.setFont(font);
                controlLabel.setString("Управление:");
                controlLabel.setCharacterSize(24);
                controlLabel.setFillColor(sf::Color(200, 200, 255));
                controlLabel.setPosition(100, 280);
                window->draw(controlLabel);

                // Клавиши отдельно - желтым цветом
                sf::Text keysText;
                keysText.setFont(font);
                keysText.setString(getKeyName(settings.playerKeys[0]) + " " +
                    getKeyName(settings.playerKeys[1]) + " " +
                    getKeyName(settings.playerKeys[2]) + " " +
                    getKeyName(settings.playerKeys[3]));
                keysText.setCharacterSize(24);
                keysText.setFillColor(sf::Color(144, 238, 144)); // Клавиши желтым
                // Позиционируем после "Управление:"
                keysText.setPosition(100 + controlLabel.getLocalBounds().width + 10, 280);
                window->draw(keysText);

                // Кнопки управления в ряд (3 штуки) - опускаем ниже
                float controlStartX = 100;
                float controlY = 330; // Опускаем немного ниже

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

                // ИМЯ ИГРОКА - на одной строке
                sf::Text nameLabel;
                nameLabel.setFont(font);
                nameLabel.setString("Имя игрока:");
                nameLabel.setCharacterSize(24);
                nameLabel.setFillColor(sf::Color(200, 200, 255));
                nameLabel.setPosition(100, 390);
                window->draw(nameLabel);

                // Текущее имя - зеленым
                sf::Text nameValueText;
                nameValueText.setFont(font);
                nameValueText.setString(settings.playerName);
                nameValueText.setCharacterSize(24);
                nameValueText.setFillColor(sf::Color(150, 255, 150));
                nameValueText.setPosition(100 + nameLabel.getLocalBounds().width + 10, 390);
                window->draw(nameValueText);

                // Кнопка изменения имени (индекс 7)
                if (7 < playerSettingsButtons.size()) {
                    playerSettingsButtons[7]->rect.setPosition(100, 430);
                    playerSettingsButtons[7]->rect.setSize(sf::Vector2f(400, 50));

                    // Центрируем текст в кнопке
                    sf::FloatRect nameBounds = playerSettingsButtons[7]->text.getLocalBounds();
                    playerSettingsButtons[7]->text.setOrigin(
                        nameBounds.left + nameBounds.width / 2.0f,
                        nameBounds.top + nameBounds.height / 2.0f
                    );
                    playerSettingsButtons[7]->text.setPosition(300, 455);

                    window->draw(playerSettingsButtons[7]->rect);
                    window->draw(playerSettingsButtons[7]->text);
                }


                sf::Text gameTitle;
                gameTitle.setFont(font);
                gameTitle.setString("Параметры игры:");
                gameTitle.setCharacterSize(24);
                gameTitle.setFillColor(sf::Color(200, 200, 255));
                gameTitle.setPosition(100, 490); // Было 530, поднимаем выше
                window->draw(gameTitle);

                // Размер поля (индекс 8) - теперь с нормальными размерами
                if (8 < playerSettingsButtons.size()) {
                    playerSettingsButtons[8]->rect.setPosition(100, 530); // Было 570, поднимаем выше
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
                    playerSettingsButtons[8]->text.setPosition(250, 555); // Было 595, поднимаем выше

                    window->draw(playerSettingsButtons[8]->rect);
                    window->draw(playerSettingsButtons[8]->text);
                }

                // Кнопка сохранения (индекс 9) - также поднимаем
                if (9 < playerSettingsButtons.size()) {
                    playerSettingsButtons[9]->rect.setPosition(450, 530); // Было 570, поднимаем выше
                    playerSettingsButtons[9]->rect.setSize(sf::Vector2f(300, 50));

                    // Центрируем текст
                    sf::FloatRect saveBounds = playerSettingsButtons[9]->text.getLocalBounds();
                    playerSettingsButtons[9]->text.setOrigin(
                        saveBounds.left + saveBounds.width / 2.0f,
                        saveBounds.top + saveBounds.height / 2.0f
                    );
                    playerSettingsButtons[9]->text.setPosition(600, 555); // Было 595, поднимаем выше

                    window->draw(playerSettingsButtons[9]->rect);
                    window->draw(playerSettingsButtons[9]->text);
                }


                float actionStartX = 100;    // Левая граница
                float actionStartY = 650;    // Было 750, поднимаем выше (т.к. убрали строку)
                float actionSpacing = 220;   // Расстояние между кнопками

                // Кнопка "Назад" (индекс 10)
                if (10 < playerSettingsButtons.size()) {
                    playerSettingsButtons[10]->rect.setPosition(actionStartX, actionStartY);
                    playerSettingsButtons[10]->rect.setSize(sf::Vector2f(200, 60));
                    playerSettingsButtons[10]->text.setString("Назад");

                    // Центрируем текст
                    sf::FloatRect backBounds = playerSettingsButtons[10]->text.getLocalBounds();
                    playerSettingsButtons[10]->text.setOrigin(
                        backBounds.left + backBounds.width / 2.0f,
                        backBounds.top + backBounds.height / 2.0f
                    );
                    playerSettingsButtons[10]->text.setPosition(
                        actionStartX + 100,
                        actionStartY + 30
                    );

                    window->draw(playerSettingsButtons[10]->rect);
                    window->draw(playerSettingsButtons[10]->text);
                }

                // Кнопка "Reset to Default" (индекс 11)
                if (11 < playerSettingsButtons.size()) {
                    playerSettingsButtons[11]->rect.setPosition(actionStartX + actionSpacing, actionStartY);
                    playerSettingsButtons[11]->rect.setSize(sf::Vector2f(250, 60));
                    playerSettingsButtons[11]->text.setString("По умолчанию");

                    // Центрируем текст
                    sf::FloatRect resetBounds = playerSettingsButtons[11]->text.getLocalBounds();
                    playerSettingsButtons[11]->text.setOrigin(
                        resetBounds.left + resetBounds.width / 2.0f,
                        resetBounds.top + resetBounds.height / 2.0f
                    );
                    playerSettingsButtons[11]->text.setPosition(
                        actionStartX + actionSpacing + 125,
                        actionStartY + 30
                    );

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
                editTitle.setString("НАСТРОЙКА УПРАВЛЕНИЯ");
                editTitle.setCharacterSize(36);
                editTitle.setFillColor(sf::Color::Yellow);
                editTitle.setStyle(sf::Text::Bold);

                // Центрируем заголовок
                sf::FloatRect titleRect = editTitle.getLocalBounds();
                editTitle.setOrigin(titleRect.left + titleRect.width / 2.0f,
                    titleRect.top + titleRect.height / 2.0f);
                editTitle.setPosition(500, 120); // Подняли заголовок выше

                // Тень заголовка
                sf::Text editTitleShadow = editTitle;
                editTitleShadow.setFillColor(sf::Color::Black);
                editTitleShadow.setPosition(502, 122);
                window->draw(editTitleShadow);
                window->draw(editTitle);

                // Инструкция (только при активном вводе)
                if (keyInputActive) {
                    sf::Text instruction;
                    instruction.setFont(font);
                    instruction.setCharacterSize(24);
                    instruction.setFillColor(sf::Color(255, 200, 100));
                    instruction.setString("Нажмите клавишу для: " + keyNames[currentKeyIndex] + "\n(ESC - отмена)");

                    // Центрируем инструкцию
                    sf::FloatRect instrRect = instruction.getLocalBounds();
                    instruction.setOrigin(instrRect.left + instrRect.width / 2.0f,
                        instrRect.top + instrRect.height / 2.0f);
                    instruction.setPosition(500, 180); // Подняли инструкцию

                    window->draw(instruction);
                }

                // Отображение текущих клавиш управления
                for (int i = 0; i < 4; i++) {
                    // Фон для строки с настройкой
                    sf::RectangleShape keyBox;
                    keyBox.setSize(sf::Vector2f(650, 60)); // Увеличили ширину
                    keyBox.setPosition(175, 240 + i * 90); // Подняли выше и увеличили расстояние (90px)

                    if (i == currentKeyIndex && keyInputActive) {
                        keyBox.setFillColor(sf::Color(80, 120, 80, 200));  // Зеленый для активного
                        keyBox.setOutlineColor(sf::Color::Yellow);
                        keyBox.setOutlineThickness(2);
                    }
                    else {
                        keyBox.setFillColor(sf::Color(60, 60, 100, 180));
                        keyBox.setOutlineColor(sf::Color(150, 150, 200));
                        keyBox.setOutlineThickness(1);
                    }
                    window->draw(keyBox);

                    // Название направления (на русском)
                    sf::Text dirText;
                    dirText.setFont(font);
                    std::string directionName;
                    switch (i) {
                    case 0: directionName = "Вверх/Ускорение"; break;
                    case 1: directionName = "Влево"; break;
                    case 2: directionName = "Вниз/Торможение"; break;
                    case 3: directionName = "Вправо"; break;
                    default: directionName = "Неизвестно"; break;
                    }
                    dirText.setString(directionName + ":");
                    dirText.setCharacterSize(24);
                    dirText.setFillColor(sf::Color::White);
                    dirText.setPosition(195, 255 + i * 90); // Подняли текст
                    window->draw(dirText);

                    // Текущая назначенная клавиша
                    sf::Text keyText;
                    keyText.setFont(font);
                    keyText.setString(getKeyName(settings.playerKeys[i]));
                    keyText.setCharacterSize(24);
                    keyText.setFillColor(sf::Color::Yellow);
                    keyText.setPosition(450, 255 + i * 90); // Сдвинули левее
                    window->draw(keyText);

                    // Инструкция для изменения (только если не активно редактирование)
                    if (!keyInputActive) {
                        sf::Text changeText;
                        changeText.setFont(font);
                        changeText.setString("[Нажмите для изменения]");
                        changeText.setCharacterSize(18);
                        changeText.setFillColor(sf::Color(180, 220, 255));
                        changeText.setPosition(580, 255 + i * 90); // Сдвинули левее, чтобы было внутри
                        window->draw(changeText);
                    }
                }

                // Кнопки внизу с увеличенными расстояниями
                float buttonY = 650; // Опустили кнопки ниже

                for (int i = 0; i < editKeysButtons.size(); i++) {
                    auto& btn = editKeysButtons[i];

                    // Устанавливаем позиции кнопок равномерно
                    float buttonWidth = 220; // Увеличили ширину кнопок
                    float totalWidth = 700; // Общая ширина для 3 кнопок
                    float spacing = (totalWidth - 3 * buttonWidth) / 3; // Увеличили расстояние между кнопками

                    float xPos = 150 + i * (buttonWidth + spacing);

                    btn->rect.setPosition(xPos, buttonY);
                    btn->rect.setSize(sf::Vector2f(buttonWidth, 60));

                    // Обновляем цвет кнопки "Назад"
                    if (btn->text.getString() == "Назад") {
                        btn->rect.setFillColor(sf::Color(150, 100, 50, 200));
                    }

                    // Центрируем текст в кнопках
                    sf::FloatRect textBounds = btn->text.getLocalBounds();
                    btn->text.setOrigin(textBounds.left + textBounds.width / 2.0f,
                        textBounds.top + textBounds.height / 2.0f);

                    // Устанавливаем позицию текста в центр кнопки
                    sf::FloatRect rectBounds = btn->rect.getGlobalBounds();
                    btn->text.setPosition(rectBounds.left + rectBounds.width / 2.0f,
                        rectBounds.top + rectBounds.height / 2.0f);

                    window->draw(btn->rect);
                    window->draw(btn->text);
                }

                // Обработка ховера для областей клавиш (только если не активно редактирование)
                if (!keyInputActive) {
                    sf::Vector2f mousePos = window->mapPixelToCoords(sf::Mouse::getPosition(*window));
                    for (int i = 0; i < 4; i++) {
                        sf::FloatRect keyRect(175, 240 + i * 90, 650, 60);
                        if (keyRect.contains(mousePos)) {
                            sf::RectangleShape hover;
                            hover.setSize(sf::Vector2f(650, 60));
                            hover.setPosition(175, 240 + i * 90);
                            hover.setFillColor(sf::Color(255, 255, 255, 30));
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