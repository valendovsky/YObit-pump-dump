// YObitPumpDump.cpp : Программа работает с API v3 криптобиржи YObit.net
// Программа получает список всех актуальных торговых пар
// Используя меню можно вывести список торговых пар, обновить его или запросить подробную информацию по определённой паре
// Выбрав номер торговой пары и глубину просмотра стакана можно получить статистику по объему торгов:
// - объем торгов
// - объём ордеров на продажу
// - цена последнего ордера на продажу по лимиту
// - объём ордеров на покупку
// - цена последнего ордера на покупку по лимиту
// - цена последней сделки
// - цена покупки
// - цена продажи
//

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include <exception>
#include <clocale>

#include <curl/curl.h>
#include <nlohmann/json.hpp>

// Тип количества торговых пар
using ticker_count = int;

// Константы
namespace CST
{
    const std::string END_STR(28, '=');
}

// Ссылки формирования запросов
namespace API
{
    const std::string INFO_API = "https://yobit.net/api/3/info";
    const std::string TICKER = "https://yobit.net/api/3/ticker/";
    const std::string DEPTH = "https://yobit.net/api/3/depth/";
}

// Пункты меню
namespace MENU
{
    enum Menu
    {
        VIEW = 'v',   // Просмотреть все торговые пары
        UPDATE = 'u', // Обновить список тикеров
        TICKER = 't', // Получить подробную информацию по торговой паре
        REPEAT = 'r', // Повторно получить подробную информацию по последней паре
        EXIT = 'q'    // Выйти из программы
    };
}

// Информация по торговой паре
struct Pair
{
    std::string ticker;
    int limit;
    double vol = -1;
    double asksVol = 0;
    double asksPrice = 0;
    double bidsVol = 0;
    double bidsPrice = 0;
    double last = -1;
    double buy = -1;
    double sell = -1;
};


// Callback функция для сохранения результатов запроса в строку
static size_t writeString(char* ptr, size_t size, size_t nmemb, std::string* content)
{
    if (content)
    {
        content->append(ptr, size * nmemb);

        return size * nmemb;
    }
    else
    {
        return 0;
    }
}


// Получает результат по запросу на API
// content - параметр вывода
int getResponse(CURL* curlHandle, const std::string& url, std::string& content)
{
    if (!curlHandle)
    {
        return -4;
    }

    curl_easy_setopt(curlHandle, CURLOPT_URL, url.c_str());

    curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, writeString);
    curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &content);

    CURLcode response = curl_easy_perform(curlHandle);
    if (response == CURLE_OK)
    {
        return 0;
    }
    else
    {
        std::cerr << curl_easy_strerror(response) << std::endl;

        return -4;
    }
}


// Формирует массив с актуальными торговыми парами
// pairs - параметр вывода
int getPairsArray(const std::string& content, std::vector<std::string>& pairs)
{
    if (!nlohmann::json::accept(content))
    {
        std::cerr << "That content is invalid!" << std::endl;

        return -2;
    }

    nlohmann::json pairsJson;
    try
    {
        auto parsed = nlohmann::json::parse(content);
        // Ключ "pairs" содержит актуальные торговые пары
        pairsJson = parsed.at("pairs");
    }
    catch (std::exception& exception) // Обработка некоректного json содержимого
    {
        std::cerr << "Standart exception: " << exception.what() << std::endl;
    }

    if (pairsJson == nullptr)
    {
        std::cout << "There are no relevant pairs.\n";

        return -2;
    }
    else
    {
        // При обновлении необходимо удалить старые данные
        if (pairs.size())
        {
            pairs.clear();
        }

        for (const auto& element : pairsJson.items())
        {
            pairs.push_back(element.key());
        }
    }

    return 0;
}


// Получает все актуальные торговые пары
// pairs - параметр вывода
int getPairs(CURL* curlHandle, std::vector<std::string>& pairs)
{
    // Делаем API-запрос
    std::string content;
    if (getResponse(curlHandle, API::INFO_API, content))
        return -4;

    // Формируем перечень торговых пар
    if (getPairsArray(content, pairs))
        return -2;

    return 0;
}


// Выбор пункта меню
char selectMenu()
{
    char choice;

    do
    {
        std::cout << std::string(11, '>') << " Menu " << std::string(11, '<') << '\n' <<
            "View pairs" << std::setw(9) << std::right << " - v\n" <<
            "Update pairs" << std::setw(7) << std::right << " - u\n" <<
            "Select ticker" << std::setw(6) << std::right << " - t\n" <<
            "Repeat ticker" << std::setw(6) << std::right << " - r\n" <<
            "Exit" << std::setw(15) << std::right << " - q\n" <<
            "Select the menu item: ";

        std::cin >> choice;
        // Проверка пользовательского ввода
        if (std::cin.fail())
        {
            std::cin.clear();
            std::cout << "That input is invalid. Please try again.\n";
        }
        std::cin.ignore(32767, '\n');

        std::cout << CST::END_STR << std::endl;
    } while (choice != MENU::EXIT &&
        choice != MENU::VIEW &&
        choice != MENU::UPDATE &&
        choice != MENU::TICKER &&
        choice != MENU::REPEAT);

    return choice;
}


