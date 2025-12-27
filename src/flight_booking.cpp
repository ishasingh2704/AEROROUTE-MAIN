#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <ctime>
#include <random>
#include <fstream>
#include <chrono>
#include <sstream>
#include <algorithm>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <queue>
#include <unordered_map>
#include <climits>
#include <cmath>
#include "config.h"
using namespace std;

string apiKey = OPENWEATHERMAP_API_KEY;

#ifndef OPENWEATHERMAP_API_KEY
#error "API key not defined! Please create config.h and define OPENWEATHERMAP_API_KEY."
#endif

struct Airport
{
    string code;
    string name;
    double latitude;
    double longitude;
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

    Date addDays(int days) const
    {
        Date newDate = *this;
        newDate.day += days;

        if (newDate.day > 28)
        {
            newDate.day = newDate.day % 28;
            newDate.month += 1;
            if (newDate.month > 12)
            {
                newDate.month = 1;
                newDate.year += 1;
            }
        }
        return newDate;
    }
};

struct FlightTicket
{
    string departureAirportCode;
    string departureAirportName;
    string arrivalAirportCode;
    string arrivalAirportName;
    string departureDate;
    string departureTime;
    string arrivalTime;
    double price;
    string passengerName;
    string seatNumber;
    string bookingReference;
    bool isBooked;
};

string generateRandomTime()
{
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> hour_dist(0, 23);
    uniform_int_distribution<> minute_dist(0, 59);

    int hour = hour_dist(gen);
    int minute = minute_dist(gen);

    stringstream ss;
    ss << setfill('0') << setw(2) << hour << ":" << setfill('0') << setw(2) << minute;

    return ss.str();
}



pair<string, string> generateFlightTimes(double distance)
{
    random_device rd;
    mt19937 gen(rd());

    string departureTime = generateRandomTime();

    int durationHours = static_cast<int>(distance / 100) + 1;
    uniform_int_distribution<> minute_dist(0, 59);
    int durationMinutes = minute_dist(gen);

    int depHour, depMinute;
    sscanf(departureTime.c_str(), "%d:%d", &depHour, &depMinute);

    int arrHour = depHour + durationHours;
    int arrMinute = depMinute + durationMinutes;

    if (arrMinute >= 60)
    {
        arrHour += 1;
        arrMinute -= 60;
    }

    if (arrHour >= 24)
    {
        arrHour -= 24;
    }

    stringstream ss;
    ss << setfill('0') << setw(2) << arrHour << ":" << setfill('0') << setw(2) << arrMinute;

    return {departureTime, ss.str()};
}

double generateRandomPrice(double distance, int dayOffset)
{
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dist(0.8, 1.2);

    double basePrice = distance * 0.5;
    double discount = 1.0 - dayOffset * 0.07;
    if (discount < 0.7)
        discount = 0.7;
    return basePrice * dist(gen) * discount;
}

string generateSeatNumber()
{
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> row_dist(1, 30);
    uniform_int_distribution<> col_dist(0, 5);

    int row = row_dist(gen);
    char col = 'A' + col_dist(gen);

    stringstream ss;
    ss << row << col;
    return ss.str();
}

string generateBookingReference()
{
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> char_dist(0, 25);
    uniform_int_distribution<> num_dist(0, 9);

    stringstream ss;

    for (int i = 0; i < 2; ++i)
    {
        ss << static_cast<char>('A' + char_dist(gen));
    }

    for (int i = 0; i < 4; ++i)
    {
        ss << num_dist(gen);
    }

    return ss.str();
}

void printLine(char c = '-', int length = 50)
{
    for (int i = 0; i < length; i++)
    {
        cout << c;
    }
    cout << endl;
}

