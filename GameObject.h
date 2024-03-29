//
// Created by Edgar on 2023/11/6.
//

#ifndef TEXTADV_GAMEOBJECT_H
#define TEXTADV_GAMEOBJECT_H
#include <string>
#include <cstdint>
#include <list>
#include <fstream>

using std::string;
class GameObject {
public:
    const string* name;
    const string* description;
    const string* keyword;
    const uint16_t id;
    virtual string getType();
    static std::list<GameObject*> allObjects;
    static GameObject* searchAll(const std::string &keyword);
    static void describe(GameObject* object);
    static uint16_t currentId;
    GameObject(const string* _name, const string* _desc, const string* _keyword);
    ~GameObject();
};

class FoodObject:public GameObject{
private:
    uint8_t energy;
public:
    FoodObject(const string* _name, const string* _desc, const string* _keyword, uint8_t _energy);
    uint8_t getEnergy() const;
    string getType() override;
};


#endif //TEXTADV_GAMEOBJECT_H
