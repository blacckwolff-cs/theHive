#include "hive.hpp"
#include <iostream>
#include <memory>
#include "../action.hpp"
#include "bots/teamLead.hpp"

void Hive::sendOrder(int botId, std::unique_ptr<action> act, teamLead& tl) {
    if (!act) {
        std::cerr << "[Hive] Tried to send null action!\n";
        return;
    }

    std::cout << "[Hive] Sending order to bot " << botId << "...\n";
    tl.orders[botId].push_back(std::move(act));
}

// Helper to create an idle action
std::unique_ptr<idle_action> create_idle_action(Tag tag) {
    auto i = std::make_unique<idle_action>();
    i->type = action_type::IDLE;
    i->tags.push_back(tag);
    return i;
}

// Helper to create a move action
std::unique_ptr<move_action> create_move_action(coords destination, int time, Tag tag) {
    auto m = std::make_unique<move_action>();
    m->type = action_type::MOVE;
    m->destination = destination;
    m->time = time;
    m->tags.push_back(tag);
    return m;
}

void Hive::process(const nlohmann::json& j) {
    actions.clear();

    bool leaderLowPower = false;
    bool memberLowAmmo = false;
    bool allMembersActive = true;

    // --- Leader status ---
    if (j.contains("fireteam_leader")) {
        const auto& leader = j["fireteam_leader"];
        int leaderPower = leader.value("power_level", 100);
        if (leaderPower < 50) {
            leaderLowPower = true;
        }
    }

    // --- Members status ---
    if (j.contains("fireteam_members") && j["fireteam_members"].is_array()) {
        for (const auto& member : j["fireteam_members"]) {
            std::string status = member.value("member_status", "inactive");
            if (status != "active") {
                allMembersActive = false;
            }

            if (member.contains("ammunition") && member["ammunition"].is_number_integer()) {
                int ammo = member["ammunition"];
                if (ammo <= 2) {
                    memberLowAmmo = true;
                }
            }
        }
    }

    // --- Decision Logic ---
    if (leaderLowPower) {
        actions.push_back(create_idle_action(Tag::CAUTION));
        std::cout << "[Hive] Action: Leader low power -> IDLE with CAUTION\n";
    }

    if (memberLowAmmo) {
        actions.push_back(create_idle_action(Tag::GATHER_INFO));
        std::cout << "[Hive] Action: Member low ammo -> IDLE with GATHER_INFO\n";
    }

    if (allMembersActive) {
        coords target = {10, 0, 10};
        actions.push_back(create_move_action(target, 5, Tag::USE_FORCE));
        std::cout << "[Hive] Action: All members active -> MOVE with USE_FORCE\n";
    }

    if (actions.empty()) {
        actions.push_back(create_idle_action(Tag::CAUTION));
        std::cout << "[Hive] Action: Default fallback -> IDLE with CAUTION\n";
    }
}
