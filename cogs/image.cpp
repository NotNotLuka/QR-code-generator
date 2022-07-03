
class image_type
{
    public:
        int version;string EC_lvl;int mask;

        int n_modules;

        bitmap_image image;
        bmp bitmap;bmp reserved;
        vector<bool> data;

        image_type(int V, string EC_level, int mask_version, vector<bool>& message)
        {
            mask = mask_version;
            version = V; EC_lvl = EC_level; data = message;
            n_modules = (((version-1)*4)+21);

            bitmap.resize(n_modules, vector<bool>(n_modules, 0));
            reserved.resize(n_modules, vector<bool>(n_modules, 0));

            bitmap_image img(n_modules * 10 + 80, n_modules * 10 + 80);image = img;
            image.set_all_channels(255);
        }


        void create_bitmap()
        {   
            add_patterns();
            add_data(reserved);
            add_mask();

            format_version_bits fvb(EC_lvl, mask, version);
            add_format_info(fvb.get_format_bits());
            if(7 <= version)add_version_info(fvb.get_version_bits());
        }


        void create_image(string save_name)
        {
            for(int y=0; y < bitmap.size(); y++)//draw
            {
                for(int x=0; x < bitmap[y].size(); x++)
                {
                    if(bitmap[y][x])draw_module(image, x, y);
                }
            }
            image.save_image("output/" + save_name + ".bmp");
        }


    private:
        void draw_module(bitmap_image& image, int x, int y)
        {
            x = x * 10 + 40;y = y * 10 + 40;
            for(int i=x; i < x + 10; i++)
            {
                for(int j=y; j < y + 10; j++)image.set_pixel(i, j, 0, 0, 0);
            }
        }


        void add_version_info(vector<bool> version_info)
        {   
            for(int i=0; i < 18; i++)
            {
                int x, y;
                y = n_modules - 11 + (i % 3);
                x = i / 3;
                
                bitmap[y][x] = version_info[17 - i];

                y = i / 3;
                x = n_modules - 11 + (i % 3);

                bitmap[y][x] = version_info[17 - i];
                
            }
        }


        void add_format_info(vector<bool> format_info)
        {
            int n_modules = (((version-1)*4)+21);
            for(int i=0; i < 8; i++)
            {
                int x, y;
                if(5 < i)x = i + 1;else x=i;
                if(1 < i)y = 8 - i - 1;else y=8-i;


                bitmap[y][8] = format_info[i+7];//top left vertical
                bitmap[8][x] = format_info[i];//top left horizontal

                bitmap[8][n_modules-(8-i)] = format_info[i+7];//top right
                
                if(i == 7)continue;
                bitmap[n_modules-1-i][8] = format_info[i];//bottom left
            }
        }

        void add_mask()
        {
            auto getChangeMask = [&] (int mask_version, array<int, 2> pos)
            {   
                bool change;
                switch(mask_version)
                {
                    case 0:{change = (pos[1] + pos[0]) % 2 == 0;break;}
                    case 1:{change = (pos[1]) % 2 == 0;break;}
                    case 2:{change = (pos[0]) % 3 == 0;break;}
                    case 3:{change = (pos[1] + pos[0]) % 3 == 0;break;}
                    case 4:{change = (pos[1] / 2 + pos[0] / 3) % 2 == 0;break;}
                    case 5:{change = ((pos[1] * pos[0]) % 2) + ((pos[1] * pos[0]) % 3) == 0;break;}
                    case 6:{change = (((pos[1] * pos[0]) % 2) + ((pos[1] * pos[0]) % 3) ) % 2 == 0;break;}
                    case 7:{change = (((pos[1] + pos[0]) % 2) + ((pos[1] * pos[0]) % 3) ) % 2 == 0;break;}

                }

                return change;
            };

            if(mask == -1)
            {
                vector<int> penalties(8);

                for(int i=0; i < 8; i++)
                {
                    vector<vector<bool>> penalty_calculator = bitmap;
                    for(int y=0; y < reserved.size(); y++)
                    {
                        for(int x=0; x < reserved[y].size(); x++)
                        {
                            if(!reserved[y][x] && getChangeMask(i, {x, y}))penalty_calculator[y][x] = !bitmap[y][x];
                        }
                    }
                    penalties[i] = calculatePenalty(penalty_calculator);
                }

                mask = 0;
                int minimal = penalties[0];
                for(int i=1; i < 8; i++){if(penalties[i] < minimal){minimal = penalties[i]; mask = i;}}
                cout << "mask: " << mask << endl;
            }

            for(int y=0; y < reserved.size(); y++)
            {
                for(int x=0; x < reserved[y].size(); x++)
                {
                    if(!reserved[y][x] && getChangeMask(mask, {x, y}))bitmap[y][x] = !bitmap[y][x];
                }
            }
        }