void generateTicketFile(const FlightTicket &ticket)
{
    string txtFilename = "ticket_" + ticket.bookingReference + ".txt";
    string htmlFilename = "ticket_" + ticket.bookingReference + ".html";

    ofstream txtFile(txtFilename);
    if (txtFile.is_open())
    {
        txtFile << "╔═══════════════════════════════════════════════════════════════════════════════╗" << endl;
        txtFile << "║                    ✈️  FLIGHT TICKET ✈️                                      ║" << endl;
        txtFile << "╠═══════════════════════════════════════════════════════════════════════════════╣" << endl;
        txtFile << "║                                                                               ║" << endl;
        txtFile << "║  🎫 Booking Reference: " << setw(35) << left << ticket.bookingReference << "  ║" << endl;
        txtFile << "║  👤 Passenger Name: " << setw(37) << left << ticket.passengerName << "        ║" << endl;
        txtFile << "║                                                                               ║" << endl;
        txtFile << "║  ┌───────────────── FLIGHT DETAILS ─────────────────┐                         ║" << endl;
        txtFile << "║  │                                                 │                          ║" << endl;
        txtFile << "║  │  🛫 From: " << setw(35) << left << ticket.departureAirportCode << " │      ║" << endl;
        txtFile << "║  │  🛬 To: " << setw(37) << left << ticket.arrivalAirportCode << " │          ║" << endl;
        txtFile << "║  │  📅 Date: " << setw(35) << left << ticket.departureDate << " │             ║" << endl;
        txtFile << "║  │  🕐 Departure: " << setw(31) << left << ticket.departureTime << " │        ║" << endl;
        txtFile << "║  │  🕐 Arrival: " << setw(33) << left << ticket.arrivalTime << " │            ║" << endl;
        txtFile << "║  │                                                 │                           ║" << endl;
        txtFile << "║  │  💺 Seat: " << setw(37) << left << ticket.seatNumber << " │                ║" << endl;
        txtFile << "║  │  💰 Price: $" << setw(34) << left << fixed << setprecision(2) << ticket.price << " │        ║" << endl;
        txtFile << "║  └─────────────────────────────────────────────────┘                           ║" << endl;
        txtFile << "║                                                                                ║" << endl;
        txtFile << "║  ╭───────────────── IMPORTANT NOTICE ─────────────────╮                        ║" << endl;
        txtFile << "║  │                                                 │                           ║" << endl;
        txtFile << "║  │  ⏰ Please arrive at least 2 hours before       │                           ║" << endl;
        txtFile << "║  │     departure time for check-in.                │                           ║" << endl;
        txtFile << "║  │  📱 Keep this ticket handy for boarding         │                           ║" << endl;
        txtFile << "║  │  🎒 Have your ID ready for verification         │                           ║" << endl;
        txtFile << "║  │                                                 │                           ║" << endl;
        txtFile << "║  ╰─────────────────────────────────────────────────╯                           ║" << endl;
        txtFile << "║                                                                                ║" << endl;
        txtFile << "║  🎉 Thank you for choosing our airline! 🎉                                    ║" << endl;
        txtFile << "║  Have a safe and pleasant journey! ✈️                                         ║" << endl;
        txtFile << "║                                                                                ║" << endl;
        txtFile << "╚════════════════════════════════════════════════════════════════════════════════╝" << endl;

        txtFile.close();
    }

    ofstream htmlFile(htmlFilename);
    if (htmlFile.is_open())
    {
        string gate = "12";
        string group = "D";
        string flight = ticket.bookingReference;
        string fromCity = ticket.departureAirportName;
        string toCity = ticket.arrivalAirportName;
        string date = ticket.departureDate;
        string boardingTime = ticket.departureTime;
        string seat = ticket.seatNumber;
        string passenger = ticket.passengerName;
        htmlFile << "<!DOCTYPE html>\n";
        htmlFile << "<html lang=\"en\">\n";
        htmlFile << "<head>\n";
        htmlFile << "<meta charset=\"UTF-8\" />\n";
        htmlFile << "<title>Boarding Pass</title>\n";
        htmlFile << "<link href=\"https://fonts.googleapis.com/css?family=Quicksand:400,700&display=swap\" rel=\"stylesheet\">\n";
        htmlFile << "<style>\n";
        htmlFile << "body { background:rgb(251, 247, 255); font-family: \"Quicksand\", Arial, sans-serif; margin: 0; padding: 20px; border-black; border-radius: 20px; }\n";
        htmlFile << ".boarding-pass { width: 950px; height: 370px; background: #fff; border-radius: 24px; box-shadow: 0 2px 12px rgba(0,0,0,0.10); margin: auto; border: 2px solid #222; position: relative; overflow: hidden; display: flex; flex-direction: column; }\n";
        htmlFile << ".header { width: 100%; height: 48px; background:rgb(103, 74, 192); display: flex; align-items: center; justify-content: space-between; position: absolute; top: 0; left: 0; z-index: 2; border-bottom: 2px solid rgb(97, 47, 236); border-top-left-radius: 24px; border-top-right-radius: 24px;}\n";
        htmlFile << ".header-title { font-size: 28px; font-family: 'Quicksand', Arial, sans-serif; font-weight: 400; color: #fff; margin-left: 80px; border-radius: 20px;}\n";
        htmlFile << ".header-title.left { margin-left: 32px; }\n";
        htmlFile << ".header-title.right { margin-right: 32px; }\n";
        htmlFile << ".main { display: flex; flex-direction: row; height: 100%; margin-top: 48px; }\n";
        htmlFile << ".left { width: 60px; background:rgb(206, 199, 227); display: flex; flex-direction: column; align-items: center; justify-content: center; border-right: 2px dashed rgb(97, 47, 236); position: relative; }\n";
        htmlFile << ".airline-vertical { writing-mode: vertical-lr; text-orientation: mixed;background:rgb(206, 199, 227); font-size: 22px; font-weight: 700; letter-spacing: 3px; color: #222; margin-top: 18px; margin-bottom: 18px; }\n";
        htmlFile << ".vertical-dashed { width: 0; border-left: 2px dashed rgb(97, 47, 236); height: 100%; margin: 0; position: relative; left: -1px; }\n";
        htmlFile << ".center { flex: 2.5; display: flex; flex-direction: row; align-items: flex-start; position: relative; padding: 18px 0 0 18px; }\n";
        htmlFile << ".section { flex: 2; display: flex; flex-direction: column; justify-content: flex-start; margin-right: 18px; }\n";
        htmlFile << ".row { display: flex; flex-direction: row; align-items: center; margin-bottom: 4px; }\n";
        htmlFile << ".label { font-size: 14px; color: #222; min-width: 60px; margin-right: 10px; }\n";
        htmlFile << ".label.to { min-width: 90px; }\n";
        htmlFile << ".value { font-size: 15px; color: #222; min-width: 60px; margin-right: 10px; }\n";
        htmlFile << ".bold { font-weight: 700; }\n";
        htmlFile << ".dashed { border-bottom: 1px dashed #222; width: 90%; margin: 6px 0; }\n";
        htmlFile << ".plane { position: absolute; right: 10px; bottom: 8px; }\n";
        htmlFile << ".plane img { width: 90px; height: 90px; }\n";
        htmlFile << ".barcode-inline img {\n";
        htmlFile << " width: 70px;\n";
        htmlFile << " height: 190px;\n";
        htmlFile << " margin-top : 5px;\n";
        htmlFile << " margin-bottom: 10px;\n";
        htmlFile << " margin-right : 15px;\n";
        htmlFile << " right : 10px;\n";
        htmlFile << "}\n";
        htmlFile << ".barcode-vertical { position: absolute; right: 0; top: 30px; display: flex; flex-direction: column; align-items: center; }\n";
        htmlFile << ".barcode-vertical img { height: 90px; width: 24px; }\n";
        htmlFile << ".barcode-num-vertical { writing-mode: vertical-rl; text-orientation: mixed; font-size: 11px; letter-spacing: 3px; margin-top: 4px; }\n";
        htmlFile << ".right { flex: 1.2; display: flex; flex-direction: column; align-items: center; padding: 18px 0 0 0; position: relative; }\n";
        htmlFile << ".barcode-horizontal { width: 90%; margin: 0 auto 10px auto; text-align: center; }\n";
        htmlFile << ".barcode-horizontal img { width: 120px; height: 30px; }\n";
        htmlFile << ".gate-warning { color:rgb(63, 9, 140); font-weight: bold; text-align: left; font-size: 18px; margin-top: 12px; letter-spacing: 1px; }\n";
        htmlFile << "@media (max-width: 1000px) { .boarding-pass { width: 98vw; } .header-title { font-size: 3vw; } }\n";
        htmlFile << " </style>\n";
        htmlFile << "</head>\n";
        htmlFile << "<body>\n";
        htmlFile << "  <div class=\"boarding-pass\">\n";
        htmlFile << "    <div class=\"notch notch-top-left\"></div>\n";
        htmlFile << "    <div class=\"notch notch-top-right\"></div>\n";
        htmlFile << "    <div class=\"header\">\n";
        htmlFile << "      <span class=\"header-title\"><strong>Boarding Pass</strong></span>";
        htmlFile << "    </div>\n";
        htmlFile << "    <div class=\"main\">\n";
        htmlFile << "      <div class=\"left\">\n";
        htmlFile << "        <div class=\"airline-vertical\"><strong>TIRNETA AIRLINES</strong></div>\n";
        htmlFile << "      </div>\n";
        htmlFile << "      <div class=\"vertical-dashed\"></div>\n";
        htmlFile << "      <div class=\"center\">\n";
        htmlFile << "        <div class=\"section\">\n";
        htmlFile << "          <div class=\"row\">\n";
        htmlFile << "            <div class=\"label\">Passenger</div>\n";
        htmlFile << "            <div class=\"value bold\">" << passenger << "</div>\n";
        htmlFile << "          </div>\n";
        htmlFile << "          <div class=\"dashed\"></div>\n";
        htmlFile << "          <div class=\"row\">\n";
        htmlFile << "            <div class=\"label\">B Time</div>\n";
        htmlFile << "            <div class=\"label\">Gate</div>\n";
        htmlFile << "            <div class=\"label\">Flight</div>\n";
        htmlFile << "            <div class=\"label to\">To</div>\n";
        htmlFile << "          </div>\n";
        htmlFile << "          <div class=\"row bold\">\n";
        htmlFile << "            <div class=\"value\">" << boardingTime << "</div>\n";
        htmlFile << "            <div class=\"value\">" << gate << "</div>\n";
        htmlFile << "            <div class=\"value\">" << flight << "</div>\n";
        htmlFile << "            <div class=\"value\">" << toCity << "</div>\n";
        htmlFile << "          </div>\n";
        htmlFile << "          <div class=\"dashed\"></div>\n";
        htmlFile << "          <div class=\"row\"style=\"column-gap: 60px;\">\n";
        htmlFile << "            <div class=\"label\">Date</div>\n";
        htmlFile << "            <div class=\"label\">From</div>\n";
        htmlFile << "          </div>\n";
        htmlFile << "          <div class=\"row bold\" style=\"column-gap: 35px;\">\n";
        htmlFile << "            <div class=\"value\">" << date << "</div>\n";
        htmlFile << "            <div class=\"value\">" << fromCity << "</div>\n";
        htmlFile << "          </div>\n";
        htmlFile << "          <div class=\"dashed\"></div>\n";
        htmlFile << "          <div class=\"row\">\n";
        htmlFile << "            <div class=\"label\">Seat</div>\n";
        htmlFile << "            <div class=\"label\">Group</div>\n";
        htmlFile << "          </div>\n";
        htmlFile << "          <div class=\"row bold\">\n";
        htmlFile << "            <div class=\"value\">" << seat << "</div>\n";
        htmlFile << "            <div class=\"value\">" << group << "</div>\n";
        htmlFile << "          </div>\n";
        htmlFile << "    <div class=\"gate-warning\">GATE CLOSES 15 MINUTES BEFORE DEPARTURE</div>\n";
        htmlFile << "        </div>\n";
        htmlFile << "        <div class=\"barcode-inline\">\n";
        htmlFile << "          <img src=\"assets/lines.png\" alt=\"Barcode\" />\n";
        htmlFile << "        </div>\n";
        htmlFile << "        <div class=\"plane\">\n";
        htmlFile << "          <img src=\"assets/aeroplane1.png\" alt=\"Airplane\" />\n";
        htmlFile << "        </div>\n";
        htmlFile << "      </div>\n";
        htmlFile << "      <div class=\"vertical-dashed\"></div>\n";
        htmlFile << "      <div class=\"right\">\n";
        htmlFile << "        <div class=\"section\">\n";
        htmlFile << "          <div class=\"row\">\n";
        htmlFile << "            <div class=\"label\">Passenger</div>\n";
        htmlFile << "            <div class=\"value bold\">" << passenger << "</div>\n";
        htmlFile << "          </div>\n";
        htmlFile << "          <div class=\"row\">\n";
        htmlFile << "            <div class=\"label\">B Time</div>\n";
        htmlFile << "            <div class=\"label\">Gate</div>\n";
        htmlFile << "            <div class=\"label\">Flight</div>\n";
        htmlFile << "          </div>\n";
        htmlFile << "          <div class=\"row bold\">\n";
        htmlFile << "            <div class=\"value\">" << boardingTime << "</div>\n";
        htmlFile << "            <div class=\"value\">" << gate << "</div>\n";
        htmlFile << "            <div class=\"value\">" << flight << "</div>\n";
        htmlFile << "          </div>\n";
        htmlFile << "           <div style =\"row-gap : 20px;\">\n";
        htmlFile << "          <div style=\"display: grid; grid-template-columns: 80px 100px; font-family: sans-serif; font-size: 14px; font-family: 'Quicksand', Arial, sans-serif; column-gap: 40px; margin-bottom: 7px;\">";
        htmlFile << "            <div>Date</div>";
        htmlFile << "            <div>From</div>";
        htmlFile << "            <div style='font-weight: bold; font-family: \"Quicksand\", Arial, sans-serif;'>" << date << "</div>";
        htmlFile << "            <div style='font-weight: bold; font-family: \"Quicksand\", Arial, sans-serif;'>" << fromCity << "</div>";
        htmlFile << "            </div>";
        htmlFile << "            <div style=\"display: grid; grid-template-columns: 80px 100px; font-family: sans-serif; font-size: 14px; font-family: 'Quicksand', Arial, sans-serif; column-gap: 40px;\">";
        htmlFile << "            <div>To</div>";
        htmlFile << "            <div> Seat</div>";
        htmlFile << "            <div style='font-weight: bold;'>" << toCity << "</div>";
        htmlFile << "            <div style='font-weight: bold;'>" << seat << "</div>";
        htmlFile << "            </div>";
        htmlFile << "            </div>";
        htmlFile << "          </div>";
        htmlFile << "          </div>\n";
        htmlFile << "        </div>\n";
        htmlFile << "      </div>\n";
        htmlFile << "    </div>\n";
        htmlFile << "  </div>\n";
        htmlFile << "</body>\n";
        htmlFile << "</html>\n";
        htmlFile.close();
    }
    else
    {
        cout << "Error: Could not create ticket files." << endl;
    }
}

