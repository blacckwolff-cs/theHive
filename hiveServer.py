import zmq
import json
import random

context = zmq.Context()
socket = context.socket(zmq.REP)
socket.bind("tcp://*:5555")

print("Hive server listening on port 5555...")

# Predefine available tags
tag_choices = ["CAUTION", "USE_FORCE", "GATHER_INFO"]

while True:
    # Receive state
    message = socket.recv()
    data = json.loads(message.decode())
    print("Received state:", json.dumps(data, indent=2))

    # Build a lookup of bot IDs to names for display
    bot_names = {}

    # Add leader
    leader_id = data["fireteam_leader"]["leader_id"]
    leader_name = data["fireteam_leader"]["leader_name"]
    bot_names[leader_id] = leader_name

    # Add members
    for member in data["fireteam_members"]:
        bot_id = member["member_id"]
        bot_name = member["member_name"]
        bot_names[bot_id] = bot_name

    # Decide randomly how many actions to create
    num_actions = random.randint(0, 3)
    actions = []

    for _ in range(num_actions):
        # Random target bot
        target_id = str(random.choice([leader_id, "2", "3"]))

        # Random destination
        dest = {
            "x": random.randint(0, 20),
            "y": 0,
            "z": random.randint(0, 20)
        }

        # Random tags
        tags = random.sample(tag_choices, k=random.randint(1, 2))

        # Build the action dict
        action = {
            "target_id": target_id,
            "type": "move",
            "destination": dest,
            "tags": tags
        }

        actions.append(action)

        # Lookup target name
        target_name = bot_names.get(target_id, "(unknown)")

        # Print nicely
        tag_list = ", ".join(tags)
        print(
            f"SENDING ACTION: Move({dest['x']},{dest['y']},{dest['z']}) "
            f"TO {target_id} [{target_name}] Tags: {tag_list}"
        )

    # Send reply
    reply = {
        "actions": actions
    }

    socket.send_string(json.dumps(reply))
