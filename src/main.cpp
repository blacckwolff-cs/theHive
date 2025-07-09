#include "hive/hive.hpp"
#include "fireteam/fireteam.hpp"
#include "bots/teamLead.hpp"
#include "bots/bot.hpp"
#include "bots/basicBot.hpp"
#include "action.hpp"
#include <nlohmann/json.hpp>
#include <memory>
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <zmq.hpp>

int main() {
    using json = nlohmann::json;

    // Load Fireteam config
    std::ifstream file("example.json");
    if (!file) {
        std::cerr << "Failed to open example.json\n";
        return 1;
    }
    json data;
    file >> data;

    std::map<int, std::unique_ptr<Bot>> teamBots;

    for (const auto& member : data["fireteam_members"]) {
        auto b = std::make_unique<basicBot>(
            member.value("member_id", "0"),
            member.value("member_name", "Unnamed"),
            member.value("member_role", "Unknown")
        );

        b->power_level = member.value("power_level", 100);
        b->active = member.value("member_status", "inactive") == "active";

        if (member.contains("position")) {
            b->position = parse_coords(member["position"]);
        }

        int bot_id = 0;
        try {
            bot_id = std::stoi(member.value("member_id", "0"));
        } catch (...) {
            std::cerr << "Invalid member_id format. Defaulting to 0.\n";
        }

        teamBots.emplace(bot_id, std::move(b));
    }

    auto fireteamPtr = std::make_unique<Fireteam>(std::move(teamBots));

    auto leaderBot = std::make_unique<teamLead>(
        data["fireteam_leader"].value("leader_id", "0"),
        data["fireteam_leader"].value("leader_name", "Unknown"),
        fireteamPtr.get()
    );

    leaderBot->power_level = data["fireteam_leader"].value("power_level", 100);
    leaderBot->active = data["fireteam_leader"].value("leader_status", "inactive") == "active";

    if (data["fireteam_leader"].contains("position")) {
        leaderBot->position = parse_coords(data["fireteam_leader"]["position"]);
    }

    fireteamPtr->setLeader(std::move(leaderBot));
    teamLead* leader = fireteamPtr->getLeader();

    zmq::context_t zmq_ctx(1);
    zmq::socket_t zmq_socket(zmq_ctx, zmq::socket_type::req);
    zmq_socket.connect("tcp://localhost:5555");

    Hive hive(*fireteamPtr);

    for (int tick = 1; tick <= 10; ++tick) {
        std::cout << "\n=== TICK " << tick << " ===\n";

        // 1. Bots send state to leader
        for (const auto& [id, botPtr] : fireteamPtr->getTeam()) {
            botPtr->sendToLeader(*leader);
        }

        // 2. Leader prepares state
        json state_json;
        state_json["fireteam_id"] = "TODO";
        state_json["fireteam_leader"] = leader->to_json();
        state_json["fireteam_members"] = json::array();
        for (const auto& [id, botPtr] : fireteamPtr->getTeam()) {
            state_json["fireteam_members"].push_back(botPtr->to_json());
        }

        // 3. Send state to Hive
        zmq::message_t request(state_json.dump());
        zmq_socket.send(request, zmq::send_flags::none);

        // 4. Receive reply
        zmq::message_t reply_msg;
        if (!zmq_socket.recv(reply_msg, zmq::recv_flags::none)) {
            std::cerr << "Failed to receive reply from Hive.\n";
            continue;
        }

        std::string reply_str(static_cast<char*>(reply_msg.data()), reply_msg.size());
        json reply_json = json::parse(reply_str);

        // 5. Clear old orders before applying new ones
        leader->clear_orders();

        if (reply_json.contains("actions") && reply_json["actions"].is_array()) {
            leader->receive_orders_json(reply_json["actions"]);
        }

        // 6. Dispatch orders to bots
        leader->dispatch_orders();

        // 7. Execute leader actions
        leader->execute_actions();

        // 8. Execute bots' actions
        for (auto& [id, botPtr] : fireteamPtr->getTeam()) {
            botPtr->execute_actions();
        }

        // 9. Dump state
        fireteamPtr->dump();

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "Simulation complete.\n";
}