struct SimpleWeather
{
    string main;
    string desc;
    double temp;
    int humidity;
    double wind;
};

time_t parseDateTime(const string &date, const string &time)
{
    tm tm = {};
    sscanf(date.c_str(), "%d/%d/%d", &tm.tm_mday, &tm.tm_mon, &tm.tm_year);
    tm.tm_year -= 1900;
    tm.tm_mon -= 1;
    sscanf(time.c_str(), "%d:%d", &tm.tm_hour, &tm.tm_min);
    tm.tm_sec = 0;
    return mktime(&tm);
}

string toAPIDate(const string &dateStr)
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

SimpleWeather getForecastWeather(double lat, double lon, const string &apiKey, const string &date, const string &time)
{
    ostringstream url;
    url << "http://api.openweathermap.org/data/2.5/forecast?lat=" << lat
        << "&lon=" << lon
        << "&appid=" << apiKey
        << "&units=metric";
    cpr::Response r = cpr::Get(cpr::Url{url.str()});
    if (r.status_code == 200)
    {
        nlohmann::json j = nlohmann::json::parse(r.text);

        tm flight_tm = {};
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
}

SimpleWeather getCurrentWeather(double lat, double lon, const string &apiKey)
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
        int humidity = j["main"]["humidity"];
        double wind = j["wind"]["speed"];
        return {main, desc, temp, humidity, wind};
    }
    return {"--", "--", -1, -1, -1};
}