// Вывод актуальных торговых пар с их порядковым номером в консоль
void viewPairs(const std::vector<std::string>& pairs)
{
    if (pairs.size() == 0)
    {
        std::cout << ">> There are no tickers <<" << std::endl;

        return;
    }


    std::cout << ">>> The list of tickers  <<<\n";

    int indexPair = 0;
    for (const auto& element : pairs)
    {
        std::cout << ++indexPair << " - " << element << '\n';
    }

    std::cout << "The end of the tickers list.\n" << CST::END_STR << std::endl;
}


// Выбор торговой пары по её порядковому номеру в списке
ticker_count getTicker(const ticker_count sizePairs)
{
    ticker_count ticker = 0;
    do
    {
        std::cout << "Enter a number of ticker: ";
        std::cin >> ticker;
        if (std::cin.fail())
        {
            std::cin.clear();
            std::cout << "That number is invalid. Please try again.\n";
        }
        std::cin.ignore(32767, '\n');
    } while (ticker <= 0 || ticker > sizePairs);

    return ticker - 1; // Пользователю выводим нумерацию с 1
}


// Получает глубину стакана
int getLimit()
{
    int limit = 0;
    do
    {
        std::cout << "Enter the limit of the depth (150 - 2000): ";
        std::cin >> limit;
        if (std::cin.fail())
        {
            std::cin.clear();
            std::cout << "That input is invalid. Please try again.\n";
        }
        std::cin.ignore(32767, '\n');
    } while (limit < 150 || limit > 2000);

    return limit;
}


// Выбор торговой пары и глубины ставок
int getPair(Pair& curPair, const std::vector<std::string>& pairs)
{
    ticker_count sizePairs = pairs.size();
    if (!sizePairs)
    {
        std::cerr << "There are no trading pairs!";

        return -3;
    }
    curPair.ticker = pairs[getTicker(sizePairs)];
    std::cout << "You chose " << curPair.ticker << ".\n";
    curPair.limit = getLimit();

    std::cout << CST::END_STR << '\n';

    return 0;
}


// Распарсим ответ по запросу ticker
// curPair - параметр вывода
int tickerParse(Pair& curPair, const std::string& content)
{
    // Проверка json на валидность
    if (!nlohmann::json::accept(content))
    {
        std::cerr << "That content is invalid!" << std::endl;

        return -5;
    }

    try
    {
        auto parsed = nlohmann::json::parse(content);
        curPair.vol = parsed.at(curPair.ticker).at("vol");
        curPair.last = parsed.at(curPair.ticker).at("last");
        curPair.buy = parsed.at(curPair.ticker).at("buy");
        curPair.sell = parsed.at(curPair.ticker).at("sell");
    }
    catch (std::exception& exception)
    {
        std::cerr << "Standard exception: " << exception.what() << std::endl;

        return -5;
    }

    return 0;
}


// Распарсим ответ по запросу depth
// curPair - параметр вывода
int depthParse(Pair& curPair, const std::string& content)
{
    // Проверка json на валидность
    if (!nlohmann::json::accept(content))
    {
        std::cerr << "That content is invalid!" << std::endl;

        return -6;
    }

    nlohmann::json asksJson;
    nlohmann::json bidsJson;
    try
    {
        auto parsed = nlohmann::json::parse(content);
        asksJson = parsed.at(curPair.ticker).at("asks");
        bidsJson = parsed.at(curPair.ticker).at("bids");
    }
    catch (std::exception& exception) // Обработка некоректного json содержимого
    {
        std::cerr << "Standard exception: " << exception.what() << std::endl;
    }

    if (asksJson == nullptr || bidsJson == nullptr)
    {
        std::cerr << "That content is invalid!" << std::endl;

        return -6;
    }
    else
    {
        // Ордера на продажу
        for (auto& element : asksJson)
        {
            // Элемент содержит цену(в единицах второго тикера) и количество(в единицах первого тикера)
            std::vector<double> asks = element;
            curPair.asksPrice = asks[0];
            curPair.asksVol += asks[1];
        }

        // Ордера на покупку
        for (auto& element : bidsJson)
        {
            // Элемент содержит цену и количество
            std::vector<double> bids = element;
            curPair.bidsPrice = bids[0];
            curPair.bidsVol += bids[1];
        }
    }

    return 0;
}


// Получает статистику по торговой паре
int getPairInfo(CURL* curlHandle, Pair& curPair)
{
    // Получаем статистику по паре за последние 24 часа
    std::string tickerUrl = API::TICKER + curPair.ticker;
    std::string content;

    if (getResponse(curlHandle, tickerUrl, content))
    {
        return -4;
    }

    if (tickerParse(curPair, content))
    {
        return -5;
    }

    // Получаем статистику по активным ордерам
    std::string depthUrl = API::DEPTH + curPair.ticker + "?" + std::to_string(curPair.limit);

    content.clear();
    if (getResponse(curlHandle, depthUrl, content))
    {
        return -4;
    }

    if (depthParse(curPair, content))
    {
        return -6;
    }

    return 0;
}


