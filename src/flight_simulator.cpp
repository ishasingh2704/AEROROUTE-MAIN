#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <cmath>
#include <algorithm>
#include <string>
#include <cctype>
#include <iomanip>
#include <ctime>
#include <random>
#include <chrono>
#include <sstream>
#include <filesystem>
#include <set>
#include <unordered_map>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include "config.h"
using namespace std;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef OPENWEATHERMAP_API_KEY
#error "API key not defined! Please create config.h and define OPENWEATHERMAP_API_KEY."
#endif

string apiKey = OPENWEATHERMAP_API_KEY;

struct Airport
{
    string code;
    string name;
    sf::Vector2f position;
    double latitude;
    double longitude;
};

struct WeatherCondition
{
    bool isBad;
    string description;
};

struct EdgeInfo
{
    int to;
    double distance;
    double cost;
    double time;
};

struct FlightGraph
{
    vector<Airport> airports;
    vector<vector<EdgeInfo>> adj;
    vector<vector<bool>> pathAvailable;
    vector<vector<WeatherCondition>> pathWeather;

    void addAirport(const string &code, float x, float y, double lat, double lon)
    {
        airports.push_back({code, "Unknown", sf::Vector2f(x, y), lat, lon});
        adj.emplace_back();

        for (auto &row : pathAvailable)
        row.push_back(false);
        pathAvailable.push_back(vector<bool>(airports.size(), false));

        for (auto &row : pathWeather)
        row.push_back({false, "Clear"});
        pathWeather.push_back(vector<WeatherCondition>(airports.size(), {false, "Clear"}));
    }

    void addEdge(int u, int v, double dist, double cost, double time)
    {
        adj[u].push_back({v, dist, cost, time});
        adj[v].push_back({u, dist, cost, time});
        pathAvailable[u][v] = true;
        pathAvailable[v][u] = true;
        pathWeather[u][v] = {false, "Clear"};
        pathWeather[v][u] = {false, "Clear"};
    }

    void updateWeather(int u, int v, bool isBad, const string &description)
    {
        pathWeather[u][v] = {isBad, description};
        pathWeather[v][u] = {isBad, description};
        pathAvailable[u][v] = !isBad;
        pathAvailable[v][u] = !isBad;
    }

    bool hasBadWeather(const vector<int> &path) const
    {
        for (size_t i = 0; i < path.size() - 1; ++i)
        {
            int u = path[i];
            int v = path[i + 1];
            if (pathWeather[u][v].isBad)
            {
                return true;
            }
        }
        return false;
    }

    vector<pair<string, string>> getPathWeatherInfo(const vector<int> &path) const
    {
        vector<pair<string, string>> result;
        for (size_t i = 0; i < path.size() - 1; ++i)
        {
            int u = path[i];
            int v = path[i + 1];
            if (pathWeather[u][v].isBad)
            {
                result.push_back({airports[u].code + "-" + airports[v].code, pathWeather[u][v].description});
            }
        }
        return result;
    }

    vector<int> dijkstra(int src, int dst, vector<pair<int, int>> &exploredEdges, const string &metric = "distance") const
    {
        int n = adj.size();
        vector<double> dist(n, numeric_limits<double>::infinity());
        vector<int> prev(n, -1);
        dist[src] = 0;
        using PDI = pair<double, int>;
        priority_queue<PDI, vector<PDI>, greater<>> pq;
        pq.push({0, src});
        while (!pq.empty())
        {
            auto [d, u] = pq.top();
            pq.pop();
            if (u == dst)
            break;
            for (const auto &e : adj[u])
            {
                if (!pathAvailable[u][e.to])
                continue;
                exploredEdges.push_back({u, e.to});
                double w = (metric == "distance") ? e.distance : (metric == "cost") ? e.cost : e.time;
                double alt = d + w;
                if (alt < dist[e.to])
                {
                    dist[e.to] = alt;
                    prev[e.to] = u;
                    pq.push({alt, e.to});
                }
            }
        }
        vector<int> path;
        for (int at = dst; at != -1; at = prev[at])
        path.push_back(at);
        reverse(path.begin(), path.end());
        if (path.empty() || path.front() != src)
        return {};
        return path;
    }

    vector<int> dijkstra(int src,int dst, const string &metric = "distance") const
    {
        vector<pair<int, int>> dummy;
        return dijkstra(src, dst, dummy, metric);
    }

    vector<int> findRouteWithWeatherRerouting(int src, int dst, bool &rerouted)
    {
        vector<int> originalPath = dijkstra(src, dst);
        if (originalPath.empty() || !hasBadWeather(originalPath))
        {
            rerouted = false;
            return originalPath;
        }

        rerouted = true;

        FlightGraph tempGraph = *this;

        for (size_t i = 0; i < originalPath.size() - 1; ++i)
        {
            int u = originalPath[i];
            int v = originalPath[i + 1];
            if (pathWeather[u][v].isBad)
            {

                tempGraph.pathAvailable[u][v] = false;
                tempGraph.pathAvailable[v][u] = false;
            }
        }
        return tempGraph.dijkstra(src, dst);
    }

    vector<int> astar(int src, int dst) const
    {
        int n = airports.size();
        vector<double> gScore(n, numeric_limits<double>::infinity());
        vector<double> fScore(n, numeric_limits<double>::infinity());
        vector<int> prev(n, -1);
        auto heuristic = [&](int a, int b)
        {
            sf::Vector2f pa = airports[a].position;
            sf::Vector2f pb = airports[b].position;
            return sqrt((pa.x - pb.x) * (pa.x - pb.x) + (pa.y - pb.y) * (pa.y - pb.y));
        };

        gScore[src] = 0;
        fScore[src] = heuristic(src, dst);

        set<pair<double, int>> openSet;
        openSet.insert(make_pair(fScore[src], src));

        while (!openSet.empty())
        {
            int u = openSet.begin()->second;
            openSet.erase(openSet.begin());
            if (u == dst)
            break;

            for (const auto &e : adj[u])
            {
                if (!pathAvailable[u][e.to])
                continue;
                double tentative = gScore[u] + e.distance;
                if (tentative < gScore[e.to])
                {
                    openSet.erase(make_pair(fScore[e.to], e.to));
                    prev[e.to] = u;
                    gScore[e.to] = tentative;
                    fScore[e.to] = tentative + heuristic(e.to, dst);
                    openSet.insert(make_pair(fScore[e.to], e.to));
                }
                
            }
        }

        vector<int> path;
        for (int at = dst; at != -1; at = prev[at])
        path.push_back(at);
        reverse(path.begin(), path.end());
        if (path.empty() || path.front() != src)
        return {};
        return path;
    }