void printDetailedWeather(double lat, double lon, const string &apiKey, const string &airportCode, const string &date, const string &time)
{
    SimpleWeather w = getCurrentWeather(lat, lon, apiKey);
    cout << "\nWEATHER AT " << airportCode << " :\n";
    if (w.temp == -1)
    {
        std::cout << "  [ Weather Unavailable ]\n";
        return;
    }
    cout << "Temperature : " << w.temp << " deg C (" << w.temp + 273.15 << " K)\n";
    cout << "Condition : " << w.main << " (" << w.desc << ")\n";
    cout << "Humidity : " << w.humidity << "%\n";
    cout << "Wind : " << w.wind << " m/s\n";
    cout << "\n";

    if (w.temp >= 30)
        cout << "ADVICE : Stay Hydrated due to high Temperature !\n";
    else if (w.temp <= 0)
        cout << "ADVICE : Dress warmly, it's Freezing !\n";
}

string extractCity(const string &airportName)
{
    if (airportName.find("Los Angeles") != string::npos)
        return "Los Angeles";
    if (airportName.find("Miami") != string::npos)
        return "Miami";
    if (airportName.find("Seattle") != string::npos)
        return "Seattle";
    if (airportName.find("Portland") != string::npos)
        return "Portland";
    if (airportName.find("San Francisco") != string::npos)
        return "San Francisco";
    if (airportName.find("Las Vegas") != string::npos)
        return "Las Vegas";
    if (airportName.find("Salt Lake City") != string::npos)
        return "Salt Lake City";
    if (airportName.find("Minneapolis") != string::npos)
        return "Minneapolis";
    if (airportName.find("Kansas City") != string::npos)
        return "Kansas City";
    if (airportName.find("New York") != string::npos)
        return "New York";
    if (airportName.find("Boston") != string::npos)
        return "Boston";
    if (airportName.find("Cleveland") != string::npos)
        return "Cleveland";
    if (airportName.find("Charlotte") != string::npos)
        return "Charlotte";
    if (airportName.find("Houston") != string::npos)
        return "Houston";
    if (airportName.find("St. Louis") != string::npos)
        return "St. Louis";
    if (airportName.find("Tampa") != string::npos)
        return "Tampa";
    return airportName;
}

