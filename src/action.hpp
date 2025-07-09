#pragma once

#include <vector>

enum class action_type {
    MOVE,
    IDLE,
};

enum class Tag {
    CAUTION,
    USE_FORCE,
    GATHER_INFO,
    // Add more tags here
};

struct coords {
    int x = 0;
    int y = 0;
    int z = 0;
};

class action {
public:
    action_type type;
    std::vector<Tag> tags;

    virtual ~action() = default;
};

class move_action : public action {
public:
    move_action() {
        type = action_type::MOVE;
    }

    coords destination;
    int time = 0; // Time in seconds to reach destination
};

class idle_action : public action {
public:
    idle_action() {
        type = action_type::IDLE;
    }
};
