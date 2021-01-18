//
// Created by dszhdankin on 13.01.2021.
//

#include "Player.h"
#include "../utils/Reader.h"
#include "../utils/Writer.h"
#include <cstring>
#include <cassert>
#include <algorithm>

Player::Player(int fd): _reader(new Reader(fd, 30)), _writer(new Writer(fd)), _shoot_channel() {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            _sea[i][j] = nullptr;
            _hit[i][j] = false;
        }
    }
    _ships.reserve(10);
    memset(_ships_types, 0, 5);
}

std::string Player::getToken() { return _reader->getToken(); }

bool Player::writeTokens(const std::string &token) { return _writer->writeTokens(token); }

bool Player::canPutSubmarine(int i, int j) {
    int stepI[] = {0, 0, 0, 1, 1, 1, -1, -1, -1};
    int stepJ[] = {0, 1, -1, 0, 1, -1, 1, 0, -1};

    if (i < 0 || i > 9 || j < 0 || j > 9)
        return false;

    for (int t = 0; t < 9; t++) {
        if (i + stepI[t] < 0 || i + stepI[t] > 9)
            continue;
        if (j + stepJ[t] < 0 || j + stepJ[t] > 9)
            continue;
        if (_sea[i + stepI[t]][j + stepJ[t]].get() != nullptr)
            return false;
    }

    return true;
}

//Reads ship and stores it
//Checks if it is valid ship and if it is possible to have such ship
bool Player::readShip() {
    std::shared_ptr<Ship> res(new Ship);
    std::string curToken;

    curToken = _reader->getToken();
    int size = 0;
    if (curToken == "battleship")
        size = 4;
    else if (curToken == "cruiser")
        size = 3;
    else if (curToken == "destroyer")
        size = 2;
    else if (curToken == "submarine")
        size = 1;
    if (size == 0) {
        return false;
    }
    res->setSize(size);
    res->setHP(size);

    curToken = _reader->getToken();
    if (curToken == "hor")
        res->setVertical(false);
    else if (curToken == "vert")
        res->setVertical(true);
    else {
        return false;
    }

    bool i_flag = false, j_flag = false;
    for (int i = 0; i < 2; i++) {
        curToken = _reader->getToken();
        if (curToken == "coord_i" || curToken == "coord_j") {
            std::string prevToken = curToken;
            curToken = _reader->getToken();
            if (curToken.size() != 1) {
                return false;
            }
            if (curToken[0] < '0' || curToken[0] > '9') {
                return false;
            }
            if (prevToken.back() == 'i') {
                res->setI(curToken[0] - '0');
                i_flag = true;
            } else if (prevToken.back() == 'j') {
                res->setJ(curToken[0] - '0');
                j_flag = true;
            }
        } else {
            return false;
        }
    }
    if (!i_flag || !j_flag)
        return false;

    int i = res->getI(), j = res->getJ(), stepI = 0, stepJ = 0;
    if (res->isVertical()) {
        stepI = 1;
        stepJ = 0;
    } else {
        stepI = 0;
        stepJ = 1;
    }
    for (int t = 0; t < res->Size(); t++, i += stepI, j += stepJ) {
        if (!canPutSubmarine(i, j))
            return false;
    }

    _ships_types[res->Size()]++;
    if (_ships_types[1] > 4 || _ships_types[2] > 3 || _ships_types[3] > 2 || _ships_types[4] > 1) {
        _ships_types[res->Size()]--;
        return false;
    }
    _ships.push_back(res);
    for (int t = 0, i = res->getI(), j = res->getJ(); t < res->Size(); t++, i += stepI, j += stepJ) {
        _sea[i][j] = res;
    }
    return true;
}

//Gets shot of this player to opponent
//Reads from socket
bool Player::tryGetShot(int &i, int &j) {
    std::string token;

    token = _reader->getToken();
    if (token != "shoot_at")
        return false;

    token = _reader->getToken();
    if (token.size() != 1)
        return false;
    if (token[0] < '0' || token[0] > '9')
        return false;
    i = token[0] - '0';

    token = _reader->getToken();
    if (token.size() != 1)
        return false;
    if (token[0] < '0' || token[0] > '9')
        return false;
    j = token[0] - '0';

    return true;
}

//Shoots THIS player
//If it is possible to perform shooting writes message to _shoot_channel
//Returns shoot status to send it to the opponent that performed the shot
//Should be called only if it is this player's turn
std::string Player::shootAt(int i, int j) {
    //TODO implement method properly
    assert(_shoot_channel == "");

    if (0 > i || i > 9 || 0 > j || j > 9)
        return "error";

    std::string status;

    if (_hit[i][j])
        return "already_shot";

    _hit[i][j] = true;
    _shoot_channel = "you_were_shot_at;";
    _shoot_channel.push_back('0' + i);
    _shoot_channel.push_back(';');
    _shoot_channel.push_back('0' + j);
    _shoot_channel.push_back(';');

    if (_sea[i][j].get() == nullptr) {
        _shoot_channel += "miss;";
        return "miss";
    }

    _sea[i][j]->shootAt();
    if (!_sea[i][j]->isAlive()) {
        _ships_types[_sea[i][j]->Size()]--;
        if (_ships_types[1] + _ships_types[2] + _ships_types[3] + _ships_types[4] == 0) {
            _shoot_channel += "game_over_you_lost;";
            return "game_over_you_win";
        }
        _shoot_channel += "destroyed;";
        return "destroyed";
    } else {
        _shoot_channel += "hit;";
        return "hit";
    }
}

bool Player::writeShootAt() {
    assert(_shoot_channel != "");
    bool res = _writer->writeTokens(_shoot_channel);
    _shoot_channel.clear();
    return res;
}

//For testing
std::string Player::getShips() {
    std::string strings[10], ans;
    std::fill(strings, strings + 10, "++++++++++");

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (_sea[i][j].get() != nullptr)
                strings[i][j] = '*';
        }
        ans += strings[i] + "\n";
    }

    return ans;
}