    vector<int> bellmanFord(int src, int dst) const
    {
        int n = adj.size();
        vector<double> dist(n, numeric_limits<double>::infinity());
        vector<int> prev(n, -1);
        dist[src] = 0;

        for (int i = 0; i < n - 1; ++i)
        {
            for (int u = 0; u < n; ++u)
            {
                for (const auto &e : adj[u])
                {
                    int v = e.to;
                    double w = e.distance;
                    if (!pathAvailable[u][v])
                    continue;
                    if (dist[u] != numeric_limits<double>::infinity() && dist[u] + w < dist[v])
                    {
                        dist[v] = dist[u] + w;
                        prev[v] = u;
                    }
                }
            }
        }

        for (int u = 0; u < n; ++u)
        {
            for (const auto &e : adj[u])
            {
                int v = e.to;
                double w = e.distance;
                if (!pathAvailable[u][v])
                continue;
                if (dist[u] != numeric_limits<double>::infinity() && dist[u] + w < dist[v])
                {
                    cerr << "Graph contains a negative cycle!" << endl;
                    return {};
                }
            }
        }

        vector<int> path;
        for (int at = dst; at != -1; at = prev[at])
        path.push_back(at);
        reverse(path.begin(), path.end());
        if (path.empty() || path.front() != src)
        return {};
        return path;
    }
};

struct FlightTicket
{
    int departureAirportIndex;
    int arrivalAirportIndex;
    string departureDate;
    string departureTime;
    string arrivalTime;
    double price;
    string seatNumber;
    string passengerName;
    string bookingReference;
    bool isBooked;
};

struct Date
{
    int day;
    int month;
    int year;

    string toString() const
    {
        stringstream ss;
        ss << setfill('0') << setw(2) << day << "/" << setfill('0') << setw(2) << month << "/" << year;
        return ss.str();
    }

    static Date fromString(const string &dateStr)
    {
        Date date;
        stringstream ss(dateStr);
        char delimiter;
        ss >> date.day >> delimiter >> date.month >> delimiter >> date.year;
        return date;
    }

    Date addDays(int days) const
    {
        Date newDate = *this;
        newDate.day += days;

        int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

        if (newDate.year % 4 == 0)
        {
            daysInMonth[2] = 29;
        }

        while (newDate.day > daysInMonth[newDate.month])
        {
            newDate.day -= daysInMonth[newDate.month];
            newDate.month++;

            if (newDate.month > 12)
            {
                newDate.month = 1;
                newDate.year++;

                if (newDate.year % 4 == 0)
                {
                    daysInMonth[2] = 29;
                }
                else
                {
                    daysInMonth[2] = 28;
                }
            }
        }
        return newDate;
    }

    static Date getCurrentDate()
    {
        auto now = chrono::system_clock::now();
        time_t now_time = chrono::system_clock::to_time_t(now);
        tm *now_tm = localtime(&now_time);

        Date currentDate;
        currentDate.day = now_tm->tm_mday;
        currentDate.month = now_tm->tm_mon + 1;
        currentDate.year = now_tm->tm_year + 1900;

        return currentDate;
    }
};

void printLine(char c = '-', int length = 50)
{
    for (int i = 0; i < length; i++)
    {
        cout << c;
    }
    cout << endl;
}

int resolveAirportIndex(const string &input, const vector<Airport> &airports)
{
    if (isdigit(input[0]))
    return stoi(input);
    for (size_t i = 0; i < airports.size(); ++i)
        if (airports[i].code == input)
        return static_cast<int>(i);
    return -1;
}

