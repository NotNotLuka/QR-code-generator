#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <algorithm>
#include <cmath>
#include <unordered_map>

#include "lib/bitmap_image.hpp"

using namespace std;

typedef vector<bool> codeword;
typedef vector<codeword> block;
typedef vector<block> group;

typedef vector<vector<bool>> bmp;


#include "cogs/data.cpp"
#include "helper/format_version_info.cpp"
#include "cogs/image.cpp"

//https://www.thonky.com/qr-code-tutorial/introduction

int main()
{   

    string input = "Do not scan random QR codes!!";

    int version = -1;
    string EC_lvl = "L";
    int mask = -1;

    data_type data(input, version, EC_lvl);

    data.encodeData();
    data.generateEC();
    data.createFinalMessage();


    image_type image(version, EC_lvl, mask, data.final_message);

    image.create_bitmap();
    image.create_image("qr-code-" + to_string(version) + EC_lvl + "-mask-" + to_string(image.mask));


    return 0;
}
