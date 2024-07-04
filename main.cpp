#include "font.h"
#include "icons.h"
#include "maps2km_1.h"
#include "maps2km_2.h"
#include "maps2km_3.h"
#include "maps4km_1.h"
#include "maps4km_2.h"
#include "maps4km_3.h"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <filesystem>
#include <vector>
#include <cmath>
#include <iomanip>
#include <sstream>  
#include <string>

using namespace std;

const int windowWidth = 1150;
const int windowHeight = 950;
const int previewSize = 100;

enum class Language {
    Russian,
    English
};

Language currentLanguage = Language::Russian;

// Функция для обновления текста в зависимости от выбранного языка.
void updateText(Language language, sf::Text& text, const std::wstring& russian, const std::wstring& english) {
    if (language == Language::Russian) {
        text.setString(russian);
    }
    else {
        text.setString(english);
    }
}

// Данные для интерполяции дистанции
const std::vector<float> distances = { 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200, 210, 230, 250, 260, 280, 300, 320, 330, 350, 380, 400, 430, 450, 480, 500, 530, 550, 580, 600, 630, 650, 680, 700, 730, 750, 780, 800, 830, 850, 880, 900, 930, 950, 980, 1000, 1030, 1050, 1080, 1100, 1130, 1150, 1170, 1180, 1190, 1200, 1210, 1220, 1230, 1240, 1250, 1260, 1270, 1280, 1290, 1300, 1310, 1320, 1330, 1340, 1350, 1360, 1370, 1380, 1390, 1400, 1410, 1420, 1430, 1440, 1450, 1460, 1470, 1480, 1490, 1500 };
const std::vector<float> angles = { 1574, 1570, 1567, 1564, 1560, 1557, 1553, 1550, 1546, 1543, 1540, 1536, 1533, 1529, 1522, 1516, 1512, 1505, 1498, 1491, 1488, 1481, 1470, 1463, 1453, 1446, 1435, 1428, 1417, 1410, 1399, 1391, 1380, 1373, 1361, 1354, 1342, 1334, 1322, 1314, 1302, 1294, 1282, 1273, 1260, 1252, 1238, 1229, 1215, 1206, 1192, 1182, 1166, 1156, 1145, 1140, 1134, 1129, 1123, 1117, 1111, 1105, 1099, 1093, 1087, 1080, 1074, 1067, 1060, 1053, 1046, 1038, 1031, 1023, 1014, 1006, 997, 988, 978, 968, 957, 945, 933, 919, 903, 884, 860, 801 };

// Данные для интерполяции наклона
const std::vector<float> alternativeAngles = { 800, 817.4, 835, 852.5, 870, 890, 906.5, 925, 941, 960, 976.5, 995, 1012.5, 1030, 1048, 1065, 1083, 1101.5, 1120, 1136.5, 1155, 1173, 1192, 1208, 1226, 1244, 1262, 1280, 1298, 1315, 1332.5, 1350.5, 1368, 1387, 1404.5, 1422.5, 1440, 1457, 1475, 1492.5, 1510, 1527.5, 1546, 1556, 1565, 1574 };
const std::vector<float> alternativeUnits = { 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87,87.5, 88, 88.3 };

// Функция для линейной интерполяции
float interpolate(float x, const std::vector<float>& xVals, const std::vector<float>& yVals) {
    for (size_t i = 1; i < xVals.size(); ++i) {
        if (x <= xVals[i]) {
            float t = (x - xVals[i - 1]) / (xVals[i] - xVals[i - 1]);
            return yVals[i - 1] + t * (yVals[i] - yVals[i - 1]);
        }
    }
    return yVals.back();
}

// Функция для интерполяции угла
float interpolateAngle(float distance) {
    return interpolate(distance, distances, angles);
}

// Функция для преобразования угла в альт. ед.
float convertAngleToAlternative(float angle) {
    return interpolate(angle, alternativeAngles, alternativeUnits);
}

// Функция для загрузки картинок
std::vector<std::pair<sf::Texture, std::string>> loadTexturesFromBytes(const unsigned char* bytes, std::size_t size, const std::string& name) {
    std::vector<std::pair<sf::Texture, std::string>> textures;
    sf::Texture texture;
    if (texture.loadFromMemory(bytes, size)) {
        textures.emplace_back(texture, name);
    }
    else {
        std::cerr << "Failed to load texture from bytes!" << std::endl;
    }
    return textures;
}

// Функция для вычисления дистанции между двумя точками
float calculateDistance(const sf::Vector2f& point1, const sf::Vector2f& point2, float scale) {
    float dx = (point2.x - point1.x) * scale;
    float dy = (point2.y - point1.y) * scale;
    return std::sqrt(dx * dx + dy * dy);
}

// Функция для вычисления азимута между двумя точками
float calculateAzimuth(const sf::Vector2f& point1, const sf::Vector2f& point2) {
    float angle = std::atan2(point2.x - point1.x, point1.y - point2.y) * 180 / 3.14159;
    if (angle < 0) angle += 360;
    return angle;
}


std::wstring formatDistance(float distance) {
    std::wostringstream distanceStream;
    distanceStream << std::fixed << std::setprecision(0) << distance;
    std::wstring distanceStr = distanceStream.str();
    return distanceStr;
}

std::wstring getAngleText(float distance) {
    float angle = interpolateAngle(distance);
    std::wostringstream angleStream;
    angleStream << std::fixed << std::setprecision(1) << angle;
    return angleStream.str();
}

std::wstring getAlternativeAngleText(float angle) {
    float alternativeAngle = convertAngleToAlternative(angle);
    std::wostringstream alternativeAngleStream;
    alternativeAngleStream << std::fixed << std::setprecision(1) << alternativeAngle;
    return alternativeAngleStream.str();
}


