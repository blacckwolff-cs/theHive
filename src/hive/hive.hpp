#pragma once

#include "nlohmann/json.hpp"
#include <vector>
#include <memory>

// Forward declarations
class teamLead;
class action;
class Fireteam;

class Hive {
public:
    explicit Hive(Fireteam& ft)
        : fireteam(ft) {}

    void process(const nlohmann::json& j);
    void sendOrder(int botId, std::unique_ptr<action> act, teamLead& tl);

private:
    std::vector<std::unique_ptr<action>> actions;
    Fireteam& fireteam;
};
