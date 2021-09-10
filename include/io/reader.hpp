#ifndef IO_READER_HPP
#define IO_READER_HPP

#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <algorithm>



std::vector<std::string> String2Tokens(std::string istr)
{
    std::stringstream ss(istr);
    std::string buf;
    std::vector<std::string> tokens;

    while (ss >> buf) 
    {
        tokens.push_back(buf);
    }

    return tokens;
}

inline std::unordered_map<std::string, float> ReadASCIIHeader(const char* fname)
{
    std::unordered_map<std::string, float> out;

    std::string temp_str;
    std::vector<std::string> temp_tokens;

    std::ifstream ifs(fname);

    std::getline(ifs, temp_str);  // TODO: proper handle if it's not ASCII
    std::getline(ifs, temp_str);  // TODO: double check if this is the type of the data

    std::getline(ifs, temp_str);
    temp_tokens = String2Tokens(temp_str);
    
    out["nx"] = static_cast<float>(std::atoi(temp_tokens.at(0).c_str()));
    out["ny"] = static_cast<float>(std::atoi(temp_tokens.at(1).c_str()));
    out["nz"] = static_cast<float>(std::atoi(temp_tokens.at(2).c_str()));

    std::getline(ifs, temp_str);
    temp_tokens = String2Tokens(temp_str);

    out["ox"] = std::atof(temp_tokens.at(0).c_str());
    out["lx"] = std::atof(temp_tokens.at(1).c_str());
    out["oy"] = std::atof(temp_tokens.at(2).c_str());
    out["ly"] = std::atof(temp_tokens.at(3).c_str());
    out["oz"] = std::atof(temp_tokens.at(4).c_str());
    out["lz"] = std::atof(temp_tokens.at(5).c_str());
    
    ifs.close();

    return out;
}

inline std::vector<unsigned char> ReadASCII(const char* fname, int n)
{
    std::vector<unsigned char> out(n);
    int nx, ny, nz;

    std::string temp_str;
    std::vector<std::string> temp_tokens;

    std::ifstream ifs(fname);

    std::getline(ifs, temp_str);
    
    for(int i=0; i<3; ++i) 
    {
        std::getline(ifs, temp_str);
    }
    
    int idata = 0;

    while (std::getline(ifs, temp_str))
    {
        temp_tokens = String2Tokens(temp_str);
        
        if (temp_tokens.size() == 0) continue;

        for (std::string istr : temp_tokens)
        {
            out[idata++] = std::atoi(istr.c_str());
        }
    }
    
    ifs.close();

    return out;
}


std::vector<int> ReadRAW(const char* fname, long int n) 
{
    std::vector<int> out(n);  
    std::ifstream fin(fname, std::ifstream::binary);
    
    unsigned char data;
    long int i = 0;

    while(!fin.eof())
    {
        fin >> data;
        out[i++] = static_cast<int>(data);
        // std::cout << (int)data << std::endl;
    }
 
    fin.close();

    return out;
}


#endif // IO_READER_HPP
