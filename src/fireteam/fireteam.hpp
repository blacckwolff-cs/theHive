#pragma once

#include <map>
#include <memory>
#include "bots/bot.hpp"

// Forward declarations
class Bot;
class teamLead;

class Fireteam {
public:
    // Construct from a map of bots
    explicit Fireteam(std::map<int, std::unique_ptr<Bot>> team);

    void addBot(int id, std::unique_ptr<Bot> bot);
    void removeBot(int id);
    void setLeader(std::unique_ptr<teamLead> lead);

    [[nodiscard]] teamLead* getLeader() const { return leader.get(); }
    [[nodiscard]] const std::map<int, std::unique_ptr<Bot>>& getTeam() const { return team; }

    void dump() const;

    friend class teamLead;

private:
    std::map<int, std::unique_ptr<Bot>> team;
    std::unique_ptr<teamLead> leader = nullptr;
};
