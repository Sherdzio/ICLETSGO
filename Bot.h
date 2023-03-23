#pragma once
#include "Module.h"
#include "Scene.h"
#include "PhoenixApi/Api.h"
#include "INIReader.h"
#include "Logger.h"
#include <utility>
#include <thread>
#include <ctime>

class Bot : public Module
{
public:

    Bot();

    void handle_key_press(char key); //

    explicit Bot(Phoenix::Api* api, Scene* scene);

    void on_recv(const std::vector<std::string>& packet_splitted, const std::string& full_packet);
    void run();
    //
    void set_api(Phoenix::Api* api);
    void set_scene(Scene* scene);

private:
    //
    Phoenix::Api* m_api;
    Scene* m_scene;

    void handle_qnamli(const std::vector<std::string>& packet_splitted, const std::string& full_packet);
    void handle_msgi(const std::vector<std::string>& packet_splitted, const std::string& full_packet);
    void handle_cmap(const std::vector<std::string>& packet_splitted, const std::string& full_packet);

    void walk(int x, int y);
    void load_config();

    static constexpr int ic_map_id = 2004;
    Phoenix::Api* api;
    Scene* scene;
    std::pair<int, int> point;
    int round;
    bool afk_last_round;
    bool move_to_players;
    bool move_to_point;
    bool moving;
};