
struct ver_info
{
    unordered_map<int, vector<int>> n_ec_codewords;
    unordered_map<int, vector<array<int, 4>>> structure;
};


int binToDec(vector<bool> binary)
{
    int dec = 0;
    for(int i=0; i < binary.size(); i++)
    {
        if(binary[i])dec += pow(2, binary.size() - 1 - i);
    }
    return dec;
}


vector<bool> decToBin(int n, int length)
{
    vector<bool> output = {};

    for(int i = 1; i <= length; i++)output.push_back((n>>(length-i)) & 1);

    return output;
}


class data_type{
    public:
        string input;
        int version;string EC_lvl;
        int n_EC_codewords;array<int, 4> structure;
        vector<group> sorted_data;vector<group> EC_codewords;
        vector<bool> final_message;


        data_type(string inp, int V, string EC_level)
        {
            EC_lvl = EC_level;
            version = V;
            input = inp;

            ver_info info = version_information();
            int EC_lvl_int;
            if(EC_lvl == "L")EC_lvl_int = 0;
            if(EC_lvl == "M")EC_lvl_int = 1;
            if(EC_lvl == "Q")EC_lvl_int = 2;
            if(EC_lvl == "H")EC_lvl_int = 3;

            n_EC_codewords = info.n_ec_codewords[version][EC_lvl_int];
            structure = info.structure[version][EC_lvl_int];
        }


        void encodeData()
        {
            vector<bool> encoded_data;

            vector<bool> mode = {0, 1, 0, 0};

            int len_bit = (version < 10) ? 8 : 16;
            vector<bool> inp_len = decToBin(input.length(), len_bit);
            vector<bool> text = {};

            for(int i=0; i < input.length(); i++)
            {
                vector<bool> character = decToBin(input[i], 8);
                text.insert(text.end(), character.begin(), character.end());
            }

            int bit_size = (structure[0] * structure[1] + structure[2] * structure[3]) * 8;

            encoded_data.reserve(bit_size);

            encoded_data.insert(encoded_data.end(), mode.begin(), mode.end());
            encoded_data.insert(encoded_data.end(), inp_len.begin(), inp_len.end());
            encoded_data.insert(encoded_data.end(), text.begin(), text.end());

            //Terminator
            for(int i=0; i < 4; i++)encoded_data.push_back(0);

            //pad bytes 236 17
            vector<vector<bool>> pad_bytes = {{1, 1, 1, 0, 1, 1, 0, 0}, {0, 0, 0, 1, 0, 0, 0, 1}};

            int n_pads = ((bit_size - encoded_data.size()) / 8);
            for(int i=0; i < n_pads; i++)encoded_data.insert(encoded_data.end(), pad_bytes[i%2].begin(), pad_bytes[i%2].end());

            sorted_data = dataSort(encoded_data);
        }


        void generateEC()
        {   
            vector<int> galois_field = galois_numbers();
            
            vector<int> gen_alpha = generate_generator_poly(galois_field);
            group EC_group;block EC_binBlock;

            for(int g=0; g < sorted_data.size(); g++)//through each group
            {
                EC_group.clear();
                
                for(int b=0; b < sorted_data[g].size(); b++)//through each block
                {
                    EC_binBlock.clear();

                    vector<int> msg_poly;
                    for(int cw=0; cw < sorted_data[g][b].size(); cw++)//creates the message polynomial
                    {
                        msg_poly.push_back(binToDec(sorted_data[g][b][cw]));
                    }
                    int n = msg_poly.size();
                    
                    //repeates the divison n times
                    for(int i=0; i < n;)msg_poly = poly_divison(msg_poly, gen_alpha, galois_field, i);
                
                    for(int i=0; i < msg_poly.size(); i++)EC_binBlock.push_back(decToBin(msg_poly[i], 8));//transforms to binary
                    EC_group.push_back(EC_binBlock);
                }
                EC_codewords.push_back(EC_group);
            }
        }


        void createFinalMessage()
        {
            vector<codeword> message = extract_and_columonize(sorted_data);
            vector<codeword> EC = extract_and_columonize(EC_codewords);

            int n = 0;
            for(codeword word : message){for(bool x : word){final_message.push_back(x);n++;}}

            int m = 0;
            for(codeword word : EC){for(bool x : word){final_message.push_back(x);m++;}}

        }
    

