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
- Manage a list of gamerooms.  
    - Send that list to a client  
        - Only when asked ( like a client clicks "Refresh" button )  
    - Create Game Room.
        - Request arrives from the client { type, Room Name, Limits, User Name }
            1. Search the map using "room name" as the key to see if it's already in use.
                - if(map.find(roomName) != map.end()) -> Send the error message to the client.
            2. Create a new room, set the values for the room and insert data into maps.
            3. Send the information of the room processed as RoomInfo class type.
            
    - Enter Room.
        - Request arrives from the client { type, Room Name, User Name }
            1. Search the map using "room name" as the key to get the roomId.
            2. Search the roomlist using "roomId" as the key to see if the room still exists.
                - if(roomList.find(roomIdToEnter) == roomList.end()) -> Send the error message to the client.
            3. Check if the game has already started, using Room::HasGameStarted() function.
                - if it has -> Send the error message to the client.
            4. Check if the room is already full, using RoomInfo::current() and RoomInfo::limit() function.
                - if it is -> Send the error message to the client.
            5. 
            
    - Push ready button.(After Entering the room)
        - Request arrives from the client { type = READY_EVENT }
        
    - Team Change
        - Request arrives from the client { type = TEAM_CHANGE }
    - Leave the room.
        - Request arrives from the client { type = LEAVE_GAMEROOM }
    - Chat
        - Message arrives from the client { type, Chat Message }