        int calculatePenalty(bmp& current)
        {
            int penalty = 0;
            int penalty_1 = 0;int penalty_2 = 0;int penalty_3 = 0; int penalty_4 = 0;

            //first condition
            int same_x;
            vector<int> same_y(current[0].size(), 1);
            vector<bool> previous_y = current[0];
            //third condition
            vector<vector<bool>> patterns = {{1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0}, {0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1}};
            vector<bool> pattern_x;
            vector<vector<bool>> pattern_y(current.size());
            //fourth condition
            int n_dark = 0;
            for(int y=0; y < current.size(); y++)
            {   
                //first condition
                same_x = 1;
                bool previous_x = current[y][0];
                for(int x=0; x < current.size(); x++)
                {
                    //first condition
                    if(x != 0)
                    {
                        if(current[y][x] == previous_x)same_x++;
                        else{
                            if(5 <= same_x)penalty_1 += same_x - 2;
                            same_x = 1;previous_x = current[y][x];
                        }
                    }

                    if(y!=0)
                    {
                        if(current[y][x] == previous_y[x])same_y[x]++;
                        else{
                            if(5 <= same_y[x])penalty_1 += same_y[x] - 2;
                            same_y[x] = 1;previous_y[x] = current[y][x];
                        }
                    }

                    //second condition
                    if((y+1) < current.size() && (x+1) << current[0].size())
                    {
                        if(current[y][x] == current[y+1][x] && current[y][x] == current[y+1][x+1] && current[y][x] == current[y][x+1])
                        {penalty_2 += 3;}
                    }

                    //third condition
                    pattern_x.push_back(current[y][x]);
                    pattern_y[x].push_back(current[y][x]);
                    if(10 <= x)
                    {
                        for(int i=0; i < 2; i++){if(patterns[i] == pattern_x)penalty_3+=40;}
                        pattern_x.erase(pattern_x.begin());
                    }
                    if(10 <= y)
                    {
                        for(int i=0; i < 2; i++){if(patterns[i] == pattern_y[x])penalty_3+=40;}
                        pattern_y[x].erase(pattern_y[x].begin());
                    }

                    //fourth condition
                    if(current[y][x] == 1)n_dark++;
                }
            }
            for(int y : same_y)if(5 <= y)penalty += y - 2;
            if(5 <= same_x)penalty += same_x - 2;

            //fourth condition
            int ratio = ((n_dark * 100) / (n_modules * n_modules)) / 5;

            penalty_4 += min(abs(ratio - 10), abs(ratio + 1 - 10)) * 10;
            penalty = penalty_1 + penalty_2 + penalty_3 + penalty_4;

            return penalty;
        }


        void add_data(bmp reserved_d)
        {
            int direction = -1;
            int counter = 1;
            array<int, 2> pos = {n_modules-1, n_modules-1};
            bool second = false;

            for(bool module : data)
            {
                if(module)bitmap[pos[1]][pos[0]] = 1;
                reserved_d[pos[1]][pos[0]] = 1;

                //gets new position
                while(reserved_d[pos[1]][pos[0]])
                {
                    if(((pos[1] == 0 && direction == -1) || (pos[1] == (n_modules - 1) && direction == 1)) && second)//new direction
                    {
                        pos[0] += -1;
                        if(pos[0] == 6)pos[0] += -1;
                        direction *= -1;

                    }else if(counter % 2){
                        pos[0] += -1;//if the same pos[0]s
                        second = true;
                    }else{
                        pos[1] += direction;
                        pos[0] += 1;
                        second = false;
                    }
                    counter++;
                    if(pos[0] == -1 || pos[1] == -1)break;
                }
            }
        }


        void add_patterns()
        {
            add_finder_patterns();
            add_alignment_pattern();
            add_timing_patterns();
            bitmap[4*version+9][8] = 1;//dark module reserved in format
            reserveFormatVersionArea();
        }

        void reserveFormatVersionArea()
        {
            //reserving format area
            reserve_rectangle({0, 8}, 9, 1);//top left horizontal
            reserve_rectangle({8, 0}, 1, 8);//top left vertical

            reserve_rectangle({n_modules - 8, 8}, 8, 1);//top right horizontal
            reserve_rectangle({8, n_modules - 8}, 1, 8);//bottom left vertical

            if(7 <= version)
            {
                reserve_rectangle({n_modules-11, 0}, 3, 6);
                reserve_rectangle({0, n_modules-11}, 6, 3);
            }
        }


        void add_finder_patterns()
        {
            auto draw_finder = [&] (int x, int y)
            {
                for(int i=0; i < 7; i++)
                {
                    bitmap[y + i][x] = 1;bitmap[y + i][x + 6] = 1;
                    bitmap[y][x + i] = 1;bitmap[y + 6][x + i] = 1;
                }
                for(int i=0; i < 3; i++)
                {
                    bitmap[y + 2][x + 2 + i] = 1;
                    bitmap[y + 3][x + 2 + i] = 1;
                    bitmap[y + 4][x + 2 + i] = 1;
                }
            };

            draw_finder(0, 0);//top left corner
            reserve_rectangle({0, 0},8, 8);
            draw_finder((version-1)*4+14, 0);//top right corner
            reserve_rectangle({(version-1)*4+14-1, 0}, 8, 8);
            draw_finder(0, (version-1)*4+14);//bottom left corner
            reserve_rectangle({0, (version-1)*4+14-1},8, 8);
        }