    private:
        vector<int> to_alpha_notation(vector<int>& polynome, vector<int>& exponents_galois_field) //transforms to alpha notation
        { 
            vector<int> exponent(polynome.size());
            for(int i=0; i < polynome.size(); i++)
            {   
                int exp;
                if(polynome[i] == 0)exp = -1;
                else exp = find(exponents_galois_field.begin(), exponents_galois_field.end(), polynome[i]) - exponents_galois_field.begin();
                exponent[i] = exp;
            }
            return exponent;
        }


        vector<int> to_decimal_notation(vector<int>& polynome, vector<int>& exponents_galois_field) //transforms from alpha notation
        { 
            vector<int> decimal(polynome.size());
            for(int i=0; i < polynome.size(); i++)
            {   
                int n;
                if(polynome[i] == -1)n=0;
                else n = exponents_galois_field[polynome[i]];
                decimal[i] = n;
            }
            return decimal;
        }


        vector<int> galois_numbers()//generates a list of exponents of 2 inside of galois field
        {
            vector<int> numbers(255);

            int n = 1;
            numbers[0] = n;

            for(int i=1; i < 256; i++)
            {
                n *= 2;
                while(255 < n)n = n ^ 285;
                numbers[i] = n;
            }

            return numbers;
        }


        vector<int> poly_divison(vector<int>& message, vector<int> generator, vector<int>& galois_numbers, int& i)
        {
            vector<int> message_alpha = to_alpha_notation(message, galois_numbers);

            for(int i=0; i < generator.size(); i++)generator[i] = (generator[i] + message_alpha[0]) % 255;

            generator = to_decimal_notation(generator, galois_numbers);

            vector<int> new_poly;
            int max_size = (generator.size() < message.size()) ? message.size() : generator.size();

            for(int i=0; i < max_size; i++)
            {
                int x = (generator.size() < i+1) ? 0 : generator[i];
                int y = (message.size() < i+1) ? 0 : message[i];

                new_poly.push_back(x^y);
            }

            while(new_poly[0] == 0)
            {
                new_poly.erase(new_poly.begin());//removing leading 0
                i++;
            }

            return new_poly;
        }


        vector<group> dataSort(vector<bool>& encoded_data)
        {
            int x = 0; //where in data
            for(int g=0; g < 2; g++)
            {
                group groups;
                for(int b=0; b < structure[2*g]; b++)
                {   
                    block blocks;
                    for(int c=0; c < structure[2*g+1]; c++)
                    {   
                        codeword codewords;
                        for(int i=0; i < 8; i++){codewords.push_back(encoded_data[x]);x++;}
                        blocks.push_back(codewords);
                    }
                    if(blocks.size() != 0)groups.push_back(blocks);
                }

                if(groups.size() != 0)sorted_data.push_back(groups);
            }

            return sorted_data;
        }


        vector<codeword> extract_and_columonize(vector<group> d)
        {
            vector<block> blocks;

            int biggest = 0;
            //collects all blocks in one vector
            for(int i=0; i < d.size(); i++)
            {
                for(int j=0; j < d[i].size(); j++)
                {   
                    if(biggest < d[i][j].size())biggest = d[i][j].size();
                    blocks.push_back(d[i][j]);
                }
            }


            //puts them into columns
            vector<codeword> new_order;
            for(int i=0; i < biggest; i++)
            {
                for(int blo=0; blo < blocks.size(); blo++)
                {
                    if(i < (blocks[blo].size()))new_order.push_back(blocks[blo][i]);
                }
            }

            return new_order;
        }

        
        vector<int> generate_generator_poly(vector<int>& galois_n)
        {
            int n = n_EC_codewords;
            vector<int> current = {0, 0};

            for(int i=1; i < n; i++)
            {
                vector<int> result(1 + current.size(), 0);
                vector<int> multiply = {0, i};

                for(int expN=0; expN < current.size(); expN++)
                {   
                    for(int expN2=0; expN2 < 2; expN2++)
                    {
                        int alpha = (current[expN] == -1) ? multiply[expN2] : (current[expN] + multiply[expN2]) % 255;

                        int exp = (current.size() - 1 - expN) + (1 - expN2);
                        result[result.size() - 1 - exp] ^= galois_n[alpha];
                    }
                }
                current = to_alpha_notation(result, galois_n);
            }
            return current;  
        }