FlightTicket bookFlight(const vector<Airport> &airports, int src, int dst)
{

    double distance = 500 + (src * 100) + (dst * 50);

    Date currentDate = Date::getCurrentDate();

    vector<FlightTicket> flightOptions;

    printLine('=');
    cout << "AVAILABLE FLIGHTS" << std::endl;
    printLine('=');
    cout << "From : " << airports[src].code << " To " << airports[dst].code << endl;

    const int colWidths[] = {4, 1, 12, 1, 9, 1, 9, 1, 12, 1, 8, 1, 7, 1, 9, 1, 12, 1, 8, 1, 7, 1, 9};
    int tableWidth = 0;
    for (int w : colWidths)
        tableWidth += w;

    cout << left << setw(4) << "No." << " "
         << setw(12) << "Date" << " "
         << setw(9) << "Departure" << " "
         << setw(9) << "Arrival" << " "
         << setw(12) << "Dep Weather" << " "
         << setw(8) << "D.Temp" << " "
         << setw(7) << "D.Hum" << " "
         << setw(9) << "D.Wind" << " "
         << setw(12) << "Arr Weather" << " "
         << setw(8) << "A.Temp" << " "
         << setw(7) << "A.Hum" << " "
         << setw(9) << "A.Wind" << endl;
    printLine('-', tableWidth);

    for (int i = 0; i < 5; ++i)
    {
        Date flightDate = currentDate.addDays(i);
        auto [departureTime, arrivalTime] = generateFlightTimes(distance);
        double price = generateRandomPrice(distance, i);

        SimpleWeather depWeather = getForecastWeather(airports[src].latitude, airports[src].longitude, apiKey, flightDate.toString(), departureTime);
        SimpleWeather arrWeather = getForecastWeather(airports[dst].latitude, airports[dst].longitude, apiKey, flightDate.toString(), arrivalTime);
        FlightTicket ticket;

        ticket.departureAirportCode = airports[src].code;
        ticket.departureAirportName = airports[src].name;
        ticket.arrivalAirportCode = airports[dst].code;
        ticket.arrivalAirportName = airports[dst].name;
        ticket.departureDate = flightDate.toString();
        ticket.departureTime = departureTime;
        ticket.arrivalTime = arrivalTime;
        ticket.price = price;
        ticket.isBooked = false;
        flightOptions.push_back(ticket);

        ostringstream depWind, arrWind, depTemp, arrTemp;

        if (depWeather.wind == -1)
            depWind << "--";
        else
            depWind << fixed << setprecision(2) << depWeather.wind << " m/s";
        if (arrWeather.wind == -1)
            arrWind << "--";
        else
            arrWind << fixed << setprecision(2) << arrWeather.wind << " m/s";
        if (depWeather.temp == -1)
            depTemp << "--";
        else
            depTemp << fixed << setprecision(2) << depWeather.temp << " C";
        if (arrWeather.temp == -1)
            arrTemp << "--";
        else
            arrTemp << fixed << setprecision(2) << arrWeather.temp << " C";
        cout << left << setw(4) << (i + 1) << " "
             << setw(12) << flightDate.toString() << " "
             << setw(9) << departureTime << " "
             << setw(9) << arrivalTime << " "
             << setw(12) << depWeather.main << " "
             << setw(8) << depTemp.str() << " "
             << setw(7) << (depWeather.humidity == -1 ? "--" : (to_string(depWeather.humidity) + "%")) << " "
             << setw(9) << depWind.str() << " "
             << setw(12) << arrWeather.main << " "
             << setw(8) << arrTemp.str() << " "
             << setw(7) << (arrWeather.humidity == -1 ? "--" : (to_string(arrWeather.humidity) + "%")) << " "
             << setw(9) << arrWind.str() << endl;
        cout.flush();
    }
    printLine('-', tableWidth);

    int selection;
    do
    {
        cout << "Select a flight [ 1 - 5 ] : ";
        cout.flush();
        cin >> selection;

        if (selection < 1 || selection > 5)
        {
            cout << "Invalid selection. Please enter a number between 1 and 5." << endl;
            cout.flush();
        }
    } while (selection < 1 || selection > 5);

    FlightTicket selectedTicket = flightOptions[selection - 1];

    string passengerName;
    cout << "Enter Passenger name : ";
    cout.flush();
    cin.ignore();
    getline(cin, passengerName);

    vector<string> availableSeats;
    for (int i = 0; i < 10; ++i)
    {
        availableSeats.push_back(generateSeatNumber());
    }

    sort(availableSeats.begin(), availableSeats.end());
    availableSeats.erase(unique(availableSeats.begin(), availableSeats.end()), availableSeats.end());

    cout << "\nAvailable seats : ";
    for (const auto &seat : availableSeats)
        cout << seat << " ";
    cout << endl;
    cout << "Choose your seat from the list above [or press Enter for random] : ";
    cout.flush();
    string seatChoice;
    getline(cin, seatChoice);
    string chosenSeat;
    if (!seatChoice.empty())
    {
        bool found = false;
        for (const auto &seat : availableSeats)
        {
            if (seat == seatChoice)
            {
                chosenSeat = seatChoice;
                found = true;
                break;
            }
        }
        if (!found)
        {
            cout << "Invalid seat choice. Assigning a random seat from the list." << endl;
            chosenSeat = availableSeats[rand() % availableSeats.size()];
        }
    }
    else
    {
        chosenSeat = availableSeats[rand() % availableSeats.size()];
    }

    cout << "Processing your booking..." << endl;
    cout.flush();

    selectedTicket.passengerName = passengerName;
    selectedTicket.seatNumber = chosenSeat;
    selectedTicket.bookingReference = generateBookingReference();
    selectedTicket.isBooked = true;

    printLine('*');
    cout << "BOOKING CONFIRMATION" << endl;
    printLine('*');
    cout << "Booking Reference : " << selectedTicket.bookingReference << endl;
    cout << "Passenger : " << selectedTicket.passengerName << endl;
    cout << "Flight : " << selectedTicket.departureAirportCode << " to " << selectedTicket.arrivalAirportCode << endl;
    cout << "Date : " << selectedTicket.departureDate << endl;
    cout << "Time : " << selectedTicket.departureTime << " - " << selectedTicket.arrivalTime << endl;
    cout << "Seat : " << selectedTicket.seatNumber << endl;
    cout << "Price : $" << fixed << setprecision(2) << selectedTicket.price << endl;
    printLine('*');

    cout << "Ticket Booked successfully!" << endl;

    generateTicketFile(selectedTicket);

    cout << "Now checking weather conditions for your flight..." << endl;
    cout.flush();

    printDetailedWeather(airports[src].latitude, airports[src].longitude, apiKey, airports[src].code, selectedTicket.departureDate, selectedTicket.departureTime);
    printDetailedWeather(airports[dst].latitude, airports[dst].longitude, apiKey, airports[dst].code, selectedTicket.departureDate, selectedTicket.arrivalTime);

    return selectedTicket;
}

