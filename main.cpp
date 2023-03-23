#include "Bot.h"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include <mutex>
#include "nlohmann/json.hpp"
#include <conio.h>
#include <functional>


#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "PhoenixApi/Api.h"
#include "Scene.h"
#include "Split_string.h"
#include "Logger.h"

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    char title[256];
    GetWindowTextA(hwnd, title, sizeof(title));
    auto titles = reinterpret_cast<std::vector<std::string>*>(lParam);
    titles->push_back(title);
    return TRUE;
}

std::vector<std::string> getAllTitles() {
    std::vector<std::string> titles;
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&titles));
    return titles;
}

std::vector<std::pair<std::string, int>> returnAllPorts() {
    std::vector<std::string> titles = getAllTitles();
    std::vector<std::pair<std::string, int>> ports;

    for (const std::string& title : titles) {
        if (title.find("Phoenix Bot") != std::string::npos) {
            std::istringstream iss(title);
            std::string name;
            int port;

            // Znajdź pozycję "[" i "]" w tytule
            std::size_t left_bracket_pos = title.find("[");
            std::size_t right_bracket_pos = title.find("]");

            // Spróbuj wczytać nick
            if (left_bracket_pos != std::string::npos && right_bracket_pos != std::string::npos) {
                name = title.substr(left_bracket_pos + 1, right_bracket_pos - left_bracket_pos - 1);
            }

            // Znajdź pozycję "Bot:"
            std::size_t bot_colon_pos = title.find("Bot:");

            // Spróbuj wczytać port
            if (bot_colon_pos != std::string::npos) {
                iss.seekg(bot_colon_pos + 4); // Przesuń wskaźnik o 4 pozycje w prawo, aby pominąć "Bot:"
                iss >> port;

                ports.push_back(std::make_pair(name, port));
            }
        }
    }

    return ports;
}


void select_characters(const std::vector<std::pair<std::string, int>>& ports, std::vector<int>& characters, const std::string& category) {
    std::cout << "Select " << category << ":\n";
    for (size_t i = 0; i < ports.size(); ++i) {
        std::cout << i + 1 << ") " << ports[i].first << "\n";
    }
    std::cout << "Enter the numbers of the characters you want to select, separated by space. Press ENTER when done.\n";

    std::string input;
    std::getline(std::cin, input);
    std::istringstream iss(input);
    int index;
    while (iss >> index) {
        if (index >= 1 && index <= ports.size()) {
            characters.push_back(ports[index - 1].second);
        }
    }
}
void run_bot(int port, Bot& bot) {

    Phoenix::Api api(port);
    Scene scene;
    bot.set_api(&api);
    bot.set_scene(&scene);
    std::vector<Module*> modules = { &scene, &bot };

    std::stringstream ss;
    ss << "[" << port << "] Bot is running..." << std::endl;
    std::cout << ss.str();

    while (true) {
        if (!api.empty()) {
            std::string message = api.get_message();

            try {
                nlohmann::json json_msg = nlohmann::json::parse(message);

                std::string packet = json_msg["packet"];
                std::vector<std::string> packet_splitted = split_string(packet);

                if (packet_splitted.size() > 0) {
                    for (auto mod : modules)
                        mod->on_send(packet_splitted, packet);
                }

                if (json_msg["type"] == "packet_recv") {
                    std::string packet = json_msg["packet"];
                    std::vector<std::string> packet_splitted = split_string(packet);

                    if (packet_splitted.size() > 0) {
                        for (auto mod : modules)
                            mod->on_recv(packet_splitted, packet);
                    }
                }
            }
            catch (const std::exception& e) {
                ss.clear();
                ss << "[" << port << "]: " << e.what() << std::endl;
                std::cerr << ss.str();
                return;
            }
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        bot.run();
        // Obsługa zdarzeń klawiatury
        if (_kbhit()) {
            char key = _getch();
            bot.handle_key_press(key);
        }
    }
}

int main() {

    std::vector<std::pair<std::string, int>> ports = returnAllPorts();

    std::vector<int> stationary_characters;
    std::vector<int> main_characters;
    std::vector<int> tank_characters;

    select_characters(ports, stationary_characters, "Stationary Characters");
    select_characters(ports, main_characters, "Main Characters");
    select_characters(ports, tank_characters, "Tank Characters");

    std::vector<Module*> modules;

    for (int port : stationary_characters) {
        Bot* bot = new Bot();
        std::thread t(run_bot, port, std::ref(*bot));
        t.detach();
    }

    for (int port : main_characters) {
        Bot* bot = new Bot();
        std::thread t(run_bot, port, std::ref(*bot));
        t.detach();
    }

    for (int port : tank_characters) {
        Bot* bot = new Bot();
        std::thread t(run_bot, port, std::ref(*bot));
        t.detach();
    }


    std::cout << "Bot is running...." << std::endl;

    // Wait for user input to stop the bot.
    for (Module* module : modules) {
        delete module;
    }

    return 0;
}
