//
// Created by dszhdankin on 13.01.2021.
//

#ifndef NAVY_COMBAT_PLAYER_H
#define NAVY_COMBAT_PLAYER_H
#include "../utils/IReader.h"
#include "../utils/IWriter.h"
#include "Ship.h"
#include <memory>
#include <string>
#include <vector>

class Player {
private:
    int _fd, ship_cnt = 0;
    int _ships_types[5];
    std::unique_ptr<IReader> _reader;
    std::unique_ptr<IWriter> _writer;
    std::shared_ptr<Ship> _sea[10][10];
    std::vector<std::shared_ptr<Ship>> _ships;
    std::string _shoot_channel;
    bool _hit[10][10];

    bool canPutSubmarine(int i, int j);

public:
    Player(int fd);

    //Shoots THIS player
    std::string shootAt(int i, int j);
    bool writeShootAt();

    bool readShip();
    //Tries read shot of this player to opponent
    bool tryGetShot(int &i, int &j);

    std::string getToken();
    bool writeTokens(const std::string &token);

    //For testing
    std::string getShips();

    ~Player() {};
};


#endif //NAVY_COMBAT_PLAYER_H
