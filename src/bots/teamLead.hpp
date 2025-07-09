#pragma once

#include "bot.hpp"
#include "fireteam/fireteam.hpp"
#include <memory>
#include <map>
#include <unordered_map>
#include <deque>
#include <iostream>

// Forward declaration
class Hive;

class teamLead : public Bot {
    friend class Hive;
    friend class Fireteam;

public:
    teamLead(const std::string& id_, const std::string& name_, Fireteam* ft)
        : fireteam(ft)
    {
        id = id_;
        name = name_;
        role = "team_lead";
    }

    void make_move() override;
    void enqueue_move(int x, int y) override;
    void execute_actions() override;
    void update_state(const nlohmann::json& data) override;
    void sendToLeader(teamLead& leader) const override;
    nlohmann::json to_json() const override;

    void assign_order(int bot_id, std::unique_ptr<action> order);
    void dispatch_orders();
    void send_data(Hive& hive);
    void clear_orders();

    Fireteam& get_fireteam() { return *fireteam; }
    void receive_member_state(const nlohmann::json& memberData);
    void receive_orders_json(const nlohmann::json& ordersJson);

    Fireteam* fireteam = nullptr;
    std::map<int, std::deque<std::unique_ptr<action>>> orders;
    std::unordered_map<std::string, nlohmann::json> last_member_states;
};

coords parse_coords(const nlohmann::json& j);
