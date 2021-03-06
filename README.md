## Server
This is a program operating as the main server for our another project.  
Language : C++  
IDE : VS 2017  
Others : Google Protocol Buffers for the data serialization between C++ and C#, WireShark to look through the packets.

## Things to be done
- Self-Study on IOCP and various concepts related to it.  
    - Async IO  
    - Non-Blocking IO  
    - How to create an IO completion port and register a socket to it.
- Self-Study on Object Serialization in C++
    - Create a data format for the better communication between the server and the client.
    - Use "Protocol Buffer" for communication between the server(C++) and client(C#)  
- Write codes for accepting connection requests from clients.
    - When the connection process is done, send the roomlist and username for the initialization on the client side.
- Event Handling  
    - Send that list to a client  
        - Only when asked ( like a client clicks "Refresh" button )  
    - Create Game Room.
        - Request arrives from the client { type, Room Name, Limits, User Name }
        - Search the map using "room name" as the key to see if it's already in use.
            - if(map.find(roomName) != map.end()) -> Send the error message to the client.
        - Create a new room, set the values for the room and insert data into maps.
        - Send the information of the room processed as RoomInfo class type.
            
    - Enter Room.
        - Request arrives from the client { type, Room Name, User Name }
        - Search the map using "room name" as the key to get the roomId.
        - Search the roomlist using "roomId" as the key to see if the room still exists.
            - if(roomList.find(roomIdToEnter) == roomList.end()) -> Send the error message to the client.
        - Check if the game has already started, using Room::HasGameStarted() function.
            - if it has -> Send the error message to the client.
        - Check if the room is already full, using RoomInfo::current() and RoomInfo::limit() function.
            - if it is -> Send the error message to the client.
        - Create Client instance, set the values, update Room and RoomInfo instances to which the client belongs.
        - Broadcast the updated RoomInfo instance to the clients in the room.
            
    - Push ready button.(After Entering the room)
        - Request arrives from the client { type = READY_EVENT }
        - Get the Room instance where the client is currently located.
        - Call Room::ProcessReadyEvent(Client*) function
            - if the client is in ready-state, make it "not-ready state" and increment RoomInfo::readycount by 1.
            - otherwise, do the opposite.
        - broadcast the updated RoomInfo instance to the clients in the room.
    - Team Change
        - Request arrives from the client { type = TEAM_CHANGE }
        - Get the Room instance where the client is currently located.
        - Call Room::ProcessTeamChangeEvent(Client*) function.
            - Get the next position for the client, remove it from the prev team array and add it to the opposite team array.
            - Adjust the positions of clients affected by this move.
            - If the client is the host of the room, update RoomInfo::host value.
        - Broadcast the updated RoomInfo instance to the clients in the room.
    - Leave the room
        - Request arrives from the client { type = LEAVE_GAMEROOM }
        - Get the Room instance where the client is currently located.
        - Call Room::ProcessLeaveGameroomEvent(Client*) function.
            - if the client is in ready-state, decrement RoomInfo::readycount by 1.
            - Remove the client from the current team.
            - Remove the client socket information from the broadcast pool.
            - if the room has now 0 client -> release the resources and destroy it.
            - otherwise... check if the client is the host of the room
                - if so, call Room::ChangeGameroomHost() function and change the host.
            - Decrement RoomInfo::current by 1.
    - Chat
        - Message arrives from the client { type, Chat Message }
        - Broadcast the Data-type object to the clients in the room.
    - WorldState
        - Clients regularly send the in-game information to the server.
        - Broadcast the WorldState-type object to the clients in the room.
