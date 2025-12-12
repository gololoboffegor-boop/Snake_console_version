// Snake_console_version.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>

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

struct Settings {
    std::string playerName = "Player";
    sf::Color playerColor = sf::Color::Blue;
    sf::Color botColor = sf::Color::Yellow;
    int fieldSize = 20;
    std::vector<sf::Keyboard::Key> playerKeys = {
        sf::Keyboard::W, sf::Keyboard::A, sf::Keyboard::S, sf::Keyboard::D
    };
};

class GameMenu {
private:
    sf::RenderWindow* window;
    sf::Font font;
    sf::Color bgColor = sf::Color(30, 30, 50);
    sf::Color titleColor = sf::Color::White;

    // Фоновая текстура и спрайт
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;
    bool backgroundLoaded = false;

    // Current screen state
    enum class Screen { MAIN, GAME_SETTINGS, PLAYER_SETTINGS, ABOUT, EXIT_CONFIRM, EDIT_KEYS };
    Screen currentScreen = Screen::MAIN;

    // Game settings
    int rounds = 1;
    int bots = 0;

    // Player settings
    Settings settings;

    // Buttons for each screen
    std::vector<Button*> mainButtons;
    std::vector<Button*> gameSettingsButtons;
    std::vector<Button*> playerSettingsButtons;
    std::vector<Button*> aboutButtons;
    std::vector<Button*> exitConfirmButtons;
    std::vector<Button*> editKeysButtons;

    // Input handling
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
    GameMenu() : window(nullptr) {}

    ~GameMenu() {
        for (auto btn : mainButtons) delete btn;
        for (auto btn : gameSettingsButtons) delete btn;
        for (auto btn : playerSettingsButtons) delete btn;
        for (auto btn : aboutButtons) delete btn;
        for (auto btn : exitConfirmButtons) delete btn;
        for (auto btn : editKeysButtons) delete btn;
    }

    bool init() {
        window = new sf::RenderWindow(sf::VideoMode(1000, 900), "Snake Game Menu");
        window->setFramerateLimit(60);

        if (!font.loadFromFile("arial.ttf")) {
            return false;
        }

        // Пытаемся загрузить фоновое изображение
        // Поддерживаемые форматы: PNG, JPG, BMP, TGA, DDS, PSD, HDR
        if (!loadBackground("background.jpg")) {
            // Если не удалось загрузить background.jpg, пробуем другие варианты
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

        return true;
    }

    void run() {
        sf::Clock clock;
        while (window->isOpen()) {
            sf::Event event;
            sf::Vector2f mousePos = window->mapPixelToCoords(sf::Mouse::getPosition(*window));

            while (window->pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window->close();
                    return;
                }

                // Обработка ввода имени
                if (nameInputActive) {
                    handleNameInput(event);
                    // Пропускаем остальную обработку событий при активном вводе имени
                    continue;
                }

                // Обработка ввода клавиш
                if (keyInputActive) {
                    handleKeyInput(event);
                    continue;
                }

                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    handleClick(event.mouseButton.button == sf::Mouse::Left ? mousePos : sf::Vector2f());
                }

                // Начать редактирование клавиш при нажатии любой клавиши на экране EDIT_KEYS
                if (currentScreen == Screen::EDIT_KEYS && event.type == sf::Event::KeyPressed) {
                    if (!keyInputActive) {
                        keyInputActive = true;
                        currentKeyIndex = 0;
                    }
                }
            }

            // Hover effects
            if (!nameInputActive && !keyInputActive) { // Не обновляем ховер эффекты при вводе
                switch (currentScreen) {
                case Screen::MAIN: handleMouseHover(mainButtons, mousePos); break;
                case Screen::GAME_SETTINGS: handleMouseHover(gameSettingsButtons, mousePos); break;
                case Screen::PLAYER_SETTINGS: handleMouseHover(playerSettingsButtons, mousePos); break;
                case Screen::ABOUT: handleMouseHover(aboutButtons, mousePos); break;
                case Screen::EXIT_CONFIRM: handleMouseHover(exitConfirmButtons, mousePos); break;
                case Screen::EDIT_KEYS: handleMouseHover(editKeysButtons, mousePos); break;
                }
            }

            window->clear(bgColor);

            // Рисуем фон (картинку или сплошной цвет)
            if (backgroundLoaded) {
                window->draw(backgroundSprite);

                // Добавляем полупрозрачный черный слой для улучшения читаемости
                sf::RectangleShape overlay;
                overlay.setSize(sf::Vector2f(1000, 900));
                overlay.setFillColor(sf::Color(0, 0, 0, 100)); // Полупрозрачный черный
                window->draw(overlay);
            }

            render();
            window->display();

            if (clock.getElapsedTime().asMilliseconds() < 16) {
                sf::sleep(sf::milliseconds(16) - clock.getElapsedTime());
            }
            clock.restart();
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
                    std::cout << "Game Starting: " << rounds << " rounds, " << bots << " bots" << std::endl;
                    window->close();
                }
                else if (action == "back") {
                    currentScreen = Screen::MAIN;
                    initMainMenu();
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

        // Reset clicked states
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

            for (auto btn : gameSettingsButtons) {
                window->draw(btn->rect);
                window->draw(btn->text);
            }
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
    GameMenu menu;
    if (menu.init()) {
        menu.run();
    }
    return 0;
}