void visualizeGraph(const FlightGraph &graph, const vector<int> &path, const vector<int> &originalPath, bool rerouted, const vector<pair<int, int>> &exploredEdges, const string &metrics, int src, int dst, double us, const std::string& bookedDate = "", const std::string& bookedTime = "")
{
    sf::RenderWindow window(sf::VideoMode(1496, 1120), "Flight Path Visualization");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.loadFromFile("assets/default.ttf"))
    {
        cerr << "Error loading font 'default.ttf'. Make sure it's available.\n";
        return;
    }
    sf::Font &mainFont = font;
    sf::Text statusText;
    statusText.setFont(mainFont);
    statusText.setString("ORIGINAL FLIGHT PATH");
    statusText.setCharacterSize(32);
    statusText.setStyle(sf::Text::Bold);
    statusText.setFillColor(sf::Color(80, 255, 120));
    statusText.setPosition(window.getSize().x / 2 - 200, 20);

    sf::Text routeText;
    routeText.setFont(mainFont);
    string routeStr = "Route: ";
    for (size_t i = 0; i < path.size(); ++i)
    {
        routeStr += graph.airports[path[i]].code;
        if (i < path.size() - 1)
        routeStr += "   ";
    }
    routeText.setString(routeStr);
    routeText.setCharacterSize(22);
    routeText.setStyle(sf::Text::Bold);
    routeText.setFillColor(sf::Color(60, 120, 255));
    routeText.setPosition(window.getSize().x / 2 - 200, 60);

    sf::Text metricsText;
    metricsText.setFont(mainFont);
    metricsText.setString(metrics);
    metricsText.setCharacterSize(20);
    metricsText.setStyle(sf::Text::Regular);
    metricsText.setFillColor(sf::Color(80, 80, 80));
    metricsText.setPosition(window.getSize().x / 2 - 200, 90);

    auto drawShadowedText = [&](sf::RenderWindow &win, sf::Text &txt)
    {
        sf::Text shadow = txt;
        shadow.setFillColor(sf::Color(0, 0, 0, 120));
        shadow.move(2, 2);
        win.draw(shadow);
        win.draw(txt);
    };

    for (size_t i = 0; i < graph.airports.size(); ++i)
    {
        const auto &airport = graph.airports[i];

        sf::Sprite airportSprite;
        {
            sf::Text label;
            label.setFont(font);
            label.setString(airport.code);
            label.setCharacterSize(15);
            label.setStyle(sf::Text::Bold);
            label.setFillColor(sf::Color::Black);
            label.setPosition(airport.position.x + 10, airport.position.y - 10);
            drawShadowedText(window, label);
        }
    }

    vector<sf::CircleShape> pathAirportShapes;
    if (!path.empty())
    {
        for (size_t i = 0; i < path.size(); ++i)
        {
            const auto &airport = graph.airports[path[i]];
            sf::CircleShape shape(10);
            shape.setFillColor(sf::Color::Magenta);
            shape.setPosition(airport.position.x - 2, airport.position.y - 2);
            pathAirportShapes.push_back(shape);
        }
    }

    int panelWidth = 300;
    int panelHeight = 150;
    int panelY = 10;
    int controlsX = 20;
    int infoX = controlsX + panelWidth + 20;
    int statusX = infoX + panelWidth + 20;

    sf::RectangleShape controlPanel(sf::Vector2f(panelWidth, panelHeight));
    controlPanel.setFillColor(sf::Color(0, 0, 0, 180));
    controlPanel.setOutlineColor(sf::Color(255, 255, 255, 80));
    controlPanel.setOutlineThickness(1);
    controlPanel.setPosition(controlsX, panelY);

    sf::RectangleShape infoPanel(sf::Vector2f(panelWidth, panelHeight));
    infoPanel.setFillColor(sf::Color(0, 0, 0, 180));
    infoPanel.setOutlineColor(sf::Color(255, 255, 255, 80));
    infoPanel.setOutlineThickness(1);
    infoPanel.setPosition(infoX, panelY);

    sf::RectangleShape statusPanel(sf::Vector2f(panelWidth, panelHeight));
    statusPanel.setFillColor(sf::Color(0, 0, 0, 200));
    statusPanel.setOutlineColor(sf::Color(255, 255, 255, 100));
    statusPanel.setOutlineThickness(2);
    statusPanel.setPosition(statusX, panelY);

    statusText.setPosition(statusX + 15, panelY + 10);
    routeText.setPosition(statusX + 15, panelY + 45);
    metricsText.setPosition(statusX + 15, panelY + 75);

    sf::Text infoTitle, info1, info2, info3, info4;
    infoTitle.setFont(mainFont);
    infoTitle.setString("Flight Details");
    infoTitle.setCharacterSize(18);
    infoTitle.setStyle(sf::Text::Bold);
    infoTitle.setFillColor(sf::Color::White);
    infoTitle.setPosition(infoX + 10, panelY + 10);

    info1.setFont(mainFont);
    info1.setString("Departure: " + graph.airports[src].code);
    info1.setCharacterSize(14);
    info1.setFillColor(sf::Color::Yellow);
    info1.setPosition(infoX + 10, panelY + 40);

    info2.setFont(mainFont);
    info2.setString("Arrival: " + graph.airports[dst].code);
    info2.setCharacterSize(14);
    info2.setFillColor(sf::Color::Yellow);
    info2.setPosition(infoX + 10, panelY + 60);

    string displayDate = bookedDate.empty() ? Date::getCurrentDate().toString() : bookedDate;
    string displayTime = bookedTime.empty() ? [](){
        auto now = chrono::system_clock::now();
        time_t now_time = chrono::system_clock::to_time_t(now);
        tm *now_tm = localtime(&now_time);
        char timeStr[10];
        strftime(timeStr, sizeof(timeStr), "%I:%M %p", now_tm);
        return std::string(timeStr);
    }() : bookedTime;

    info3.setFont(mainFont);
    info3.setString("Date: " + displayDate);
    info3.setCharacterSize(14);
    info3.setFillColor(sf::Color::Yellow);
    info3.setPosition(infoX + 10, panelY + 80);

    info4.setFont(mainFont);
    info4.setString("Time: " + displayTime);
    info4.setCharacterSize(14);
    info4.setFillColor(sf::Color::Yellow);
    info4.setPosition(infoX + 10, panelY + 100);

    auto landing_time = chrono::system_clock::now() + chrono::hours(2) + chrono::minutes(15);
    time_t landing_time_t = chrono::system_clock::to_time_t(landing_time);
    tm *landing_tm = localtime(&landing_time_t);
    char landingStr[10];
    strftime(landingStr, sizeof(landingStr), "%I:%M %p", landing_tm);

    sf::Text info5;
    info5.setFont(mainFont);
    info5.setString("Landing: " + string(landingStr));
    info5.setCharacterSize(14);
    info5.setFillColor(sf::Color::Yellow);
    info5.setPosition(infoX + 10, panelY + 120);

    sf::Text controlText, control1, control2, control3;
    controlText.setFont(mainFont);
    controlText.setString("Controls:");
    controlText.setCharacterSize(18);
    controlText.setStyle(sf::Text::Bold);
    controlText.setFillColor(sf::Color::White);
    controlText.setPosition(controlsX + 10, panelY + 10);

    control1.setFont(mainFont);
    control1.setString("Space: Pause/Play");
    control1.setCharacterSize(14);
    control1.setFillColor(sf::Color::Yellow);
    control1.setPosition(controlsX + 10, panelY + 40);

    control2.setFont(mainFont);
    control2.setString("R: Reset Animation");
    control2.setCharacterSize(14);
    control2.setFillColor(sf::Color::Yellow);
    control2.setPosition(controlsX + 10, panelY + 60);

    control3.setFont(mainFont);
    control3.setString("ESC: Exit");
    control3.setCharacterSize(14);
    control3.setFillColor(sf::Color::Yellow);
    control3.setPosition(controlsX + 10, panelY + 80);

    sf::Clock clock;
    float animationProgress = 0.0f;
    const float animationSpeed = 0.3f;
    bool animationPaused = false;

    sf::Texture airplaneTexture;
    bool airplaneIconLoaded = airplaneTexture.loadFromFile("assets/aeroplane.png");
    sf::Sprite airplaneSprite;
    if (airplaneIconLoaded)
    {
        airplaneSprite.setTexture(airplaneTexture);
        airplaneSprite.setOrigin(airplaneTexture.getSize().x / 2.f, airplaneTexture.getSize().y / 2.f);
        airplaneSprite.setScale(0.035f, 0.035f);
    }

    sf::CircleShape airplane(6, 3);
    airplane.setFillColor(sf::Color::White);
    airplane.setOrigin(6, 6);

    sf::Texture mapTexture;
    if (!mapTexture.loadFromFile("assets/map.png"))
    {
        cerr << "Error loading map.png\n";
    }
    sf::Sprite mapSprite(mapTexture);

    auto drawBoldText = [&](sf::RenderWindow &win, sf::Text &txt)
    {
        sf::Text shadow = txt;
        shadow.setFillColor(sf::Color(60, 60, 60));
        for (int dx = -1; dx <= 1; ++dx)
        {
            for (int dy = -1; dy <= 1; ++dy)
            {
                if (dx != 0 || dy != 0)
                {
                    shadow.setPosition(txt.getPosition().x + dx, txt.getPosition().y + dy);
                    win.draw(shadow);
                }
            }
        }
        txt.setFillColor(sf::Color::Black);
        win.draw(txt);
    };

    vector<float> pathLengths;
    float totalPathLength = 0.0f;
    if (!path.empty())
    {
        for (size_t i = 1; i < path.size(); ++i)
        {
            int fromIdx = path[i - 1];
            int toIdx = path[i];
            sf::Vector2f start = graph.airports[fromIdx].position;
            sf::Vector2f end = graph.airports[toIdx].position;
            float dx = end.x - start.x;
            float dy = end.y - start.y;
            float length = sqrt(dx * dx + dy * dy);
            pathLengths.push_back(length);
            totalPathLength += length;
        }
    }

    double totalCost = 0.0;
    double totalTime = 0.0;
    for (size_t i = 1; i < path.size(); ++i)
    {
        int u = path[i - 1], v = path[i];
        for (auto &e : graph.adj[u])
        {
            if (e.to == v)
            {
                totalCost += e.cost;
                totalTime += e.time;
                break;
            }
        }
    }

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            window.close();

            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape)
                window.close();
                if (event.key.code == sf::Keyboard::Space)
                animationPaused = !animationPaused;
                if (event.key.code == sf::Keyboard::R)
                animationProgress = 0.0f;
            }
        }

        float deltaTime = clock.restart().asSeconds();
        if (!path.empty() && path.size() > 1 && !animationPaused)
        {
            animationProgress += deltaTime * animationSpeed;
            if (animationProgress > 1.0f)
            {
                animationProgress = 0.0f;
            }
        }

        window.clear();
        window.draw(mapSprite);
        window.draw(controlPanel);
        window.draw(infoPanel);
        window.draw(controlText);
        window.draw(control1);
        window.draw(control2);
        window.draw(control3);
        window.draw(infoTitle);
        window.draw(info1);
        window.draw(info2);
        window.draw(info3);
        window.draw(info4);
        window.draw(info5);

        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mousePos);

        int hoveredAirport = -1;
        for (size_t i = 0; i < graph.airports.size(); ++i)
        {
            sf::Vector2f pos = graph.airports[i].position;
            float dx = mouseWorldPos.x - pos.x;
            float dy = mouseWorldPos.y - pos.y;
            if (sqrt(dx * dx + dy * dy) < 12)
            {
                hoveredAirport = i;
                break;
            }
        }

        if (hoveredAirport != -1)
        {
            sf::RectangleShape tooltipBg(sf::Vector2f(200, 80));
            tooltipBg.setFillColor(sf::Color(0, 0, 0, 220));
            tooltipBg.setOutlineColor(sf::Color(255, 255, 255, 150));
            tooltipBg.setOutlineThickness(2);
            tooltipBg.setPosition(mouseWorldPos.x + 15, mouseWorldPos.y - 15);

            sf::Text tooltipTitle;
            tooltipTitle.setFont(mainFont);
            tooltipTitle.setString(graph.airports[hoveredAirport].code);
            tooltipTitle.setCharacterSize(18);
            tooltipTitle.setStyle(sf::Text::Bold);
            tooltipTitle.setFillColor(sf::Color(255, 255, 100));
            tooltipTitle.setPosition(mouseWorldPos.x + 20, mouseWorldPos.y - 10);

            sf::Text tooltipInfo;
            tooltipInfo.setFont(mainFont);
            tooltipInfo.setString("Click for details");
            tooltipInfo.setCharacterSize(14);
            tooltipInfo.setFillColor(sf::Color::White);
            tooltipInfo.setPosition(mouseWorldPos.x + 20, mouseWorldPos.y + 10);

            window.draw(tooltipBg);
            window.draw(tooltipTitle);
            window.draw(tooltipInfo);
        }

        if (rerouted && !originalPath.empty())
        {
            for (size_t i = 1; i < originalPath.size(); ++i)
            {
                int fromIdx = originalPath[i - 1];
                int toIdx = originalPath[i];
                sf::Vector2f start = graph.airports[fromIdx].position;
                sf::Vector2f end = graph.airports[toIdx].position;
                float dashLength = 16.0f;
                float gapLength = 10.0f;
                sf::Vector2f dir = end - start;
                float totalLength = sqrt(dir.x * dir.x + dir.y * dir.y);
                sf::Vector2f unitDir = dir / totalLength;
                int numDashes = static_cast<int>(totalLength / (dashLength + gapLength));
                for (int thick = -2; thick <= 2; ++thick)
                {
                    for (int d = 0; d < numDashes; ++d)
                    {
                        float segStart = d * (dashLength + gapLength);
                        float segEnd = segStart + dashLength;
                        if (segEnd > totalLength)
                        segEnd = totalLength;
                        sf::Vector2f offset = sf::Vector2f(-unitDir.y, unitDir.x) * static_cast<float>(thick);
                        sf::Vector2f p1 = start + unitDir * segStart + offset;
                        sf::Vector2f p2 = start + unitDir * segEnd + offset;
                        sf::VertexArray dash(sf::Lines, 2);
                        dash[0] = sf::Vertex(p1, sf::Color(120, 120, 120));
                        dash[1] = sf::Vertex(p2, sf::Color(120, 120, 120));
                        window.draw(dash);
                    }
                }
            }
        }

        if (!path.empty())
        {
            for (size_t i = 1; i < path.size(); ++i)
            {
                int fromIdx = path[i - 1];
                int toIdx = path[i];
                sf::Vector2f start = graph.airports[fromIdx].position;
                sf::Vector2f end = graph.airports[toIdx].position;
                float dashLength = 16.0f;
                float gapLength = 10.0f;
                sf::Vector2f dir = end - start;
                float totalLength = sqrt(dir.x * dir.x + dir.y * dir.y);
                sf::Vector2f unitDir = dir / totalLength;
                int numDashes = static_cast<int>(totalLength / (dashLength + gapLength));
                for (int thick = -2; thick <= 2; ++thick)
                {
                    for (int d = 0; d < numDashes; ++d)
                    {
                        float segStart = d * (dashLength + gapLength);
                        float segEnd = segStart + dashLength;
                        if (segEnd > totalLength)
                        segEnd = totalLength;
                        sf::Vector2f offset = sf::Vector2f(-unitDir.y, unitDir.x) * static_cast<float>(thick);
                        sf::Vector2f p1 = start + unitDir * segStart + offset;
                        sf::Vector2f p2 = start + unitDir * segEnd + offset;
                        sf::VertexArray dash(sf::Lines, 2);
                        dash[0] = sf::Vertex(p1, sf::Color::Black);
                        dash[1] = sf::Vertex(p2, sf::Color::Black);
                        window.draw(dash);
                    }
                }
            }
        }

        if (!path.empty() && path.size() > 1)
        {
            float distanceCovered = animationProgress * totalPathLength;
            float accumulatedLength = 0.0f;
            int currentSegment = -1;
            float segmentProgress = 0.0f;
            for (size_t i = 0; i < pathLengths.size(); ++i)
            {
                if (distanceCovered <= accumulatedLength + pathLengths[i])
                {
                    currentSegment = i;
                    segmentProgress = (distanceCovered - accumulatedLength) / pathLengths[i];
                    break;
                }
                accumulatedLength += pathLengths[i];
            }
            if (currentSegment == -1)
            {
                currentSegment = pathLengths.size() - 1;
                segmentProgress = 1.0f;
            }
            if (currentSegment < path.size() - 1)
            {
                int fromIdx = path[currentSegment];
                int toIdx = path[currentSegment + 1];
                sf::Vector2f start = graph.airports[fromIdx].position;
                sf::Vector2f end = graph.airports[toIdx].position;

                for (int i = 0; i < currentSegment; ++i)
                {
                    int segFrom = path[i];
                    int segTo = path[i + 1];
                    sf::VertexArray trailLine(sf::Lines, 2);
                    trailLine[0] = sf::Vertex(graph.airports[segFrom].position, sf::Color(0, 120, 255));
                    trailLine[1] = sf::Vertex(graph.airports[segTo].position, sf::Color(0, 120, 255));
                    window.draw(trailLine);
                }

                sf::Vector2f airplanePos = start + (end - start) * segmentProgress;
                sf::VertexArray partialTrail(sf::Lines, 2);
                partialTrail[0] = sf::Vertex(start, sf::Color(0, 120, 255));
                partialTrail[1] = sf::Vertex(airplanePos, sf::Color(0, 120, 255));
                window.draw(partialTrail);

                sf::Vector2f direction = end - start;
                float angle = atan2(direction.y, direction.x) * 180 / 3.14159f;

                if (airplaneIconLoaded)
                {
                    airplaneSprite.setPosition(airplanePos);
                    airplaneSprite.setRotation(angle - 90);
                    window.draw(airplaneSprite);
                }
                else
                {
                    airplane.setPosition(airplanePos);
                    airplane.setRotation(angle - 90);
                    window.draw(airplane);
                }
            }
        }

        if (!exploredEdges.empty())
        {
            for (const auto &edge : exploredEdges)
            {
                int u = edge.first, v = edge.second;
                sf::VertexArray exploredLine(sf::Lines, 2);
                exploredLine[0] = sf::Vertex(graph.airports[u].position, sf::Color(255, 140, 0, 120));
                exploredLine[1] = sf::Vertex(graph.airports[v].position, sf::Color(255, 140, 0, 120));
                window.draw(exploredLine);
            }
        }

        sf::Text statusTitle;
        statusTitle.setFont(mainFont);
        statusTitle.setString(rerouted ? "REROUTED FLIGHT PATH" : "ORIGINAL FLIGHT PATH");
        statusTitle.setCharacterSize(32);
        statusTitle.setStyle(sf::Text::Bold);
        statusTitle.setFillColor(sf::Color(80, 255, 120));
        statusTitle.setPosition(statusX + 10, panelY + 10);

        sf::Text routeLabel;
        routeLabel.setFont(mainFont);
        routeLabel.setString("Route:");
        routeLabel.setCharacterSize(20);
        routeLabel.setStyle(sf::Text::Bold);
        routeLabel.setFillColor(sf::Color(60, 120, 255));
        routeLabel.setPosition(statusX + 10, panelY + 55);

        vector<sf::RectangleShape> routeBoxes;
        vector<sf::Text> routeCodes;
        float routeStartX = statusX + 90;
        float routeY = panelY + 52;
        float boxW = 48, boxH = 32, boxPad = 8;
        for (size_t i = 0; i < path.size(); ++i)
        {
            sf::RectangleShape box(sf::Vector2f(boxW, boxH));
            box.setFillColor(sf::Color(60, 120, 255));
            box.setOutlineColor(sf::Color(30, 60, 120));
            box.setOutlineThickness(2);
            float boxX = routeStartX + i * (boxW + boxPad);
            float boxY = routeY;
            box.setPosition(boxX, boxY);
            routeBoxes.push_back(box);

            sf::Text code;
            code.setFont(mainFont);
            code.setString(graph.airports[path[i]].code);
            code.setCharacterSize(20);
            code.setStyle(sf::Text::Bold);
            code.setFillColor(sf::Color::White);

            sf::FloatRect textRect = code.getLocalBounds();
            code.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
            code.setPosition(boxX + boxW / 2.0f, boxY + boxH / 2.0f);

            routeCodes.push_back(code);
        }

        sf::Text metricsText;
        ostringstream metricsSS;
        metricsSS << "Distance: " << std::fixed << std::setprecision(2) << totalPathLength << " km    "
                  << "Cost: $" << totalCost << "    "
                  << "Duration: " << (int)totalTime << " min    "
                  << "Computation: " << (int)us << " ms";
        metricsText.setFont(mainFont);
        metricsText.setString(metricsSS.str());
        metricsText.setCharacterSize(20);
        metricsText.setFillColor(sf::Color(80, 80, 80));
        metricsText.setPosition(statusX + 10, panelY + 95);

        window.draw(statusTitle);
        window.draw(routeLabel);
        for (auto &box : routeBoxes)
        window.draw(box);
        for (auto &code : routeCodes)
        {
            code.setStyle(sf::Text::Bold);
            window.draw(code);
        }
        window.draw(metricsText);

        window.display();
    }
}

