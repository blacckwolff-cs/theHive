#include <iostream>
#include "fireteam.hpp"
#include "bots/bot.hpp"
#include "bots/teamLead.hpp"
#include "action.hpp"


Fireteam::Fireteam(std::map<int, std::unique_ptr<Bot>> team)
    : team(std::move(team)) {}

void Fireteam::setLeader(std::unique_ptr<teamLead> lead) {
    leader = std::move(lead);
}


// Helper: print tags
void print_tags(const std::vector<Tag>& tags) {
    std::cout << "Tags: ";
    if (tags.empty()) {
        std::cout << "(none)";
    } else {
        for (auto tag : tags) {
            switch (tag) {
                case Tag::CAUTION:      std::cout << "CAUTION "; break;
                case Tag::USE_FORCE:    std::cout << "USE_FORCE "; break;
                case Tag::GATHER_INFO:  std::cout << "GATHER_INFO "; break;
                default:                std::cout << "UNKNOWN "; break;
            }
        }
    }
    std::cout << "\n";
}

// Helper: print an action
void print_action(const std::unique_ptr<action>& act) {
    std::cout << "    - Action type: ";
    switch (act->type) {
        case action_type::MOVE: {
            auto* move = dynamic_cast<move_action*>(act.get());
            if (!move) {
                std::cout << "MOVE (cast failed) ";
                break;
            }
            std::cout << "MOVE to ("
                      << move->destination.x << ","
                      << move->destination.y << ","
                      << move->destination.z << ") "
                      << "Time: " << move->time << "s ";
            break;
        }
        case action_type::IDLE:
            std::cout << "IDLE ";
            break;
        default:
            std::cout << "UNKNOWN ";
            break;
    }
    print_tags(act->tags);
}

void Fireteam::dump() const {
    std::cout << "=== Fireteam Dump ===\n";

    // Leader
    if (leader) {
        std::cout << "Leader:\n";
        std::cout << "  ID: " << leader->id << "\n";
        std::cout << "  Name: " << leader->name << "\n";
        std::cout << "  Role: " << leader->role << "\n";
        std::cout << "  Power Level: " << leader->power_level << "\n";
        std::cout << "  Active: " << (leader->active ? "Yes" : "No") << "\n";
        std::cout << "  Position: (" << leader->position.x << ", " << leader->position.y << ", " << leader->position.z << ")\n";
        std::cout << "  Orders: " << leader->orders.size() << " pending orders\n";

        for (const auto& [botId, queue] : leader->orders) {
            std::string botName = "(unknown)";
            auto it = team.find(botId);
            if (it != team.end() && it->second) {
                botName = it->second->name;
            } else if (botId == 1) { // adjust logic if needed
                botName = leader->name;
            }

            std::cout << "    - Orders for Bot ID " << botId << " [" << botName << "]: " << queue.size() << " queued actions\n";
            for (const auto& act : queue) {
                print_action(act);
            }
        }

        std::cout << "  Action Queue: " << leader->action_queue.size() << " actions\n";
        for (const auto& act : leader->action_queue) {
            print_action(act);
        }
    } else {
        std::cout << "Leader: None\n";
    }

    // Members
    std::cout << "Members:\n";
    if (team.empty()) {
        std::cout << "  (none)\n";
    } else {
        for (const auto& [id, botPtr] : team) {
            if (!botPtr) {
                std::cout << "  ID " << id << " is null.\n";
                continue;
            }

            std::cout << "  ID: " << botPtr->id << "\n";
            std::cout << "    Name: " << botPtr->name << "\n";
            std::cout << "    Role: " << botPtr->role << "\n";
            std::cout << "    Power Level: " << botPtr->power_level << "\n";
            std::cout << "    Active: " << (botPtr->active ? "Yes" : "No") << "\n";
            std::cout << "    Position: (" << botPtr->position.x << ", " << botPtr->position.y << ", " << botPtr->position.z << ")\n";
            std::cout << "    Action Queue: " << botPtr->action_queue.size() << " actions\n";
            for (const auto& act : botPtr->action_queue) {
                print_action(act);
            }
        }
    }

    std::cout << "=====================\n";
}
