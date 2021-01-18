#ifndef NAVY_COMBAT_SHIP_H
#define NAVY_COMBAT_SHIP_H


class Ship {
private:
    int _size, _hp, _i, _j;
    bool _vertical;

public:
    Ship(int size = 1, bool vertical = true): _size(size), _vertical(vertical) {}
    void shootAt() { _hp--; }
    bool isAlive() { return _hp > 0; }
    bool isVertical() { return _vertical; }
    int Size() { return _size; }
    int HP() { return _hp; }
    void setSize(int size) { _size = size; }
    void setHP(int hp) { _hp = hp; }
    void setVertical(int vertical) { _vertical = vertical; }
    void setI(int i) { _i = i; }
    void setJ(int j) { _j = j; }
    int getI() { return _i; }
    int getJ() { return _j; }
};


#endif //NAVY_COMBAT_SHIP_H