string getWeatherDescription(double lat, double lon, const string &apiKey)
{
    ostringstream url;
    url << "http://api.openweathermap.org/data/2.5/weather?lat=" << lat
        << "&lon=" << lon
        << "&appid=" << apiKey
        << "&units=metric";

    cpr::Response r = cpr::Get(cpr::Url{url.str()});
    if (r.status_code == 200)
    {
        nlohmann::json j = nlohmann::json::parse(r.text);
        string main = j["weather"][0]["main"].get<string>();
        string desc = j["weather"][0]["description"].get<string>();
        double temp = j["main"]["temp"];
        ostringstream oss;
        oss << main << " (" << desc << "), " << temp << " C";
        return oss.str();
    }
    else
    {
        return "[Weather unavailable]";
    }
}

bool isBadWeather(const string &mainWeather)
{
    string mw = mainWeather;
    transform(mw.begin(), mw.end(), mw.begin(), ::tolower);
    return mw == "thunderstorm" || mw == "snow" || mw == "tornado" || mw == "squall" || mw == "ash" || mw == "sand" || mw == "dust" || mw == "heavy rain" || mw == "rain" || mw == "extreme";
}

string convertToAPIDate(const std::string &dateStr)
{
    int d, m, y;
    char sep;
    istringstream iss(dateStr);
    iss >> d >> sep >> m >> sep >> y;
    ostringstream oss;
    oss << y << "-";
    if (m < 10)
    oss << "0";
    oss << m << "-";
    if (d < 10)
    oss << "0";
    oss << d;
    return oss.str();
}

