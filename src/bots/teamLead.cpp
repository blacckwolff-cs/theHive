#include "teamLead.hpp"
#include "fireteam/fireteam.hpp"
#include "hive/hive.hpp"
#include <iostream>

// Utility to parse coords safely
coords parse_coords(const nlohmann::json& j) {
    coords c{0, 0, 0};
    c.x = j.value("x", 0);
    c.y = j.value("y", 0);
    c.z = j.value("z", 0);
    return c;
}

// Utility to create move action from JSON
std::unique_ptr<move_action> create_move_from_json(const nlohmann::json& j) {
    auto m = std::make_unique<move_action>();
    m->type = action_type::MOVE;
    
    if (j.contains("destination")) {
        m->destination = parse_coords(j["destination"]);
    } else {
        std::cerr << "[teamLead] ERROR: Move action missing 'destination'. Defaulting to (0,0,0).\n";
    }

    m->time = 5;

    if (j.contains("tags") && j["tags"].is_array()) {
        for (const auto& tagStr : j["tags"]) {
            if (tagStr == "CAUTION") m->tags.push_back(Tag::CAUTION);
            else if (tagStr == "USE_FORCE") m->tags.push_back(Tag::USE_FORCE);
            else if (tagStr == "GATHER_INFO") m->tags.push_back(Tag::GATHER_INFO);
            else std::cerr << "[teamLead] WARNING: Unknown tag '" << tagStr << "' ignored.\n";
        }
    }

    return m;
}

void teamLead::clear_orders() {
    orders.clear();
}


void teamLead::enqueue_move(int x, int y) {
    auto m = std::make_unique<move_action>();
    m->destination = coords{x, 0, y};
    m->time = 5;
    m->tags.push_back(Tag::USE_FORCE);
    action_queue.push_back(std::move(m));
}

void teamLead::update_state(const nlohmann::json& data) {
    if (data.contains("position")) {
        position = parse_coords(data["position"]);
    }

    if (data.contains("power_level")) {
        power_level = data["power_level"];
    }

    if (data.contains("active")) {
        active = data["active"];
    }
}

void teamLead::make_move() {
    std::cout << "[teamLead] Making default move.\n";
}

void teamLead::execute_actions() {
    for (auto& act : action_queue) {
        switch (act->type) {
            case action_type::MOVE: {
                auto* moveAct = dynamic_cast<move_action*>(act.get());
                if (!moveAct) {
                    std::cerr << "[teamLead] ERROR: MOVE action cast failed.\n";
                    break;
                }
                std::cout << "[teamLead] Moving to ("
                          << moveAct->destination.x << ", "
                          << moveAct->destination.y << ", "
                          << moveAct->destination.z << ").\n";
                position = moveAct->destination;
                break;
            }

            case action_type::IDLE:
                std::cout << "[teamLead] Performing idle action.\n";
                break;

            default:
                std::cerr << "[teamLead] ERROR: Unknown action type.\n";
                break;
        }
    }

    action_queue.clear();
}

void teamLead::dispatch_orders() {
    for (auto& [botId, queue] : orders) {
        if (botId == 1) {
            while (!queue.empty()) {
                action_queue.push_back(std::move(queue.front()));
                queue.pop_front();
            }
            std::cout << "[teamLead] Dispatched orders to self.\n";
            continue;
        }

        auto it = fireteam->team.find(botId);
        if (it != fireteam->team.end()) {
            auto& bot = it->second;
            while (!queue.empty()) {
                bot->action_queue.push_back(std::move(queue.front()));
                queue.pop_front();
            }
            std::cout << "[teamLead] Dispatched orders to bot " << botId << ".\n";
        } else {
            std::cerr << "[teamLead] ERROR: Bot ID " << botId << " not found.\n";
        }
    }

    orders.clear();
}

void teamLead::send_data(Hive& hive) {
    nlohmann::json j;
    j["fireteam_id"] = "TODO:ID";
    j["fireteam_name"] = "TODO:Name";
    j["fireteam_description"] = "Auto-generated status report";

    j["fireteam_leader"] = to_json();
    j["fireteam_members"] = nlohmann::json::array();

    for (const auto& [botId, botPtr] : fireteam->team) {
        j["fireteam_members"].push_back(botPtr->to_json());
    }

    std::cout << "[teamLead] Sending data to Hive...\n";
    hive.process(j);
}

nlohmann::json teamLead::to_json() const {
    return {
        {"leader_id", id},
        {"leader_name", name},
        {"leader_role", role},
        {"leader_status", active ? "active" : "inactive"},
        {"power_level", power_level},
        {"position", {
            {"x", position.x},
            {"y", position.y},
            {"z", position.z}
        }}
    };
}

void teamLead::sendToLeader(teamLead& leader) const {
    leader.receive_member_state(to_json());
}

void teamLead::receive_member_state(const nlohmann::json& memberData) {
    std::string memberId = memberData.value("member_id", "unknown");
    last_member_states[memberId] = memberData;
}

void teamLead::receive_orders_json(const nlohmann::json& ordersJson) {
    for (const auto& action : ordersJson) {
        int botId = action["target_id"].is_string()
                        ? std::stoi(action["target_id"].get<std::string>())
                        : action["target_id"].get<int>();

        if (action["type"] == "move") {
            auto moveAct = create_move_from_json(action);
            orders[botId].push_back(std::move(moveAct));
        } else {
            std::cerr << "[teamLead] WARNING: Unknown action type '" << action["type"] << "'.\n";
        }
    }
}
