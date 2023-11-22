#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
 
using namespace std;
using filesystem::path;
 
path operator""_p(const char* data, std::size_t sz) {
    return path(data, data + sz);
}

bool RecPreprocess(const path& in_file, ostream& ostr, const vector<path>& include_directories) {
    ifstream input(in_file);
    if (!input.is_open()) return false;
    string str; 
    int current_line = 1;
    while (getline(input, str)) {         
        
        static regex find_file(R"/(\s*#\s*include\s*"([^"]*)"\s*)/");
        smatch file_match;
        bool not_found = true;
        if (regex_match(str, file_match, find_file)) {
            path p = string(file_match[1]);
            path file_path = in_file.parent_path() / p;
            if (filesystem::exists(file_path)) {
                not_found = false;
                if (!RecPreprocess(file_path, ostr, include_directories)) return false;
            } else {
                for (path dir_path : include_directories) {
                    dir_path = dir_path / p; 
                    if (filesystem::exists(dir_path)) {                        
                        not_found = false;                       
                        if (!RecPreprocess(dir_path, ostr, include_directories)) return false;
                        break;
                    }
                }
            }
            if (not_found) {
                cout << "unknown include file " << p.filename().string() << " at file " << in_file.string() << " at line " << current_line << endl;
                return false;
            }
        }
        static regex find_lib(R"/(\s*#\s*include\s*<([^>]*)>\s*)/");
        smatch lib_match;
        if (regex_match(str, lib_match, find_lib)) {
            path p = string(lib_match[1]);            
            for (path dir_path : include_directories) {
                dir_path = dir_path / p;
 
                if (filesystem::exists(dir_path)) {
                    not_found = false;
                    if (!RecPreprocess(dir_path, ostr, include_directories)) return false;
                    break;
                }
            }
            if (not_found) {              
                cout << "unknown include file " << p.filename().string() << " at file " << in_file.string() << " at line " << current_line << endl;
                return false;
            }           
        }
        current_line += 1;
        if (!regex_match(str, file_match, find_file) && !regex_match(str, lib_match, find_lib)) ostr << str << endl;        
    }
    input.close();
    return true;
}

bool Preprocess(const path& in_file, const path& out_file, const vector<path>& include_directories) {
 
    ifstream input(in_file);
    if (!input.is_open()) return false;
 
    fstream output(out_file, ios::out | ios::app);
    
    return RecPreprocess(in_file, output, include_directories);
}

 
string GetFileContents(string file) {
    ifstream stream(file);
 
    // конструируем string по двум итераторам
    return { (istreambuf_iterator<char>(stream)), istreambuf_iterator<char>() };
}
 
void Test() {
    error_code err;
    filesystem::remove_all("sources"_p, err);
    filesystem::create_directories("sources"_p / "include2"_p / "lib"_p, err);
    filesystem::create_directories("sources"_p / "include1"_p, err);
    filesystem::create_directories("sources"_p / "dir1"_p / "subdir"_p, err);
 
    {
        ofstream file("sources/a.cpp");
        file << "// this comment before include\n"
            "#include \"dir1/b.h\"\n"
            "// text between b.h and c.h\n"
            "#include \"dir1/d.h\"\n"
            "\n"
            "int SayHello() {\n"
            "    cout << \"hello, world!\" << endl;\n"
            "#   include<dummy.txt>\n"
            "}\n"s;
    }
    {
        ofstream file("sources/dir1/b.h");
        file << "// text from b.h before include\n"
            "#include \"subdir/c.h\"\n"
            "// text from b.h after include"s;
    }
    {
        ofstream file("sources/dir1/subdir/c.h");
        file << "// text from c.h before include\n"
            "#include <std1.h>\n"
            "// text from c.h after include\n"s;
    }
    {
        ofstream file("sources/dir1/d.h");
        file << "// text from d.h before include\n"
            "#include \"lib/std2.h\"\n"
            "// text from d.h after include\n"s;
    }
    {
        ofstream file("sources/include1/std1.h");
        file << "// std1\n"s;
    }
    {
        ofstream file("sources/include2/lib/std2.h");
        file << "// std2\n"s;
    }
 
    assert((!Preprocess("sources"_p / "a.cpp"_p, "sources"_p / "a.in"_p,
        { "sources"_p / "include1"_p,"sources"_p / "include2"_p })));
 
    ostringstream test_out;
    test_out << "// this comment before include\n"
        "// text from b.h before include\n"
        "// text from c.h before include\n"
        "// std1\n"
        "// text from c.h after include\n"
        "// text from b.h after include\n"
        "// text between b.h and c.h\n"
        "// text from d.h before include\n"
        "// std2\n"
        "// text from d.h after include\n"
        "\n"
        "int SayHello() {\n"
        "    cout << \"hello, world!\" << endl;\n"s;
 
    assert(GetFileContents("sources/a.in"s) == test_out.str());
    
}
 
int main() {
    Test();
}