struct DetailedWeather
{
    string main;
    string desc;
    double temp;
    int humidity;
    double wind;
};

auto getForecastWeather = [](double lat, double lon, const string &apiKey, const string &date, const string &time) -> DetailedWeather
{
    std::ostringstream url;
    url << "http://api.openweathermap.org/data/2.5/forecast?lat=" << lat
        << "&lon=" << lon
        << "&appid=" << apiKey
        << "&units=metric";
    cpr::Response r = cpr::Get(cpr::Url{url.str()});
    if (r.status_code == 200)
    {
        nlohmann::json j = nlohmann::json::parse(r.text);
        std::tm flight_tm = {};

        sscanf(date.c_str(), "%d/%d/%d", &flight_tm.tm_mday, &flight_tm.tm_mon, &flight_tm.tm_year);
        flight_tm.tm_year -= 1900;
        flight_tm.tm_mon -= 1;
        sscanf(time.c_str(), "%d:%d", &flight_tm.tm_hour, &flight_tm.tm_min);
        flight_tm.tm_sec = 0;

        time_t flight_time = mktime(&flight_tm);
        time_t min_diff = LLONG_MAX;
        nlohmann::json bestEntry;
        for (const auto &entry : j["list"])
        {
            string dt_txt = entry["dt_txt"];
            tm entry_tm = {};
            sscanf(dt_txt.c_str(), "%d-%d-%d %d:%d:%d", &entry_tm.tm_year, &entry_tm.tm_mon, &entry_tm.tm_mday, &entry_tm.tm_hour, &entry_tm.tm_min, &entry_tm.tm_sec);
            entry_tm.tm_year -= 1900;
            entry_tm.tm_mon -= 1;
            time_t entry_time = mktime(&entry_tm);
            time_t diff = abs(entry_time - flight_time);
            if (diff < min_diff)
            {
                min_diff = diff;
                bestEntry = entry;
            }
        }

        if (!bestEntry.is_null())
        {
            string main = bestEntry["weather"][0]["main"].get<string>();
            string desc = bestEntry["weather"][0]["description"].get<string>();
            double temp = bestEntry["main"]["temp"];
            int humidity = bestEntry["main"]["humidity"];
            double wind = bestEntry["wind"]["speed"];
            return {main, desc, temp, humidity, wind};
        }
    }
    return {"--", "--", -1, -1, -1};
};