struct Edge
{
    int to;
    double distance;
};

vector<int> dijkstra(int src, int dst, const vector<vector<Edge>> &graph, double &totalDistance)
{
    int n = graph.size();
    vector<double> dist(n, 1e9);
    vector<int> prev(n, -1);
    priority_queue<pair<double, int>, vector<pair<double, int>>, greater<>> pq;
    dist[src] = 0;
    pq.push({0, src});
    while (!pq.empty())
    {
        auto [d, u] = pq.top();
        pq.pop();

        if (d > dist[u])
            continue;

        for (const auto &edge : graph[u])
        {
            if (dist[edge.to] > dist[u] + edge.distance)
            {
                dist[edge.to] = dist[u] + edge.distance;
                prev[edge.to] = u;
                pq.push({dist[edge.to], edge.to});
            }
        }
    }

    vector<int> path;
    for (int at = dst; at != -1; at = prev[at])
        path.push_back(at);
    reverse(path.begin(), path.end());
    totalDistance = dist[dst];

    if (path.front() != src)
        path.clear();
    return path;
}

void dfsAllRoutes(int src, int dst, vector<vector<Edge>> &graph, vector<int> &path, vector<vector<int>> &allPaths, vector<bool> &visited)
{
    if (src == dst)
    {
        allPaths.push_back(path);
        return;
    }

    visited[src] = true;
    for (auto &edge : graph[src])
    {
        if (!visited[edge.to])
        {
            path.push_back(edge.to);
            dfsAllRoutes(edge.to, dst, graph, path, allPaths, visited);
            path.pop_back();
        }
    }
    visited[src] = false;
}

