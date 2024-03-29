
#include <sstream>
#include "State.h"
#include "wordwrap.h"
#include "strings.h"


/**
 * Current state of the game.
 */

/**
 * Display the description of the room the player is in. */

void State::announceLoc() const {
    this->currentRoom->describe();
}

/**
 * Constructor.
 * @param startRoom Pointer to the room to start in.
 */
State::State(Room *startRoom) : currentRoom(startRoom), strength(100) {};

/**
 * Move to a specified room and print its description.
 * @param target Pointer to the room to move to.
 */
void State::goTo(Room *target) {
    this->currentRoom = target;
    this->announceLoc();
}

/**
 * Return a pointer to the current room.
 * @return Pointer to the current room.
 */
Room* State::getCurrentRoom() const {
    return this->currentRoom;
}

/**
 * Change player’s physical strength.
 * @param variation Value used to change the strength.
 */
void State::changeStrength(int16_t variation) {
    this->strength += variation;
    if(this->strength > 100) strength = 100;
}

/**
 * Get the strength of current state
 * @return
 */
int16_t State::getStrength() const {
    return this->strength;
}

/**
 * Function to search for a specific object in the current room based on a keyword.
 * @param keyword The keyword to search for.
 * @return A pointer to the matching GameObject if found, otherwise nullptr.
 */
GameObject* State::searchRoom(const std::string &keyword) {
    GameObject* objectFound = nullptr;
    for(GameObject* object:currentRoom->roomObjects){
        if (keyword == *(object->keyword)){
            objectFound = object;
            break;
        }
    }
    return objectFound;
}

/**
 * Search for a specific object in the player's inventory based on a keyword.
 * @param keyword The keyword to search for.
 * @return A pointer to the matching GameObject if found, otherwise nullptr.
 */
GameObject* State::searchInventory(const std::string &keyword) {
    GameObject* objectFound = nullptr;
    for (GameObject* object:inventory){
        if (keyword == *(object->keyword)){
            objectFound = object;
            break;
        }
    }
    return objectFound;
}

/**
 * Put selected object into inventory and delete it from room
 * @param keyword The unique keyword of selected object
 * @return The state of operation, 0 for successfully operation, 1 for object already in inventory, 2 for the object not
 * in this room, 3 for this object doesn't exist.
 */
uint8_t State::pickObject(const std::string &keyword) {
    uint8_t operationState;
    GameObject* pickedObject = searchRoom(keyword);
    if (pickedObject != nullptr){
        this->inventory.push_back(pickedObject);
        currentRoom->roomObjects.remove(pickedObject);
        operationState = 0; // successfully get
    }
    else if (searchInventory(keyword) != nullptr) operationState = 1; // already in inventory
    else if (GameObject::searchAll(keyword) != nullptr) operationState = 2; // not in this room
    else operationState = 3; // not exist
    return operationState;
}

/**
 * Drop object to thw room
 * @param keyword The unique keyword of selected object
 * @return The state of operation, 0 for successfully operation, 1 for object already in room, 2 for the object not
 * in this inventory, 3 for this object doesn't exist.
 */
uint8_t State::dropObject(const std::string &keyword) {
    uint8_t operationState;
    GameObject* droppedObject = searchInventory(keyword);
    if (droppedObject != nullptr){
        currentRoom->roomObjects.push_back(droppedObject);
        inventory.remove(droppedObject);
        operationState = 0; // successfully drop
    }
    else if(searchRoom(keyword) != nullptr) operationState = 1; // already in room
    else if(GameObject::searchAll(keyword) == nullptr) operationState = 3; // not exist
    else operationState = 2; // not in inventory
    return operationState;
}

/**
 * Describe the objects in the inventory.
 */
void State::inventoryDescribe() const {
    wrapOut(&inventoryObjectsMessage);
    wrapEndPara();
    for (GameObject* object:inventory){
        wrapOut(object->name);
        wrapEndPara();
    }
}

/**
 * Eat the food and add strength
 * @param keyword the keyword of food
 * @return the state of operation, 0 for successfully eating, 1 for can't found that food in the inventory, 2 for the
 * selected object is not a food.
 */
uint8_t State::eat(const string &keyword) {
    uint8_t state = 0;
    GameObject* food = searchInventory(keyword);
    if (food == nullptr) state = 1;
    else if(food->getType() == "food") {
        this->changeStrength(dynamic_cast<FoodObject*>(food)->getEnergy());
        inventory.remove(food);
    }
    else state = 2;
    return state;
}

/**
 * Export the inventory in plain text.
 * @return The string with the information of objects in the inventory.
 */
string State::exportInventory() {
    string idString = "0\40";
    for(GameObject* object:inventory){
        idString.append(std::to_string(object->id));
        idString.append("\40");
    }
    return idString;
}

/**
 * Load the state of game from a file stream.
 * @param ifs The input file stream to load state.
 */
void State::loadState(std::ifstream &ifs) {
    string validateStr;  //string to validate the file
    string line;
    uint16_t id;
    getline(ifs, validateStr);
    std::vector<uint16_t> idArray;
    if(validateStr == "EDGAR VER1.3.0"){
        ifs >> this->strength >> id;
        for(Room* room:Room::rooms){
            if(room->id == id){
                this->currentRoom = room;
                break; // stop iterating rooms after found
            }
        }
        while(ifs.good()){
            getline(ifs,line);
            std::stringstream ss(line);
            while(ss.good()){
                ss >> id;
                idArray.push_back(id);
            }
            for(Room* room:Room::rooms){
                if(room->id == idArray[0]){
                    room->roomObjects.clear(); // Empty for restore
                    if (idArray.size() > 1) {
                        for (uint16_t i = 1; i < idArray.size() - 1; i++) {
                            for (GameObject *object: GameObject::allObjects) {
                                if (object->id == idArray[i]) {
                                    room->roomObjects.push_back(object);
                                    break; // stop iterating objects after found
                                }
                            }
                        }
                    }
                    break; // stop iterating rooms after found
                }else if(idArray[0] == 0){ // 0 is reserved for inventory
                    this->inventory.clear(); // Empty to restore
                    if(idArray.size() > 1) {
                        for (uint16_t i = 1; i < idArray.size() - 1; i++) {
                            for (GameObject *object: GameObject::allObjects) {
                                if (object->id == idArray[i]) {
                                    this->inventory.push_back(object);
                                    break; // stop iterating objects after found
                                }
                            }
                        }
                    }
                    break; // stop iterating rooms after identified inventory
                }
            }
            idArray.clear(); //  Empty the idArray to be ready for next line
        }
        if(ifs.eof()){ // read to the end
            wrapOut(&loadOkMessage);
            wrapEndPara();
        }
    }
    else if(!ifs.good()){ // read fail
        wrapOut(&loadFailMessage);
        wrapEndPara();
    }
    else{ // validation fail
        wrapOut(&validateFailMessage);
        wrapEndPara();
    }
}