double haversine(double lat1, double lon1, double lat2, double lon2)
{
    const double R = 6371.0;
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;
    double a = sin(dLat / 2) * sin(dLat / 2) + cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) * sin(dLon / 2) * sin(dLon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return R * c;
}

int main(int argc, char *argv[])
{
    ios_base::sync_with_stdio(true);

    int src = -1, dst = -1;
    bool useCommandLineArgs = false;

    if (argc >= 3)
    {
        try
        {
            src = stoi(argv[1]);
            dst = stoi(argv[2]);
            useCommandLineArgs = true;
            cout << "Using Command Line Arguments : " << endl;
            cout << "   START = " << src << endl;
            cout << "   DESTINATION = " << dst << endl;

            if (argc >= 5)
            {
                string bookedDate = argv[3];
                string bookedTime = argv[4];
                cout << "BOOKED FLIGHT : " << bookedDate << " AT " << bookedTime << endl;
            }
        }
        catch (const exception &e)
        {
            cerr << "Error parsing Command Line Arguments : " << e.what() << endl;
            useCommandLineArgs = false;
        }
    }

    FlightGraph graph;

    vector<Airport> airports = {

        {"SEA", "SEATTLE TACOMA INTERNATIONAL AIRPORT", sf::Vector2f(225, 269), 47.4502, -122.3088},
        {"PDX", "PORTLAND INTERNATIONAL AIRPORT", sf::Vector2f(153, 369), 45.5898, -122.5951},
        {"SFO", "SAN FRANCISCO INTERNATIONAL AIRPORT", sf::Vector2f(82, 586), 37.6213, -122.3790},
        {"LAS", "HARRY REID INTERNATIONAL AIRPORT", sf::Vector2f(250, 624), 36.0840, -115.1537},
        {"SLC", "SALT LAKE CITY INTERNATIONAL AIRPORT", sf::Vector2f(378, 558), 40.7899, -111.9791},
        {"LAX", "LOS ANGELES INTERNATIONAL AIRPORT", sf::Vector2f(166, 721), 33.9416, -118.4085},
        {"MSP", "MINNEAPOLIS SAINT PAUL INTERNATIONAL AIRPORT", sf::Vector2f(831, 405), 44.8848, -93.2223},
        {"MCI", "KANSAS CITY INTERNATIONAL AIRPORT", sf::Vector2f(742, 642), 39.2976, -94.7139},
        {"JFK", "JOHN F. KENNEDY INTERNATIONAL AIRPORT", sf::Vector2f(1324, 511), 40.6413, -73.7781},
        {"BOS", "BOSTON LOGAN INTERNATIONAL AIRPORT", sf::Vector2f(1393, 448), 42.3656, -71.0096},
        {"CLE", "CLEVELAND HOPKINS INTERNATIONAL AIRPORT", sf::Vector2f(1150, 550), 41.4117, -81.8498},
        {"CLT", "CHARLOTTE DOUGLAS INTERNATIONAL AIRPORT", sf::Vector2f(1237, 727), 35.2140, -80.9431},
        {"MIA", "MIAMI INTERNATIONAL AIRPORT", sf::Vector2f(1259, 1016), 25.7959, -80.2870},
        {"IAH", "GEORGE BUSH INTERCONTINENTAL AIRPORT", sf::Vector2f(786, 962), 29.9902, -95.3368},
        {"STL", "ST. LOUIS LAMBERT INTERNATIONAL AIRPORT", sf::Vector2f(905, 647), 38.7500, -90.3700},
        {"TPA", "TAMPA INTERNATIONAL AIRPORT", sf::Vector2f(1160, 991), 27.9755, -82.5332},
        {"DEN", "DENVER INTERNATIONAL AIRPORT", sf::Vector2f(549, 616), 39.8561, -104.6737},
        {"PHX", "PHOENIX SKY HARBOR INTERNATIONAL AIRPORT", sf::Vector2f(342, 778), 33.4373, -112.0078},
        {"DFW", "DALLAS/FORT WORTH INTERNATIONAL AIRPORT", sf::Vector2f(756, 853), 32.8998, -97.0403},
        {"AUG", "AUGUSTA STATE AIRPORT", sf::Vector2f(1413, 331), 44.3206, -69.7973}
    };

    for (const auto &ap : airports)
    {
        graph.addAirport(ap.code, ap.position.x, ap.position.y, ap.latitude, ap.longitude);
    }

    int n = airports.size();

    for (int i = 0; i < n; ++i)
    {
        for (int j = i + 1; j < n; ++j)
        {
            const auto &pi = graph.airports[i].position;
            const auto &pj = graph.airports[j].position;
            double dx = pi.x - pj.x;
            double dy = pi.y - pj.y;
            double dist = sqrt(dx * dx + dy * dy);

            if (dist > 800)
            continue;

            string code1 = graph.airports[i].code;
            string code2 = graph.airports[j].code;

            if ((code1 == "STL" && code2 == "SEA") || (code1 == "SEA" && code2 == "STL"))
            continue;
            if ((code1 == "TPA" && code2 == "SEA") || (code1 == "SEA" && code2 == "TPA"))
            continue;
            if ((code1 == "MIA" && code2 == "SEA") || (code1 == "SEA" && code2 == "MIA"))
            continue;
            if ((code1 == "BOS" && code2 == "LAX") || (code1 == "LAX" && code2 == "BOS"))
            continue;
            if ((code1 == "CLE" && code2 == "LAX") || (code1 == "LAX" && code2 == "CLE"))
            continue;

            double cost = 100 + (rand() % 200);
            double time = 30 + (rand() % 120);
            double distance = haversine(graph.airports[i].latitude, graph.airports[i].longitude, graph.airports[j].latitude, graph.airports[j].longitude);
            double avgSpeed = 800.0;
            double duration = distance / avgSpeed * 60.0;
            if (duration < 10.0)
            duration = 10.0;
            graph.addEdge(i, j, dist, cost, duration);
        }
    }

    printLine('=');
    cout << "WELCOME TO FLIGHT SIMULATOR" << endl;
    printLine('=');

    if (!useCommandLineArgs)
    {
        cout << "Available Airports [ index : code ] :" << endl;
        for (int i = 0; i < n; ++i)
        {
            cout << "  " << i << ": " << graph.airports[i].code << endl;
        }
        printLine();

        string input;
        cout << "FLIGHT SELECTION" << endl;
        printLine();

        do
        {
            cout << "Enter Departure index or code : ";
            cout.flush();
            cin >> input;
            src = resolveAirportIndex(input, graph.airports);

            if (src < 0 || src >= n)
            {
                cout << "Invalid Airport. Please try again." << endl;
                cout.flush();
            }
        } while (src < 0 || src >= n);

        do
        {
            cout << "Enter Arrival index or code :   ";
            cout.flush();
            cin >> input;
            dst = resolveAirportIndex(input, graph.airports);

            if (dst < 0 || dst >= n)
            {
                cout << "Invalid Airport. Please try again." << endl;
                cout.flush();
            }
        } while (dst < 0 || dst >= n);
    }

    if (useCommandLineArgs)
    {
        if (src < 0 || src >= n || dst < 0 || dst >= n)
        {
            cerr << "Invalid Airport indices provided via Command Line." << endl;
            cerr << "Valid range is 0 to " << (n - 1) << endl;
            cerr << "Received : START = " << src
                 << "DESTINATION = " << dst << endl;
            return 1;
        }
        cout << "Successfully loaded Route : " << graph.airports[src].code << " -> " << graph.airports[dst].code << endl;
    }

    cout << "Selected Route : " << graph.airports[src].code << " to " << graph.airports[dst].code << endl;

    FlightGraph allOpenGraph = graph;
    for (int i = 0; i < allOpenGraph.airports.size(); ++i)
    {
        for (int j = 0; j < allOpenGraph.airports.size(); ++j)
        {
            if (i != j)
            {
                allOpenGraph.pathAvailable[i][j] = true;
            }
        }
    }
    vector<int> originalPath = allOpenGraph.dijkstra(src, dst);

    cout << "Path : ";
    for (size_t k = 0; k < originalPath.size(); ++k)
    {
        cout << graph.airports[originalPath[k]].code << " ";
    }
    cout << endl;

    const int segW = 12, dateW = 12, depTimeW = 8, arrTimeW = 8, weatherW = 10, tempW = 10, humW = 8, windW = 10;
    int totalTableWidth = segW + dateW + depTimeW + arrTimeW + weatherW * 2 + tempW * 2 + humW * 2 + windW * 2 + 10 * 3;
    printLine('=', totalTableWidth);
    cout << "                                 WEATHER AT DEPARTURE AND ARRIVAL AIRPORTS FOR EACH SEGMENT " << endl;
    printLine('-', totalTableWidth);
    cout << left << setw(segW) << "Segment" << " | "
         << setw(dateW) << "Date" << " | "
         << setw(depTimeW) << "Dep" << " | "
         << setw(8) << "Duration" << " | "
         << setw(weatherW) << "D.Weather" << " | "
         << setw(tempW) << "D.Temp" << "  | "
         << setw(humW) << "D.Hum" << " | "
         << setw(windW) << "D.Wind" << "   | "
         << setw(weatherW) << "A.Weather" << " | "
         << setw(tempW) << "A.Temp" << "  | "
         << setw(humW) << "A.Hum" << " | "
         << setw(windW) << "A.Wind" << endl;
    printLine('-', totalTableWidth);

    chrono::system_clock::time_point currentTime;
    string initialDepTimeStr;
    if (argc >= 5)
    {
        std::tm tm = {};
        sscanf(argv[3], "%d/%d/%d", &tm.tm_mday, &tm.tm_mon, &tm.tm_year);
        tm.tm_year -= 1900;
        tm.tm_mon -= 1;
        sscanf(argv[4], "%d:%d", &tm.tm_hour, &tm.tm_min);
        tm.tm_sec = 0;
        time_t t = mktime(&tm);
        currentTime = chrono::system_clock::from_time_t(t);
        initialDepTimeStr = argv[4];
    }
    else
    {
        auto now = chrono::system_clock::now();
        currentTime = now + chrono::minutes(15);
        time_t t = chrono::system_clock::to_time_t(currentTime);
        tm *tm_now = localtime(&t);
        char buf[6];
        snprintf(buf, sizeof(buf), "%02d:%02d", tm_now->tm_hour, tm_now->tm_min);
        initialDepTimeStr = buf;
    }

    bool rerouted = false;

    vector<int> tempPath = graph.dijkstra(src, dst);
    for (size_t i = 0; i + 1 < tempPath.size(); ++i)
    {
        int u = tempPath[i];
        int v = tempPath[i + 1];
        double timeOfFlight = 0.0;
        double distance = haversine(graph.airports[u].latitude, graph.airports[u].longitude, graph.airports[v].latitude, graph.airports[v].longitude);
        for (const auto &e : graph.adj[u])
        {
            if (e.to == v)
            {
                timeOfFlight = e.time;
                break;
            }
        }

        cout << "Segment : " << graph.airports[u].code << " -> " << graph.airports[v].code << " || Distance : " << distance << " km || Time Of Flight : " << timeOfFlight << " min" << std::endl;
        auto depTimePoint = currentTime;
        using clock = chrono::system_clock;
        auto arrTimePoint = depTimePoint + chrono::duration_cast<clock::duration>(chrono::duration<double>(timeOfFlight * 60));

        time_t dep_t = chrono::system_clock::to_time_t(depTimePoint);
        time_t arr_t = chrono::system_clock::to_time_t(arrTimePoint);
        tm *dep_tm = localtime(&dep_t);
        tm *arr_tm = localtime(&arr_t);
        char dateBuf[11], depBuf[6], arrBuf[6];
        snprintf(dateBuf, sizeof(dateBuf), "%02d/%02d/%04d", dep_tm->tm_mday, dep_tm->tm_mon + 1, dep_tm->tm_year + 1900);
        if (i == 0 && argc >= 5)
        {
            strncpy(depBuf, initialDepTimeStr.c_str(), sizeof(depBuf));
            depBuf[sizeof(depBuf) - 1] = '\0';
        }
        else
        {
            snprintf(depBuf, sizeof(depBuf), "%02d:%02d", dep_tm->tm_hour, dep_tm->tm_min);
        }
        snprintf(arrBuf, sizeof(arrBuf), "%02d:%02d", arr_tm->tm_hour, arr_tm->tm_min);
        string bookedDate = dateBuf;
        string depTime = depBuf;
        string arrTime = arrBuf;

        DetailedWeather depWeather = getForecastWeather(graph.airports[u].latitude, graph.airports[u].longitude, apiKey, bookedDate, depTime);
        DetailedWeather arrWeather = getForecastWeather(graph.airports[v].latitude, graph.airports[v].longitude, apiKey, bookedDate, arrTime);
        bool badDep = isBadWeather(depWeather.main);
        bool badArr = isBadWeather(arrWeather.main);
        if (badDep || badArr)
        {
            string desc = (badDep ? depWeather.main : "") + (badDep && badArr ? ", " : "") + (badArr ? arrWeather.main : "");
            graph.updateWeather(u, v, true, desc);
        }
        else
        {
            graph.updateWeather(u, v, false, "Clear");
        }
        cout << left << setw(segW) << (graph.airports[u].code + "-" + graph.airports[v].code) << " | "
             << setw(dateW) << bookedDate << " | "
             << setw(depTimeW) << depTime << " | "
             << setw(8) << chrono::duration_cast<std::chrono::minutes>(arrTimePoint - depTimePoint).count() << " | "
             << setw(weatherW) << depWeather.main << " | "
             << setw(tempW) << (depWeather.temp == -1 ? "--" : (to_string(depWeather.temp) + " C")) << " | "
             << setw(humW) << (depWeather.humidity == -1 ? "--" : to_string(depWeather.humidity) + "%") << " | "
             << setw(windW) << (depWeather.wind == -1 ? "--" : to_string(depWeather.wind) + " m/s") << " | "
             << setw(weatherW) << arrWeather.main << " | "
             << setw(tempW) << (arrWeather.temp == -1 ? "--" : (to_string(arrWeather.temp) + " C")) << " | "
             << setw(humW) << (arrWeather.humidity == -1 ? "--" : to_string(arrWeather.humidity) + "%") << " | "
             << setw(windW) << (arrWeather.wind == -1 ? "--" : to_string(arrWeather.wind) + " m/s") << endl;

        currentTime = arrTimePoint;
    }
    printLine('-', totalTableWidth);
    cout << "Note : Weather Data is based on the closest Available Forecast for Each Segment." << endl;

    vector<pair<string, vector<int>>> weatherSafePaths;
    vector<pair<string, string>> metricNames = {{"distance", "Shortest"}, {"cost", "Cheapest"}, {"time", "Fastest"}};
    for (const auto &[metric, label] : metricNames)
    {
        vector<pair<int, int>> explored;
        vector<int> path = graph.dijkstra(src, dst, explored, metric);
        weatherSafePaths.push_back({metric, path});
    }

    bool reroutedForShortest = false;
    vector<int> originalPathForVis = allOpenGraph.dijkstra(src, dst);
    vector<int> reroutedPathForVis = weatherSafePaths[0].second;

    if (originalPathForVis != reroutedPathForVis)
    {
        reroutedForShortest = true;
        printLine('-');
        cout << "PATH REROUTED DUE TO BAD WEATHER" << endl;
    }

    cout << "\n==============================\n";
    cout << "  Path Suggestion Menu\n";
    cout << "==============================\n";
    cout << "1) Shortest Path [Distance]\n";
    cout << "2) Cheapest Path [Cost]\n";
    cout << "3) Fastest Path  [Time]\n";
    cout << "------------------------------\n";
    cout << "Enter choice [ 1-3, default 1 ] : \n";

    for (size_t i = 0; i < weatherSafePaths.size(); ++i)
    {
        const auto &[metric, path] = weatherSafePaths[i];
        const auto &label = metricNames[i].second;

        double total = 0.0;
        for (size_t j = 1; j < path.size(); ++j)
        {
            int u = path[j - 1], v = path[j];
            for (const auto &e : graph.adj[u])
            {
                if (e.to == v)
                {
                    if (metric == "distance")
                    total += e.distance;
                    else if (metric == "cost")
                    total += e.cost;
                    else
                    total += e.time;
                    break;
                }
            }
        }
        cout << label << "Path : ";

        if (path.empty())
        {
            cout << "[NO VALID PATH DUE TO WEATHER]";
        }
        else
        {
            for (int idx : path)
            cout << graph.airports[idx].code << " ";
            if (metric == "distance")
            cout << "| Length: " << total;
            else if (metric == "cost")
            cout << "| Cost: $" << total;
            else
            cout << "| Time: " << total << " min";
        }
        cout << endl;
    }

    cout << "\nWhich Path would you like to Visualize?\n";
    cout << "1) Shortest [Distance]\n";
    cout << "2) Cheapest [Cost]\n";
    cout << "3) Fastest  [Time]\n";
    cout << "Enter choice [ 1-3, default 1 ] : ";
    int metricChoice = 1;
    string metricInput;
    getline(cin >> ws, metricInput);
    if (!metricInput.empty() && (metricInput[0] == '2' || metricInput[0] == '3'))
    metricChoice = metricInput[0] - '0';

    
    vector<int> path;
    vector<pair<int, int>> exploredEdges;
    if (metricChoice >= 1 && metricChoice <= 3)
    {
        path = weatherSafePaths[metricChoice - 1].second;
        string selectedMetric = weatherSafePaths[metricChoice - 1].first;
        graph.dijkstra(src, dst, exploredEdges, selectedMetric);
    }
    else
    {
        path = weatherSafePaths[0].second; 
        graph.dijkstra(src, dst, exploredEdges, "distance");
    }

    if (path.empty())
    {
        cout << "\nERROR : The selected path is not Available due to Weather Conditions !" << endl;
        cout << "Please select a different path or try again later." << endl;
        return 1;
    }

    cout << "\n==============================\n";
    cout << "  PathFinding Algorithm Menu\n";
    cout << "==============================\n";
    cout << "1) Dijkstra      -   FAST, CLASSIC SHORTEST PATH\n";
    cout << "2) A* (A-Star)   -   USES HEURISTIC, OFTEN FASTER\n";
    cout << "3) Bellman-Ford  -   HANDLES NEGATIVE WEIGHTS\n";
    cout << "------------------------------\n";
    cout << "Enter choice [ 1-3, default 1 ] : ";

    int algo = 1;
    string algoInput;
    getline(cin >> ws, algoInput);

    if (!algoInput.empty() && (algoInput[0] == '2' || algoInput[0] == '3'))
    algo = algoInput[0] - '0';

    using namespace std::chrono;
    double totalCost = 0.0;
    double totalLength = 0.0;
    double totalTime = 0.0;
    auto t1 = high_resolution_clock::now();
    auto t2 = high_resolution_clock::now();
    double us = duration_cast<microseconds>(t2 - t1).count();
 
    for (size_t i = 1; i < path.size(); ++i)
    {
        int u = path[i - 1], v = path[i];
        for (auto &e : graph.adj[u])
        {
            if (e.to == v)
            {
                totalLength += e.distance;
                totalCost += e.cost;
                totalTime += e.time;
                break;
            }
        }
    }
    
    ostringstream metrics;
    metrics << "\nLength: " << totalLength << "   Cost: $" << totalCost << "   Time: " << totalTime << " min   Computation: " << us << " μs";

    string bookedDate, bookedTime;
    if (argc >= 5)
    {
        bookedDate = argv[3];
        bookedTime = argv[4];
    }

    visualizeGraph(graph, path, originalPathForVis, reroutedForShortest, exploredEdges, metrics.str(), src, dst, totalTime, bookedDate, bookedTime);

    if (!bookedDate.empty() && !bookedTime.empty())
    {
        auto depWeather = getForecastWeather(graph.airports[src].latitude, graph.airports[src].longitude, apiKey, bookedDate, bookedTime);
        auto arrWeather = getForecastWeather(graph.airports[dst].latitude, graph.airports[dst].longitude, apiKey, bookedDate, bookedTime);
        cout << "Weather at " << graph.airports[src].code << " [ " << bookedDate << ", " << bookedTime << " ] : " << depWeather.main << " (" << depWeather.desc << ", " << depWeather.temp << " deg C)" << endl;
        cout << "Weather at " << graph.airports[dst].code << " [ " << bookedDate << ", " << bookedTime << " ] : " << arrWeather.main << " (" << arrWeather.desc << ", " << arrWeather.temp << " deg C)" << endl;
    }
    else
    {
        cout << "No Booked date/time provided. Run with booking arguments for accurate weather forecast." << endl;
    }
    return 0;
}