int resolveAirportIndex(const string &input, const vector<Airport> &airports)
{
    bool isNumber = true;
    for (char c : input)
    {
        if (!std::isdigit(c))
        {
            isNumber = false;
            break;
        }
    }

    if (isNumber)
    {
        int index = stoi(input);
        if (index >= 0 && index < airports.size())
        {
            return index;
        }
    }
    else
    {
        for (int i = 0; i < airports.size(); ++i)
        {
            if (airports[i].code == input)
            {
                return i;
            }
        }
    }
    return -1;
}

int mapBookingToSimIndex(const string &code)
{
    vector<string> simCodes = {"SEA", "PDX", "SFO", "LAS", "SLC", "LAX", "MSP", "MCI", "JFK", "BOS", "CLE", "CLT", "MIA", "IAH", "STL", "TPA", "DEN", "PHX", "DFW", "AUG"};
    for (int i = 0; i < simCodes.size(); ++i)
    {
        if (simCodes[i] == code)
            return i;
    }
    return -1;
}

void findBookingByReference(const unordered_map<string, FlightTicket> &bookingMap)
{
    string ref;
    cout << "Enter Booking Reference : ";
    cin >> ref;
    auto it = bookingMap.find(ref);
    if (it != bookingMap.end())
    {
        const FlightTicket &t = it->second;
        cout << "Booking found for " << t.passengerName << ": " << t.departureAirportCode << " to " << t.arrivalAirportCode << " on " << t.departureDate << endl;
    }
    else
    {
        cout << "No booking found with that reference. " << endl;
    }
}

