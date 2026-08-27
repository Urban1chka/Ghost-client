#pragma once

#include <unordered_map>
#include <vector>
#include <string>

class LangManager {
    int lang = 0;

    struct Lang {
        const char* label;
        int font;
        std::unordered_map< std::string, std::string > dict{ };
    };

    std::vector< Lang > languages;
public:
    void add_language(const char* label, int font, std::unordered_map< std::string, std::string > dict) {
        languages.push_back(Lang{ label, font, dict });
    }

    int& get_lang() {
        return lang;
    }

    const char* get_lang_name() {
        return languages[lang].label;
    }

    std::vector< Lang >& get_langs() {
        return languages;
    }

    int get_font() {
        return (int)languages[lang].font;
    }

    void set_lang(int i) {
        lang = i;
    }

    static LangManager& get() {
        static LangManager s;
        return s;
    }

    const char* translate(const char* str) {
        if (lang == 0)
            return str;

        auto it = languages[lang].dict.find(str);
        if (it == languages[lang].dict.end()) return str;

        return it->second.c_str();
    }

    void initialize() {
        add_language("English", 0, { });
        add_language("Russian", 0, {
            { "Enable", u8"Включить" },
            { "Silent", u8"Незаметный" },
            { "Field of view", u8"Область" },
            { "Smooth", u8"Плавность" },
            { "Bones", u8"Кости" },
            { "Head", u8"Голова" },
            { "Neck", u8"Шея" },
            { "Body", u8"Тело" },
            { "Pelvis", u8"Таз" },
            { "Legs", u8"Ноги" },
            { "Arms", u8"Руки" },
            { "Standalone", u8"Стандартный" },
            { "Custom", u8"Кастомный" },
            { "Smart", u8"Умный" },
            { "Type", u8"Тип" },
            { "Enable##rcs", u8"Включить" },
            { "Draw FOV", u8"Рисовать область" },
            { "Drawing", u8"Рисовка" },
            { "Enemies only", u8"Только враги" },
            { "Visible only", u8"Только видимые" },
            { "Ignore bots", u8"Игнорировать ботов" },
            { "Type the text...", u8"Введите текст..." },
            { "Recoil control system", u8"Контроль разброса" },
            { "Misc", u8"Прочее" },
            { "Triggerbot", u8"Триггербот" },
            { "Draw crosshair", u8"Рисовать прицел" },
            { "Hide shots", u8"Скрытые выстрелы" },
            { "Aim lock", u8"Аим лок" },
            { "Rainbow", u8"Радуга" },
            { "Thickness", u8"Толщина" },
            { "Space", u8"Пробел" },
            { "Search...", u8"Поиск..." },
            { "Settings", u8"Настройки" },
            { "Predict", u8"Предикт" },
            { "Draw target", u8"Рисовать цель" },
            { "Save target", u8"Сохранять цель" },
            { "Delay", u8"Задержка" },
            { "CONFIG", u8"КОНФИГ" },
            { "Name:", u8"Название:" },
            { "Created:", u8"Дата создания:" },
            { "Created by:", u8"Создатель:" },
            { "Enter the name...", u8"Введите название..." },
            { "Load", u8"Загрузить" },
            { "Delete", u8"Удалить" },
            { "Loaded the config", u8"Конфиг загружен" },
            { "Removed the config", u8"Конфиг удален" },
            { "Enter the name!", u8"Введите название!" },
            { "Successfully created a config", u8"Конфиг успешно создан" },
            { "SUCCESS", u8"УСПЕХ" },
            { "ERROR", u8"ОШИБКА" },
            { "INFO", u8"ИНФОРМАЦИЯ" },
            { "AimAssistance", u8"Аимбот" },
            { "Visuals", u8"Визуалы" },
            { "Weapon", u8"Оружие" },
            { "Miscellaneous", u8"Прочее" },
            { "Configs", u8"Конфигурации" },
            { "General", u8"Основное" },
            { "Manipulator", u8"Манипулятор" },
            { "Manipulator", u8"манипулятор" },
            { "Draw Manipulator Angles", u8"Рисовать углы манипулятора" },
            { "Max Angles", u8"Макс. углов" },
            { "Manip Distance", u8"Дистанция манипулятора" },
            { "Players", u8"Игроки" },
            { "Chams", u8"Чамсы" },
            { "World", u8"Мир" },
            { "Other", u8"Другое" },
            { "Combat", u8"Бой" },
            { "Movement", u8"Движение" },
            { "Are you sure u want to enable it?", u8"Вы точно хотите включить это?" },
            { "This function is ", u8"Эта функция " },
            { "dangerous!", u8"опасная!" },
            { "YES", u8"ДА" },
            { "NO", u8"НЕТ" },
            { "Enable Aim", u8"Включить аимбот" },
            { "Silent Aim", u8"Тихий аим" },
            { "Auto Fire", u8"Авто-стрельба" },
            { "Fov", u8"Обзор" },
            { "Bone", u8"Кость" },
            { "Draw Aim Fov", u8"Рисовать область аима" },
            { "Aim Fov Color", u8"Цвет области аима" },
            { "Draw Aim Target", u8"Рисовать цель аима" },
            { "Aim Target Color", u8"Цвет цели аима" },
            { "Visible Check", u8"Проверка видимости" },
            { "Ignore Npc", u8"Игнорировать НПС" },
            { "Ignore Team", u8"Игнорировать команду" },
            { "Ignore Sleepers", u8"Игнорировать спящих" },
            { "Enable ESP", u8"Включить ESP" },
            { "Box", u8"Рамка" },
            { "Name", u8"Имя" },
            { "Health", u8"Здоровье" },
            { "Distance", u8"Дистанция" },
            { "Skeleton", u8"Скелет" },
            { "Inventory", u8"Инвентарь" },
            { "Icon Size", u8"Размер иконок" },
            { "Team", u8"Команда" },
            { "NPC", u8"НПС" },
            { "Sleepers", u8"Спящие" },
            { "Box Color", u8"Цвет рамки" },
            { "Name Color", u8"Цвет имени" },
            { "Distance Color", u8"Цвет дистанции" },
            { "Skeleton Color", u8"Цвет скелета" },
            { "Visible Color", u8"Видимый цвет" },
            { "Filters", u8"Фильтры" },
            { "Filters colors", u8"Цвета фильтров" },
            { "Colors", u8"Цвета" },
            { "Time Changer", u8"Смена времени" },
            { "Time", u8"Время" },
            { "Bright Night", u8"Яркая ночь" },
            { "Star color", u8"Цвет звезд" },
            { "Stars Changer", u8"Смена звезд" },
            { "Size", u8"Размер" },
            { "Brightness", u8"Яркость" },
            { "Draw Tracers", u8"Рисовать трассеры" },
            { "Tracer Color", u8"Цвет трассера" },
            { "Lifetime", u8"Время жизни" },
            { "FOV Changer", u8"Смена FOV" },
            { "FOV Value", u8"Значение FOV" },
            { "Zoom", u8"Приближение" },
            { "Zoom Fov", u8"FOV приближения" },
            { "Enable crosshair", u8"Включить прицел" },
            { "Crosshair color", u8"Цвет прицела" },
            { "Cross", u8"Крест" },
            { "Dot", u8"Точка" },
            { "Rotating cross", u8"Вращающийся крест" },
            { "Local Player", u8"Локальный игрок" },
            { "Custom Model", u8"Кастомная модель" },
            { "Browse", u8"Обзор" },
            { "Custom Hands", u8"Кастомные руки" },
            { "Reset", u8"Сбросить" },
            { "No Recoil", u8"Без отдачи" },
            { "No Spread", u8"Без разброса" },
            { "No Sway", u8"Без покачивания" },
            { "Insta Eoka", u8"Мгновенная еока" },
            { "Silent Reload", u8"Тихая перезарядка" },
            { "Reload Indicator", u8"Индикатор перезарядки" },
            { "Wall Shot", u8"Простреливать стены" },
            { "Bullet TP", u8"Телепорт пуль" },
            { "Infinite Jump", u8"Бесконечный прыжок" },
            { "No Mini Sprint", u8"Без мини-спринта" },
            { "Anti Aim", u8"Анти-аим" },
            { "Spider", u8"Паук" },
            { "Air Stack", u8"Воздушный стак" },
            { "Tp To Head", u8"Телепорт к голове" },
            { "Speed Hack", u8"Спидхак" },
            { "Speed Value", u8"Значение скорости" },
            { "Insta Pickup Wounded", u8"Мгновенный подбор раненых" },
            { "Can Wield Items", u8"Можно держать предметы" },
            { "Fast Loot", u8"Быстрый лут" },
            { "Hit Sound", u8"Звук попадания" },
            { "All Sounds Volume", u8"Громкость всех звуков" },
            { "Unload cheat", u8"Выгрузить чит" },
            { "Unloading...", u8"Выгрузка..." },
            { "Config name...", u8"Название конфига..." },
            { "Config name", u8"Название конфига" },
            { "Config saved", u8"Конфиг сохранен" },
            { "Config loaded", u8"Конфиг загружен" },
            { "Config deleted", u8"Конфиг удален" },
            { "No config selected!", u8"Конфиг не выбран!" },
            { "No config to delete!", u8"Нет конфига для удаления!" },
            { "No configs found", u8"Конфиги не найдены" },
            { "Refresh list", u8"Обновить список" },
            { "Open Dir", u8"Открыть папку" },
            { "List refreshed", u8"Список обновлен" },
            { "Directory opened", u8"Папка открыта" },
            { "Config actions", u8"Действия с конфигом" },
            { "Bind", u8"Привязка" },
            { " LIST", u8" СПИСОК" },
            { "HOLD", u8"УДЕРЖИВАТЬ" },
            { "TOGGLE", u8"ПЕРЕКЛЮЧАТЬ" },
            { "ALWAYS", u8"ВСЕГДА" },
            { "Hold", u8"Удерживать" },
            { "Toggle", u8"Переключать" },
            { "Always On", u8"Всегда вкл" },
            { "None", u8"Нет" },
            { "UNKNOWN", u8"НЕИЗВЕСТНО" },
            { "Manipulator not included!", u8"Манипулятор не включен!" },
            { "Aimbot", u8"Аимбот" },
            { "Visuals", u8"Визуал" },
            { "Filters", u8"Фильтры" },
            { "Filters colors", u8"Цвета фильтров" },
            { "Environment", u8"Окружение" },
            { "Bullet tracers", u8"Трассеры пуль" },
            { "Camera", u8"Камера" },
            { "Crosshair", u8"Прицел" },
            { "Bullet Mods", u8"Модификации пуль" },
            { "Player", u8"Игрок" },
            { "Sound", u8"Звук" },
            { "Unload", u8"Выгрузка" },
            { "ESP", u8"ESP" },
            { "add", u8"добавить" },
            { "Night color", u8"Цвет ночи" },
            { "Opacity", u8"Прозрачность" },
            { "Save", u8"Сохранить" },
            { "Gravity Value", u8"Значение гравитации" },
            { "Fly", u8"Полёт" },
            { "No Fall Damage", u8"Без урона от падения" },
            { "Enable Desync", u8"Включить десинк" },
            { "Max Desync", u8"Макс. десинк" },
            { "Desync Indicator", u8"Индикатор десинка" },
            { "Hitscan", u8"Хитскан" },
            { "Hitscan Range", u8"Дальность хитскана" },
            { "Anti-Aim", u8"Анти-аим" },
            { "AA Mode", u8"Режим АА" },
            { "Realistic", u8"Реалистичный" },
            { "Jitter", u8"Джиттер" },
            { "Freestanding", u8"Свободный" },
            { "Yaw Speed", u8"Скорость yaw" },
            { "Jitter Range", u8"Диапазон джиттера" },
            { "Pitch", u8"Тангаж" },
            { "LBY Breaker", u8"Слом LBY" },
            { "LBY Offset", u8"Смещение LBY" },
            { "Anti-Aim Lines", u8"Линии анти-аима" },
            { "Line Length", u8"Длина линии" },
            { "Resource ESP", u8"ESP ресурсов" },
            { "Text Size", u8"Размер текста" },
            { "Stone", u8"Камень" },
            { "Metal", u8"Металл" },
            { "Sulfur", u8"Сера" },
            { "Hemp", u8"Ткань" },
            { "Stone Color", u8"Цвет камня" },
            { "Metal Color", u8"Цвет металла" },
            { "Sulfur Color", u8"Цвет серы" },
            { "Hemp Color", u8"Цвет ткани" },
            });
    }
};