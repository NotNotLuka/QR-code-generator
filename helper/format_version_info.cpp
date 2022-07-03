
class format_version_bits
{
    public:
        string EC_lvl;int mask;int version;

        format_version_bits(string EC_level, int maskVersion, int V)
        {
            EC_lvl = EC_level;
            mask = maskVersion;
            version = V;
        }


        vector<bool> get_format_bits()
        {
            vector<bool> gen_poly = {1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1};
            unordered_map<string, vector<bool>> EC_levels;
            EC_levels["L"] = {0, 1};EC_levels["M"] = {0, 0}; EC_levels["Q"] = {1, 1}; EC_levels["H"] = {1, 0};


            vector<bool> format_info; 
            format_info.insert(format_info.end(), EC_levels[EC_lvl].begin(), EC_levels[EC_lvl].end());

            vector<bool> mask_info = decToBin(mask, 3);
            format_info.insert(format_info.end(),mask_info.begin(), mask_info.end());
            

            format_info = EC_form_ver_divison(format_info, gen_poly, 15);
            format_info = decToBin(binToDec(format_info)^21522, 15);

            return format_info;
        }


        vector<bool> get_version_bits()
        {
            vector<bool> version_info = decToBin(version, 6); 
            vector<bool> gen_poly = {1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1};
            
            version_info = EC_form_ver_divison(version_info, gen_poly, 18);

            return version_info;
        }
        

    private:
        vector<bool> EC_form_ver_divison(vector<bool> info, vector<bool> gen, int poly_size)
        {
            int end_size = poly_size * 2 / 3;

            vector<bool> poly = info;
            int size = (poly_size - poly.size());
            for(int i=0; i < size; i++)poly.push_back(0);
            while(0 < poly.size() && poly[0] == 0)poly.erase(poly.begin());

            while(end_size < poly.size())
            {   
                vector<bool> gen_poly = gen;
                
                int poly_n = binToDec(poly);
                while(poly[0] == 0)poly.erase(poly.begin());//removing leading 0


                size = poly.size() - gen_poly.size();
                for(int i=0; i < size; i++)gen_poly.push_back(0);

                int gen_n = binToDec(gen_poly);

                int n = gen_n^poly_n;
                poly = decToBin(n, poly_size);
                while(poly[0] == 0)poly.erase(poly.begin());//removing leading 0        
            }
            if(poly.size() < end_size)
            {
                size = end_size - poly.size();
                for(int i=0; i < size; i++)poly.insert(poly.begin(), 0);
            }

            info.insert(info.end(), poly.begin(), poly.end());

            return info;
        }
};