int main()
{
    ios_base::sync_with_stdio(true);
    vector<Airport> airports = {
        {"SEA", "SEATTLE TACOMA INTERNATIONAL AIRPORT", 47.4502, -122.3088},
        {"PDX", "PORTLAND INTERNATIONAL AIRPORT", 45.5898, -122.5951},
        {"SFO", "SAN FRANCISCO INTERNATIONAL AIRPORT", 37.6213, -122.3790},
        {"LAS", "HARRY REID INTERNATIONAL AIRPORT", 36.0840, -115.1537},
        {"SLC", "SALT LAKE CITY INTERNATIONAL AIRPORT", 40.7899, -111.9791},
        {"LAX", "LOS ANGELES INTERNATIONAL AIRPORT", 33.9416, -118.4085},
        {"MSP", "MINNEAPOLIS SAINT PAUL INTERNATIONAL AIRPORT", 44.8848, -93.2223},
        {"MCI", "KANSAS CITY INTERNATIONAL AIRPORT", 39.2976, -94.7139},
        {"JFK", "JOHN F. KENNEDY INTERNATIONAL AIRPORT", 40.6413, -73.7781},
        {"BOS", "BOSTON LOGAN INTERNATIONAL AIRPORT", 42.3656, -71.0096},
        {"CLE", "CLEVELAND HOPKINS INTERNATIONAL AIRPORT", 41.4117, -81.8498},
        {"CLT", "CHARLOTTE DOUGLAS INTERNATIONAL AIRPORT", 35.2140, -80.9431},
        {"MIA", "MIAMI INTERNATIONAL AIRPORT", 25.7959, -80.2870},
        {"IAH", "GEORGE BUSH INTERCONTINENTAL AIRPORT", 29.9902, -95.3368},
        {"STL", "ST. LOUIS LAMBERT INTERNATIONAL AIRPORT", 38.7500, -90.3700},
        {"TPA", "TAMPA INTERNATIONAL AIRPORT", 27.9755, -82.5332},
        {"DEN", "DENVER INTERNATIONAL AIRPORT", 39.8561, -104.6737},
        {"PHX", "PHOENIX SKY HARBOR INTERNATIONAL AIRPORT", 33.4373, -112.0078},
        {"DFW", "DALLAS/FORT WORTH INTERNATIONAL AIRPORT", 32.8998, -97.0403},
        {"AUG", "AUGUSTA STATE AIRPORT", 44.3206, -69.7973}};

    vector<vector<Edge>> airportGraph(airports.size());
    auto addEdge = [&](int u, int v, double dist)
    {
        airportGraph[u].push_back({v, dist});
        airportGraph[v].push_back({u, dist});
    };

    addEdge(0, 1, 233);
    addEdge(1, 2, 550);
    addEdge(2, 5, 543);
    addEdge(5, 3, 386);
    addEdge(3, 4, 684);
    addEdge(4, 6, 991);
    addEdge(6, 7, 376);
    addEdge(7, 8, 1800);
    addEdge(8, 9, 300);
    addEdge(9, 10, 900);
    addEdge(10, 11, 600);
    addEdge(11, 12, 1000);
    addEdge(12, 13, 1200);
    addEdge(13, 14, 1100);
    addEdge(14, 15, 900);
    addEdge(0, 2, 1090);
    addEdge(2, 3, 670);
    addEdge(5, 8, 3970);
    addEdge(3, 8, 2240);
    addEdge(1, 4, 1200);
    addEdge(7, 14, 400);
    addEdge(1, 5, 1320);
    addEdge(4, 16, 820);
    addEdge(6, 16, 1100);
    addEdge(3, 17, 480);
    addEdge(5, 17, 590);
    addEdge(16, 18, 1050);
    addEdge(13, 18, 400);
    addEdge(9, 19, 260);
    addEdge(6, 14, 700);
    addEdge(10, 14, 660);

    printLine('=');
    cout << "WELCOME TO FLIGHT BOOKING SYSTEM" << endl;
    printLine('=');
    cout << "AVAILABLE AIRPORTS [INDEX : CODE] " << endl;
    for (int i = 0; i < airports.size(); ++i)
    {
        cout << "  " << i << ": " << airports[i].code << " - " << airports[i].name << endl;
    }
    printLine();
    string input;
    int src = -1, dst = -1;
    cout << "FLIGHT BOOKING" << endl;
    printLine();

    do
    {
        cout << "Enter Departure index or code : ";
        cout.flush();
        cin >> input;
        src = resolveAirportIndex(input, airports);
        if (src < 0 || src >= airports.size())
        {
            cout << "Invalid Airport. Please try again." << endl;
            cout.flush();
        }
    } while (src < 0 || src >= airports.size());

    do
    {
        cout << "Enter Arrival index or code : ";
        cout.flush();
        cin >> input;
        dst = resolveAirportIndex(input, airports);
        if (dst < 0 || dst >= airports.size())
        {
            cout << "Invalid Airport. Please try again." << endl;
            cout.flush();
        }
    } while (dst < 0 || dst >= airports.size());

    FlightTicket ticket = bookFlight(airports, src, dst);
    printLine('=');
    cout << "LAUNCHING FLIGHT SIMULATOR" << endl;
    printLine('=');
    cout << "Starting flight simulation for " << ticket.departureAirportCode << " to " << ticket.arrivalAirportCode << "..." << endl;

    int simSrc = mapBookingToSimIndex(airports[src].code);
    int simDst = mapBookingToSimIndex(airports[dst].code);

    if (ticket.departureTime.empty())
    {
        ticket.departureTime = generateRandomTime();
    }

    string command = "flight_simulator.exe " + to_string(simSrc) + " " + to_string(simDst) + " " + ticket.departureDate + " " + ticket.departureTime;

    int result = system(command.c_str());
    if (result != 0)
    {
        cout << "\nERROR : Could not launch Flight Simulator." << endl;
        cout << "This could be due to missing SFML libraries or other dependencies." << endl;
        cout << "\nFlight Summary : " << endl;
        printLine('-');
        cout << "Departure : " << ticket.departureAirportCode << endl;
        cout << "Arrival : " << ticket.arrivalAirportCode << endl;
        cout << "Date : " << ticket.departureDate << endl;
        cout << "Time : " << ticket.departureTime << " - " << ticket.arrivalTime << endl;
        cout << "\nThank you for using our Booking system!" << endl;
    }
    cout << "\nPress Enter to exit...";
    cin.ignore(1);
    cin.get();
    return 0;
}