int main() {

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "PRBF2 Mortar Calculator v3");
    window.setFramerateLimit(30);
    std::string titleProgram = "PRBF2 Mortar Calculator v3";

    // Иконка
    sf::Image icon;
    if (!icon.loadFromMemory(ico_h, sizeof(ico_h))) {
        std::cerr << "Failed to load icon!" << std::endl;
        return -1;
    }

    // Установка иконки на окно
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

    // Шрифт
    std::locale::global(std::locale(""));
    sf::Font font;
    if (!font.loadFromMemory(font_h, sizeof(font_h))) {
        std::cerr << "Failed to load font!" << std::endl;
        return -1;
    }


    // Загружаем карты
    auto texturesAlbasrah_2 = loadTexturesFromBytes(Albasrah_2, sizeof(Albasrah_2), "Albasrah 2");
    auto texturesAssault_on_Grozny = loadTexturesFromBytes(Assault_on_Grozny, sizeof(Assault_on_Grozny), "Assault on Grozny");
    auto texturesBattle_of_Ia_Drang = loadTexturesFromBytes(Battle_of_Ia_Drang, sizeof(Battle_of_Ia_Drang), "Battle of Ia Drang");
    auto texturesBeirut = loadTexturesFromBytes(Beirut, sizeof(Beirut), "Beirut");
    auto texturesCharlies_Point = loadTexturesFromBytes(Charlies_Point, sizeof(Charlies_Point), "Charlies Point");
    auto texturesKokan = loadTexturesFromBytes(Kokan, sizeof(Kokan), "Kokan");
    auto texturesKozelsk = loadTexturesFromBytes(Kozelsk, sizeof(Kozelsk), "Kozelsk");
    auto texturesMuttrah_City_2 = loadTexturesFromBytes(Muttrah_City_2, sizeof(Muttrah_City_2), "Muttrah City 2");

    auto texturesNuijamaa = loadTexturesFromBytes(Nuijamaa, sizeof(Nuijamaa), "Nuijamaa");
    auto texturesOmaha_Beach = loadTexturesFromBytes(Omaha_Beach, sizeof(Omaha_Beach), "Omaha Beach");
    auto texturesOp_Barracuda = loadTexturesFromBytes(Op_Barracuda, sizeof(Op_Barracuda), "Op Barracuda");
    auto texturesOperation_Falcon = loadTexturesFromBytes(Operation_Falcon, sizeof(Operation_Falcon), "Operation Falcon");
    auto texturesOperation_Marlin = loadTexturesFromBytes(Operation_Marlin, sizeof(Operation_Marlin), "Operation Marlin");
    auto texturesOutpost = loadTexturesFromBytes(Outpost, sizeof(Outpost), "Outpost");
    auto texturesRoute = loadTexturesFromBytes(Route, sizeof(Route), "Route");
    auto texturesSahel = loadTexturesFromBytes(Sahel, sizeof(Sahel), "Sahel");

    auto texturesSbeneh_Outskirts = loadTexturesFromBytes(Sbeneh_Outskirts, sizeof(Sbeneh_Outskirts), "Sbeneh Outskirts");
    auto texturesShahadah = loadTexturesFromBytes(Shahadah, sizeof(Shahadah), "Shahadah");
    auto texturesUlyanovsk = loadTexturesFromBytes(Ulyanovsk, sizeof(Ulyanovsk), "Ulyanovsk");
    auto texturesZakho = loadTexturesFromBytes(Zakho, sizeof(Zakho), "Zakho");


    auto texturesAdak = loadTexturesFromBytes(Adak, sizeof(Adak), "Adak");
    auto texturesAscheberg = loadTexturesFromBytes(Ascheberg, sizeof(Ascheberg), "Ascheberg");
    auto texturesBamyan = loadTexturesFromBytes(Bamyan, sizeof(Bamyan), "Bamyan");
    auto texturesBlack_Gold = loadTexturesFromBytes(Black_Gold, sizeof(Black_Gold), "Black Gold");
    auto texturesBurning_Sands = loadTexturesFromBytes(Burning_Sands, sizeof(Burning_Sands), "Burning Sands");
    auto texturesHades_Peak = loadTexturesFromBytes(Hades_Peak, sizeof(Hades_Peak), "Hades Peak");
    auto texturesKashan_Desert = loadTexturesFromBytes(Kashan_Desert, sizeof(Kashan_Desert), "Kashan Desert");
    auto texturesKhamisiyah = loadTexturesFromBytes(Khamisiyah, sizeof(Khamisiyah), "Khamisiyah");

    auto texturesMasirah = loadTexturesFromBytes(Masirah, sizeof(Masirah), "Masirah");
    auto texturesOperation_Soul_Rebel = loadTexturesFromBytes(Operation_Soul_Rebel, sizeof(Operation_Soul_Rebel), "Operation Soul Rebel");
    auto texturesOperation_Thunder = loadTexturesFromBytes(Operation_Thunder, sizeof(Operation_Thunder), "Operation Thunder");
    auto texturesPavlovsk_Bay = loadTexturesFromBytes(Pavlovsk_Bay, sizeof(Pavlovsk_Bay), "Pavlovsk Bay");
    auto texturesRoad_to_Damascus = loadTexturesFromBytes(Road_to_Damascus, sizeof(Road_to_Damascus), "Road to Damascus");
    auto texturesSaaremaa = loadTexturesFromBytes(Saaremaa, sizeof(Saaremaa), "Saaremaa");
    auto texturesShijiavalley = loadTexturesFromBytes(Shijiavalley, sizeof(Shijiavalley), "Shijiavalley");
    auto texturesSilent_Eagle = loadTexturesFromBytes(Silent_Eagle, sizeof(Silent_Eagle), "Silent Eagle");

    auto texturesVadso_City = loadTexturesFromBytes(Vadso_City, sizeof(Vadso_City), "Vadso City");
    auto texturesWanda_Shan = loadTexturesFromBytes(Wanda_Shan, sizeof(Wanda_Shan), "Wanda Shan");
    auto texturesXiangshan = loadTexturesFromBytes(Xiangshan, sizeof(Xiangshan), "Xiangshan");
    auto texturesYamalia = loadTexturesFromBytes(Yamalia, sizeof(Yamalia), "Yamalia");

    if (texturesAlbasrah_2.empty() && texturesAssault_on_Grozny.empty()) {
        std::cerr << "No maps loaded. Ensure that the Maps folder contains .png files." << std::endl;
        return -1;
    }



    sf::ConvexShape backArrow;
    backArrow.setPointCount(3);
    backArrow.setPoint(0, sf::Vector2f(20, 0));
    backArrow.setPoint(1, sf::Vector2f(0, 10));
    backArrow.setPoint(2, sf::Vector2f(20, 20));
    backArrow.setFillColor(sf::Color::White);
    backArrow.setPosition(10, 25);

    sf::Text backText(L"Назад", font, 24);
    backText.setFillColor(sf::Color::White);
    backText.setPosition(42.5, 18.3);

    sf::Text backText1(L"Назад", font, 24);
    backText1.setFillColor(sf::Color(255, 255, 255, 0));
    backText1.setPosition(0, 0);
    sf::FloatRect backButtonBounds1 = backText1.getGlobalBounds();
    backButtonBounds1.width += 70;
    backButtonBounds1.height += 32;

    sf::Text distanceTextPreviews(L"Расстояние:", font, 20);
    distanceTextPreviews.setFillColor(sf::Color::White);
    distanceTextPreviews.setPosition(10, windowHeight / 2 - 40);

    sf::Text angleTextPreviews(L"Угол:", font, 20);
    angleTextPreviews.setFillColor(sf::Color::White);
    angleTextPreviews.setPosition(10, windowHeight / 2);

    sf::Text azimuthTextPreviews(L"Азимут:", font, 20);
    azimuthTextPreviews.setFillColor(sf::Color::White);
    azimuthTextPreviews.setPosition(10, windowHeight / 2 + 40);

    sf::Text fallTime(L"Время прилёта: 19-21с", font, 17);
    fallTime.setFillColor(sf::Color::White);
    fallTime.setPosition(10, windowHeight - 125);

    sf::Text lmbText(L"ЛКМ - Миномет", font, 17);
    lmbText.setFillColor(sf::Color::White);
    lmbText.setPosition(10, windowHeight - 75);

    sf::Text rmbText(L"ПКМ - Цель", font, 17);
    rmbText.setFillColor(sf::Color::White);
    rmbText.setPosition(10, windowHeight - 50);

    sf::Text contactText(L"Желаете добавить карту или дать совет?\n                 Telegram: @binoopstg", font, 15);
    contactText.setFillColor(sf::Color::White);
    contactText.setPosition(687, windowHeight - 40);

    sf::Text versionText(L"Version: 3", font, 15);
    versionText.setFillColor(sf::Color::White);
    versionText.setPosition(1070, windowHeight - 25);

    sf::Text languageButton(L"EN", font, 24);
    languageButton.setFillColor(sf::Color::White);
    languageButton.setPosition(windowWidth - 100, 10);
    sf::FloatRect languageButtonBounds = languageButton.getGlobalBounds();
    languageButtonBounds.width += 20;
    languageButtonBounds.height += 20;

    // Векторы для хранения спрайтов карт
    std::vector<sf::Sprite> previewsAlbasrah_2, previewsAssault_on_Grozny, previewsBattle_of_Ia_Drang, previewsBeirut, previewsCharlies_Point, previewsKokan, previewsKozelsk, previewsMuttrah_City_2, previewsNuijamaa, previewsOmaha_Beach, previewsOp_Barracuda, previewsOperation_Falcon, previewsOperation_Marlin, previewsOutpost, previewsRoute, previewsSahel, previewsSbeneh_Outskirts, previewsShahadah, previewsUlyanovsk, previewsZakho,
        previewsAdak, previewsAscheberg, previewsBamyan, previewsBlack_Gold, previewsBurning_Sands, previewsHades_Peak, previewsKashan_Desert, previewsKhamisiyah, previewsMasirah, previewsOperation_Soul_Rebel, previewsOperation_Thunder, previewsPavlovsk_Bay, previewsRoad_to_Damascus, previewsSaaremaa, previewsShijiavalley, previewsSilent_Eagle, previewsVadso_City, previewsWanda_Shan, previewsXiangshan, previewsYamalia;
    for (const auto& [texture, filename] : texturesAlbasrah_2) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsAlbasrah_2.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesAssault_on_Grozny) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsAssault_on_Grozny.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesBattle_of_Ia_Drang) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsBattle_of_Ia_Drang.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesBeirut) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsBeirut.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesCharlies_Point) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsCharlies_Point.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesKokan) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsKokan.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesKozelsk) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsKozelsk.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesMuttrah_City_2) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsMuttrah_City_2.push_back(sprite);
    } // 1 stroka end   

    for (const auto& [texture, filename] : texturesNuijamaa) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsNuijamaa.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesOmaha_Beach) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsOmaha_Beach.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesOp_Barracuda) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsOp_Barracuda.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesOperation_Falcon) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsOperation_Falcon.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesOperation_Marlin) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsOperation_Marlin.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesOutpost) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsOutpost.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesRoute) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsRoute.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesSahel) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsSahel.push_back(sprite);
    } // 2 stroka end

    for (const auto& [texture, filename] : texturesSbeneh_Outskirts) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsSbeneh_Outskirts.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesShahadah) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsShahadah.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesUlyanovsk) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsUlyanovsk.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesZakho) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsZakho.push_back(sprite);
    } // 3 stroka end



    for (const auto& [texture, filename] : texturesAdak) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsAdak.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesAscheberg) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsAscheberg.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesBamyan) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsBamyan.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesBlack_Gold) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsBlack_Gold.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesBurning_Sands) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsBurning_Sands.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesHades_Peak) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsHades_Peak.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesKashan_Desert) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsKashan_Desert.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesKhamisiyah) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsKhamisiyah.push_back(sprite);
    } // 4 line end

    for (const auto& [texture, filename] : texturesMasirah) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsMasirah.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesOperation_Soul_Rebel) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsOperation_Soul_Rebel.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesOperation_Thunder) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsOperation_Thunder.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesPavlovsk_Bay) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsPavlovsk_Bay.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesRoad_to_Damascus) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsRoad_to_Damascus.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesSaaremaa) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsSaaremaa.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesShijiavalley) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsShijiavalley.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesSilent_Eagle) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsSilent_Eagle.push_back(sprite);
    } // 5 line end

    for (const auto& [texture, filename] : texturesVadso_City) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsVadso_City.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesWanda_Shan) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsWanda_Shan.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesXiangshan) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsXiangshan.push_back(sprite);
    }
    for (const auto& [texture, filename] : texturesYamalia) {
        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setScale(
            static_cast<float>(previewSize) / texture.getSize().x,
            static_cast<float>(previewSize) / texture.getSize().y
        );
        previewsYamalia.push_back(sprite);
    } // 6 line end


    // Загружаем иконки миномета и цели
    sf::Texture mortarTexture, targetTexture;
    if (!mortarTexture.loadFromMemory(mortar_green, sizeof(mortar_green)) || !targetTexture.loadFromMemory(target_red, sizeof(target_red))) {
        std::cerr << "Failed to load icons!" << std::endl;
        return -1;
    }
    sf::Sprite mortarSprite(mortarTexture);
    sf::Sprite targetSprite(targetTexture);


    sf::Vector2f mortarPos, targetPos;
    bool mortarSet = false, targetSet = false;
    bool inCalculator = false;
    sf::Texture selectedMapTexture;
    sf::Sprite selectedMapSprite;
    float mapScale = 1.0f;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            // Событие зум
            else if (event.type == sf::Event::MouseWheelScrolled) {
                // Получаем координаты курсора
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

                // Проверяем, находится ли курсор в разрешенной области
                if (mousePos.x >= 225 && mousePos.x <= 1125 && mousePos.y >= 25 && mousePos.y <= 925) {
                    // Кратность зума (3 состояния)
                    float scaleFactor = (event.mouseWheelScroll.delta > 0) ? 1.8f : 0.5555555555555556f;

                    // Получаем текущий масштаб карты
                    sf::Vector2f currentScale = selectedMapSprite.getScale();
                    sf::Vector2f newScale = currentScale * scaleFactor;

                    if ((scaleFactor > 1.0f && newScale.x <= 3.24f) || (scaleFactor < 1.0f && newScale.x >= 1.0f)) {
                        selectedMapSprite.setScale(newScale);

                        mapScale /= scaleFactor;
                        sf::Vector2f mapPosition = selectedMapSprite.getPosition();

                        sf::Vector2f newMapPosition = mousePos - scaleFactor * (mousePos - mapPosition);

                        sf::FloatRect mapBounds = selectedMapSprite.getGlobalBounds();

                        if (newMapPosition.x > 225) {
                            newMapPosition.x = 225;
                        }
                        if (newMapPosition.y > 25) {
                            newMapPosition.y = 25;
                        }
                        if (newMapPosition.x + mapBounds.width < 1125) {
                            newMapPosition.x = 1125 - mapBounds.width;
                        }
                        if (newMapPosition.y + mapBounds.height < 925) {
                            newMapPosition.y = 925 - mapBounds.height;
                        }

                        selectedMapSprite.setPosition(newMapPosition);


                        if (mortarSet) {
                            mortarPos = sf::Vector2f((mortarPos.x - mapPosition.x) * scaleFactor + newMapPosition.x, (mortarPos.y - mapPosition.y) * scaleFactor + newMapPosition.y);
                        }
                        if (targetSet) {
                            targetPos = sf::Vector2f((targetPos.x - mapPosition.x) * scaleFactor + newMapPosition.x, (targetPos.y - mapPosition.y) * scaleFactor + newMapPosition.y);
                        }
                    }

                    if (scaleFactor < 1.0f) {
                        sf::Vector2f finalScale = selectedMapSprite.getScale();

                        if (finalScale.x < 1.0f) {
                            finalScale.x = 1.0f;
                            finalScale.y = 1.0f;
                            selectedMapSprite.setScale(finalScale);
                            selectedMapSprite.setPosition(225, 25);
                        }
                    }
                }
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                if (inCalculator) {
                    if (backButtonBounds1.contains(static_cast<sf::Vector2f>(sf::Mouse::getPosition(window)))) {
                        window.setTitle(titleProgram);
                        inCalculator = false;
                        mortarSet = false;
                        targetSet = false;
                    }
                    else if (mousePos.x > 225 && mousePos.x < 1125 && mousePos.y > 25 && mousePos.y < 925) { // Запрещаем устанавливать миномет и цель в области HUD
                        if (event.mouseButton.button == sf::Mouse::Left) {
                            mortarPos = mousePos;
                            mortarSet = true;
                        }
                        else if (event.mouseButton.button == sf::Mouse::Right) {
                            targetPos = mousePos;
                            targetSet = true;
                        }
                    }
                }
                else {
                    bool mapSelected = false;
                    for (size_t i = 0; i < previewsAlbasrah_2.size(); ++i) {
                        if (previewsAlbasrah_2[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesAlbasrah_2[i].first;
                            mapScale = 2.2752f;
                            window.setTitle(titleProgram + " | Albasrah 2");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsAssault_on_Grozny.size(); ++i) {
                        if (previewsAssault_on_Grozny[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesAssault_on_Grozny[i].first;
                            mapScale = 2.2752f;
                            window.setTitle(titleProgram + " | Assault on Grozny");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsBattle_of_Ia_Drang.size(); ++i) {
                        if (previewsBattle_of_Ia_Drang[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesBattle_of_Ia_Drang[i].first;
                            mapScale = 2.2752f;
                            window.setTitle(titleProgram + " | Battle of Ia Drang");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsBeirut.size(); ++i) {
                        if (previewsBeirut[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesBeirut[i].first;
                            mapScale = 2.2752f;
                            window.setTitle(titleProgram + " | Beirut");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsCharlies_Point.size(); ++i) {
                        if (previewsCharlies_Point[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesCharlies_Point[i].first;
                            mapScale = 2.2752f;
                            window.setTitle(titleProgram + " | Charlies Point");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsKokan.size(); ++i) {
                        if (previewsKokan[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesKokan[i].first;
                            mapScale = 2.2752f;
                            window.setTitle(titleProgram + " | Kokan");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsKozelsk.size(); ++i) {
                        if (previewsKozelsk[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesKozelsk[i].first;
                            mapScale = 2.2752f;
                            window.setTitle(titleProgram + " | Kozelsk");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsMuttrah_City_2.size(); ++i) {
                        if (previewsMuttrah_City_2[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesMuttrah_City_2[i].first;
                            mapScale = 2.2752f;
                            window.setTitle(titleProgram + " | Muttrah City 2");
                            mapSelected = true;
                            break;
                        }
                    } // 1 stroka end

                    for (size_t i = 0; i < previewsNuijamaa.size(); ++i) {
                        if (previewsNuijamaa[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesNuijamaa[i].first;
                            mapScale = 2.2752f; 
                            window.setTitle(titleProgram + " | Nuijamaa");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsOmaha_Beach.size(); ++i) {
                        if (previewsOmaha_Beach[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesOmaha_Beach[i].first;
                            mapScale = 2.2752f; 
                            window.setTitle(titleProgram + " | Omaha Beach");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsOp_Barracuda.size(); ++i) {
                        if (previewsOp_Barracuda[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesOp_Barracuda[i].first;
                            mapScale = 2.2752f; 
                            window.setTitle(titleProgram + " | Op Barracuda");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsOperation_Falcon.size(); ++i) {
                        if (previewsOperation_Falcon[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesOperation_Falcon[i].first;
                            mapScale = 2.2752f; 
                            window.setTitle(titleProgram + " | Operation Falcon");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsOperation_Marlin.size(); ++i) {
                        if (previewsOperation_Marlin[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesOperation_Marlin[i].first;
                            mapScale = 2.2752f; 
                            window.setTitle(titleProgram + " | Operation Marlin");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsOutpost.size(); ++i) {
                        if (previewsOutpost[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesOutpost[i].first;
                            mapScale = 2.2752f; 
                            window.setTitle(titleProgram + " | Outpost");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsRoute.size(); ++i) {
                        if (previewsRoute[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesRoute[i].first;
                            mapScale = 2.2752f; 
                            window.setTitle(titleProgram + " | Route");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsSahel.size(); ++i) {
                        if (previewsSahel[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesSahel[i].first;
                            mapScale = 2.2752f; 
                            window.setTitle(titleProgram + " | Sahel");
                            mapSelected = true;
                            break;
                        }
                    } // 2 stroka end

                    for (size_t i = 0; i < previewsSbeneh_Outskirts.size(); ++i) {
                        if (previewsSbeneh_Outskirts[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesSbeneh_Outskirts[i].first;
                            mapScale = 2.2752f; 
                            window.setTitle(titleProgram + " | Sbeneh Outskirts");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsShahadah.size(); ++i) {
                        if (previewsShahadah[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesShahadah[i].first;
                            mapScale = 2.2752f; 
                            window.setTitle(titleProgram + " | Shahadah");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsUlyanovsk.size(); ++i) {
                        if (previewsUlyanovsk[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesUlyanovsk[i].first;
                            mapScale = 2.2752f; 
                            window.setTitle(titleProgram + " | Ulyanovsk");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsZakho.size(); ++i) {
                        if (previewsZakho[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesZakho[i].first;
                            mapScale = 2.2752f;
                            window.setTitle(titleProgram + " | Zakho");
                            mapSelected = true;
                            break;
                        }
                    } // 3 stroka end



                    for (size_t i = 0; i < previewsAdak.size(); ++i) {
                        if (previewsAdak[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesAdak[i].first;
                            mapScale = 4.5504f; 
                            window.setTitle(titleProgram + " | Adak");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsAscheberg.size(); ++i) {
                        if (previewsAscheberg[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesAscheberg[i].first;
                            mapScale = 4.5504f; 
                            window.setTitle(titleProgram + " | Ascheberg");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsBamyan.size(); ++i) {
                        if (previewsBamyan[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesBamyan[i].first;
                            mapScale = 4.5504f; 
                            window.setTitle(titleProgram + " | Bamyan");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsBlack_Gold.size(); ++i) {
                        if (previewsBlack_Gold[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesBlack_Gold[i].first;
                            mapScale = 4.5504f; 
                            window.setTitle(titleProgram + " | Black Gold");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsBurning_Sands.size(); ++i) {
                        if (previewsBurning_Sands[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesBurning_Sands[i].first;
                            mapScale = 4.5504f; 
                            window.setTitle(titleProgram + " | Burning Sands");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsHades_Peak.size(); ++i) {
                        if (previewsHades_Peak[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesHades_Peak[i].first;
                            mapScale = 4.5504f; 
                            window.setTitle(titleProgram + " | Hades Peak");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsKashan_Desert.size(); ++i) {
                        if (previewsKashan_Desert[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesKashan_Desert[i].first;
                            mapScale = 4.5504f; 
                            window.setTitle(titleProgram + " | Kashan Desert");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsKhamisiyah.size(); ++i) {
                        if (previewsKhamisiyah[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesKhamisiyah[i].first;
                            mapScale = 4.5504f; 
                            window.setTitle(titleProgram + " | Khamisiyah");
                            mapSelected = true;
                            break;
                        }
                    } // 4 line end


                    for (size_t i = 0; i < previewsMasirah.size(); ++i) {
                        if (previewsMasirah[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesMasirah[i].first;
                            mapScale = 4.5504f; 
                            window.setTitle(titleProgram + " | Masirah");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsOperation_Soul_Rebel.size(); ++i) {
                        if (previewsOperation_Soul_Rebel[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesOperation_Soul_Rebel[i].first;
                            mapScale = 4.5504f; 
                            window.setTitle(titleProgram + " | Operation Soul Rebel");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsOperation_Thunder.size(); ++i) {
                        if (previewsOperation_Thunder[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesOperation_Thunder[i].first;
                            mapScale = 4.5504f; 
                            window.setTitle(titleProgram + " | Operation Thunder");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsPavlovsk_Bay.size(); ++i) {
                        if (previewsPavlovsk_Bay[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesPavlovsk_Bay[i].first;
                            mapScale = 4.5504f; 
                            window.setTitle(titleProgram + " | Pavlovsk Bay");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsRoad_to_Damascus.size(); ++i) {
                        if (previewsRoad_to_Damascus[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesRoad_to_Damascus[i].first;
                            mapScale = 4.5504f; 
                            window.setTitle(titleProgram + " | Road to Damascus");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsSaaremaa.size(); ++i) {
                        if (previewsSaaremaa[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesSaaremaa[i].first;
                            mapScale = 4.5504f; 
                            window.setTitle(titleProgram + " | Saaremaa");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsShijiavalley.size(); ++i) {
                        if (previewsShijiavalley[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesShijiavalley[i].first;
                            mapScale = 4.5504f; 
                            window.setTitle(titleProgram + " | Shijiavalley");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsSilent_Eagle.size(); ++i) {
                        if (previewsSilent_Eagle[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesSilent_Eagle[i].first;
                            mapScale = 4.5504f; 
                            window.setTitle(titleProgram + " | Silent Eagle");
                            mapSelected = true;
                            break;
                        }
                    } // 5 line end


                    for (size_t i = 0; i < previewsVadso_City.size(); ++i) {
                        if (previewsVadso_City[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesVadso_City[i].first;
                            mapScale = 4.5504f; 
                            window.setTitle(titleProgram + " | Vadso City");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsWanda_Shan.size(); ++i) {
                        if (previewsWanda_Shan[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesWanda_Shan[i].first;
                            mapScale = 4.5504f; 
                            window.setTitle(titleProgram + " | Wanda Shan");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsXiangshan.size(); ++i) {
                        if (previewsXiangshan[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesXiangshan[i].first;
                            mapScale = 4.5504f;
                            window.setTitle(titleProgram + " | Xiangshan");
                            mapSelected = true;
                            break;
                        }
                    }
                    for (size_t i = 0; i < previewsYamalia.size(); ++i) {
                        if (previewsYamalia[i].getGlobalBounds().contains(mousePos)) {
                            selectedMapTexture = texturesYamalia[i].first;
                            mapScale = 4.5504f;
                            window.setTitle(titleProgram + " | Yamalia");
                            mapSelected = true;
                            break;
                        }
                    }
                    if (mapSelected) {
                        selectedMapSprite.setTexture(selectedMapTexture);
                        selectedMapSprite.setPosition(225, 25);
                        inCalculator = true;
                    }
                    if (languageButtonBounds.contains(event.mouseButton.x, event.mouseButton.y)) {
                        if (currentLanguage == Language::Russian) {
                            currentLanguage = Language::English;
                            updateText(currentLanguage, backText, L"Назад", L"Back");
                            updateText(currentLanguage, distanceTextPreviews, L"Расстояние:", L"Distance:");
                            updateText(currentLanguage, angleTextPreviews, L"Угол:", L"Angle:");
                            updateText(currentLanguage, azimuthTextPreviews, L"Азимут:", L"Azimuth:");
                            updateText(currentLanguage, fallTime, L"Время прилёта: 19-21с", L"Fall time: 19-21s");
                            updateText(currentLanguage, lmbText, L"ЛКМ - Миномет", L"LMB - Mortar");
                            updateText(currentLanguage, rmbText, L"ПКМ - Цель", L"RMB - Target");
                            updateText(currentLanguage, contactText, L"Желаете добавить карту или дать совет?\n                 Telegram: @binoopstg", L"Want to add a map or give advice?\n              Telegram: @binoopstg");
                            updateText(currentLanguage, versionText, L"Version: 3", L"Version: 3");
                            languageButton.setString("RU");
                        }
                        else {
                            currentLanguage = Language::Russian;
                            updateText(currentLanguage, backText, L"Назад", L"Back");
                            updateText(currentLanguage, distanceTextPreviews, L"Расстояние:", L"Distance:");
                            updateText(currentLanguage, angleTextPreviews, L"Угол:", L"Angle:");
                            updateText(currentLanguage, azimuthTextPreviews, L"Азимут:", L"Azimuth:");
                            updateText(currentLanguage, fallTime, L"Время прилёта: 19-21с", L"Fall time: 19-21s");
                            updateText(currentLanguage, lmbText, L"ЛКМ - Миномет", L"LMB - Mortar");
                            updateText(currentLanguage, rmbText, L"ПКМ - Цель", L"RMB - Target");
                            updateText(currentLanguage, contactText, L"Желаете добавить карту или дать совет?\n                 Telegram: @binoopstg", L"Want to add a map or give advice?\n              Telegram: @binoopstg");
                            updateText(currentLanguage, versionText, L"Version: 3", L"Version: 3");
                            languageButton.setString("EN");
                        }
                    }
                }
            }
        }

        window.clear(sf::Color(50, 50, 50));

        if (inCalculator) {
            window.draw(selectedMapSprite);

            // HUD
            sf::RectangleShape hud(sf::Vector2f(200, windowHeight));
            hud.setFillColor(sf::Color(50, 50, 50));
            hud.setPosition(0, 0);
            window.draw(hud);

            sf::RectangleShape rectangleLeft(sf::Vector2f(25, 950));
            rectangleLeft.setPosition(200, 0);
            rectangleLeft.setFillColor(sf::Color(50, 50, 50));
            window.draw(rectangleLeft);

            sf::RectangleShape rectangleTop(sf::Vector2f(925, 25));
            rectangleTop.setPosition(225, 0);
            rectangleTop.setFillColor(sf::Color(50, 50, 50));
            window.draw(rectangleTop);

            sf::RectangleShape rectangleRight(sf::Vector2f(25, 950));
            rectangleRight.setPosition(1125, 0);
            rectangleRight.setFillColor(sf::Color(50, 50, 50));
            window.draw(rectangleRight);

            sf::RectangleShape rectangleBottom(sf::Vector2f(925, 25));
            rectangleBottom.setPosition(225, 925);
            rectangleBottom.setFillColor(sf::Color(50, 50, 50));
            window.draw(rectangleBottom);

            window.draw(backArrow);
            window.draw(backText);
            window.draw(backText1);

            window.draw(fallTime);
            window.draw(lmbText);
            window.draw(rmbText);

            window.draw(distanceTextPreviews);
            window.draw(angleTextPreviews);
            window.draw(azimuthTextPreviews);

            // Отображаем маркеры и линии
            if (mortarSet) {
                mortarSprite.setPosition(mortarPos.x - mortarTexture.getSize().x / 2, mortarPos.y - mortarTexture.getSize().y / 2);
                window.draw(mortarSprite);
            }
            if (targetSet) {
                targetSprite.setPosition(targetPos.x - targetTexture.getSize().x / 2, targetPos.y - targetTexture.getSize().y / 2);
                window.draw(targetSprite);
            }
            if (mortarSet && targetSet) {
                sf::VertexArray line(sf::Quads, 4);

                sf::Vector2f direction = targetPos - mortarPos;
                sf::Vector2f unitDirection = direction / std::sqrt(direction.x * direction.x + direction.y * direction.y);
                sf::Vector2f perpendicular(-unitDirection.y, unitDirection.x);

                float thickness = 2.f;  // Уменьшил толщину линии
                sf::Vector2f offset = (thickness / 2.f) * perpendicular;

                line[0].position = mortarPos + offset;
                line[1].position = targetPos + offset;
                line[2].position = targetPos - offset;
                line[3].position = mortarPos - offset;

                for (int i = 0; i < 4; ++i)
                    line[i].color = sf::Color(64, 129, 255);

                window.draw(line);

                float distance = calculateDistance(mortarPos, targetPos, mapScale);
                float angle = interpolateAngle(distance);
                float alternativeAngle = convertAngleToAlternative(angle);
                float azimuth = calculateAzimuth(mortarPos, targetPos);

                std::wostringstream distanceStream;
                distanceStream << std::fixed << std::setprecision(0) << distance;
                sf::Text distanceText(currentLanguage == Language::Russian ?
                    L"Расстояние: " + distanceStream.str() + L"м" :
                    L"Distance: " + distanceStream.str() + L"m", font, 20);
                distanceText.setFillColor(sf::Color::White);
                distanceText.setPosition(10, windowHeight / 2 - 40);
                window.draw(distanceText);

                std::wostringstream angleStream, alternativeAngleStream;
                angleStream << std::fixed << std::setprecision(0) << angle << L" (" << std::setprecision(1) << alternativeAngle << L"\272)";
                sf::Text angleText;
                if (distance < 79.999999) {
                    sf::Text angleText(currentLanguage == Language::Russian ? L"Угол: Близко" : L"Angle: Close", font, 20);
                    angleText.setFillColor(sf::Color::White);
                    angleText.setPosition(10, windowHeight / 2);
                    window.draw(angleText);
                }
                else if (distance > 1500.999999) {
                    sf::Text angleText(currentLanguage == Language::Russian ? L"Угол: Далеко" : L"Angle: Far Away", font, 20);
                    angleText.setFillColor(sf::Color::White);
                    angleText.setPosition(10, windowHeight / 2);
                    window.draw(angleText);
                }
                else {
                    sf::Text angleText(currentLanguage == Language::Russian ? L"Угол: " + angleStream.str() : L"Angle: " + angleStream.str(), font, 20);
                    angleText.setFillColor(sf::Color::White);
                    angleText.setPosition(10, windowHeight / 2);
                    window.draw(angleText);
                }

                std::wostringstream azimuthStream;
                azimuthStream << std::fixed << std::setprecision(1) << azimuth;
                sf::Text azimuthText(currentLanguage == Language::Russian ?
                    L"Азимут: " + azimuthStream.str() + L"\272" :
                    L"Azimuth: " + azimuthStream.str() + L"\272", font, 20);
                azimuthText.setFillColor(sf::Color::White);
                azimuthText.setPosition(10, windowHeight / 2 + 40);
                window.draw(azimuthText);
            }
        }
        else {
            sf::Text header2km(currentLanguage == Language::Russian ?
                L"Карты 2км (квадрат 150м)" :
                L"2km Maps (150m square)", font, 28);
            header2km.setFillColor(sf::Color::White);
            header2km.setPosition(402, 12.5);
            window.draw(header2km);

            sf::Text header4km(currentLanguage == Language::Russian ?
                L"Карты 4км (квадрат 300м)" :
                L"4km Maps (300m square)", font, 28);
            header4km.setFillColor(sf::Color::White);
            header4km.setPosition(402, windowHeight / 2 + 12.5);
            window.draw(header4km);

            // 1 строка
            for (size_t i = 0; i < previewsAlbasrah_2.size(); ++i) {
                previewsAlbasrah_2[i].setPosition(38.9 + (i % 10) * (previewSize + 10), 65 + (i / 10) * (previewSize + 10));
                window.draw(previewsAlbasrah_2[i]);

                std::string mapName = texturesAlbasrah_2[i].second.substr(0, texturesAlbasrah_2[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(38.9 + (i % 10) * (previewSize + 10), 65 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsAssault_on_Grozny.size(); ++i) {
                previewsAssault_on_Grozny[i].setPosition(138.9 + 38.9 + (i % 10) * (previewSize + 10), 65 + (i / 10) * (previewSize + 10));
                window.draw(previewsAssault_on_Grozny[i]);

                std::string mapName = texturesAssault_on_Grozny[i].second.substr(0, texturesAssault_on_Grozny[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(138.9 + 38.9 + (i % 10) * (previewSize + 10), 65 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsBattle_of_Ia_Drang.size(); ++i) {
                previewsBattle_of_Ia_Drang[i].setPosition(277.8 + 38.9 + (i % 10) * (previewSize + 10), 65 + (i / 10) * (previewSize + 10));
                window.draw(previewsBattle_of_Ia_Drang[i]);

                std::string mapName = texturesBattle_of_Ia_Drang[i].second.substr(0, texturesBattle_of_Ia_Drang[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(277.8 + 38.9 + (i % 10) * (previewSize + 10), 65 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsBeirut.size(); ++i) {
                previewsBeirut[i].setPosition(416.7 + 38.9 + (i % 10) * (previewSize + 10), 65 + (i / 10) * (previewSize + 10));
                window.draw(previewsBeirut[i]);

                std::string mapName = texturesBeirut[i].second.substr(0, texturesBeirut[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(416.7 + 38.9 + (i % 10) * (previewSize + 10), 65 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsCharlies_Point.size(); ++i) {
                previewsCharlies_Point[i].setPosition(555.6 + 38.9 + (i % 10) * (previewSize + 10), 65 + (i / 10) * (previewSize + 10));
                window.draw(previewsCharlies_Point[i]);

                std::string mapName = texturesCharlies_Point[i].second.substr(0, texturesCharlies_Point[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(555.6 + 38.9 + (i % 10) * (previewSize + 10), 65 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsKokan.size(); ++i) {
                previewsKokan[i].setPosition(694.5 + 38.9 + (i % 10) * (previewSize + 10), 65 + (i / 10) * (previewSize + 10));
                window.draw(previewsKokan[i]);

                std::string mapName = texturesKokan[i].second.substr(0, texturesKokan[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(694.5 + 38.9 + (i % 10) * (previewSize + 10), 65 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsKozelsk.size(); ++i) {
                previewsKozelsk[i].setPosition(833.4 + 38.9 + (i % 10) * (previewSize + 10), 65 + (i / 10) * (previewSize + 10));
                window.draw(previewsKozelsk[i]);

                std::string mapName = texturesKozelsk[i].second.substr(0, texturesKozelsk[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(833.4 + 38.9 + (i % 10) * (previewSize + 10), 65 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsMuttrah_City_2.size(); ++i) {
                previewsMuttrah_City_2[i].setPosition(972.3 + 38.9 + (i % 10) * (previewSize + 10), 65 + (i / 10) * (previewSize + 10));
                window.draw(previewsMuttrah_City_2[i]);

                std::string mapName = texturesMuttrah_City_2[i].second.substr(0, texturesMuttrah_City_2[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(972.3 + 38.9 + (i % 10) * (previewSize + 10), 65 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            } // 1 stroka end

            for (size_t i = 0; i < previewsNuijamaa.size(); ++i) {
                previewsNuijamaa[i].setPosition(38.9 + (i % 10) * (previewSize + 10), 205 + (i / 10) * (previewSize + 10));
                window.draw(previewsNuijamaa[i]);

                std::string mapName = texturesNuijamaa[i].second.substr(0, texturesNuijamaa[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(38.9 + (i % 10) * (previewSize + 10), 205 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsOmaha_Beach.size(); ++i) {
                previewsOmaha_Beach[i].setPosition(138.9 + 38.9 + (i % 10) * (previewSize + 10), 205 + (i / 10) * (previewSize + 10));
                window.draw(previewsOmaha_Beach[i]);

                std::string mapName = texturesOmaha_Beach[i].second.substr(0, texturesOmaha_Beach[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(138.9 + 38.9 + (i % 10) * (previewSize + 10), 205 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsOp_Barracuda.size(); ++i) {
                previewsOp_Barracuda[i].setPosition(277.8 + 38.9 + (i % 10) * (previewSize + 10), 205 + (i / 10) * (previewSize + 10));
                window.draw(previewsOp_Barracuda[i]);

                std::string mapName = texturesOp_Barracuda[i].second.substr(0, texturesOp_Barracuda[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(277.8 + 38.9 + (i % 10) * (previewSize + 10), 205 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsOperation_Falcon.size(); ++i) {
                previewsOperation_Falcon[i].setPosition(416.7 + 38.9 + (i % 10) * (previewSize + 10), 205 + (i / 10) * (previewSize + 10));
                window.draw(previewsOperation_Falcon[i]);

                std::string mapName = texturesOperation_Falcon[i].second.substr(0, texturesOperation_Falcon[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(416.7 + 38.9 + (i % 10) * (previewSize + 10), 205 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsOperation_Marlin.size(); ++i) {
                previewsOperation_Marlin[i].setPosition(555.6 + 38.9 + (i % 10) * (previewSize + 10), 205 + (i / 10) * (previewSize + 10));
                window.draw(previewsOperation_Marlin[i]);

                std::string mapName = texturesOperation_Marlin[i].second.substr(0, texturesOperation_Marlin[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(555.6 + 38.9 + (i % 10) * (previewSize + 10), 205 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsOutpost.size(); ++i) {
                previewsOutpost[i].setPosition(694.5 + 38.9 + (i % 10) * (previewSize + 10), 205 + (i / 10) * (previewSize + 10));
                window.draw(previewsOutpost[i]);

                std::string mapName = texturesOutpost[i].second.substr(0, texturesOutpost[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(694.5 + 38.9 + (i % 10) * (previewSize + 10), 205 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsRoute.size(); ++i) {
                previewsRoute[i].setPosition(833.4 + 38.9 + (i % 10) * (previewSize + 10), 205 + (i / 10) * (previewSize + 10));
                window.draw(previewsRoute[i]);

                std::string mapName = texturesRoute[i].second.substr(0, texturesRoute[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(833.4 + 38.9 + (i % 10) * (previewSize + 10), 205 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsSahel.size(); ++i) {
                previewsSahel[i].setPosition(972.3 + 38.9 + 3.125 + (i % 10) * (previewSize + 10), 205 + (i / 10) * (previewSize + 10));
                window.draw(previewsSahel[i]);

                std::string mapName = texturesSahel[i].second.substr(0, texturesSahel[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(972.3 + 38.9 + 3.125 + (i % 10) * (previewSize + 10), 205 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            } // 2 stroka end

            for (size_t i = 0; i < previewsSbeneh_Outskirts.size(); ++i) {
                previewsSbeneh_Outskirts[i].setPosition(38.9 + (i % 10) * (previewSize + 10), 345 + (i / 10) * (previewSize + 10));
                window.draw(previewsSbeneh_Outskirts[i]);

                std::string mapName = texturesSbeneh_Outskirts[i].second.substr(0, texturesSbeneh_Outskirts[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(38.9 + (i % 10) * (previewSize + 10), 345 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsShahadah.size(); ++i) {
                previewsShahadah[i].setPosition(138.9 + 38.9 + (i % 10) * (previewSize + 10), 345 + (i / 10) * (previewSize + 10));
                window.draw(previewsShahadah[i]);

                std::string mapName = texturesShahadah[i].second.substr(0, texturesShahadah[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(138.9 + 38.9 + (i % 10) * (previewSize + 10), 345 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsUlyanovsk.size(); ++i) {
                previewsUlyanovsk[i].setPosition(277.8 + 38.9 + (i % 10) * (previewSize + 10), 345 + (i / 10) * (previewSize + 10));
                window.draw(previewsUlyanovsk[i]);

                std::string mapName = texturesUlyanovsk[i].second.substr(0, texturesUlyanovsk[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(277.8 + 38.9 + (i % 10) * (previewSize + 10), 345 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsZakho.size(); ++i) {
                previewsZakho[i].setPosition(416.7 + 38.9 + (i % 10) * (previewSize + 10), 345 + (i / 10) * (previewSize + 10));
                window.draw(previewsZakho[i]);

                std::string mapName = texturesZakho[i].second.substr(0, texturesZakho[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(416.7 + 38.9 + (i % 10) * (previewSize + 10), 345 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            } // 3 stroka end



            for (size_t i = 0; i < previewsAdak.size(); ++i) {
                previewsAdak[i].setPosition(38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 65 + (i / 10) * (previewSize + 10));
                window.draw(previewsAdak[i]);

                std::string mapName = texturesAdak[i].second.substr(0, texturesAdak[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 65 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsAscheberg.size(); ++i) {
                previewsAscheberg[i].setPosition(138.9 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 65 + (i / 10) * (previewSize + 10));
                window.draw(previewsAscheberg[i]);

                std::string mapName = texturesAscheberg[i].second.substr(0, texturesAscheberg[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(138.9 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 65 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsBamyan.size(); ++i) {
                previewsBamyan[i].setPosition(277.8 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 65 + (i / 10) * (previewSize + 10));
                window.draw(previewsBamyan[i]);

                std::string mapName = texturesBamyan[i].second.substr(0, texturesBamyan[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(277.8 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 65 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsBlack_Gold.size(); ++i) {
                previewsBlack_Gold[i].setPosition(416.7 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 65 + (i / 10) * (previewSize + 10));
                window.draw(previewsBlack_Gold[i]);

                std::string mapName = texturesBlack_Gold[i].second.substr(0, texturesBlack_Gold[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(416.7 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 65 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsBurning_Sands.size(); ++i) {
                previewsBurning_Sands[i].setPosition(555.6 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 65 + (i / 10) * (previewSize + 10));
                window.draw(previewsBurning_Sands[i]);

                std::string mapName = texturesBurning_Sands[i].second.substr(0, texturesBurning_Sands[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(555.6 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 65 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsHades_Peak.size(); ++i) {
                previewsHades_Peak[i].setPosition(694.5 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 65 + (i / 10) * (previewSize + 10));
                window.draw(previewsHades_Peak[i]);

                std::string mapName = texturesHades_Peak[i].second.substr(0, texturesHades_Peak[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(694.5 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 65 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsKashan_Desert.size(); ++i) {
                previewsKashan_Desert[i].setPosition(833.4 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 65 + (i / 10) * (previewSize + 10));
                window.draw(previewsKashan_Desert[i]);

                std::string mapName = texturesKashan_Desert[i].second.substr(0, texturesKashan_Desert[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(833.4 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 65 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsKhamisiyah.size(); ++i) {
                previewsKhamisiyah[i].setPosition(972.3 + 38.9 + 3.125 + (i % 10) * (previewSize + 10), windowHeight / 2 + 65 + (i / 10) * (previewSize + 10));
                window.draw(previewsKhamisiyah[i]);

                std::string mapName = texturesKhamisiyah[i].second.substr(0, texturesKhamisiyah[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(972.3 + 38.9 + 3.125 + (i % 10) * (previewSize + 10), windowHeight / 2 + 65 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            } // 4 line end

            for (size_t i = 0; i < previewsMasirah.size(); ++i) {
                previewsMasirah[i].setPosition(38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 205 + (i / 10) * (previewSize + 10));
                window.draw(previewsMasirah[i]);

                std::string mapName = texturesMasirah[i].second.substr(0, texturesMasirah[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 205 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsOperation_Soul_Rebel.size(); ++i) {
                previewsOperation_Soul_Rebel[i].setPosition(138.9 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 205 + (i / 10) * (previewSize + 10));
                window.draw(previewsOperation_Soul_Rebel[i]);

                std::string mapName = texturesOperation_Soul_Rebel[i].second.substr(0, texturesOperation_Soul_Rebel[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(138.9 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 205 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsOperation_Thunder.size(); ++i) {
                previewsOperation_Thunder[i].setPosition(277.8 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 205 + (i / 10) * (previewSize + 10));
                window.draw(previewsOperation_Thunder[i]);

                std::string mapName = texturesOperation_Thunder[i].second.substr(0, texturesOperation_Thunder[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(277.8 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 205 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsPavlovsk_Bay.size(); ++i) {
                previewsPavlovsk_Bay[i].setPosition(416.7 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 205 + (i / 10) * (previewSize + 10));
                window.draw(previewsPavlovsk_Bay[i]);

                std::string mapName = texturesPavlovsk_Bay[i].second.substr(0, texturesPavlovsk_Bay[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(416.7 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 205 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsRoad_to_Damascus.size(); ++i) {
                previewsRoad_to_Damascus[i].setPosition(555.6 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 205 + (i / 10) * (previewSize + 10));
                window.draw(previewsRoad_to_Damascus[i]);

                std::string mapName = texturesRoad_to_Damascus[i].second.substr(0, texturesRoad_to_Damascus[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(555.6 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 205 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsSaaremaa.size(); ++i) {
                previewsSaaremaa[i].setPosition(694.5 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 205 + (i / 10) * (previewSize + 10));
                window.draw(previewsSaaremaa[i]);

                std::string mapName = texturesSaaremaa[i].second.substr(0, texturesSaaremaa[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(694.5 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 205 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsShijiavalley.size(); ++i) {
                previewsShijiavalley[i].setPosition(833.4 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 205 + (i / 10) * (previewSize + 10));
                window.draw(previewsShijiavalley[i]);

                std::string mapName = texturesShijiavalley[i].second.substr(0, texturesShijiavalley[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(833.4 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 205 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsSilent_Eagle.size(); ++i) {
                previewsSilent_Eagle[i].setPosition(972.3 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 205 + (i / 10) * (previewSize + 10));
                window.draw(previewsSilent_Eagle[i]);

                std::string mapName = texturesSilent_Eagle[i].second.substr(0, texturesSilent_Eagle[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(972.3 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 205 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            } // 5 line end

            for (size_t i = 0; i < previewsVadso_City.size(); ++i) {
                previewsVadso_City[i].setPosition(38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 345 + (i / 10) * (previewSize + 10));
                window.draw(previewsVadso_City[i]);

                std::string mapName = texturesVadso_City[i].second.substr(0, texturesVadso_City[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 345 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsWanda_Shan.size(); ++i) {
                previewsWanda_Shan[i].setPosition(138.9 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 345 + (i / 10) * (previewSize + 10));
                window.draw(previewsWanda_Shan[i]);

                std::string mapName = texturesWanda_Shan[i].second.substr(0, texturesWanda_Shan[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(138.9 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 345 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsXiangshan.size(); ++i) {
                previewsXiangshan[i].setPosition(277.8 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 345 + (i / 10) * (previewSize + 10));
                window.draw(previewsXiangshan[i]);

                std::string mapName = texturesXiangshan[i].second.substr(0, texturesXiangshan[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(277.8 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 345 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            }
            for (size_t i = 0; i < previewsYamalia.size(); ++i) {
                previewsYamalia[i].setPosition(416.7 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 345 + (i / 10) * (previewSize + 10));
                window.draw(previewsYamalia[i]);

                std::string mapName = texturesYamalia[i].second.substr(0, texturesYamalia[i].second.find_last_of('.'));
                sf::Text mapText(mapName, font, 13);
                mapText.setFillColor(sf::Color::White);
                mapText.setPosition(416.7 + 38.9 + (i % 10) * (previewSize + 10), windowHeight / 2 + 345 + (i / 10) * (previewSize + 10) + previewSize + 5);
                window.draw(mapText);
            } // 6 line end

            window.draw(contactText);
            window.draw(versionText);
            window.draw(languageButton);

        }

        window.display();
    }

    return 0;
}
