#pragma once

#include <string>
#include <deque>
#include <memory>
#include "../action.hpp"
#include "nlohmann/json.hpp"

class teamLead;

class Bot {
public:
    virtual ~Bot() = default;

    // Core: perform the next action (whatever it is)
    virtual void make_move() = 0;

    // Optional: enqueue an explicit move
    virtual void enqueue_move(int x, int y) = 0;

    // Send this bot's state to the team lead
    virtual void sendToLeader(teamLead& leader) const = 0;

    // Update state from JSON data
    virtual void update_state(const nlohmann::json& data) = 0;

    // Execute all queued actions
    virtual void execute_actions() = 0;

    // Convert state to JSON
    virtual nlohmann::json to_json() const = 0;

    // Friend class (only needed if state is protected/private)
    friend class Hive;

    // Action Queue
    std::deque<std::unique_ptr<action>> action_queue;

    // Identity
    std::string id;
    std::string name;
    std::string role;

    // State
    coords position;
    int power_level = 100;
    bool active = true;

};
