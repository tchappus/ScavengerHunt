#include "thread.h"
#include "socketserver.h"
#include <stdlib.h>
#include <time.h>
#include <list>
#include <vector>
#include <string>
#include <locale>
#include "SharedObject.h"
#include <iostream>
#include <fstream>
#include <sstream>

class Game {
public:
    std::string ID;
    std::string status;
    std::vector<std::string> players;
    std::string word;
    std::vector<std::string> words;
    Sync::ThreadSem waitingSem;
    time_t timeStarted;
    int numMatches;
    Game() {

        words = {"Kettle",
                "Toaster",
                "Spatula",
                "Spoon",
                "Cup",
                "Fork",
                "Mug",
                "Bowl",
                "Plate",
                "Mixing bowl",
                "Glass",
                "Fruit bowl",
                "Clock",
                "Curtain",
                "Cushion",
                "Picture",
                "Stereo",
                "CD",
                "Vase",
                "Book",
                "Jug",
                "Saucepan",
                "Knife",
                "Hook",
                "Fabric",
                "Shoe",
                "Soap",
                "Washing powder",
                "Sponge",
                "Cloth",
                "Baking tray",
                "Cake tin",
                "Rug",
                "Towel",
                "Quilt",
                "Mirror",
                "Broom",
                "Brush",
                "Garbage",
                "Paper",
                "Pen",
                "Pencil",
                "Light",
                "Vacuum",
                "Shelves",
                "Toilet roll",
                "Food",
                "Drink",
                "Duster",
                "Plant",
                "Bucket",
                "TV",
                "Box",
                "Clothes hanger",
                "Tin",
                "Jewellery",
                "Toiletries",
                "Bed",
                "Pillow",
                "Computer",
                "Laptop",
                "Lamp",
                "Blinds"
        };

        srand(time(0));
        
        int randomID = rand() % 99999 + 900000;
        ID = std::to_string(randomID);

        status = "WAITING";
        int index = rand() % words.size();
        word = words[index];
        timeStarted = time(0);
        numMatches = 0;
    }
    ~Game() {
        //deleting vectors
        std::vector<std::string>().swap(players);
        std::vector<std::string>().swap(words);

    }
};