// Выводит информацию по торговой паре на русском
void printPairInfoRu(const Pair& curPair)
{
    setlocale(LC_ALL, "RU");

    std::cout << std::fixed << std::setprecision(8) <<
        ">>>> ТОРГОВАЯ ПАРА: " << curPair.ticker <<
        std::setw(28) << std::left << "\nОбъём торгов: " << curPair.vol <<
        std::setw(28) << std::left << "\nГлубина стакана: " << curPair.limit <<
        std::setw(28) << std::left << "\nОрдера на продажу: " << curPair.asksVol <<
        std::setw(28) << std::left << "\nОрдера на покупку: " << curPair.bidsVol <<
        std::setw(28) << std::left << "\nЦена на продажу по лимиту: " << curPair.asksPrice <<
        std::setw(28) << std::left << "\nЦена на покупку по лимиту: " << curPair.bidsPrice <<
        std::setw(28) << std::left << "\nЦена последней сделки: " << curPair.last <<
        std::setw(28) << std::left << "\nЦена покупки: " << curPair.buy <<
        std::setw(28) << std::left << "\nЦена продажи: " << curPair.sell << '\n' <<
        CST::END_STR << std::endl;
}


// Выводит информацию по торговой паре
void printPairInfo(const Pair& curPair)
{
    std::cout << std::fixed << std::setprecision(8) <<
        ">>>> TICKER: " << curPair.ticker <<
        std::setw(14) << std::left << "\nVol: " << curPair.vol <<
        std::setw(14) << std::left << "\nDepth limit: " << curPair.limit <<
        std::setw(14) << std::left << "\nVol of asks: " << curPair.asksVol <<
        std::setw(14) << std::left << "\nVol of bids: " << curPair.bidsVol <<
        std::setw(14) << std::left << "\nAsks price: " << curPair.asksPrice <<
        std::setw(14) << std::left << "\nBids price: " << curPair.bidsPrice <<
        std::setw(14) << std::left << "\nLast: " << curPair.last <<
        std::setw(14) << std::left << "\nBuy: " << curPair.buy <<
        std::setw(14) << std::left << "\nSell: " << curPair.sell << '\n' <<
        CST::END_STR << std::endl;
}


int main()
{
    std::cout << "The application is loading...\n";

    // Инициализируем сессию
    CURL* curlHandle = curl_easy_init();
    if (!curlHandle)
    {
        return -1;
    }

    // Запрашиваем актуальные торговые пары
    std::vector<std::string> pairs;
    int getPairsStatus = getPairs(curlHandle, pairs);
    if (getPairsStatus)
    {
        curl_easy_cleanup(curlHandle);

        return getPairsStatus;
    }

    // Основной цикл программы
    char choice = '0';
    std::string tickerPrev = "None";
    int limitPrev = 150;
    while (choice != 'q')
    {
        choice = selectMenu();

        switch (choice)
        {
        case MENU::VIEW: // Просмотр актуальных торговых пар
            viewPairs(pairs);
            break;
        case MENU::UPDATE: // Обновить список актуальных торговых пар
        {
            std::cout << "Pairs are updating...\n";

            getPairsStatus = getPairs(curlHandle, pairs);
            if (getPairsStatus)
            {
                curl_easy_cleanup(curlHandle);

                return getPairsStatus;
            }

            std::cout << "Completed\n" << CST::END_STR << std::endl;

            break;
        }
        case MENU::TICKER: // Выбрать торговую пару и посмотреть информацию по ней
        {
            Pair curPair;

            // Запрашиваем тикер и глубину стакана
            if (getPair(curPair, pairs))
            {
                curl_easy_cleanup(curlHandle);

                return -3;
            }

            std::cout << "Processing...\n";

            // Запоминаем пару для повторного запроса
            tickerPrev = curPair.ticker;
            limitPrev = curPair.limit;

            // Получаем информацию по торговой паре
            int infoStatus;
            infoStatus = getPairInfo(curlHandle, curPair);
            if (infoStatus)
            {
                curl_easy_cleanup(curlHandle);

                return infoStatus;
            }

            // Выводим статистику по торговой паре в консоль
            printPairInfo(curPair);

            break;
        }
        case MENU::REPEAT:
        {
            if (tickerPrev == "None")
            {
                std::cout << "There are no a previous ticker.\n" << CST::END_STR << std::endl;

                break;
            }

            std::cout << "Processing...\n";

            Pair prevPair;
            prevPair.ticker = tickerPrev;
            prevPair.limit = limitPrev;

            // Получаем информацию по торговой паре
            int repeatStatus;
            repeatStatus = getPairInfo(curlHandle, prevPair);
            if (repeatStatus)
            {
                curl_easy_cleanup(curlHandle);

                return repeatStatus;
            }

            // Выводим статистику по торговой паре в консоль
            printPairInfo(prevPair);

            break;
        }
        case MENU::EXIT: // Выход из программы
            std::cout << "The application has completed its task and will close now." << std::endl;
            break;
        default:
            std::cout << "Unknown command!" << std::endl;
            break;
        }
    }

    curl_easy_cleanup(curlHandle);

    std::cin.get();

    return 0;
}