        ver_info version_information()
        {
            unordered_map<int, vector<int>> EC_info;
            unordered_map<int, vector<array<int, 4>>> data_info;
            EC_info[1] = {7, 10, 13, 17}; data_info[1] = {{1, 19, 0, 0}, {1, 16, 0, 0}, {1, 13, 0, 0}, {1, 9, 0, 0}};
            EC_info[2] = {10, 16, 22, 28}; data_info[2] = {{1, 34, 0, 0}, {1, 28, 0, 0}, {1, 22, 0, 0}, {1, 16, 0, 0}};
            EC_info[3] = {15, 26, 18, 22}; data_info[3] = {{1, 55, 0, 0}, {1, 44, 0, 0}, {2, 17, 0, 0}, {2, 13, 0, 0}};
            EC_info[4] = {20, 18, 26, 16}; data_info[4] = {{1, 80, 0, 0}, {2, 32, 0, 0}, {2, 24, 0, 0}, {4, 9, 0, 0}};
            EC_info[5] = {26, 24, 18, 22}; data_info[5] = {{1, 108, 0, 0}, {2, 43, 0, 0}, {2, 15, 2, 16}, {2, 11, 2, 12}};
            EC_info[6] = {18, 16, 24, 28}; data_info[6] = {{2, 68, 0, 0}, {4, 27, 0, 0}, {4, 19, 0, 0}, {4, 15, 0, 0}};
            EC_info[7] = {20, 18, 18, 26}; data_info[7] = {{2, 78, 0, 0}, {4, 31, 0, 0}, {2, 14, 4, 15}, {4, 13, 1, 14}};
            EC_info[8] = {24, 22, 22, 26}; data_info[8] = {{2, 97, 0, 0}, {2, 38, 2, 39}, {4, 18, 2, 19}, {4, 14, 2, 15}};
            EC_info[9] = {30, 22, 20, 24}; data_info[9] = {{2, 116, 0, 0}, {3, 36, 2, 37}, {4, 16, 4, 17}, {4, 12, 4, 13}};
            EC_info[10] = {18, 26, 24, 28}; data_info[10] = {{2, 68, 2, 69}, {4, 43, 1, 44}, {6, 19, 2, 20}, {6, 15, 2, 16}};
            EC_info[11] = {20, 30, 28, 24}; data_info[11] = {{4, 81, 0, 0}, {1, 50, 4, 51}, {4, 22, 4, 23}, {3, 12, 8, 13}};
            EC_info[12] = {24, 22, 26, 28}; data_info[12] = {{2, 92, 2, 93}, {6, 36, 2, 37}, {4, 20, 6, 21}, {7, 14, 4, 15}};
            EC_info[13] = {26, 22, 24, 22}; data_info[13] = {{4, 107, 0, 0}, {8, 37, 1, 38}, {8, 20, 4, 21}, {12, 11, 4, 12}};
            EC_info[14] = {30, 24, 20, 24}; data_info[14] = {{3, 115, 1, 116}, {4, 40, 5, 41}, {11, 16, 5, 17}, {11, 12, 5, 13}};
            EC_info[15] = {22, 24, 30, 24}; data_info[15] = {{5, 87, 1, 88}, {5, 41, 5, 42}, {5, 24, 7, 25}, {11, 12, 7, 13}};
            EC_info[16] = {24, 28, 24, 30}; data_info[16] = {{5, 98, 1, 99}, {7, 45, 3, 46}, {15, 19, 2, 20}, {3, 15, 13, 16}};
            EC_info[17] = {28, 28, 28, 28}; data_info[17] = {{1, 107, 5, 108}, {10, 46, 1, 47}, {1, 22, 15, 23}, {2, 14, 17, 15}};
            EC_info[18] = {30, 26, 28, 28}; data_info[18] = {{5, 120, 1, 121}, {9, 43, 4, 44}, {17, 22, 1, 23}, {2, 14, 19, 15}};
            EC_info[19] = {28, 26, 26, 26}; data_info[19] = {{3, 113, 4, 114}, {3, 44, 11, 45}, {17, 21, 4, 22}, {9, 13, 16, 14}};
            EC_info[20] = {28, 26, 30, 28}; data_info[20] = {{3, 107, 5, 108}, {3, 41, 13, 42}, {15, 24, 5, 25}, {15, 15, 10, 16}};
            EC_info[21] = {28, 26, 28, 30}; data_info[21] = {{4, 116, 4, 117}, {17, 42, 0, 0}, {17, 22, 6, 23}, {19, 16, 6, 17}};
            EC_info[22] = {28, 28, 30, 24}; data_info[22] = {{2, 111, 7, 112}, {17, 46, 0, 0}, {7, 24, 16, 25}, {34, 13, 0, 0}};
            EC_info[23] = {30, 28, 30, 30}; data_info[23] = {{4, 121, 5, 122}, {4, 47, 14, 48}, {11, 24, 14, 25}, {16, 15, 14, 16}};
            EC_info[24] = {30, 28, 30, 30}; data_info[24] = {{6, 117, 4, 118}, {6, 45, 14, 46}, {11, 24, 16, 25}, {30, 16, 2, 17}};
            EC_info[25] = {26, 28, 30, 30}; data_info[25] = {{8, 106, 4, 107}, {8, 47, 13, 48}, {7, 24, 22, 25}, {22, 15, 13, 16}};
            EC_info[26] = {28, 28, 28, 30}; data_info[26] = {{10, 114, 2, 115}, {19, 46, 4, 47}, {28, 22, 6, 23}, {33, 16, 4, 17}};
            EC_info[27] = {30, 28, 30, 30}; data_info[27] = {{8, 122, 4, 123}, {22, 45, 3, 46}, {8, 23, 26, 24}, {12, 15, 28, 16}};
            EC_info[28] = {30, 28, 30, 30}; data_info[28] = {{3, 117, 10, 118}, {3, 45, 23, 46}, {4, 24, 31, 25}, {11, 15, 31, 16}};
            EC_info[29] = {30, 28, 30, 30}; data_info[29] = {{7, 116, 7, 117}, {21, 45, 7, 46}, {1, 23, 37, 24}, {19, 15, 26, 16}};
            EC_info[30] = {30, 28, 30, 30}; data_info[30] = {{5, 115, 10, 116}, {19, 47, 10, 48}, {15, 24, 25, 25}, {23, 15, 25, 16}};
            EC_info[31] = {30, 28, 30, 30}; data_info[31] = {{13, 115, 3, 116}, {2, 46, 29, 47}, {42, 24, 1, 25}, {23, 15, 28, 16}};
            EC_info[32] = {30, 28, 30, 30}; data_info[32] = {{17, 115, 0, 0}, {10, 46, 23, 47}, {10, 24, 35, 25}, {19, 15, 35, 16}};
            EC_info[33] = {30, 28, 30, 30}; data_info[33] = {{17, 115, 1, 116}, {14, 46, 21, 47}, {29, 24, 19, 25}, {11, 15, 46, 16}};
            EC_info[34] = {30, 28, 30, 30}; data_info[34] = {{13, 115, 6, 116}, {14, 46, 23, 47}, {44, 24, 7, 25}, {59, 16, 1, 17}};
            EC_info[35] = {30, 28, 30, 30}; data_info[35] = {{12, 121, 7, 122}, {12, 47, 26, 48}, {39, 24, 14, 25}, {22, 15, 41, 16}};
            EC_info[36] = {30, 28, 30, 30}; data_info[36] = {{6, 121, 14, 122}, {6, 47, 34, 48}, {46, 24, 10, 25}, {2, 15, 64, 16}};
            EC_info[37] = {30, 28, 30, 30}; data_info[37] = {{17, 122, 4, 123}, {29, 46, 14, 47}, {49, 24, 10, 25}, {24, 15, 46, 16}};
            EC_info[38] = {30, 28, 30, 30}; data_info[38] = {{4, 122, 18, 123}, {13, 46, 32, 47}, {48, 24, 14, 25}, {42, 15, 32, 16}};
            EC_info[39] = {30, 28, 30, 30}; data_info[39] = {{20, 117, 4, 118}, {40, 47, 7, 48}, {43, 24, 22, 25}, {10, 15, 67, 16}};
            EC_info[40] = {30, 28, 30, 30}; data_info[40] = {{19, 118, 6, 119}, {18, 47, 31, 48}, {34, 24, 34, 25}, {20, 15, 61, 16}};

            ver_info info;
            info.n_ec_codewords = EC_info;
            info.structure = data_info;

            return info;
        }
};
