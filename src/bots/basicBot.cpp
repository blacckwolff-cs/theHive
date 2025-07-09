#include "basicBot.hpp"
#include "../action.hpp"
#include "teamLead.hpp"
#include <iostream>

// Constructor
basicBot::basicBot(const std::string& id_, const std::string& name_, const std::string& role_) {
    id = id_;
    name = name_;
    role = role_;
}

// Enqueue a movement action
void basicBot::enqueue_move(int x, int y) {
    auto m = std::make_unique<move_action>();
    m->destination = coords{x, 0, y};  // Assuming z = 0 (ground level)
    m->time = 5;                       // Example time to reach destination
    m->tags.push_back(Tag::CAUTION);

    action_queue.push_back(std::move(m));
}

// Update internal state from JSON
void basicBot::update_state(const nlohmann::json& data) {
    if (data.contains("position")) {
        position.x = data["position"]["x"];
        position.y = data["position"]["y"];
        position.z = data["position"]["z"];
    }

    if (data.contains("power_level")) {
        power_level = data["power_level"];
    }

    if (data.contains("active")) {
        active = data["active"];
    }
}

// Default move implementation
void basicBot::make_move() {
    std::cout << "[basicBot] Making default move.\n";
}

// Execute all queued actions
void basicBot::execute_actions() {
    for (auto& act : action_queue) {
        switch (act->type) {
            case action_type::MOVE: {
                auto* moveAct = dynamic_cast<move_action*>(act.get());
                if (!moveAct) {
                    std::cerr << "[basicBot] ERROR: Action is MOVE but dynamic_cast failed!\n";
                    break;
                }
                std::cout << "[basicBot] Moving to ("
                          << moveAct->destination.x << ", "
                          << moveAct->destination.y << ", "
                          << moveAct->destination.z << ").\n";
                position = moveAct->destination;
                break;
            }

            case action_type::IDLE:
                std::cout << "[basicBot] Performing idle action.\n";
                break;

            default:
                std::cerr << "[basicBot] ERROR: Unknown action type.\n";
                break;
        }
    }

    action_queue.clear();
}

// Convert bot state to JSON
nlohmann::json basicBot::to_json() const {
    return {
        {"member_id", id},
        {"member_name", name},
        {"member_role", role},
        {"member_status", active ? "active" : "inactive"},
        {"power_level", power_level},
        {"position", {
            {"x", position.x},
            {"y", position.y},
            {"z", position.z}
        }}
    };
}

// Send state to leader
void basicBot::sendToLeader(teamLead& leader) const {
    auto botData = to_json();
    leader.receive_member_state(botData);
    std::cout << "[basicBot] Sent state to team lead: " << botData.dump() << "\n";
}