        void add_alignment_pattern()
        {
            auto draw_alignment = [&] (int x, int y)
            {
                bitmap[y][x] = 1;
                for(int i=0; i < 5; i++)
                {
                    bitmap[y - 2 + i][x - 2] = 1;bitmap[y - 2 + i][x + 2] = 1;
                    bitmap[y - 2][x - 2 + i] = 1;bitmap[y + 2][x - 2 + i] = 1;
                }
                reserve_rectangle({x-2, y-2}, 5, 5);
            };

            vector<int> locations = get_version_alignment_locations();
            for(int y : locations)
            {
                for(int x : locations)
                {
                    vector<array<int, 2>> corners = {{x-2, y-2}, {x+2, y-2}, {x-2, y+2}, {x+2, y+2}};
                    bool outside = true;

                    for(array<int, 2> corner : corners)
                    {if(reserved[corner[1]][corner[0]]){outside = false;break;}}

                    if(outside)draw_alignment(x, y);
                }
            }
        }


        void add_timing_patterns()
        {
            for(int i=0; i < (version-1)*4+21; i++)//timing patterns
            {
                //bool first = true, second = true;

                if(!reserved[6][i])bitmap[6][i] = (i + 1) % 2;reserved[6][i]=1;
                if(!reserved[i][6]){bitmap[i][6] = (i + 1) % 2;reserved[i][6]=1;}
            }
        }


        void reserve_rectangle(array<int, 2> pos, int width, int height)
        {
            
            for(int y=pos[1]; y < pos[1] + height; y++)
            {
                for(int x=pos[0]; x < pos[0] + width; x++)reserved[y][x] = 1;
            }
        }


        vector<int> get_version_alignment_locations()
        {
            unordered_map<int, vector<int>> version_alignment_locations;
            version_alignment_locations[1] = {};
            version_alignment_locations[2] = {6, 18}; 
            version_alignment_locations[3] = {6, 22}; 
            version_alignment_locations[4] = {6, 26,}; 
            version_alignment_locations[5] = {6, 30,}; 
            version_alignment_locations[6] = {6, 34}; 
            version_alignment_locations[7] = {6, 22, 38}; 
            version_alignment_locations[8] = {6, 24, 42}; 
            version_alignment_locations[9] = {6, 26, 46}; 
            version_alignment_locations[10] = {6, 28, 50}; 
            version_alignment_locations[11] = {6, 30, 54}; 
            version_alignment_locations[12] = {6, 32, 58}; 
            version_alignment_locations[13] = {6, 34, 62}; 
            version_alignment_locations[14] = {6, 26, 46, 66}; 
            version_alignment_locations[15] = {6, 26, 48, 70}; 
            version_alignment_locations[16] = {6, 26, 50, 74}; 
            version_alignment_locations[17] = {6, 30, 54, 78}; 
            version_alignment_locations[18] = {6, 30, 56, 82}; 
            version_alignment_locations[19] = {6, 30, 58, 86}; 
            version_alignment_locations[20] = {6, 34, 62, 90}; 
            version_alignment_locations[21] = {6, 28, 50, 72, 94}; 
            version_alignment_locations[22] = {6, 26, 50, 74, 98}; 
            version_alignment_locations[23] = {6, 30, 54, 78, 102}; 
            version_alignment_locations[24] = {6, 28, 54, 80, 106}; 
            version_alignment_locations[25] = {6, 32, 58, 84, 110}; 
            version_alignment_locations[26] = {6, 30, 58, 86, 114}; 
            version_alignment_locations[27] = {6, 34, 62, 90, 118}; 
            version_alignment_locations[28] = {6, 26, 50, 74, 98, 122}; 
            version_alignment_locations[29] = {6, 30, 54, 78, 102, 126}; 
            version_alignment_locations[30] = {6, 26, 52, 78, 104, 130}; 
            version_alignment_locations[31] = {6, 30, 56, 82, 108, 134}; 
            version_alignment_locations[32] = {6, 34, 60, 86, 112, 138}; 
            version_alignment_locations[33] = {6, 30, 58, 86, 114, 142}; 
            version_alignment_locations[34] = {6, 34, 62, 90, 118, 146}; 
            version_alignment_locations[35] = {6, 30, 54, 78, 102, 126, 150}; 
            version_alignment_locations[36] = {6, 24, 50, 76, 102, 128, 154}; 
            version_alignment_locations[37] = {6, 28, 54, 80, 106, 132, 158}; 
            version_alignment_locations[38] = {6, 32, 58, 84, 110, 136, 162}; 
            version_alignment_locations[39] = {6, 26, 54, 82, 110, 138, 166}; 
            version_alignment_locations[40] = {6, 30, 58, 86, 114, 142, 170}; 

            return version_alignment_locations[version];
        }
};
