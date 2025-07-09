#pragma once

#include "bot.hpp"

// Forward declaration
class teamLead;

class basicBot final : public Bot {
    friend class Hive;

public:
    basicBot(const std::string& id_, const std::string& name_, const std::string& role_);

    void enqueue_move(int x, int y) override;
    void execute_actions() override;
    void update_state(const nlohmann::json& data) override;
    void make_move() override;
    nlohmann::json to_json() const override;
    void sendToLeader(teamLead& leader) const override;
};
